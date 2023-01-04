#include <iostream>
#include "TinyTCPClient.hpp"
#include "TinyHTTPClient/TinyHTTPClient.hpp"
#include "TinyHTTPServer/HTTPMessage.hpp"
#include <unordered_map>
#include <winsock2.h>

struct Action_WSA{

  Action_WSA(){
    WORD version = MAKEWORD(2,2);
    WSADATA wsaData;
    if(0!=WSAStartup(version, &wsaData)){
      std::cout << "WSAStartup() failed!" << std::endl;
      exit(0);
    }
    std::cout << "WSAStartup() done!" << std::endl;
  }

  ~Action_WSA(){
    WSACleanup();
    std::cout << "WSACleanup() done!" << std::endl;
  }
  
};

static Action_WSA action;

int main(int argc, char const *argv[])
{
  TinyHTTPClient c("127.0.0.1",6324);
  auto ip = c.getServerIP();
  auto port = c.getServerPort();
  HTTPRequest req;
  // req.set(http_method::HTTP_POST)
  //    .set("/null")
  //    .set_chunked("./Makefile");
  req.set(http_method::HTTP_GET)
     .set("/chunked");
  c.send(req);
  c.send(req);
  HTTPResponse res;
  c.recv(res);
  c.recv(res);

  // TinyTCPClient c("127.0.0.1",6324);
  // c.send("abcdefg",8);
  // char s[8];
  // while(0>=c.recv(s,8)){}
  // std::cout << s << std::endl;
  
  return 0;
}
