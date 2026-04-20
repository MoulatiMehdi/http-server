#include "FileServe.hpp"
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>

FileServe::FileServe(const std::string &path)
	: _fd(-1), _size(0), _tmp_offset(0), _tmp_len(0) {
	_fd = open(path.c_str(), O_RDONLY);
	if (_fd < 0) return; // THROW

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

bool FileServe::done() const { return _fd < 0; }

#include "Client.hpp"
int FileServe::sendChunk(int fd) {
	if (_tmp_len == 0) {
		int n = read(_fd, _tmp, sizeof(_tmp));
		if (n == 0) {
			close(_fd);
			_fd = -1;
			return 0;
		}
		if (n == ERROR) {
			close(_fd);
			_fd = -1;
			return ERROR;
		}
		_tmp_len = n;
		_tmp_offset = 0;
	}

	int n = write(fd, _tmp + _tmp_offset, _tmp_len - _tmp_offset);
	if (n == ERROR) return ERROR;
	_tmp_offset += n;

	if (_tmp_offset == _tmp_len) {
		_tmp_len = 0;
		_tmp_offset = 0;
	}

	return n;
}

off_t FileServe::size() const { return _size; }
