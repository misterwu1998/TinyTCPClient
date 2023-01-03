/**
 * @file Time.hpp
 * @brief 稍微封装一下<chrono>
 */

#if !defined(_Time_hpp)
#define _Time_hpp

#include <chrono>
#include <functional>
#include <memory>

/**
 * @brief 
 * UNIX纪元时钟/毫秒
 * @return int64_t 
 */
static int64_t currentTimeMillis()
{
  return std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now().time_since_epoch()
  ).count();
}

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

  /**
   * @brief 额外信息
   */
  std::shared_ptr<void> info;

  TimerTask(){}

  /**
   * @brief Construct a new Timer Task object
   * 
   * @param interval 间隔/毫秒
   * @param task 定时任务
   */
  TimerTask(bool toRepeat, uint32_t interval, std::function<void ()> task)
  :   repeating(toRepeat)
    , interval(interval)
    , nextTimestamp(currentTimeMillis() + interval)
    , task(task) {}
  
  void operator()(){
    task();
  }

  ~TimerTask(){}

};

#endif // _Time_hpp
