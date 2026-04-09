#ifndef HELPER_HPP
#define HELPER_HPP
#include <string>
#include <sstream>
#define ERROR -1
template <typename T>
std::string to_stringg(T val) {
	std::ostringstream oss;
	oss << val;
	return oss.str();
}

void make_non_blocking(int fd) ;
void exitError(std::string msg) ;

#endif
