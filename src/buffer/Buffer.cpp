#include "Buffer.hpp"
#include <cstddef>
#include <string>

Buffer::Buffer() : m_ptr(NULL), m_read_pos(0), m_size(0)
{
}

Buffer::Buffer(const char *str, size_t len)
    : m_ptr(str),
      m_read_pos(0),
      m_size(len)
{
}

Buffer::Buffer(std::string& str)
    : m_ptr(str.c_str()),
      m_read_pos(0),
      m_size(str.size())
{
}

Buffer::~Buffer()
{
}

size_t Buffer::size() const
{
    return m_size - m_read_pos;
}

size_t Buffer::capacity() const
{
    return m_size;
}

bool Buffer::empty() const
{
    return m_size == m_read_pos;
}

const char *Buffer::current() const
{
    return &m_ptr[m_read_pos];
}

char Buffer::peek() const
{
    return m_ptr[m_read_pos];
}

char Buffer::getc()
{
    return m_ptr[m_read_pos++];
}

void Buffer::consume(size_t n)
{
    m_read_pos += n;
}
