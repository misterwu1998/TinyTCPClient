#if !defined(MAX_BUFFER_SIZE)
#define MAX_BUFFER_SIZE (1<<19) //每个TCPConnection有读、写两个缓冲区，每个缓冲区允许的最大容量 /字节; 不得小于2*max(LENGTH_PER_RECV,LENGTH_PER_SEND)
#endif // MAX_BUFFER_SIZE

#if !defined(_Buffer_hpp)
#define _Buffer_hpp

#include <cstdint>

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

  Buffer();
  Buffer(unsigned int capacity);

  unsigned int getLength();

  /// @brief 暴露空闲空间首地址
  /// @param expectedLength 期望多大的连续空间（字节）
  /// @return 空闲空间首地址，或NULL表示expectedLength字节的连续空间无法开辟
  void* operator[](unsigned int expectedLength);

  /**
   * @brief 标记新增数据量; 调用getWritingPtr()并拷贝数据之后，标记才是有意义的
   * @param length 
   * @return int64_t 实际被标记的新增数据量; 或-1表示出错
   */
  int64_t push(unsigned int length);

  /// @return 缓冲区内未读数据的首地址；或NULL表示当前没有未读的数据
  const void* operator*();
  
  /**
   * @brief 丢弃队头数据
   * @param length 
   * @return int64_t 实际被丢弃的数据量; 或-1表示出错
   */
  int64_t pop(unsigned int length);

  virtual ~Buffer();

};

#include <iostream>

/// @brief 把输入流中的数据全挪过来
/// @param i 
/// @param b 
/// @return i（如果未能全部读取，i中就还有数据）
std::istream& operator>>(std::istream& i, Buffer& b);

/// @brief 把数据全部交给输出流
/// @param o 
/// @param b （如果未能全部输出，b中就还有数据）
/// @return o
std::ostream& operator<<(std::ostream& o, Buffer& b);
  
#endif // _Buffer_hpp
