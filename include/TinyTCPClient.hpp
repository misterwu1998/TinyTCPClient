#if !defined(_TinyTCPClient_hpp)
#define _TinyTCPClient_hpp

#include <sstream>
#include <string>

class TinyTCPClient
{
private:
  int skt;
  TinyTCPClient(TinyTCPClient const&) : serverPort(0){}
  TinyTCPClient& operator=(TinyTCPClient const&){}

public:

  std::string const serverIP;
  unsigned short const serverPort;

  TinyTCPClient(const char* serverIP, unsigned short port);

  /// @brief 阻塞式发送
  /// @return 非负数表示实际发送的字节数目，或-1表示无法和服务端通信
  int64_t send(const char* data, uint32_t length);

  /// @brief 阻塞式接收
  /// @return 非负数表示实际接收的字节数目，其中0表示暂时无数据可接收；-1表示无法和服务端通信
  int64_t recv(char* data, uint32_t capacity);

  /// @brief 非阻塞式接收
  /// @return 非负数表示实际接收的字节数目，其中0表示暂时无数据可接收；-1表示无法和服务端通信
  int64_t recv_nowait(char* data, uint32_t capacity);

  virtual~TinyTCPClient();
};

#endif // _TinyTCPClient_hpp
