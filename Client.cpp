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
  if (url_parser_.GetProtocol() != "http") {
    std::cerr << "Sorry , working only with HTTP protocol" << std::endl;
    return false;
  }

  std::cout << "Protocol = " << url_parser_.GetProtocol()
            << " ,Host= " << url_parser_.GetHost()
            << " , Port = " << url_parser_.GetPort()
            << " ,path=" << url_parser_.GetPath() << std::endl;

  addrinfo hints, *res;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  int status;
  if (status = getaddrinfo(url_parser_.GetHost().c_str(), "http", &hints,
                           &res) != 0) {
    std::cerr << "getaddrinfo:" << gai_strerror(status) << std::endl;
    return false;
  }

  char ipstr[INET6_ADDRSTRLEN];
  for (addrinfo *p = res; p != NULL; p = p->ai_next) {
    void *addr;
    std::string ipver = "";
    if (p->ai_family == AF_INET) {
      struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
      addr = &(ipv4->sin_addr);
      ipver = "Addres in IPv4 : ";
    } else {
      struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
      addr = &(ipv6->sin6_addr);
      ipver = "Addres in IPv6 :";
    }
    inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
    std::cout << ipver << ipstr << std::endl;
  }

  socket_ = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (socket_ < 0) {
    std::cerr << "Can't create socket" << std::endl;
    return false;
  }

  int port = 80;
  if (url_parser_.GetPort() != "") {
    std::istringstream str(url_parser_.GetPort());
    str >> port;
  }

  if (res->ai_family == AF_INET) {
    ((struct sockaddr_in *)(res->ai_addr))->sin_port = htons(port);
  } else {
    ((struct sockaddr_in6 *)(res->ai_addr))->sin6_port = htons(port);
  }

  if (connect(socket_, res->ai_addr, res->ai_addrlen) < 0) {
    std::cerr << "Can't connect" << std::endl;
    return false;
  } else {
    std::cout << " Connected OK \n" << std::endl;
  }

  return true;
};

const std::string Client::GenerateHTTPRequest(const std::string &host,
                                              const std::string &path) {
  std::ostringstream str;
  str << "GET " << path << " HTTP/1.1\r\nHost: " << host << "\r\n\r\n\r\n";
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
  std::cout << "Request send to server : \n" << http_request << std::endl;

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
  int data_length_not_readed = 0;
  int content_length = 0;

  while (data_length != 0) {
    data_length = recv(socket_, buffer, 10000, 0);
    std::string buf = buffer;
    if (counter == 0) {
      size_t it = buf.find("\r\n\r\n");
      if (it != std::string::npos) {
        std::string HTTPHeader(buf.begin(), buf.begin() + it + 2);
        std::cout << "Http response header: \n\n" << HTTPHeader << std::endl;
        http_parser.Parse(HTTPHeader);

        if (http_parser.Redirected()) break;
      }
      std::string buff_without_header(buf.begin() + it + 4,
                                      buf.begin() + data_length);
      if (buff_without_header.size() != 0) {
        if (!http_parser.IsChunkedEncoding()) {
          file << buff_without_header;
        } else {
          std::string str = http_parser.ParseIfChuckedBuff(buff_without_header);
          data_length_not_readed = http_parser.GetChunkSize();
          if (str == "") break;
          file << str;
        }
      }

      content_length = http_parser.GetContentLength();
      if (buff_without_header.size() == http_parser.GetContentLength()) break;
      content_length -= buff_without_header.size();

    } else {
      buf = std::string(buf.begin(), buf.begin() + data_length);
      if (http_parser.IsChunkedEncoding()) {
        if (data_length_not_readed == 0) {
          std::string str = http_parser.ParseIfChuckedBuff(buf);
          data_length_not_readed = http_parser.GetChunkSize();
          if (str == "") break;
          file << str;
        } else {
          if (data_length_not_readed > data_length) {
            file << buf;
            data_length_not_readed -= data_length;
          } else {
            file << std::string(buf.begin(),
                                buf.begin() + data_length_not_readed);

            if (std::string(buf.begin() + data_length_not_readed + 2, buf.end())
                    .size() != 0) {
              std::string str = http_parser.ParseIfChuckedBuff(std::string(
                  buf.begin() + data_length_not_readed + 2, buf.end()));

              data_length_not_readed = http_parser.GetChunkSize();
              if (str == "" && data_length_not_readed == 0) break;
              file << str;
            } else
              data_length_not_readed = 0;
          }
        }

      } else {
        content_length -= buf.size();
        file << buf;
        if (content_length == 0) break;
      }
    }
    ++counter;
  }

  close(socket_);
  if (http_parser.Redirected()) {
    std::cout << "\nRedirected URL : " << http_parser.GetRedirectedURL() << "\n"
              << std::endl;
    file.close();
    std::remove(filename.c_str());

    if (http_parser.OnlyPathRedirected())
       url_parser_.SetPath(http_parser.GetRedirectedURL());
     else
       url_parser_.Parse(http_parser.GetRedirectedURL());

    Connect();
    LoadData();
  }
};