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
    h->responseNow->header.insert({h->headerKeyNow, h->headerValueNow});
    h->headerKeyNow.clear();
    h->headerValueNow.clear();
  }
  return 0;
}

int TinyHTTPClient::onBody(http_parser* parser, const char *at, size_t length){
  auto it = THIS->responseNow->header.find("Transfer-Encoding");
  if(it!=THIS->responseNow->header.end() && it->second.find("chunked")!=std::string::npos){//是分块模式
    if(THIS->bodyFileNow.is_open()==false){//暂未有文件
      auto prefix = "./temp/response_data_";
      THIS->responseNow->filePath = prefix + std::to_string(TTCPS2::currentTimeMillis());
      while(true){//循环直到文件名不重复
        THIS->bodyFileNow.open(THIS->responseNow->filePath, std::ios::in | std::ios::binary);
        if(THIS->bodyFileNow.is_open()){//说明这个同名文件已存在
          THIS->bodyFileNow.close();
          THIS->responseNow->filePath = prefix + std::to_string(TTCPS2::currentTimeMillis());//换个名字
        }else{
          THIS->bodyFileNow.close();
          break;
        }
      }
      THIS->bodyFileNow.open(THIS->responseNow->filePath, std::ios::out | std::ios::binary);
    }
    THIS->bodyFileNow.write(at,length);
  }else{//不是分块模式
    if(!THIS->responseNow->body){//还未创建Buffer
      THIS->responseNow->body = std::make_shared<TTCPS2::Buffer>(length);
    }
    uint32_t al; auto wp = THIS->responseNow->body->getWritingPtr(length,al);
    memcpy(wp,at,length);
    THIS->responseNow->body->push(length);
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
  THIS->bodyFileNow.close();
  THIS->headerKeyNow.clear();
  THIS->headerValueNow.clear();
  THIS->responseNow = NULL;
  return 114514;//直接跳出，结束解析
}

TinyHTTPClient::TinyHTTPClient(const char* serverIP, unsigned short port)
: c(new TinyTCPClient(serverIP,port))
, buf(new char[BUFFER_SIZE])
, dataLen_buf(0)
, responseNow(NULL){
  // 不定长的HTTP消息体需要创建文件来保存，需要保证目录存在
  if(0!=system("mkdir -p ./temp/")) exit(-1);

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

#include <iomanip>
std::string _dec2hex(int64_t i){
  std::stringstream ss;
  ss << std::setiosflags(std::ios_base::fmtflags::_S_uppercase) << std::hex << i;
  std::string ret;
  ss >> ret;
  return ret;
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
  }else if(! r.filePath.empty()){//是chunk data
    std::ifstream f(r.filePath, std::ios::in|std::ios::binary);
    if(f.is_open()){//文件存在，才有内容可发
      char buf[BUFFER_SIZE];
      while(! f.eof()){
        f.read(buf,BUFFER_SIZE);
        auto nRead = f.gcount();
        auto toSend = _dec2hex(nRead) + "\r\n" + std::string(buf,nRead) + "\r\n";
        nRead = 0;//复用
        do{
          auto ret = c->send(toSend.data()+nRead, toSend.length()-nRead);
          if(0>ret){
            return -1;
          }
          nRead += ret;
        }while(nRead<toSend.length());
      }
    }
    // 不管文件存不存在，都要发最后的空块
    auto toSend = "0\r\n\r\n";
    l = 0;//复用
    do{
      auto ret = c->send(toSend+l, 5-l);
      if(0>ret){
        return -1;
      }
      l += ret;
    }while(l<5);
  }
  return 0;
}

int TinyHTTPClient::recv(TTCPS2::HTTPResponse& r){
  if(responseNow){//按照预期，上一个response应当已经整个被读走、responseNow被onMessageComplete()设为空（而onMessageComplete()返回非0使parser立即停止，也是因此需要每次都初始化parser），因为这里会阻塞式recv()直到一个response完整读入，甚至会有多余的数据
    return -1;
  }

  r.header.clear();
  if(r.body) r.body->pop(r.body->getLength());
  r.filePath.clear();
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
