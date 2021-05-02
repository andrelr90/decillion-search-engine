#include <string>

class UrlCrawler {
    private:
        int size;
        int original_size;
        bool br;
        std::string url;
    
    public:
        UrlCrawler(std::string url);
        int getSize() const;
        void incrementSize();
        int getOriginalSize();
        std::string getUrl() const;
        bool isBr();
};