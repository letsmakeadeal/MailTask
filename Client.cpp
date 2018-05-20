#include "Client.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include "HTTPResponseParser.h"
Client::Client(const std::string &url) { url_parser_.Parse(url); }

bool Client::Connect() {
  if (url_parser_.GetProtocol() != "http" )
    throw std::logic_error("Sorry , working only with HTTP protocol");

  sockaddr_in addr;
  socket_ = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_ < 0) {
    throw std::runtime_error("Can't create socket");
  }

  addr.sin_family = AF_INET;
  addr.sin_port = htons(80);
  hostent *hp = gethostbyname(url_parser_.GetHost().c_str());

  char *value = inet_ntoa(*(struct in_addr *)(hp->h_addr_list[0]));
  addr.sin_addr.s_addr = inet_addr(value);

  if (connect(socket_, (sockaddr *)&addr, sizeof(addr)) < 0) {
    std::cerr << "Can't connect" << std::endl;
    return false;
  }

  return true;
};

const std::string Client::GenerateHTTPRequest(const std::string &host,
                                              const std::string &path) {
  std::ostringstream str;
  str << "GET " << path << " HTTP/1.0\r\nHost: " << host << "\r\n\r\n\r\n";
  return str.str();
}

const std::string Client::GenerateFileName(const std::string &host,
                                           const std::string &path) {
  std::string path_copy = path;
  std::replace(path_copy.begin(), path_copy.end(), '/', '_');
  std::ostringstream str;
  str << "data_from_" << host << path_copy;
  return str.str();
}

void Client::LoadData() {
  std::string http_request =
      GenerateHTTPRequest(url_parser_.GetHost(), url_parser_.GetPath());
  std::cout << http_request << std::endl;

  send(socket_, http_request.c_str(), http_request.length(), 0);
  char buffer[10000];
  std::string filename =
      GenerateFileName(url_parser_.GetHost(), url_parser_.GetPath());
  std::fstream file;
  file.open(filename, std::ios_base::out | std::ios_base::in);
  int counter = 0;

  if (file.is_open()) {
    while (1) {
      std::ostringstream cntr;
      cntr << counter;
      file.close();
      file.open(filename + cntr.str(), std::ios_base::out | std::ios_base::in);
      std::cout << file.is_open() << std::endl;
      if (!file.is_open()) {
        file.clear();
        file.open(filename + cntr.str(), std::ios_base::out);
        filename += cntr.str();
        break;
      }
      ++counter;
    }
  } else {
    file.clear();
    file.open(filename, std::ios_base::out);
  }

  int data_length = 1;
  counter = 0;

  HTTPResponseParser http_parser;
  while (data_length != 0) {
    data_length = recv(socket_, buffer, 10000, 0);

    if (counter == 0) {
      std::string buf = buffer;
      size_t it = buf.find("\r\n\r\n");
      if (it != std::string::npos) {
        std::string HTTPHeader(buf.begin(), buf.begin() + it);
        http_parser.Parse(HTTPHeader);
      }
      file << std::string(buf.begin() + it + 4, buf.end());
    } else
      file << buffer;

    ++counter;
  }

  close(socket_);
  if (http_parser.Redirected()) {
    file.close();
    std::remove(filename.c_str());
    url_parser_.Parse(http_parser.GetRedirectedURL());
    Connect();
    LoadData();
  }
};
