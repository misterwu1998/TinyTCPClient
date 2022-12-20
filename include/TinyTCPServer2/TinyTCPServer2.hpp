#if !defined(LENGTH_PER_RECV)
#define LENGTH_PER_RECV 2048 //每次从内核接收缓冲区拷贝多少字节
#endif // LENGTH_PER_RECV

#if !defined(LENGTH_PER_SEND)
#define LENGTH_PER_SEND 2048 //每次向内核发送缓冲区拷贝多少字节
#endif // LENGTH_PER_SEND

#if !defined(EPOLL_SIZE)
#define EPOLL_SIZE 1024 //epoll监听树结点上限
#endif // EPOLL_SIZE


#if !defined(_TinyTCPServer2_hpp)
#define _TinyTCPServer2_hpp

#include <memory>
#include <vector>
#include <thread>
#include <mutex>
#include <unordered_map>

namespace TTCPS2
{
  class TCPConnectionFactory;
  class TCPConnection;
  class ThreadPool;
  class Acceptor;
  class NetIOReactor;

  class TinyTCPServer2
  {
  public:

    const char* ip;
    unsigned short port;
    unsigned int listenSize;
    unsigned int nNetIOReactors;
    std::shared_ptr<TCPConnectionFactory> factory;
    ThreadPool* const tp;

    // 常量

    std::shared_ptr<Acceptor> acceptor;
    std::vector<std::shared_ptr<NetIOReactor>> netIOReactors;
    std::vector<std::thread> oneLoopPerThread;//nNetIOReactors个网络IO反应堆 + 1个Acceptor

    // 常量 //

    /// @brief <socket文件描述符, TCPConnection对象>
    std::unordered_map<int, std::shared_ptr<TCPConnection>> connections;
    std::mutex m_connections;

    TinyTCPServer2(
        const char* ip
      , unsigned short port 
      , unsigned int listenSize
      , unsigned int nNetIOReactors
      , std::shared_ptr<TCPConnectionFactory> const& factory
      , ThreadPool* const tp //线程池是单例
      // 日志器改为全局单例，封装后交给库使用者去指定, std::shared_ptr<spdlog::logger> logger //默认值: spdlog::rotating_logger_mt<spdlog::async_factory>("TinyTCPServer2.logger","./.log/",4*1024*1024,4);
    );

    int run();
    int shutdown();

    ~TinyTCPServer2();
  };

} // namespace TTCPS2

#endif // _TinyTCPServer2_hpp
