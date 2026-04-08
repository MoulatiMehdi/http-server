#ifndef LOGGER_HPP
#define LOGGER_HPP
#include <ctime>
#include <iostream>

#include <string>

class Logger {
   public:
	static void info(const std::string &msg);
	static void error(const std::string &msg);

   private:
	static void log(const std::string &level, const std::string &msg);
	static std::string timestamp();
};

#endif
