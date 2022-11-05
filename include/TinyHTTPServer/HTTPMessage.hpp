#if !defined(_HTTPMessage_hpp)
#define _HTTPMessage_hpp

#include <string>
#include <unordered_map>
#include <memory>
#include "http-parser/http_parser.h"

namespace TTCPS2
{
  class Buffer;

  class HTTPRequest
  {
  public:
    http_method method;
    std::string url;
    // std::string version;
    std::unordered_multimap<std::string, std::string> header;//严格来讲，HTTP没有禁止header的键重复
    std::shared_ptr<Buffer> body;//body可能是长度不定的chunk
    std::string filePath;//chunk数据如果很大，需要存放到磁盘，记住文件路径

    HTTPRequest& set(http_method method);
    HTTPRequest& set(std::string const& headerKey, std::string const& headerValue);
    HTTPRequest& set(std::string const& url);
    HTTPRequest& append(const void* data, uint32_t length);
    HTTPRequest& set_chunked(std::string const& filepath);
  };

  class HTTPResponse
  {
  public:
    // std::string version;
    http_status status;
    std::unordered_multimap<std::string, std::string> header;//严格来讲，HTTP没有禁止header的键重复
    std::shared_ptr<Buffer> body;//body可能是长度不定的chunk
    std::string filePath;//chunk数据如果很大，需要存放到磁盘，记住文件路径

    HTTPResponse();
    HTTPResponse& set(http_status s);
    HTTPResponse& set(std::string const& headerKey, std::string const& headerValue);
    HTTPResponse& append(const void* data, uint32_t length);
    HTTPResponse& set_chunked(std::string const& filepath);
  };

} // namespace TTCPS2

#endif // _HTTPMessage_hpp
