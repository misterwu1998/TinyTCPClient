#include <iostream>
#include "TinyTCPClient.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

TinyTCPClient::TinyTCPClient(const char* serverIP, unsigned short port)
: skt(socket(AF_INET, SOCK_STREAM, 0))
, serverIP(serverIP)
, serverPort(port){
  sockaddr_in a;
  if(0>skt){
    std::cout << "TinyTCPClient::TinyTCPClient(): fail to create socket." << std::endl;
  }else{
    a.sin_addr.s_addr = inet_addr(serverIP);
    a.sin_port = htons(port);
    a.sin_family = AF_INET;
    if(0>connect(skt, (const sockaddr*)&a, sizeof(a))){
      std::cout << "TinyTCPClient::TinyTCPClient(): fail to connect to server with IP="<<serverIP
                << " and port="<<port
                << ", errno means: "<<strerror(errno)
                << std::endl;
      close(skt);
      skt = -1;
    }
  }
}

int64_t TinyTCPClient::send(const char* data, uint32_t length){
  if(0>skt){
    std::cout << "TinyTCPClient::send(): no socket." << std::endl;
    return -1;
  }
  auto ret = ::send(skt,data,length, MSG_NOSIGNAL);
  if(0>ret){
    std::cout << "TinyTCPClient::send(): fail to send data, errno means: "<<strerror(errno) << std::endl;
    return -1;
  }
  return ret;
}

int64_t TinyTCPClient::recv(char* data, uint32_t capacity){
  if(0>skt){
    std::cout << "TinyTCPClient::recv(): no socket." << std::endl;
    return -1;
  }
  auto ret = ::recv(skt,data,capacity, 0);
  if(0>ret){
    std::cout << "TinyTCPClient::send(): fail to send data, errno means: "<<strerror(errno) << std::endl;
    return -1;
  }else if(0==ret){//socket已经有序关闭
    close(skt);
    skt = -1;
    return -1;
  }
  return ret;
}

int64_t TinyTCPClient::recv_nowait(char* data, uint32_t capacity){
  auto ret = ::recv(skt,data,capacity, MSG_DONTWAIT);
  if(ret>0){//正常收取
    return ret;
  }
  else if (ret==0 || (errno!=EWOULDBLOCK && errno!=EAGAIN)){//对方挂断或其它导致不能再正常通信的意外
    std::cout << "TinyTCPClient::recv_nowait(): the connection through socket "<<skt<<" has dropped." << std::endl;
    close(skt);
    skt = -1;
    return -1;
  }
  else{//errno==EWOULDBLOCK, 没数据可以收了
    return 0;
  }
}

TinyTCPClient::~TinyTCPClient(){
  if(0<=skt){
    close(skt);
  }
}
