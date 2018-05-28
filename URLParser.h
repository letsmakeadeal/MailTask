#pragma once
#include <string>

class URLParser {
 public:
  void Parse(const std::string& url);
  const std::string& GetProtocol();
  const std::string& GetHost();
  const std::string& GetPath();
  const std::string& GetPort();
  void SetPath(const std::string&
                   path);  // For the case if just path changed in redirection
 private:
  std::string host_;
  std::string protocol_;
  std::string path_;
  std::string port_;
};