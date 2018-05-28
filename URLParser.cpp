#include "URLParser.h"
#include <algorithm>
#include <exception>
#include <iostream>
void URLParser::Parse(const std::string& url) {
  // Parsing host and protocol
  size_t pos_of_prot_beg = url.find("http://");
  if (pos_of_prot_beg == std::string::npos) {
    throw std::runtime_error("Can't find http protocol in URL ");
  }
  size_t pos_of_prot_end = url.find("://");
  protocol_ =
      std::string(url.begin() + pos_of_prot_beg, url.begin() + pos_of_prot_end);

  const int http_size = 7;

  if (url.size() == 7) return;

  size_t pos_of_host_end = 0;
  if ( url[pos_of_prot_end + 3] == '[' ) {
    size_t end_of_ipv6_host = url.find(']', pos_of_prot_end + 3);

    if (end_of_ipv6_host == std::string::npos)
      throw std::runtime_error("Ipv6 invalid host");

    host_ = std::string(url.begin() + pos_of_prot_end + 4,
                        url.begin() + end_of_ipv6_host );
    pos_of_host_end = end_of_ipv6_host + 1;

  } else {
    pos_of_host_end = url.find(':', pos_of_prot_end + 3);

    size_t pos_of_host_end_with_slash = url.find('/', pos_of_prot_end + 3);

    if ((pos_of_host_end > pos_of_host_end_with_slash ||
         pos_of_host_end == std::string::npos) &&
        pos_of_host_end_with_slash != std::string::npos) {
      pos_of_host_end = pos_of_host_end_with_slash;
    }

    if (pos_of_host_end == std::string::npos) {
      pos_of_host_end = url.find('?', pos_of_prot_end + 3);
    }

    if (pos_of_host_end == std::string::npos) {
      pos_of_host_end = url.find('#', pos_of_prot_end + 3);
    }

    if (pos_of_host_end == std::string::npos) pos_of_host_end = url.size();

    host_ = std::string(url.begin() + pos_of_prot_end + 3,
                        url.begin() + pos_of_host_end);

    if (pos_of_host_end == url.size() ||
        (pos_of_host_end == url.size() - 1 && url[pos_of_host_end] == '/')) {
      path_ = '/';
      return;
    }
  }
  // Parsing port
  size_t pos_of_port_end = 0;

  bool url_ended_with_port = false;

  if (url[pos_of_host_end] == ':') {
    pos_of_port_end = url.find('/', pos_of_host_end);

    if (pos_of_port_end == pos_of_host_end + 1) {
      throw std::runtime_error("Incorrect port");
    } else if (pos_of_port_end == std::string::npos) {
      pos_of_port_end = url.find('?', pos_of_host_end);

      if (pos_of_port_end != std::string::npos) {
        port_ = std::string(url.begin() + pos_of_host_end + 1,
                            url.begin() + pos_of_port_end);
      } else {
        pos_of_port_end = url.find('#', pos_of_host_end);
      }

      if (pos_of_port_end == std::string::npos) {
        port_ = std::string(url.begin() + pos_of_host_end + 1,
                            url.begin() + url.size());
        url_ended_with_port = true;
      }

    } else
      port_ = std::string(url.begin() + pos_of_host_end + 1,
                          url.begin() + pos_of_port_end);
  } else {
    port_ = "";
    pos_of_port_end = pos_of_host_end;
  }

  if (url_ended_with_port) {
    path_ = '/';
    return;
  }
  // Parsing path
  size_t end_of_path = url.find('?', pos_of_port_end);
  if (end_of_path == pos_of_port_end + 1) {
    throw std::runtime_error(" Incorrect path ");
    return;

  } else if (end_of_path == std::string::npos) {
    end_of_path = url.find('#', pos_of_port_end);

    if (end_of_path == pos_of_port_end + 1) {
      throw std::runtime_error(" Incorrect path ");
      return;
    } else if (end_of_path == std::string::npos) {
      path_ =
          std::string(url.begin() + pos_of_port_end, url.begin() + url.size());
    }
  } else {
    path_ =
        std::string(url.begin() + pos_of_port_end, url.begin() + end_of_path);
  }
}
void URLParser::SetPath(const std::string& path) { path_ = path; };
const std::string& URLParser::GetProtocol() { return protocol_; }
const std::string& URLParser::GetHost() { return host_; }
const std::string& URLParser::GetPath() { return path_; }
const std::string& URLParser::GetPort() { return port_; }