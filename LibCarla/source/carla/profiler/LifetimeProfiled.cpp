// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#ifndef LIBCARLA_ENABLE_LIFETIME_PROFILER  // 如果没有定义生命周期分析器
#  define LIBCARLA_ENABLE_LIFETIME_PROFILER  // 定义生命周期分析器
#endif // LIBCARLA_ENABLE_LIFETIME_PROFILER  // 结束条件编译

#include "carla/Debug.h"  // 引入调试头文件
#include "carla/Logging.h"  // 引入日志记录头文件
#include "carla/profiler/LifetimeProfiled.h"  // 引入生命周期分析的头文件

#include <mutex>  // 引入互斥锁头文件
#include <unordered_map>  // 引入无序映射头文件

namespace carla {  // 定义 carla 命名空间
namespace profiler {  // 定义 profiler 命名空间

  // 日志记录函数模板，用于输出对象生命周期相关的信息
  template <typename ... Args>
  static inline void log(Args && ... args) {
    logging::write_to_stream(std::cerr, "LIFETIME:", std::forward<Args>(args) ..., '\n');  // 将信息写入标准错误流
  }

  class LifetimeProfiler {  // 生命周期分析器类
  public:

    ~LifetimeProfiler() {  // 析构函数
      std::lock_guard<std::mutex> lock(_mutex);  // 加锁以保护共享资源
      if (!_objects.empty()) {  // 如果有未析构的对象
        log("WARNING! the following objects were not destructed.");  // 输出警告信息
        for (const auto &item : _objects) {  // 遍历未析构的对象
          log(item.second, "still alive.");  // 输出每个对象的名称
        }
        DEBUG_ERROR;  // 记录调试错误
      }
    }

    // 注册对象，记录其生命周期
    void Register(void *object, std::string display_name) {
#if LIBCARLA_LOG_LEVEL <= LIBCARLA_LOG_LEVEL_DEBUG  // 如果日志级别低于等于调试
      log('+', display_name);  // 输出注册对象的信息
#endif
      std::lock_guard<std::mutex> lock(_mutex);  // 加锁以保护共享资源
      _objects.emplace(object, std::move(display_name));  // 将对象及其名称添加到映射中
    }

    // 注销对象，移除其生命周期记录
    void Deregister(void *object) {
      std::lock_guard<std::mutex> lock(_mutex);  // 加锁以保护共享资源
      auto it = _objects.find(object);  // 查找对象
      DEBUG_ASSERT(it != _objects.end());  // 断言对象存在
#if LIBCARLA_LOG_LEVEL <= LIBCARLA_LOG_LEVEL_DEBUG  // 如果日志级别低于等于调试
      log('-', it->second);  // 输出注销对象的信息
#endif
      _objects.erase(it);  // 从映射中移除对象
    }

  private:

    std::mutex _mutex;  // 互斥锁，保护共享资源

    std::unordered_map<void *, std::string> _objects;  // 存储对象指针和其名称的无序映射
  };

  static LifetimeProfiler PROFILER;  // 创建一个静态的生命周期分析器实例

  LifetimeProfiled::LifetimeProfiled(std::string display_name) {  // 生命周期分析类的构造函数
    PROFILER.Register(this, std::move(display_name));  // 注册当前对象
  }

  LifetimeProfiled::~LifetimeProfiled() {  // 生命周期分析类的析构函数
    PROFILER.Deregister(this);  // 注销当前对象
  }

} // namespace profiler
} // namespace carla