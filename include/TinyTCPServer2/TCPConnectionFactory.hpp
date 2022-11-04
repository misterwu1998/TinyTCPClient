#if !defined(_TCPConnectionFactory_hpp)
#define _TCPConnectionFactory_hpp

#include <memory>

namespace TTCPS2
{
  class TCPConnection;
  class NetIOReactor;
  class Acceptor;

  /// @brief To be inherited.
  class TCPConnectionFactory
  {
  public:
  
    /// @brief 
    /// @param netIOReactor TCPConnection::TCPConnection() <- 新TCPConnection对象会被分发到哪个网络IO反应堆
    /// @param clientSocket TCPConnection::TCPConnection() <- 与客户端通信的socket
    /// @return 
    virtual std::shared_ptr<TCPConnection> operator() (
        NetIOReactor* netIOReactor
      , int clientSocket
    );

    virtual ~TCPConnectionFactory(){};
  };

} // namespace TTCPS2

#endif // _TCPConnectionFactory_hpp
