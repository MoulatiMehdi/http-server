// SessionManager.hpp
#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H
#include <cstdlib>
#include <map>
#include <sstream>
#include <string>

typedef std::map<std::string, std::string> SessionData;

class SessionManager {
	std::map<std::string, SessionData> _sessions;

   public:
	std::string create();
	SessionData *get(const std::string &id);
	void destroy(const std::string &id);

   private:
	std::string generateId();

};
#endif	// !SESSION_MANAGER_H
