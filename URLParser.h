#pragma once
#include <string>

class URLParser {
 public:
  void Parse(const std::string& url);
  const std::string& GetProtocol();
  const std::string& GetHost();
  const std::string& GetPath();

 private:
  std::string host_;
  std::string protocol_;
  std::string path_;
};