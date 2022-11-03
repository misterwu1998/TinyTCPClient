#include <iostream>
#include "TinyTCPClient.hpp"
#include "TinyHTTPClient/TinyHTTPClient.hpp"
#include "TinyHTTPServer/HTTPMessage.hpp"

int main(int argc, char const *argv[])
{
  TinyHTTPClient c("127.0.0.1",6324);
  TTCPS2::HTTPRequest req;
  // req << http_method::HTTP_GET
  //     << "/login.html";
  req << http_method::HTTP_POST
      << "/null"
      <<= "./Makefile";
  c.send(req);
  c.send(req);
  TTCPS2::HTTPResponse res;
  c.recv(res);
  c.recv(res);

  // TinyTCPClient c("127.0.0.1",6324);
  // c.send("abcdefg",8);
  // char s[8];
  // while(0>=c.recv(s,8)){}
  // std::cout << s << std::endl;
  
  return 0;
}
