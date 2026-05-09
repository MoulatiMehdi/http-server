// SessionManager.hpp
#include <cstdlib>
#include <map>
#include <sstream>
#include <string>
#include "Logger.hpp"
#include "SessionManager.hpp"


SessionData *SessionManager::get(const std::string &id) {
	std::map<std::string, SessionData>::iterator it = _sessions.find(id);
	if (it == _sessions.end()) return NULL;
	return &it->second;
}

void SessionManager::destroy(const std::string &id) { _sessions.erase(id); }

std::string SessionManager::generateId() {
	// good enough for 42 eval — not cryptographically secure
	std::ostringstream ss;
	for (int i = 0; i < 8; ++i)
		ss << std::hex << (rand() % 0xffff);
	return ss.str();
}
