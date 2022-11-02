#include "util/Buffer.hpp"
#include <algorithm>
#include <string.h>
#include <stdlib.h>

namespace TTCPS2
{
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

  void* Buffer::getWritingPtr(unsigned int expectedLength, unsigned int& actualLength){

    if(expectedLength > capacity-getLength()){//容量不够
      if(capacity>=MAX_BUFFER_SIZE){//不允许再扩容了

        // 把数据移动到头部（左移firstData字节），容量剩多少就多少
        memmove(data, data+firstData, firstBlank-firstData);
        firstBlank -= firstData;
        firstData = 0;
        actualLength = capacity-firstBlank;
        return data+firstBlank;

      }else{//还可以扩容

        // 目标容量
        while(capacity < getLength() + (expectedLength<<1)) capacity<<=1; // 限制容量取值为2的若干次幂// capacity = (expectedLength<<1) + getLength();//在现有数据量的基础上留两倍的expectedLength
        if(MAX_BUFFER_SIZE<capacity){//目标容量过大
          capacity = MAX_BUFFER_SIZE;
        }
        data = ::realloc(data,capacity);

        // 移动数据到头部（左移firstData字节）
        memmove(data, data+firstData, firstBlank-firstData);
        firstBlank -= firstData;
        firstData = 0;
        actualLength = (firstBlank+expectedLength <= capacity) ? expectedLength:(capacity-firstBlank);//余量可能达不到期望
        return data+firstBlank;

      }
    }else{//容量还够
      if(firstBlank+expectedLength <= capacity){//右侧还有足够的连续空间
        actualLength = expectedLength;
        return data+firstBlank;
      }else{//容量够，但不连续
      
        // 移动数据到头部（左移firstData字节）
        memmove(data, data+firstData, firstBlank-firstData);
        firstBlank -= firstData;
        firstData = 0;
        actualLength = expectedLength;
        return data+firstBlank;

      }
    }

  }

  int64_t Buffer::push(unsigned int length){
    if(length > capacity-firstBlank){//右侧实际没有那么多空间可被标记
      length = capacity-firstBlank;
    }
    firstBlank += length;
    return length;
  }

  const void* Buffer::getReadingPtr(unsigned int expectedLength, unsigned int& actualLength){
    actualLength = std::min(expectedLength, firstBlank-firstData);
    return data+firstData;
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

} // namespace TTCPS2

std::istream& operator>>(std::istream& i, TTCPS2::Buffer& b){
  uint32_t l; void* wp;
  while(!i.eof()){
    wp = b.getWritingPtr(512,l);
    if(l<1) return i;  
    i.read((char*)wp,l);
    b.push(i.gcount());
  }
  return i;
}

std::ostream& operator<<(std::ostream& o, TTCPS2::Buffer& b){
  uint32_t l; auto rp = b.getReadingPtr(b.getLength(), l);
  o.write((const char*)rp,l);
  b.pop(l);
  return o;
}
