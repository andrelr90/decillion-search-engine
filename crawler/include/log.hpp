#include <mutex>
#include <string>
#include <iostream>

class Log{
	private:
		static std::mutex time_lock;
		static std::mutex log_lock;
	
	public:
		static const std::string currentDateTime();
		static void write_log(int crawlerId, std::string message);
};