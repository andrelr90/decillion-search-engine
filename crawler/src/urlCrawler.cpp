#include "urlCrawler.hpp"

UrlCrawler::UrlCrawler(std::string url): url(url){
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

int UrlCrawler::getSize() const{
    return this->size;
}

void UrlCrawler::incrementSize(){
    this->size+=1;
}

int UrlCrawler::getOriginalSize(){
    return this->original_size;
}

std::string UrlCrawler::getUrl() const{
    return this->url;
}

bool UrlCrawler::isBr(){
    return br;
}