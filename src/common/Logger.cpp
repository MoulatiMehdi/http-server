#include "Logger.hpp"

std::string Logger::timestamp() {
	char buf[32];
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	strftime(buf, sizeof(buf), "%Y/%m/%d %H:%M:%S", t);
	return std::string(buf);
}

void Logger::log(const std::string &level, const std::string &msg) {
	std::cerr << timestamp() << " [" << level << "] " << msg << "\n";
}

void Logger::info(const std::string &msg) { log("INFO", msg); }
void Logger::error(const std::string &msg) { log("ERROR", msg); }
