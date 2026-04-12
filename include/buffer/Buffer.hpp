#ifndef BUFFER_HPP
#define BUFFER_HPP
#include <cstddef>
#include <string>

class Buffer
{
  private:
    const char *m_ptr;
    size_t      m_size;
    size_t      m_read_pos;

  public:
    Buffer();
    Buffer(const char *str, size_t len);
    Buffer(std::string& str);
    ~Buffer();

    bool        empty() const;
    char        peek() const;
    const char *current() const;
    size_t      size() const;
    size_t      capacity() const;
    char        getc();

    void consume(size_t n);
};
#endif
