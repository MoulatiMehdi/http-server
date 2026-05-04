#ifndef URI_HPP
#define URI_HPP

#include <string>

class Uri
{
  private:
    void process_params();
    void process_query();
    void process_path();

    std::string m_origin;
    std::string m_path;
    std::string m_query;
    std::string m_params;

  public:
    bool m_isvalid;

    const std::string path() const;
    const std::string query() const;
    const std::string params() const;
    const std::string origin() const;

    bool isvalid() const;

    void setUri(const std::string &uri);
    Uri();
    Uri(const std::string &uri);
    ~Uri();
};
#endif
