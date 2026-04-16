#ifndef FILESERVE_HPP
#define FILESERVE_HPP
#include <sys/types.h>
#include <string>

#define BUFF_SIZE 4096
class FileServe {
   private:
	int _fd;
	int _size;
	char _tmp[BUFF_SIZE];
	int _tmp_offset;
	int _tmp_len;

	FileServe(const FileServe &);
	FileServe &operator=(const FileServe &);

   public:
	FileServe(const std::string &path);
	~FileServe();

	bool done() const;
	int sendChunk(int fd);
	off_t size() const;
};
#endif	// !FILESERVE_HPP
