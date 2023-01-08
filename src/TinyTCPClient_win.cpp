#include "TinyTCPClient.hpp"
#include <winsock2.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

TinyTCPClient::TinyTCPClient(const char* serverIP, unsigned short port)
: serverIP(serverIP), serverPort(port), skt(socket(AF_INET,SOCK_STREAM,0)){
  sockaddr_in a;
  if(INVALID_SOCKET==skt){
    std::cout << "TinyTCPClient::TinyTCPClient(): fail to create socket." << std::endl;
  }else{
    a.sin_addr.s_addr = inet_addr(serverIP);
    a.sin_port = htons(port);
    a.sin_family = AF_INET;
    if(SOCKET_ERROR==connect(skt, (const sockaddr*)&a, sizeof(a))){
      std::cout << "TinyTCPClient::TinyTCPClient(): fail to connect to server with IP="<<serverIP
                << " and port="<<port
                << std::endl;
      closesocket(skt);
      skt = INVALID_SOCKET;
    }
  }
}

int64_t TinyTCPClient::send(const char* data, uint32_t length){
  if(INVALID_SOCKET==skt){
    std::cout << "TinyTCPClient::send(): no socket." << std::endl;
    return -1;
  }
  auto ret = ::send(skt,data,length,0);
  if(SOCKET_ERROR==ret){
    std::cout << "TinyTCPClient::send(): fail to send data" << std::endl;
    return -1;
  }
  return ret;
}

int64_t TinyTCPClient::recv(char* data, uint32_t capacity){
  if(INVALID_SOCKET==skt){
    std::cout << "TinyTCPClient::recv(): no socket." << std::endl;
    return -1;
  }
  auto ret = ::recv(skt,data,capacity,0);
  if(SOCKET_ERROR==ret){
    std::cout << "TinyTCPClient::send(): fail to recv data" << std::endl;
    return -1;
  }
  if(0==ret){//正常关闭
    ::closesocket(skt);
    skt = INVALID_SOCKET;
    return -1;
  }
  return ret;
}

int64_t TinyTCPClient::recv_nowait(char* data, uint32_t capacity){
  if(INVALID_SOCKET==skt){
    std::cout << "TinyTCPClient::recv_nowait(): no socket." << std::endl;
    return -1;
  }

  // 临时改为非阻塞socket
  u_long flag = 1;
  auto ret = ioctlsocket(skt,FIONBIO, &flag);
  if(SOCKET_ERROR==ret){
    std::cout << "TinyTCPClient::recv_nowait(): fail to set socket unblocked." << std::endl;
    return -1;
  }

  auto len = ::recv(skt,data,capacity,0);
  if(SOCKET_ERROR==len &&
     WSAGetLastError()!=WSAEWOULDBLOCK/*并非资源暂时不可用的情况*/)
  {
    // 改回阻塞socket
    flag = 0;
    ret = ioctlsocket(skt,FIONBIO, &flag);
    if(SOCKET_ERROR==ret){
      std::cout << "TinyTCPClient::recv_nowait(): fail to reset socket blocked." << std::endl;
      return -1;
    }

    std::cout << "TinyTCPClient::recv_nowait(): fail to recv data from socket "
              << skt << std::endl;
    return -1;
  }

  // 改回阻塞socket
  flag = 0;
  ret = ioctlsocket(skt,FIONBIO, &flag);
  if(SOCKET_ERROR==ret){
    std::cout << "TinyTCPClient::recv_nowait(): fail to reset socket blocked." << std::endl;
    return -1;
  }

  return len;
}

TinyTCPClient::~TinyTCPClient(){
  if(INVALID_SOCKET!=skt){
    closesocket(skt);
  }
}
