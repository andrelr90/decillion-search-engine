#include "log.hpp"

std::mutex Log::time_lock;
std::mutex Log::log_lock;

const std::string Log::currentDateTime() {
    time_lock.lock();
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
    time_lock.unlock();

    return buf;
}

void Log::write_log(int crawlerId, std::string message){
    log_lock.lock();
    if(crawlerId != -1)
        std::cout << "From crawler " + std::to_string(crawlerId) + " - " + currentDateTime() + ":\n";
    else
        std::cout << currentDateTime() + ":\n";
    std::cout << message;
    std::cout << "\r\n\n";
    log_lock.unlock();
}