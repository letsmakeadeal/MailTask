#include "HTTPResponseParser.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>
void HTTPResponseParser::Parse(const std::string& header) {
  std::vector<size_t> redirectes;
  redirectes.push_back(header.find("301 Moved Permanently"));
  redirectes.push_back(header.find("302 Found"));
  redirectes.push_back(header.find("302 Moved Temporarily"));
  redirectes.push_back(header.find("307 Temporary Redirect"));

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
    std::string location = "Location:";

    size_t pos = header.find(location);
    std::string str_with_ws =
        std::string(header.begin() + pos + location.size(),
                    header.begin() + header.find("\r\n", pos));

    redirected_url_ = std::string(
        str_with_ws.begin(),
        std::remove(str_with_ws.begin(), str_with_ws.end(), ' '));

    if (redirected_url_[0] == '/'){
        redirected_only_path_ = true;
      }

  } else {
    redirected_ = false;
    std::string key_chunk = "Transfer-Encoding:";
    size_t chunked = header.find(key_chunk);

    if (chunked != std::string::npos) {
      std::string str_with_ws =
          std::string(header.begin() + chunked + key_chunk.size(),
                      header.begin() + header.find("\r\n", chunked));

      std::string str_without_ws(
          str_with_ws.begin(),
          std::remove(str_with_ws.begin(), str_with_ws.end(), ' '));

      if (str_without_ws == "chunked") chunked_ = true;
    }

    std::string key_length = "Content-Length:";
    size_t con_len = header.find(key_length);

    if (con_len != std::string::npos) {
      std::string str_with_ws =
          std::string(header.begin() + con_len + key_length.size(),
                      header.begin() + header.find("\r\n", con_len));

      std::string str_without_ws(
          str_with_ws.begin(),
          std::remove(str_with_ws.begin(), str_with_ws.end(), ' '));

      std::istringstream str_val(str_without_ws);
      str_val >> content_length_;
    }
  }
}
bool HTTPResponseParser::Redirected() { return redirected_; }

const std::string& HTTPResponseParser::GetRedirectedURL() {
  return redirected_url_;
}

bool HTTPResponseParser::OnlyPathRedirected(){ return redirected_only_path_;}

int HTTPResponseParser::GetContentLength() { return content_length_; }

bool HTTPResponseParser::IsChunkedEncoding() { return chunked_; }

std::string HTTPResponseParser::ParseIfChuckedBuff(const std::string& buff) {
  std::string message;
  int size_buf = buff.size();
  size_t pos = 0;
  bool flag = true;

  while (flag) {
    size_t crf = buff.find("\r\n", pos);
    std::string str(buff.begin(), buff.begin() + crf);
    std::istringstream size(str);
    int size_int = 0;
    size >> std::hex >> size_int;
    chunk_size_ = size_int;
    if (size_int == 0) {
      chunk_size_ = 0;
      return std::string();
    };

    pos = crf + 2;
    size_t end = 0;
    size_buf -= 2 + size_int + str.size();

    if (size_buf < 0) {
      end = buff.size();
      chunk_size_ -= (buff.size() - pos);
      flag = false;
    } else {
      end = pos + size_int;
    }

    std::string text(buff.begin() + pos, buff.begin() + end);
    pos += size_int + 2;
    size_buf -= 2;
    message += text;
  }
  return message;
}

const int HTTPResponseParser::GetChunkSize() { return chunk_size_; }