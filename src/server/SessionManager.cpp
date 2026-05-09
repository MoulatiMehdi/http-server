#include "SessionManager.hpp"
#include "Logger.hpp"

std::string SessionManager::create() {
	std::string id = generateId();
	_sessions[id] = SessionData();
	_sessions[id]["created_at"] = Logger::timestamp();
	_sessions[id]["visits"] = "0";
	_sessions[id]["last_page"] = "";
	_sessions[id]["last_upload"] = "";
	return id;
}
SessionData *SessionManager::get(const std::string &id) {
	std::map<std::string, SessionData>::iterator it = _sessions.find(id);
	if (it == _sessions.end()) return NULL;
	return &it->second;
}
void SessionManager::destroy(const std::string &id) { _sessions.erase(id); }

std::string SessionManager::generateId() {
	std::ostringstream ss;
	for (int i = 0; i < 8; ++i)
		ss << std::hex << (std::rand() % 0xffff);
	return ss.str();
}
