#define _GLIBCXX_USE_CXX11_ABI 0

#include "CarlaListener.h"
#include <iostream> // 包含标准输入输出库，用于标准输出流操作

#include <fastdds/dds/publisher/DataWriterListener.hpp> // 包含fastdds库中DataWriterListener的头文件
#include <fastdds/dds/core/status/PublicationMatchedStatus.hpp> // 包含fastdds库中PublicationMatchedStatus的头文件，这个类包含了与发布匹配状态相关的信息

namespace carla {
namespace ros2 { // 定义一个嵌套命名空间carla::ros2，用于组织代码

  namespace efd = eprosima::fastdds::dds; // 使用eprosima::fastdds::dds命名空间，并简化为efd

    class CarlaListenerImpl : public efd::DataWriterListener { // 定义CarlaListenerImpl类，继承自efd::DataWriterListener
      public:
      void on_publication_matched( // 重写DataWriterListener中的on_publication_matched方法
              efd::DataWriter* writer, // 数据写入对象的指针
              const efd::PublicationMatchedStatus& info) override; // 包含匹配状态信息的对象


      int _matched {0}; // 成员变量_matched，用于记录当前匹配的订阅者数量
      bool _first_connected {false}; // 成员变量_first_connected，用于标记是否是第一次有订阅者连接
    };

    void CarlaListenerImpl::on_publication_matched(efd::DataWriter* writer, const efd::PublicationMatchedStatus& info) // 实现CarlaListenerImpl类的on_publication_matched方法
    {
      if (info.current_count_change == 1) { // 如果当前计数变化为1，表示有新的订阅者连接
          _matched = info.total_count; // 更新匹配的订阅者数量
          _first_connected = true;  // 标记为第一次连接
      } else if (info.current_count_change == -1) { // 如果当前计数变化为-1，表示有订阅者断开连接
          _matched = info.total_count;  // 更新匹配的订阅者数量
      } else {  // 如果当前计数变化不是1或-1，输出错误信息
          std::cerr << info.current_count_change 
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
      }
    }

    CarlaListener::CarlaListener() : // 定义CarlaListener类，它包含CarlaListenerImpl类的实例
    _impl(std::make_unique<CarlaListenerImpl>()) { } // 构造函数，初始化_impl成员，使用std::make_unique创建CarlaListenerImpl的实例

    CarlaListener::~CarlaListener() {} // CarlaListener类的析构函数，用于在对象销毁时进行清理

}}
