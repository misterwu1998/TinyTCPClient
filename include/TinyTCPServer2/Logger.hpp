#if !defined(_Logger_hpp)
#define _Logger_hpp

#include <memory>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"

namespace TTCPS2
{
  class Logger
  {
  private:

      std::shared_ptr<spdlog::logger> spdLogger;

      Logger(std::shared_ptr<spdlog::logger> spdLogger){
        if(spdLogger)        this->spdLogger = spdLogger;
        else{
          this->spdLogger = spdlog::rotating_logger_mt(
              "default logname"
            , "./temp/log/log"
            , 1024*1024*4, 4);
          this->spdLogger->set_pattern("[%Y%m%d %H:%M:%S.%e %z][%l][thread %t] %v");
          this->spdLogger->set_level(spdlog::level::level_enum::info);
          this->spdLogger->flush_on(spdlog::level::level_enum::info);
        }
      }
      Logger(Logger const& another){}
      Logger& operator=(Logger const& another){}
      ~Logger(){}

  public:

      /// @brief Logger是全局单例，初始化或获取此单例
      /// @param spdLogger 指定logger，仅首次调用当前函数时有作用，此后缺省或提供nullptr即可；缺省：spdlog::rotating_logger_mt<spdlog::async_factory>("TinyTCPServer2.logger","./.log/",4*1024*1024,4); 其它logger参见spdlog的文档
      /// @return 直接返回spdLogger
      static std::shared_ptr<spdlog::logger> initOrGet(std::shared_ptr<spdlog::logger> spdLogger = nullptr){
          static Logger instance(spdLogger);
          return instance.spdLogger;
      }

  };

} // namespace TTCPS2

#define TTCPS2_SPD_LOGGER (*(TTCPS2::Logger::initOrGet())) 
#define TTCPS2_LOGGER TTCPS2_SPD_LOGGER 

#endif // _Logger_hpp
