#include "util/TimerTask.hpp"
#include <chrono>

int64_t currentTimeMillis()
{
  return std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now().time_since_epoch()
  ).count();
}

TimerTask::TimerTask(){}

TimerTask::TimerTask(bool toRepeat, uint32_t interval, std::function<void ()> task)
:   repeating(toRepeat)
  , interval(interval)
  , nextTimestamp(currentTimeMillis() + interval)
  , task(task) {}

void TimerTask::operator()(){
  task();
}

bool TimerTask::operator<(TimerTask const& b){
  return nextTimestamp<b.nextTimestamp;
}

bool TimerTask::operator>(TimerTask const& b){
  return nextTimestamp>b.nextTimestamp;
}

bool TimerTask::operator<=(TimerTask const& b){
  return nextTimestamp<=b.nextTimestamp;
}

bool TimerTask::operator>=(TimerTask const& b){
  return nextTimestamp>=b.nextTimestamp;
}

bool TimerTask::operator==(TimerTask const& b){
  return nextTimestamp==b.nextTimestamp;
}

bool TimerTask::operator!=(TimerTask const& b){
  return nextTimestamp!=b.nextTimestamp;
}

TimerTask::~TimerTask(){}

bool TimerTask::notEarlier(TimerTask const& a, TimerTask const& b){
  return a.nextTimestamp>=b.nextTimestamp;
}

bool TimerTask::notLater(TimerTask const& a, TimerTask const& b){
  return a.nextTimestamp<=b.nextTimestamp;
}
