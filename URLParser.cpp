#include "URLParser.h"
#include <algorithm>
#include <iostream>

void URLParser::Parse(const std::string& url) {
  size_t pos_of_prot_beg = url.find("http://");
  size_t pos_of_prot_end = pos_of_prot_beg + 7;
  protocol_ = std::string(url.begin() + pos_of_prot_beg,
                          url.begin() + pos_of_prot_end - 3);
  size_t pos_of_host_end = url.find("/", pos_of_prot_end);

  if (pos_of_host_end == std::string::npos) pos_of_host_end = url.size();

  host_ =
      std::string(url.begin() + pos_of_prot_end, url.begin() + pos_of_host_end);

  if (url.begin() + pos_of_host_end != url.end()) {
    path_ = std::string(url.begin() + pos_of_host_end, url.end());
  } else
    path_ = '/';

  std::cout << "Protocol = " << protocol_ << " ,Host= " << host_
            << " ,path=" << path_ << std::endl;
}
const std::string& URLParser::GetProtocol() { return protocol_; }
const std::string& URLParser::GetHost() { return host_; }
const std::string& URLParser::GetPath() { return path_; }