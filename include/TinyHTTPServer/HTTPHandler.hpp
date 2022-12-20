#if !defined(_HTTPHandler_hpp)
#define _HTTPHandler_hpp

#include <fstream>
#include "TinyTCPServer2/TCPConnection.hpp"
#include "http-parser/http_parser.h"

class Buffer;
class HTTPRequest;
class HTTPResponse;

class HTTPHandler : virtual public TCPConnection
{
private:
  std::unordered_map<
    http_method,
    std::unordered_map<
        std::string
      , std::function<std::shared_ptr<HTTPResponse> (std::shared_ptr<HTTPRequest>)>>> const& router;
  Buffer* unParsed;
  http_parser parser;
  http_parser_settings settings;

  std::shared_ptr<HTTPRequest> requestNow;//当前正在解析，或恰好解析完、还未响应的Request
  std::string headerValueNow;//用于解析HTTP的临时变量，一旦requestNow解析完整，就应当置空
  std::string headerKeyNow;//用于解析HTTP的临时变量，一旦requestNow解析完整，就应当置空
  std::fstream bodyFileNow;//用于解析HTTP的临时变量，一旦requestNow解析完整，就应当置空

public:
    
  /// from http-parser.h
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
  *///
  static int onMessageBegin(http_parser* parser);
  static int onURL(http_parser* parser, const char *at, size_t length);
  static int onHeaderField(http_parser* parser, const char *at, size_t length);
  static int onHeaderValue(http_parser* parser, const char *at, size_t length);
  static int onHeadersComplete(http_parser* parser);
  static int onBody(http_parser* parser, const char *at, size_t length);
  static int onChunkHeader(http_parser* parser);
  static int onChunkComplete(http_parser* parser);
  static int onMessageComplete(http_parser* parser);

  HTTPHandler(
      NetIOReactor* netIOReactor
    , int clientSocket
    , std::unordered_map<
        http_method,
        std::unordered_map<
            std::string
          , std::function<std::shared_ptr<HTTPResponse> (std::shared_ptr<HTTPRequest>)>>> const& router);
  virtual int handle();
  virtual ~HTTPHandler();
  
};

#endif // _HTTPHandler_hpp
