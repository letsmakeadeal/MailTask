#pragma once
#include <exception>
#include <string>
#include "URLParser.h"

class Client {
 public:
  Client(const std::string& url);
  bool Connect();
  void LoadData();

 private:
  const std::string GenerateHTTPRequest(const std::string& host,
                                        const std::string& path);
  const std::string GenerateFileName(const std::string& host,
                                     const std::string& path);
  URLParser url_parser_;
  int socket_;
};