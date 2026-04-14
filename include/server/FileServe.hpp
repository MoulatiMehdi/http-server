#ifndef FILESERVE_HPP
#define FILESERVE_HPP
#include <sys/types.h>
#include <string>

class FileServe {
   private:
	int _fd;
	off_t _offset;
	off_t _size;

	FileServe(const FileServe &);
	FileServe &operator=(const FileServe &);

   public:
	FileServe(const std::string &path);
	~FileServe();

	bool done() const;
	int sendChunk(int client_fd);
	off_t size() const;
};
#endif	// !FILESERVE_HPP
