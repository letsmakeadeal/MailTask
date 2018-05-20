#pragma once
#include <iostream>
#include <string>

class HTTPResponseParser {
 public:
  HTTPResponseParser(){};
  bool Redirected();
  const std::string GetRedirectedURL();
  void Parse(const std::string& header);

 private:
  bool redirected_;
  std::string redirected_url_;
};