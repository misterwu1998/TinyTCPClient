#include "TinyHTTPClient/TinyHTTPClient.hpp"
#include "TinyTCPClient.hpp"
#include "TinyHTTPServer/HTTPMessage.hpp"
#include <sstream>
#include <string.h>
#include <fstream>
#include "util/Buffer.hpp"
#include "http-parser/http_parser.h"
#include "util/TimerTask.hpp"

#define BUFFER_SIZE 1024
#define THIS ((TinyHTTPClient*)(parser->data))
#define RESPONSE_NOW (((TinyHTTPClient*)(parser->data))->responseNow)

int TinyHTTPClient::onMessageBegin(http_parser* parser){
  return 0;
}

int TinyHTTPClient::onStatus(http_parser* parser, const char *at, size_t length){
  RESPONSE_NOW->status = (http_status)(parser->status_code);
  return 0;
}

int TinyHTTPClient::onHeaderField(http_parser* parser, const char *at, size_t length){
  if(! THIS->headerValueNow.empty()){//上一个header键值对还没处理好
    // http-parser.c 似乎有bug，探测"Content-Length"时把字符全转为小写了，却拿它跟未转小写的"Content-Length"作比较，因此，手动补回Content-Length
    if(THIS->headerKeyNow == "Content-Length" && parser->content_length==((uint64_t) -1)){
      std::stringstream ss;
      ss << THIS->headerValueNow;
      int32_t contentLength;
      ss >> contentLength;
      if(contentLength>=0)      parser->content_length = contentLength;
    }
    
    THIS->responseNow->header.insert({THIS->headerKeyNow, THIS->headerValueNow});
    THIS->headerKeyNow.clear();
    THIS->headerValueNow.clear();
  }
  THIS->headerKeyNow += std::string(at,length);
  return 0;
}

int TinyHTTPClient::onHeaderValue(http_parser* parser, const char *at, size_t length){
  THIS->headerValueNow += std::string(at,length);
  return 0;
}

int TinyHTTPClient::onHeadersComplete(http_parser* parser){
  auto h = THIS;
  if(!h->headerValueNow.empty()){//上一个header键值对还没处理好
    // http-parser.c 似乎有bug，探测"Content-Length"时把字符全转为小写了，却拿它跟未转小写的"Content-Length"作比较，因此，手动补回Content-Length
    if(THIS->headerKeyNow == "Content-Length" && parser->content_length==((uint64_t) -1)){
      std::stringstream ss;
      ss << THIS->headerValueNow;
      int32_t contentLength;
      ss >> contentLength;
      if(contentLength>=0)      parser->content_length = contentLength;
    }
    
    h->responseNow->header.insert({h->headerKeyNow, h->headerValueNow});
    h->headerKeyNow.clear();
    h->headerValueNow.clear();
  }
  return 0;
}

int TinyHTTPClient::onBody(http_parser* parser, const char *at, size_t length){
  /// 填入body, 填不进去了就写出到文件
  auto h = THIS;
  uint32_t actualLen;
  if(!h->responseNow->body){//还未创建Buffer
    h->responseNow->body = std::make_shared<TTCPS2::Buffer>(length);
  }
  auto wp = h->responseNow->body->getWritingPtr(length,actualLen);
  if(!h->bodyFileNow.is_open() && actualLen<length){//暂未有文件，且位置不够了
    auto dir = "./temp/response_data/";
    h->responseNow->filePath = dir + std::to_string(TTCPS2::currentTimeMillis());
    while(true){//确保名称不重复
      h->bodyFileNow.open(h->responseNow->filePath, std::ios::in | std::ios::binary);
      if(h->bodyFileNow.is_open()){//说明这个同名文件已存在
        h->bodyFileNow.close();
        h->responseNow->filePath = dir + std::to_string(TTCPS2::currentTimeMillis());//换个名字
      }else{
        h->bodyFileNow.close();
        break;
      }
    }
    h->bodyFileNow.open(h->responseNow->filePath, std::ios::out | std::ios::binary);

    // 先写原有的内容再写新内容
    auto rp = h->responseNow->body->getReadingPtr(h->responseNow->body->getLength(),actualLen);
    h->bodyFileNow.write((char*)rp, h->responseNow->body->getLength())
                  .write(at,length);
    h->responseNow->body = nullptr; // 舍弃Buffer // h->responseNow->body->pop(h->responseNow->body->getLength());
  }else if(h->bodyFileNow.is_open()){//已经有文件
    h->bodyFileNow.write(at,length);
  }else{//暂未有文件但位置还够
    memcpy(wp,at,length);
    h->responseNow->body->push(length);
  }
  return 0;
}

int TinyHTTPClient::onChunkHeader(http_parser* parser){
  return 0;
}

int TinyHTTPClient::onChunkComplete(http_parser* parser){
  return 0;
}

int TinyHTTPClient::onMessageComplete(http_parser* parser){
  if(THIS->bodyFileNow.is_open()){
    THIS->bodyFileNow.close();
  }
  THIS->responseNow = NULL;
  return 114514;//直接跳出，结束解析
}

TinyHTTPClient::TinyHTTPClient(const char* serverIP, unsigned short port)
: c(new TinyTCPClient(serverIP,port))
, buf(new char[BUFFER_SIZE])
, dataLen_buf(0)
, responseNow(NULL){
  http_parser_init(&parser, http_parser_type::HTTP_RESPONSE);
  parser.data = this;
  http_parser_settings_init(&settings);
  settings.on_message_begin = onMessageBegin;
  settings.on_status = onStatus;
  settings.on_header_field = onHeaderField;
  settings.on_header_value = onHeaderValue;
  settings.on_headers_complete = onHeadersComplete;
  settings.on_body = onBody;
  settings.on_chunk_header = onChunkHeader;
  settings.on_chunk_complete = onChunkComplete;
  settings.on_message_complete = onMessageComplete;
}

int TinyHTTPClient::send(TTCPS2::HTTPRequest const& r){
  std::string reqHead;
 {std::stringstream ss;

  // 请求方法
  auto methodStr = http_method_str(r.method);
  ss.write(methodStr, strlen(methodStr)).write(" ",1)

  // URL
    .write(r.url.c_str(), r.url.length())
    .write(" ",1)

  // 协议版本
    .write("HTTP/1.1", 8)
    .write("\r\n",2);

  // headers
  for(auto& kv : r.header){
    ss.write(kv.first.c_str(), kv.first.length())
      .write(":", 1).write(" ",1)
      .write(kv.second.c_str(), kv.second.length())
      .write("\r\n",2);
  }
  ss.write("\r\n",2);

  // body不需要用到ss，所以先把前面的发了
  reqHead = ss.str();}//delete ss
  int l = 0;
  do{
    auto nSent = c->send(reqHead.data()+l, reqHead.length()-l);
    if(0>nSent){
      return -1;
    }
    l += nSent;
  }while(l<reqHead.length());
  
  if(r.body && r.body->getLength()>0){//是长度已知的消息体
    uint32_t len; auto rp = r.body->getReadingPtr(r.body->getLength(), len);
    l = 0;
    do{
      auto nSent = c->send((char*)rp, len-l);
      if(0>nSent){
        return -1;
      }
      l += nSent;
      rp = (char*)rp + nSent;
    }while(l<len);
  // 暂不考虑chunk_data模式的Request
  // }else if(! r.filePath.empty()){//是chunk data
  //   std::ifstream f(r.filePath, std::ios::in|std::ios::binary);
  //   if(! f.is_open()){
  //     return -1;
  //   }
  //   char buf[BUFFER_SIZE];
  //   while(! f.eof()){
  //     f.read(buf,BUFFER_SIZE);
  //     auto nRead = f.gcount();
  //     l = 0;
  //     do{
  //       auto nSent = c->send(buf, nRead-l);
  //       if(nSent<0){
  //         return -1;
  //       }
  //       l += nSent;
  //     }while(l<nRead);
  //   }
  // 暂不考虑chunk_data模式的Request
  }
  return 0;
}

int TinyHTTPClient::recv(TTCPS2::HTTPResponse& r){
  if(responseNow){//按照预期，上一个response应当已经整个被读走、responseNow被onMessageComplete()设为空（而onMessageComplete()返回非0使parser立即停止，也是因此需要每次都初始化parser），因为这里会阻塞式recv()直到一个response完整读入，甚至会有多余的数据
    return -1;
  }
  responseNow = &r;//新的response开始读取
  http_parser_init(&parser, http_parser_type::HTTP_RESPONSE);

  while(responseNow){//response完整读取的标志：onMessageComplete()将responseNow设为NULL
    if(dataLen_buf>0){//目前有数据
      // 先不recv
    }else{//目前没数据
      auto ret = c->recv(buf, BUFFER_SIZE);
      if(0>ret){
        return -1;
      }
      dataLen_buf = ret;
    }//此时有dataLen_buf字节的数据

    // 消费这些数据；可能消费不完
    auto nParsed = http_parser_execute(&parser, &settings, buf, dataLen_buf);//onMessageComplete()会返回非0，立即跳出
    if(nParsed==dataLen_buf){//消费完
      dataLen_buf = 0;
    }else{//没消费完
      memmove(buf, buf+nParsed, dataLen_buf-nParsed);//挪到前面去
      dataLen_buf -= nParsed;
    }
  }
  return 0;
}

TinyHTTPClient::~TinyHTTPClient(){
  delete[] buf;
  delete c;
}

TTCPS2::HTTPRequest& operator<<(TTCPS2::HTTPRequest& r, http_method method){
  r.method = method;
  return r;
}

TTCPS2::HTTPRequest& operator<<(TTCPS2::HTTPRequest& r, std::pair<std::string, std::string> const& headerKV){
  auto it = r.header.find(headerKV.first);
  while(it!=r.header.end() && it->second!=headerKV.second){
    it++;
  }
  if(it==r.header.end()){//确实还没有这个键值对
    r.header.insert(headerKV);
  }
  return r;
}

TTCPS2::HTTPRequest& operator<<(TTCPS2::HTTPRequest& r, std::string const& url){
  r.url = url;
  return r;
}

TTCPS2::HTTPRequest& operator<<(TTCPS2::HTTPRequest& r, std::pair<const void*, uint32_t> const& bodyData_and_length){
  if(! r.body) r.body = std::make_shared<TTCPS2::Buffer>(bodyData_and_length.second);
  uint32_t len; auto wp = r.body->getWritingPtr(bodyData_and_length.second, len);
  if(1>len) return r;
  memcpy(wp, bodyData_and_length.first, len);
  r.body->push(len);
  return r;
}

// TTCPS2::HTTPRequest& operator<<=(TTCPS2::HTTPRequest& r, std::string const& filepath){
//   auto it = r.header.find("Content-Length");
//   if(it!=r.header.end()){
//     r.header.erase(it);
//   }
//   r.filePath = filepath;
//   return r;
// }
// 暂不考虑
