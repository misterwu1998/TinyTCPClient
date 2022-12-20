#if !defined(_TCPConnection_hpp)
#define _TCPConnection_hpp

#include <functional>
#include <memory>
#include <mutex>

struct sockaddr_in;

class NetIOReactor;
class Acceptor;
class TimerTask;
class Buffer;

class TCPConnection
{
private:

  friend class Acceptor;
  friend class NetIOReactor;

  NetIOReactor* netIOReactor;
  int clientSocket;

  // 读缓冲（未经处理的数据的缓冲区）
  std::unique_ptr<Buffer> rb;
  std::mutex m_rb;

  // 标记当前是否有线程池线程在执行handle()
  bool working;
  std::mutex m_working;

  // 写缓冲（经过处理，待发送的数据的缓冲区）
  std::unique_ptr<Buffer> wb;
  std::mutex m_wb;

public:
  TCPConnection(
      NetIOReactor* netIOReactor
    , int clientSocket
  );

protected:
  /// @brief 由NetIOReactor线程调用，尽可能从内核缓冲区读取length字节数据
  /// @param length 
  /// @return 实际读取的数据量 /字节; 或-1表示客户端不再能正常通信
  int readFromSocket(int length);

public:
  /// @brief 线程安全地查看当前有多少数据未处理 /字节
  /// @return -1表示异常
  int getUnprocessedLength();

protected:

  /// @brief 由数据处理线程调用，线程安全地取走length字节数据放到dst
  /// @param length 
  /// @param dst 
  /// @return 实际取走的数据量 /字节; 或-1表示出错
  int takeData(int length, void* dst);

  /// @brief 对于当前是否有线程池线程在执行handle()的布尔flag的CAS“原子”操作，如果当前flag等于v0，就为其赋值v1。
  /// v0,v1 = false,true 时，这次CAS操作用于争抢执行handle()的唯一的名额，返回值为false说明争抢成功；
  /// v0,v1 = true,false 时，这次CAS错做用于归还handle()的执行名额，返回值为true说明归还成功，为false说明程序出错；
  /// v0,v1 的其它取值无意义。
  /// 线程池线程A在handle()中刚刚完成takeData()时，网络IO反应堆恰好又令当前TCPConnection完成readFromSocket()，并给线程池线程B指派了工作任务，这时候handle()内完全可能有多线程在跑。简而言之，handle()本身并非线程安全的。
  /// 然而如果盲目给整个代码块上锁，可能导致线程池“饥饿”，例如上述两个线程A、B，假如线程池就它俩线程，那么加锁的情况下其它TCPConnection的任务就都得往后稍稍了。换句话说，我们只是希望一个TCPConnection在同一时刻只能由一条线程池线程来执行其handle()。
  /// 因此需要立working这个flag，需要CAS操作。
  /// @return flag原来的值
  bool compareAndSwap_working(bool v0, bool v1);

public:

  /// @brief 由数据处理线程调用：takeData()取走数据到指定的内存空间进行处理，处理后bringData()放回。
  /// 缺省实现：echo。
  /// @return -1表示出错
  virtual int handle();
  
  /// @brief 由数据处理线程调用，向服务器添加一项定时任务
  /// @param tt 
  /// @return 1表示成功；-1表示出错
  int addTimerTask(TimerTask const& tt);

  /// @brief 由数据处理线程调用，从服务器撤除满足filter条件的定时任务
  /// @param filter 筛选条件，遇到符合条件的TimerTask作参数时返回true
  /// @return 1表示成功；-1表示出错
  int removeTimerTask(std::function<bool (TimerTask const&)> filter);

protected:
  /// @brief 由数据处理线程调用，线程安全地从src带来length字节数据
  /// @param src 
  /// @param length 
  /// @return 实际接纳的数据量 /字节; 或-1表示出错
  int bringData(const void* src, int length);

public:
  /// @brief 线程安全地查看当前有多少数据未发送
  /// @return -1表示出错
  int getUnsentLength();

protected:
  /// @brief 由NetIOReactor线程调用，尽可能向内核发送缓冲区追加length字节的数据
  /// @param length 
  /// @return 实际追加的数据量 /字节; -1表示客户端不再能正常通信
  int sendToSocket(int length);

public:

  /// @brief 如果当前TCPConnection暂未被丢弃，就返回其共享指针
  /// @return nullptr，表示当前TCPConnection已经被server和NetIOReactor所丢弃
  std::shared_ptr<TCPConnection> getSharedPtr_threadSafe();

  /// @brief 立即提醒网络IO反应堆可以发送数据（实现方式：让网络IO反应堆立即监听可写事件）
  /// @return 0表示成功; -1表示出错
  int remindNetIOReactor();

  /// @brief 获取对方的IPv4地址、端口
  /// @param addr 
  /// @return 0表示成功；-1表示TCP连接已断开；-2表示其它错误
  int getClientAddress(::sockaddr_in& addr) const;

public:
  virtual ~TCPConnection();
};

#endif // _TCPConnection_hpp
