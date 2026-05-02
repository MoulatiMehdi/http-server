#ifndef BODY_STORAGE_HPP
#define BODY_STORAGE_HPP
#include <cstddef>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <sys/types.h>
#include <unistd.h>

class BodyStorage
{
  private:
    static const std::string m_dir;
    int                      m_fd;
    std::string              m_path;
    ssize_t                  m_size;

  public:
    BodyStorage();
    BodyStorage(const std::string &dir);
    ~BodyStorage();

    ssize_t append(char c);
    ssize_t append(const std::string &str);
    ssize_t append(const char *str, size_t len);

    int open_file();
    int open_file(const std::string &path);

    size_t             size() const;

    std::string       &path();
    const std::string &path() const;
    const char        *c_path() const;
    
    bool               is_open() const;
    void               consume(size_t len);
    void               clear();
    void               close();

    static const std::string generateName();
};

#endif
