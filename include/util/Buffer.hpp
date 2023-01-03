#if !defined(MAX_BUFFER_SIZE)
#define MAX_BUFFER_SIZE (1<<19) //每个TCPConnection有读、写两个缓冲区，每个缓冲区允许的最大容量 /字节; 不得小于2*max(LENGTH_PER_RECV,LENGTH_PER_SEND)
#endif // MAX_BUFFER_SIZE

#if !defined(_Buffer_hpp)
#define _Buffer_hpp

#include <cstdint>
#include <algorithm>
#include <string.h>
#include <stdlib.h>

/**
 * @brief 
 * (非线程安全)
 */
class Buffer
{
private:

  void* data;
  unsigned int capacity;
  unsigned int firstData;
  unsigned int firstBlank;
  
  Buffer(Buffer const& b){}
  Buffer& operator=(Buffer const& b){}
  Buffer(Buffer&& b){}

public:

  Buffer(): Buffer(512) {}
  Buffer(unsigned int capacity): firstData(0), firstBlank(0) {
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

  unsigned int getLength() const{
    return firstBlank-firstData;
  }

  /// @brief 暴露空闲空间首地址
  /// @param expectedLength 期望多大的连续空间（字节）
  /// @return 空闲空间首地址，或NULL表示expectedLength字节的连续空间无法开辟
  void* operator[](unsigned int expectedLength){
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

  /**
   * @brief 标记新增数据量; 调用getWritingPtr()并拷贝数据之后，标记才是有意义的
   * @param length 
   * @return int64_t 实际被标记的新增数据量; 或-1表示出错
   */
  int64_t push(unsigned int length){
    if(length > capacity-firstBlank){//右侧实际没有那么多空间可被标记
      length = capacity-firstBlank;
    }
    firstBlank += length;
    return length;
    }

  /// @return 缓冲区内未读数据的首地址；或NULL表示当前没有未读的数据
  const void* operator*(){
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
  
  /**
   * @brief 丢弃队头数据
   * @param length 
   * @return int64_t 实际被丢弃的数据量; 或-1表示出错
   */
  int64_t pop(unsigned int length){
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

  virtual ~Buffer(){
    free(data);
  }

};

#include <iostream>

/// @brief 把输入流中的数据全挪过来
/// @param i 
/// @param b 
/// @return i（如果未能全部读取，i中就还有数据）
static std::istream& operator>>(std::istream& i, Buffer& b){
  void* wp;
  while(!i.eof()){
    wp = b[512];
    if(NULL==wp) return i; 
    i.read((char*)wp, 512);
    b.push(i.gcount());
  }
  return i;
}

/// @brief 把数据全部交给输出流
/// @param o 
/// @param b （如果未能全部输出，b中就还有数据）
/// @return o
static std::ostream& operator<<(std::ostream& o, Buffer& b){
  auto rp = *b;
  if(NULL==rp) return o;
  o.write((const char*)rp, b.getLength());
  b.pop(b.getLength());
  return o;
}
  
#endif // _Buffer_hpp
