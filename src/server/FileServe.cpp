#include "FileServe.hpp"
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

FileServe::FileServe(const std::string &path) : _fd(-1), _offset(0), _size(0) {
	_fd = open(path.c_str(), O_RDONLY);
	if (_fd < 0) return;

	struct stat st;
	if (fstat(_fd, &st) == -1) {
		close(_fd);
		_fd = -1;
		return;
	}
	_size = st.st_size;
}

FileServe::~FileServe() {
	if (_fd >= 0) close(_fd);
}

bool FileServe::done() const { return _fd < 0 || _offset >= _size; }

#define BUFF_SIZE 4096
#include "Client.hpp"
int FileServe::sendChunk(int client_fd) {
	char tmp[4096];

	int n = read(_fd, tmp, sizeof(tmp));
	if (n == 0) {
		close(_fd);
		_fd = -1;
		return DONE_WRITE;
	}

	if (n == ERROR) {
		close(_fd);
		_fd = -1;
		return DISCONNECT;
	}

	write(client_fd, tmp, n);
	return OK;
}

off_t FileServe::size() const { return _size; }
