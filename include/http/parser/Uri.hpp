#ifndef URI_HPP
#define URI_HPP

#include <string>

class Uri
{
  public:
    std::string path;
    std::string query;
    std::string params;

    bool isvalid;

    void process_params();
    void process_query();
    void process_path();

    Uri(const std::string &uri);
    ~Uri();
};
#endif
