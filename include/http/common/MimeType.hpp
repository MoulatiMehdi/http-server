#ifndef MIME_TYPE_HPP
#define MIME_TYPE_HPP

#include <map>
#include <string>

class MimeType
{

  private:
    const static std::string                   DEFAULT;

    typedef std::map<std::string, std::string> Map;
    typedef Map::iterator                      iterator;
    typedef Map::const_iterator                const_iterator;

    Map map;

  public:
    MimeType();
    ~MimeType();

    const std::string &getContentType(const std::string &ext);
};
#endif
