#ifndef BODY_STORAGE_HPP
#define BODY_STORAGE_HPP
#include <cstddef>
#include <fcntl.h>
#include <iostream>
#include <ostream>
#include <string>
#include <sys/types.h>
#include <unistd.h>

class BodyStorage
{
  private:
    static const std::string m_dir;
    int                      m_fd;
    ssize_t                  m_size;
    std::string              m_path;

  public:
    BodyStorage();
    BodyStorage(const std::string &dir);
    ~BodyStorage();

    ssize_t append(char c);
    ssize_t append(const std::string &str);
    ssize_t append(const char *str, size_t len);

    int open_file();

    ssize_t            size() const;
    const std::string &path() const;
    const char        *c_path() const;
    bool               is_open() const;
    void               clear();

    static const std::string generateName();
};

std::ostream &operator<<(std::ostream &os, const BodyStorage &body);
#endif
