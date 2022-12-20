#include "util/Buffer.hpp"
#include <algorithm>
#include <string.h>
#include <stdlib.h>

Buffer::Buffer()
: Buffer(512) {}

Buffer::Buffer(unsigned int capacity)
: firstData(0)
, firstBlank(0) {
  // 找到恰好不小于capacity的2的次幂
  this->capacity = 1;
  while(this->capacity < capacity){
    (this->capacity) <<= 1;
  }
  if(this->capacity > MAX_BUFFER_SIZE){//控制上限
    this->capacity = MAX_BUFFER_SIZE;
  }
  this->data = malloc(this->capacity);
}

unsigned int Buffer::getLength(){
  return firstBlank-firstData;
}

void* Buffer::operator[](unsigned int expectedLength){
  firstBlank %= capacity;
  firstData %= capacity;

  // 已有的数据挪到头部
  if(firstData>0){
    memmove(data, data+firstData, firstBlank-firstData);
    firstBlank -= firstData;
    firstData = 0; 
  }

  if(firstBlank+expectedLength<=capacity){//容量还够用
    return data+firstBlank;
  }//容量不够

  unsigned int c = capacity;
  while(c<=MAX_BUFFER_SIZE && firstBlank+expectedLength>c){//还没爆表，并且容量仍然不够
    c <<= 1;
  }
  if(c>MAX_BUFFER_SIZE){//容量爆表了
    return NULL;
  }

  data = realloc(data,c);
  capacity = c;
  return data+firstBlank;
}

int64_t Buffer::push(unsigned int length){
  if(length > capacity-firstBlank){//右侧实际没有那么多空间可被标记
    length = capacity-firstBlank;
  }
  firstBlank += length;
  return length;
}

const void* Buffer::operator*(){
  firstBlank %= capacity;
  firstData %= capacity;
  if(firstBlank==firstData) return NULL;
  if(firstData>0){
    memmove(data, data+firstData, firstBlank-firstData);
    firstBlank -= firstData;
    firstData = 0;
  }
  return data;
}

int64_t Buffer::pop(unsigned int length){
  if(capacity >= (1<<7) && getLength()*4 <= capacity){//容量不小，但当中有四分之三都是空的
    // 还要留着的数据挪到头部，然后realloc
    uint64_t ret;
    if(getLength()<=length){//这次pop会把全部数据都pop掉
      //不用管数据了
      ret = getLength();
      firstBlank = 0;
      firstData = 0;
    }else{//这次pop后还剩下部分数据
      ret = length;
      ::memmove(data, data+firstData+length, getLength()-length);
      firstBlank = getLength()-length;
      firstData = 0;
    }
    capacity >>= 1;//容量减半
    data = ::realloc(data, capacity);
    return ret;
  }else{
    if(length > firstBlank-firstData){//实际没有那么多数据可以弹出
      length = firstBlank - firstData;
    }
    firstData += length;
    return length;
  }
}

Buffer::~Buffer(){
  free(data);
}

std::istream& operator>>(std::istream& i, Buffer& b){
  void* wp;
  while(!i.eof()){
    wp = b[512];
    if(NULL==wp) return i; 
    i.read((char*)wp, 512);
    b.push(i.gcount());
  }
  return i;
}

std::ostream& operator<<(std::ostream& o, Buffer& b){
  auto rp = *b;
  if(NULL==rp) return o;
  o.write((const char*)rp, b.getLength());
  b.pop(b.getLength());
  return o;
}
