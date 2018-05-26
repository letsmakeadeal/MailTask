#pragma once
#include <iostream>
#include <string>

class HTTPResponseParser {
 public:
  HTTPResponseParser()
      : redirected_(false), chunked_(false), redirected_only_path_(false){};
  bool Redirected();
  const std::string& GetRedirectedURL();
  void Parse(const std::string& header);
  bool IsChunkedEncoding();
  const int GetChunkSize();
  std::string ParseIfChuckedBuff(const std::string& buff);
  int GetContentLength();
  bool OnlyPathRedirected();
 private:
  bool redirected_;
  std::string redirected_url_;
  bool chunked_;
  int chunk_size_;
  int content_length_;
  bool redirected_only_path_;
};