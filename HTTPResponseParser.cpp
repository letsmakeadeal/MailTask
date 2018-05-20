#include "HTTPResponseParser.h"
#include <iostream>
#include <vector>
void HTTPResponseParser::Parse(const std::string& header) {
  std::vector<size_t> redirectes;
  redirectes.push_back(header.find("301 Moved Permanently"));
  redirectes.push_back(header.find("302 Moved Temporarily"));

  size_t position = std::string::npos;
  size_t size = redirectes.size();
  for (size_t i = 0; i < size; ++i) {
    if (redirectes[i] != std::string::npos) {
      position = redirectes[i];
      break;
    }
  }

  if (position != std::string::npos) {
    redirected_ = true;
    const int sizeof_word = 9;
    size_t pos = header.find("Location:");
    size_t end_of_path = header.find("\r\n", pos);
    redirected_url_ = std::string(header.begin() + pos + sizeof_word,
                                  header.begin() + end_of_path);
  } else
    redirected_ = false;
}

bool HTTPResponseParser::Redirected() { return redirected_; }
const std::string HTTPResponseParser::GetRedirectedURL() {
  return redirected_url_;
}