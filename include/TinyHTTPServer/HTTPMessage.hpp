#if !defined(_HTTPMessage_hpp)
#define _HTTPMessage_hpp

#include <string>
#include <unordered_map>
#include <memory>
#include <string.h>
#include "http-parser/http_parser.h"
#include "util/Buffer.hpp"

class HTTPRequest
{
public:
  http_method method;
  std::string url;
  // std::string version;
  std::unordered_multimap<std::string, std::string> header;//严格来讲，HTTP没有禁止header的键重复
  std::shared_ptr<Buffer> body;//body可能是长度不定的chunk
  std::string filePath;//chunk数据如果很大，需要存放到磁盘，记住文件路径

  HTTPRequest& set(http_method method){
    this->method = method;
    return *this;
  }

  HTTPRequest& set(std::string const& headerKey, std::string const& headerValue){
    auto& r = *this;
    auto it = r.header.find(headerKey);
    while(it!=r.header.end() && it->second!=headerValue){
      it++;
    }
    if(it==r.header.end()){//确实还没有这个键值对
      r.header.insert({headerKey,headerValue});
    }
    return *this;
  }

  HTTPRequest& set(std::string const& url){
    this->url = url;
    return *this;
  }

  HTTPRequest& append(const void* data, uint32_t length){
    auto& r = *this;
    if(! r.body) r.body = std::make_shared<Buffer>(length);
    auto wp = (*(r.body))[length];
    if(NULL==wp){
      return r;
    }
    memcpy(wp, data, length);
    r.body->push(length);
    
    auto it = r.header.find("Content-Length");
    if(it==r.header.end()) r.header.insert({"Content-Length", std::to_string(r.body->getLength())});
    else it->second = std::to_string(r.body->getLength());

    it = r.header.find("Transfer-Encoding");
    if(it!=r.header.end() && it->second.find("chunked")!=std::string::npos){//原本是分块传输模式
      r.header.erase(it);
    }
    r.filePath.clear();
    return *this;
  }

  HTTPRequest& set_chunked(std::string const& filepath){
    auto& r = *this;
    r.filePath = filepath;
    auto it = r.header.find("Transfer-Encoding");
    if(it==r.header.end() || it->second.find("chunked")==std::string::npos){//原本非分块传输模式
      r.header.insert({"Transfer-Encoding", "chunked"});
    }

    it = r.header.find("Content-Length");
    if(it!=r.header.end()){
      r.header.erase(it);
    }
    return *this;
  }

};

class HTTPResponse
{
public:
  // std::string version;
  http_status status;
  std::unordered_multimap<std::string, std::string> header;//严格来讲，HTTP没有禁止header的键重复
  std::shared_ptr<Buffer> body;//body可能是长度不定的chunk
  std::string filePath;//chunk数据如果很大，需要存放到磁盘，记住文件路径

  HTTPResponse(){
    // 默认设为0长body
    set("Content-Length","0");
  }

  HTTPResponse& set(http_status s){
    auto& r = *this;
    r.status = s;
    return r;
  }

  HTTPResponse& set(std::string const& headerKey, std::string const& headerValue){
    auto& r = *this;
    auto it = r.header.find(headerKey);
    while(it!=r.header.end() && it->second!=headerValue) it++;
    if(it==r.header.end()){//确实新来的
      r.header.insert({headerKey,headerValue});
    }
    return r;
  }

  HTTPResponse& append(const void* data, uint32_t length){
    auto& r = *this;
    r.filePath.clear();
    auto it = r.header.find("Transfer-Encoding");
    if(it!=r.header.end() && it->second.find("chunked")!=std::string::npos){
      r.header.erase(it);
    }

    if(!(r.body)) r.body = std::make_shared<Buffer>(length);
    // uint32_t len; auto wp = r.body->getWritingPtr(length,len);
    auto wp = (*(r.body))[length];
    if(NULL==wp){
      return r;
    }
    memcpy(wp, data, length);
    r.body->push(length);

    it = r.header.find("Content-Length");
    if(it==r.header.end()) r.header.insert({"Content-Length", std::to_string(r.body->getLength())});
    else it->second = std::to_string(r.body->getLength());
    return r;
  }

  HTTPResponse& set_chunked(std::string const& filepath){
    auto& r = *this;
    auto it = r.header.find("Content-Length");
    if(it!=r.header.end())
      r.header.erase(it);
    r.body = nullptr;

    it = r.header.find("Transfer-Encoding");
    if(it==r.header.end())
      r.header.insert({"Transfer-Encoding","chunked"});
    else 
      it->second = "chunked";
    r.filePath = filepath;
    return r;
  }

};

#endif // _HTTPMessage_hpp
