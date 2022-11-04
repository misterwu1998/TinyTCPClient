#if !defined(_TinyWebServer_hpp)
#define _TinyWebServer_hpp

#include <memory>

namespace TTCPS2
{
  class TinyTCPServer2;
  class HTTPHandlerFactory;
  class ThreadPool;

  class TinyHTTPServer
  {
  private:
    TinyTCPServer2* tcpServer;
  public:
    TinyHTTPServer(
        const char* ip
      , unsigned short port 
      , unsigned int listenSize
      , unsigned int nNetIOReactors
      , std::shared_ptr<HTTPHandlerFactory> const& HTTPSettings
      , ThreadPool* const tp //线程池是单例
    );
    int run();
    int shutdown();
    virtual ~TinyHTTPServer();
        
  };
  
} // namespace TTCPS2

#endif // _TinyWebServer_hpp
