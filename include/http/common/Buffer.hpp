#ifndef BUFFER_HPP
#define BUFFER_HPP
#include <cstddef>
#include <string>

class Buffer
{
  private:
    const char *m_ptr;
    size_t      m_read_pos;
    size_t      m_size;

  public:
    Buffer();
    Buffer(const char *str, size_t len);
    Buffer(std::string &str);
    ~Buffer();

    bool        empty() const;
    const char *current() const;

    size_t size() const;
    size_t capacity() const;

    char peek() const;
    char getc();
    char ugetc();

    void consume(size_t n);
};
#endif
