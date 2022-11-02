#if !defined(_TimerTask_hpp)
#define _TimerTask_hpp

#include <inttypes.h>
#include <functional>
#include <memory>

namespace TTCPS2
{
  /**
   * @brief 
   * UNIX纪元时钟/毫秒
   * @return int64_t 
   */
  int64_t currentTimeMillis();

  /**
   * @brief 定时任务
   * 比较运算符的判定依据：nextTimestamp（下一时间戳）
   */
  class TimerTask
  {  
  public:

    int64_t nextTimestamp;
    uint32_t interval;
    bool repeating;
    std::function<void ()> task;

    bool operator<(TimerTask const& b);
    bool operator>(TimerTask const& b);
    bool operator<=(TimerTask const& b);
    bool operator>=(TimerTask const& b);
    bool operator==(TimerTask const& b);
    bool operator!=(TimerTask const& b);

    static bool notEarlier(TimerTask const& a, TimerTask const& b);
    static bool notLater(TimerTask const& a, TimerTask const& b);

    /**
     * @brief 额外信息
     */
    std::shared_ptr<void> info;

    TimerTask();

    /**
     * @brief Construct a new Timer Task object
     * 
     * @param interval 间隔/毫秒
     * @param task 定时任务
     */
    TimerTask(bool toRepeat, uint32_t interval, std::function<void ()> task);
    
    void operator()();

    ~TimerTask();

  };
  
} // namespace TTCPS2

#endif // _TimerTask_hpp
