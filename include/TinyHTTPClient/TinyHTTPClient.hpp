#if !defined(_TinyHTTPClient_hpp)
#define _TinyHTTPClient_hpp

#include <memory>
#include <string>
#include <fstream>
#include "http-parser/http_parser.h"

class HTTPRequest;
class HTTPResponse;
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

  HTTPResponse* responseNow;
  http_parser parser;
  http_parser_settings settings;
  char* buf;
  uint32_t dataLen_buf;

  std::string headerKeyNow;//解析HTTP数据的过程中使用的暂存变量
  std::string headerValueNow;//解析HTTP数据的过程中使用的暂存变量
  std::fstream bodyFileNow;//解析HTTP数据的过程中使用的暂存变量

public:
  TinyHTTPClient(const char* serverIP, unsigned short port);

  std::string getServerIP() const;
  unsigned short getServerPort() const;

  /// @brief 
  /// @param r 
  /// @return 0=OK; -1=ERROR
  int send(HTTPRequest const& r);

  /// @brief 
  /// @param r 
  /// @return 0=OK; -1=ERROR
  int recv(HTTPResponse& r);
  virtual~TinyHTTPClient();
};

#endif // _TinyHTTPClient_hpp
