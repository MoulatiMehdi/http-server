#include "BodyStorage.hpp"
#include "sys/time.h"
#include <cstddef>
#include <cstdio>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <unistd.h>

const std::string BodyStorage::m_dir = "/tmp";

BodyStorage::BodyStorage() : m_fd(-1), m_path(), m_size(0)
{
}

int BodyStorage::open_file()
{
    m_path = m_dir + "/" + generateName();
    m_fd   = open(m_path.c_str(), O_WRONLY | O_TRUNC | O_CREAT | O_EXCL, 0600);
    if (m_fd < 0)
    {
        perror("BodyStorage::open");
    }
    return m_fd;
}

int BodyStorage::open_file(const std::string& path)
{
    m_path = path;
    m_fd   = open(m_path.c_str(), O_WRONLY | O_TRUNC | O_CREAT | O_EXCL, 0600);
    if (m_fd < 0)
    {
        perror("BodyStorage::open");
    }
    return m_fd;
}
size_t BodyStorage::size() const
{
    return m_size;
}

ssize_t BodyStorage::append(char ch)
{
    m_size += 1;
    return write(m_fd, &ch, 1);
}

ssize_t BodyStorage::append(const std::string &str)
{
    m_size += str.size();
    return write(m_fd, str.c_str(), str.size());
}

ssize_t BodyStorage::append(const char *str, size_t len)
{
    m_size += len;
    return write(m_fd, str, len);
}

void BodyStorage::consume(size_t len)
{
    m_size += len;
}

bool BodyStorage::is_open() const
{
    return m_fd < 0;
}

std::string &BodyStorage::path()
{
    return m_path;
}

const std::string &BodyStorage::path() const
{
    return m_path;
}

const char *BodyStorage::c_path() const
{
    return m_path.c_str();
}

void BodyStorage::clear()
{
    BodyStorage::close();
    m_size = 0;
    m_path = "";
}

BodyStorage::~BodyStorage()
{
    BodyStorage::close();
}

void BodyStorage::close()
{
    if (m_fd >= 0)
    {
        ::close(m_fd);
        m_fd = -1;
    }
}

const std::string BodyStorage::generateName()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    std::ostringstream iss;
    iss << tv.tv_sec << "." << tv.tv_usec;
    return iss.str();
}
