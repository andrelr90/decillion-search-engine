#include <CkSpider.h>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <cstdlib>
#include <cstdio>
#include <locale.h>

#include <tuple>
#include <thread>
#include <mutex>
#include <queue>
#include <set>
#include <map>
#include <list>
#include <string>
#include <fstream>
#include <exception>
#include <signal.h>

class UrlCrawler {
    private:
        int size;
        int original_size;
        bool br;
        std::string url;
    
    public:
        UrlCrawler(std::string url): url(url){
            this->size = 0;
            this->br = false;
            this->original_size=0;
            for (unsigned int i = 0; i < url.size(); i++){
                if (url[i] == '/' || url[i] == '.'){
                    this->size++;
                    this->original_size++;
                }
                if (i < url.size()-2){
                    //finds the pattern /br/ or .br. or .bra. or .br
                    if((url[i]=='.' || url[i]=='/') && url[i+1]=='b' && url[i+2]=='r'){
                        if((i+3<url.size()-2 && (url[i+3]=='.' || url[i+3]=='/' || url[i+3]=='a')) || (i+3 >= url.size() - 2) ){
                            this->size-=4;  //adds privilege for br pages.
                            this->br=true;
                        }
                    }
                }
            }
            this->original_size-=2; //disconsiders http://, https://
        }
        int getSize() const{
            return this->size;
        }
        void incrementSize(){
            this->size+=1;
        }
        int getOriginalSize(){
            return this->original_size;
        }
        std::string getUrl() const{
            return this->url;
        }
        bool isBr(){
            return br;
        }
};
 
bool operator<(const UrlCrawler& url1, const UrlCrawler& url2) {
    // operator overloading for priorityQueue of urls. < is given in terms of priority and therefore compares the "greatest" url. 
    return url1.getSize() > url2.getSize();
}

// Control variables
int limit = 10000;
bool jsonl = true;    // Turns jsonl saving instead of individual files.

double time_taken = 0;
int success_counter = 0;
int level0_counter = 0;
std::priority_queue<UrlCrawler> longTermScheduler;
std::set<std::string> collected;
std::set<std::string> collecting;

// Output variables:
std::map<std::string,std::tuple<int, float, float>> level0pages;    // number_of_level1_pages / average_time_taken / average_size
std::map<std::string,std::tuple<std::string,float, float, int>> full_info;  // title / time_taken / FileSize / urlSize
int brCounter;

// Control access:
std::mutex time_taken_lock, success_counter_lock, level0_counter_lock, longTermScheduler_lock, collected_lock, collecting_lock, file_lock;
std::mutex level0pages_lock, full_info_lock, brCounter_lock; // full_pages_size_lock, full_time_taken_lock, full_url_sizes_lock;

bool max_number_pages_reached(int crawlerId, CkSpider &spider){
    success_counter_lock.lock();
    if(success_counter == limit){
        success_counter_lock.unlock();
        std::cout << "From crawler " + std::to_string(crawlerId) + ":\n" + "Max number of pages reached. Returning. \r\n\n";
        return true;
    }
    success_counter_lock.unlock();
    return false;
}

bool wait(int crawlerId, CkSpider &spider){
    int limit_tries = 30;
    longTermScheduler_lock.lock();
    while(longTermScheduler.empty()){
        longTermScheduler_lock.unlock();
        std::cout <<  "Crawler " + std::to_string(crawlerId) + " is waiting. \n\n";
        spider.SleepMs(1000);
        limit_tries --;
        if(limit_tries == 0){    // Stops the thread due to inactivity.
            std::cout << "Stopping crawler " + std::to_string(crawlerId) + " due to inactivity. \n\n";
            return true;
        }
        longTermScheduler_lock.lock();
    }
    longTermScheduler_lock.unlock();
    return false;
}

UrlCrawler get_next_url(int crawlerId, CkSpider &spider){
    longTermScheduler_lock.lock();
    if(longTermScheduler.size() == 0){
        longTermScheduler_lock.unlock();
        return UrlCrawler("URLError");
    }
    
    UrlCrawler url = longTermScheduler.top();
    longTermScheduler.pop();

    std::list<UrlCrawler> popped;
    collecting_lock.lock();
    while(collecting.find(std::string(spider.getUrlDomain(url.getUrl().c_str()))) != collecting.end() and longTermScheduler.size() != 0){
        // The domain is already being crawled by other thread.
        // std::cout << "From crawler " + std::to_string(crawlerId) + ":\n" + "Domain " + spider.getUrlDomain(url.getUrl().c_str()) + " is already being crawled. \r\n\n";

        popped.push_back(url);
        url = longTermScheduler.top();
        url.incrementSize();
        longTermScheduler.pop();
    }

    bool allCollected = false;
    if(collecting.find(std::string(spider.getUrlDomain(url.getUrl().c_str()))) != collecting.end() and longTermScheduler.size() == 0){
        allCollected = true;
    }

    for (std::list<UrlCrawler>::iterator it = popped.begin(); it != popped.end(); ++it){
        longTermScheduler.push(*it);
    }
    longTermScheduler_lock.unlock();
    collecting_lock.unlock();

    if(allCollected)
        return UrlCrawler("URLError");

    return url;
}

bool check_already_collected(int crawlerId, CkSpider &spider){
    collected_lock.lock();
    bool urlError = false;
    try{
        if(spider.get_NumUnspidered() != 0) 
            std::string(spider.getUnspideredUrl(0));
    }catch(std::exception& e){
        std::cout << ("From crawler " + std::to_string(crawlerId) + ":\n" + "Exception detected in url.\n\n") ;
        urlError = true;
    }

    if (!urlError && spider.get_NumUnspidered() != 0 && collected.find( std::string(spider.getUnspideredUrl(0)) ) == collected.end()) {
        // Page not yet collected
        // collected.insert(spider.lastUrl());
        collected.insert(spider.getUnspideredUrl(0));
        collected_lock.unlock();
    } else {
        // Page already collected
        collected_lock.unlock();
        return true;
    }
    return false;
}

void write_file_quote(std::string text, std::ofstream& file){
    for (unsigned int i=0; i< text.size(); i++){
        if(text[i] == '"')
            file << "\\\"";
        else if (text[i] != '\n' && text[i] != '\t')
            file << text[i];
    }
}

bool save_page(int crawlerId, CkSpider &spider){
    success_counter_lock.lock();
    if(success_counter >= limit){
        success_counter_lock.unlock();
        return false;
    }
    int local_success_counter = success_counter++;
    success_counter_lock.unlock();

    // Show the URL of the page just spidered.
    std::cout << "From crawler " + std::to_string(crawlerId) + ":\n" + "URL " + std::to_string(local_success_counter) + ": " + spider.lastUrl() + "\r\n\n";

    if (!jsonl){
        std::ofstream output_file("data/" + std::to_string(local_success_counter) + ".html");
        output_file<<spider.lastHtml();
        output_file.close();

        std::ofstream output_url_file("links/" + std::to_string(local_success_counter) + ".url");
        output_url_file<<spider.lastUrl();
        output_url_file.close();
    } else {
        file_lock.lock();
        std::ofstream collection_file("data/collection.jl", std::ofstream::out | std::ofstream::app);

        // Quotes in strings must be replaced by \" for jsonl work properly
        collection_file << "{\"url\": \"";
        write_file_quote(spider.lastUrl(), collection_file);
        collection_file << "\", ";
         
        collection_file << "\"title\": \"";
        write_file_quote(spider.lastHtmlTitle(), collection_file);
        collection_file<< "\", ";

        collection_file << "\"html_content\": \"";
        write_file_quote(spider.lastHtml(), collection_file);
        collection_file<< "\"}\n";

        collection_file.close();

        std::ofstream counter_file("data/counter");
        counter_file << local_success_counter+1;
        counter_file.close();

        file_lock.unlock();
    }
    return true;
}

void update_longTermScheduler(int crawlerId, CkSpider &spider){
    int n_outbound = spider.get_NumOutboundLinks();
    std::cout << ("From crawler " + std::to_string(crawlerId) + ":\n" + "Adding " + std::to_string(n_outbound) + " pages to longTermScheduler.\n\n") ;
    for(int i=0; i < n_outbound; ++i){
        try{
            std::string domain_string_teste(spider.getUrlDomain(spider.getOutboundLink(i)));
            std::string outbound_string_teste(spider.getOutboundLink(i));
        }catch (std::exception& e){
            std::cout << ("From crawler " + std::to_string(crawlerId) + ":\n" + "Exception detected in url.\n\n") ;
            continue;
        }
        std::string outbound_string(spider.getOutboundLink(i));
        UrlCrawler outbound_url(outbound_string);

        longTermScheduler_lock.lock();
        longTermScheduler.push(outbound_url);
        longTermScheduler_lock.unlock();
    }
    spider.ClearOutboundLinks();
}

void shortTermScheduler(int crawlerId){
    while(true){
        CkSpider spider;
        spider.put_Utf8(true);
        spider.put_ConnectTimeout(10);
        spider.put_ReadTimeout(10);
        spider.AddAvoidPattern("*facebook.com*");
        spider.AddAvoidPattern("*instagram.com*");
        spider.AddAvoidPattern("*twitter.com*");
        spider.AddAvoidPattern("*youtu.be*");

        if (max_number_pages_reached(crawlerId, spider)) {
            return;
        }
        if (wait(crawlerId, spider))  {
            return;
        }
        UrlCrawler urlCustomObject =  get_next_url(crawlerId, spider);
        std::string url = urlCustomObject.getUrl();
        
        if(url == "URLError"){
            std::cout << "From crawler " + std::to_string(crawlerId) + ":\n" + "All available domains are already being crawled. \r\n\n";
            
            spider.SleepMs(2000);
            continue;
        } 

        collecting_lock.lock();
        collected_lock.lock();
        if(collected.find(std::string(spider.canonicalizeUrl(url.c_str()))) == collected.end()){
            collecting.insert(std::string(spider.getUrlDomain(url.c_str())));
        } else{
            std::cout << "From crawler " + std::to_string(crawlerId) + ":\nPage already collected. Continuing. \r\n\n";
            collected_lock.unlock();
            collecting_lock.unlock();
            continue;
        }
        collected_lock.unlock();
        collecting_lock.unlock();
        
        std::cout << "From crawler " + std::to_string(crawlerId) + ":\n" + "Getting " + url + " from longTermScheduler. \r\n\n";

        spider.Initialize(spider.getUrlDomain(url.c_str()));
        spider.AddUnspidered(spider.canonicalizeUrl(url.c_str()));

        bool level0 = true;
        int level1 = 0;
        std::string level0url = url;
        do { // Level 1 Crawl
            if (max_number_pages_reached(crawlerId, spider)) {
                return;
            }

            bool already_collected = false;
            bool success=false;
            double local_time_taken = 0;
            double local_size = 0;
            if(!check_already_collected(crawlerId, spider)){
                auto start_crawl = std::chrono::high_resolution_clock::now();  //starts timer.
                success = spider.CrawlNext();
                auto end_crawl = std::chrono::high_resolution_clock::now();

                local_size = std::string(spider.lastHtml()).size() * sizeof(char);
                local_time_taken = std::chrono::duration_cast<std::chrono::nanoseconds>(end_crawl - start_crawl).count();
                
                time_taken_lock.lock();
                time_taken += local_time_taken;
                time_taken_lock.unlock();

                spider.SleepMs(100);    // Sleep 0.1 second before spidering the next URL.
            }else{
                already_collected = true;
            }

            if (success && local_size!=0) {
                if(level0){
                    update_longTermScheduler(crawlerId, spider);

                    level0 = false;
                    level1 = spider.get_NumUnspidered();
                    std::cout << "From crawler " + std::to_string(crawlerId) + ":\n" + std::to_string(level1) + " level 1 pages will be crawled. \r\n\n";

                    level0_counter_lock.lock();
                    level0_counter++;
                    level0_counter_lock.unlock();

                    level0pages_lock.lock();
                    level0pages.insert( std::pair<std::string,std::tuple<int, float, float>>( url,std::make_tuple(0, local_time_taken, local_size) ) );
                    level0pages_lock.unlock();
                } else{
                    level1--;

                    level0pages_lock.lock();
                    std::get<0>(level0pages.at(level0url)) += 1;
                    std::get<1>(level0pages.at(level0url)) += local_time_taken;
                    std::get<2>(level0pages.at(level0url)) += local_size;
                    level0pages_lock.unlock();
                }

                if(!save_page(crawlerId, spider)) { // Tries to save the page.
                    std::cout << "From crawler " + std::to_string(crawlerId) + ":\nCrawler Stopped. \r\n\n";
                    return;
                } else{
                    full_info_lock.lock();
                    full_info.insert(std::pair<std::string,std::tuple<std::string, float, float, int>>
                                    (spider.lastUrl(), std::make_tuple(spider.lastHtmlTitle(), local_time_taken, local_size, UrlCrawler(spider.lastUrl()).getOriginalSize())));
                    full_info_lock.unlock();
                    if(urlCustomObject.isBr()){
                        brCounter_lock.lock();
                        brCounter++;
                        brCounter_lock.unlock();
                    }
                }
            } else {
                if(level0){
                    break;
                } else{
                    level1--;
                }
                //some error happened
                if(!already_collected){
                    // std::cout << "From crawler " + std::to_string(crawlerId) + ":\n" + spider.lastErrorText() + "\r\n\n";
                    std::cout << "From crawler " + std::to_string(crawlerId) + ":\nChilkat Error\r\n\n";
                } else{
                    std::cout << "From crawler " + std::to_string(crawlerId) + ":\nPage already collected 2. Continuing. \r\n\n";
                    if (spider.get_NumUnspidered() != 0)
                        spider.SkipUnspidered(0);
                }
            }
            
        } while (level1 > 0);

        collecting_lock.lock();
        collecting.erase(std::string(spider.getUrlDomain(url.c_str())));
        collecting_lock.unlock();
    }
}

void sigpipe_handler(int signal){
    std::cout<<"Sigpipe exception detected. Signal " + std::to_string(signal) + ". Continuing, this exception is not a problem in this context. \n\n";
}

int main(int argc, char *argv[]){
    signal(SIGPIPE, sigpipe_handler);
    setlocale(LC_ALL, "Portuguese");

    if(jsonl){
        // Replace collection file.
        std::ofstream collection_file("data/collection.jl");
        collection_file.close();
    }

    argc = argc;
    std::vector<std::thread*> threads;
    std::string seedLink;

    // Settings (parameters)
    char *seedsFile = argv[1];
    std::ifstream infile(seedsFile);
    int numberOfThreads = atoi(argv[2]);
    limit = atoi(argv[3]);
    if (std::string(argv[4]) == "true")
        jsonl = false;
    else
        jsonl = true;

    while (infile >> seedLink){
        UrlCrawler url(seedLink);
        std::cout << "Seed "<<seedLink<<std::endl<<std::endl;
        longTermScheduler.push(url);
    }
    infile.close();

    auto start_program = std::chrono::high_resolution_clock::now();
    for(int i=0; i<numberOfThreads; i++){
        std::thread *t = new std::thread(shortTermScheduler, i);
        threads.push_back(t);
    }

    for(int i=0; i<numberOfThreads; i++){
        threads[i]->join();
    }

    for(int i=0; i<numberOfThreads; i++){
        delete threads[i];
    }

    auto end_program = std::chrono::high_resolution_clock::now();
    double program_time_taken = std::chrono::duration_cast<std::chrono::nanoseconds>(end_program - start_program).count();
    program_time_taken*=1e-9;       //Converting nanoseconds to seconds
    time_taken = time_taken* 1e-9;  //Converting nanoseconds to seconds

    std::cout<<"\n\n---------------------------------\nLevel 0 pages report:\n";
    for (auto& i: level0pages) {
        std::cout   << "- "<< i.first << ": \n"
                    << "Level 1 pages: " << (std::get<0>(i.second)) << "\n"
                    << "Average time for crawling:" << (std::get<1>(i.second))*1e-9/float((std::get<0>(i.second))+1) << " second (s) \n"
                    << "Average size: " << (std::get<2>(i.second))/8.0/1024.0/float((std::get<0>(i.second))+1) << " kb\n\n";
    }

    std::cout<<"\n\n---------------------------------\nReport:\n";
    std::cout<<"Average time for crawling a page: "<<(success_counter == 0 ? 0 : time_taken/(success_counter))<<std::setprecision(5)<<" second(s).\n";
    std::cout<<"Total execution time: "<<program_time_taken/60.0<<" minuts.\n\n";
    std::cout<<"Level 0 pages: "<<level0_counter<<"\n";
    std::cout<<"Level 1 pages: "<<success_counter - level0_counter<<"\n";
    std::cout<<"Identified br pages: "<<brCounter<<"\n";

    return 0;
}
