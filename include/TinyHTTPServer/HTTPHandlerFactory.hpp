#include "TinyTCPServer2/TCPConnectionFactory.hpp"

#if !defined(_HTTPHandlerFactory_hpp)
#define _HTTPHandlerFactory_hpp

#include <string>
#include <functional>
#include <unordered_map>
#include "http-parser/http_parser.h"

class HTTPRequest;
class HTTPResponse;

class HTTPHandlerFactory : virtual public TCPConnectionFactory
{
protected:

  /// @brief vector<谓词<是否满足条件 (HTTP请求)>, 回调函数<HTTP响应 (HTTP请求)>>
  std::vector<std::pair<
    std::function<bool (std::shared_ptr<HTTPRequest>)>,
    std::function<std::shared_ptr<HTTPResponse> (std::shared_ptr<HTTPRequest>)>
  >> router;

public:

  HTTPHandlerFactory();

  /// @brief 将满足predicate的HTTP请求分发给相应的回调函数；顺序上先被route的predicate，就先被考虑
  /// @param predicate function<是否满足条件 (HTTP请求)>
  /// @param callback function<HTTP响应 (HTTP请求)>
  /// @return 0
  int route(
    std::function<bool (std::shared_ptr<HTTPRequest>)> const& predicate,
    std::function<std::shared_ptr<HTTPResponse> (std::shared_ptr<HTTPRequest>)> const& callback
  );

  virtual std::shared_ptr<TCPConnection> operator()(NetIOReactor* netIOReactor, int clientSocket);

  virtual ~HTTPHandlerFactory();

};

#endif // _HTTPHandlerFactory_hpp
