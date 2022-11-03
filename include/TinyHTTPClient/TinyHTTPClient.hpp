#if !defined(_TinyHTTPClient_hpp)
#define _TinyHTTPClient_hpp

#include <memory>
#include <string>
#include <fstream>
#include "http-parser/http_parser.h"

namespace TTCPS2
{
  class HTTPRequest;
  class HTTPResponse;
} // namespace TTCPS2
class TinyTCPClient;

class TinyHTTPClient
{
private:
  
  TinyTCPClient* c;
  
  /* Callbacks should return non-zero to indicate an error. The parser will
  * then halt execution.
  *
  * The one exception is on_headers_complete. In a HTTP_RESPONSE parser
  * returning '1' from on_headers_complete will tell the parser that it
  * should not expect a body. This is used when receiving a response to a
  * HEAD request which may contain 'Content-Length' or 'Transfer-Encoding:
  * chunked' headers that indicate the presence of a body.
  *
  * Returning `2` from on_headers_complete will tell parser that it should not
  * expect neither a body nor any futher responses on this connection. This is
  * useful for handling responses to a CONNECT request which may not contain
  * `Upgrade` or `Connection: upgrade` headers.
  *
  * http_data_cb does not return data chunks. It will be called arbitrarily
  * many times for each string. E.G. you might get 10 callbacks for "on_url"
  * each providing just a few characters more data.
  */
  static int onMessageBegin(http_parser* parser);
  static int onStatus(http_parser* parser, const char *at, size_t length);
  static int onHeaderField(http_parser* parser, const char *at, size_t length);
  static int onHeaderValue(http_parser* parser, const char *at, size_t length);
  static int onHeadersComplete(http_parser* parser);
  static int onBody(http_parser* parser, const char *at, size_t length);
  static int onChunkHeader(http_parser* parser);
  static int onChunkComplete(http_parser* parser);
  static int onMessageComplete(http_parser* parser);

  TTCPS2::HTTPResponse* responseNow;
  http_parser parser;
  http_parser_settings settings;
  char* buf;
  uint32_t dataLen_buf;
  std::string headerKeyNow;
  std::string headerValueNow;
  std::fstream bodyFileNow;

public:
  TinyHTTPClient(const char* serverIP, unsigned short port);

  /// @brief 
  /// @param r 
  /// @return 0=OK; -1=ERROR
  int send(TTCPS2::HTTPRequest const& r);

  /// @brief 
  /// @param r 
  /// @return 0=OK; -1=ERROR
  int recv(TTCPS2::HTTPResponse& r);
  virtual~TinyHTTPClient();
};

TTCPS2::HTTPRequest& operator<<(TTCPS2::HTTPRequest& r, http_method method);
TTCPS2::HTTPRequest& operator<<(TTCPS2::HTTPRequest& r, std::pair<std::string, std::string> const& headerKV);
TTCPS2::HTTPRequest& operator<<(TTCPS2::HTTPRequest& r, std::string const& url);

/// @brief 指定为非chunk data模式，丢弃{"Transfer-Encoding": "chunked"}、filepath
/// @param r 
/// @param bodyData_and_length 
/// @return 
TTCPS2::HTTPRequest& operator<<(TTCPS2::HTTPRequest& r, std::pair<const void*, uint32_t> const& bodyData_and_length);

/// @brief 指定为chunk data模式，丢弃Content-Length键值对、body
/// @param r 
/// @param filepath 
/// @return 
TTCPS2::HTTPRequest& operator<<=(TTCPS2::HTTPRequest& r, std::string const& filepath);

#endif // _TinyHTTPClient_hpp
