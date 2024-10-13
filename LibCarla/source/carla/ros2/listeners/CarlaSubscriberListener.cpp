#define _GLIBCXX_USE_CXX11_ABI 0
// 定义宏，指定使用 C++11 ABI 的版本为 0

#include "CarlaSubscriberListener.h"
#include <iostream>
// 包含头文件，用于输入输出流
#include <fastdds/dds/subscriber/DataReader.hpp>
// 包含 Fast DDS 的数据读取器头文件
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
// 包含 Fast DDS 的数据读取器监听器头文件
#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
// 包含 Fast DDS 的订阅匹配状态头文件
#include <fastdds/dds/subscriber/SampleInfo.hpp>
// 包含 Fast DDS 的样本信息头文件
#include "carla/ros2/types/CarlaEgoVehicleControl.h"
// 包含 Carla 和 ROS2 之间的自定义消息类型头文件，用于自动驾驶车辆控制
#include "carla/ros2/subscribers/CarlaEgoVehicleControlSubscriber.h"
// 包含 Carla 和 ROS2 的订阅者相关头文件
#include "carla/ros2/ROS2CallbackData.h"
// 包含 Carla 和 ROS2 的回调数据头文件

namespace carla {
namespace ros2 {

  namespace efd = eprosima::fastdds::dds;
  // 为 eprosima::fastdds::dds 命名空间定义别名 efd
  using erc = eprosima::fastrtps::types::ReturnCode_t;
  // 定义类型别名 erc，表示 eprosima::fastrtps 的返回码类型

    class CarlaSubscriberListenerImpl : public efd::DataReaderListener {
      public:
      void on_subscription_matched(
              efd::DataReader* reader,
              const efd::SubscriptionMatchedStatus& info);
      // 声明订阅匹配时的回调函数
      void on_data_available(efd::DataReader* reader);
      // 声明数据可用时的回调函数

      int _matched {0};
      // 匹配的数量初始化为 0
      bool _first_connected {false};
      // 表示是否首次连接的标志初始化为 false
      CarlaEgoVehicleControlSubscriber* _owner {nullptr};
      // 指向订阅者的指针初始化为 nullptr
      carla_msgs::msg::CarlaEgoVehicleControl _message {};
      // 用于存储接收到的消息
    };

    void CarlaSubscriberListenerImpl::on_subscription_matched(efd::DataReader* reader, const efd::SubscriptionMatchedStatus& info)
    {
      if (info.current_count_change == 1) {
          _matched = info.total_count;
          _first_connected = true;
      // 如果当前匹配数量变化为 1，更新匹配数量并设置首次连接标志为 true
      } else if (info.current_count_change == -1) {
          _matched = info.total_count;
          if (_matched == 0) {
            _owner->DestroySubscriber();
          // 如果当前匹配数量变化为 -1，更新匹配数量，若匹配数量为 0，则销毁订阅者
          }
      } else {
          std::cerr << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
      // 如果当前匹配数量变化不是 1 或 -1，则输出错误信息
      }
    }

    void CarlaSubscriberListenerImpl::on_data_available(efd::DataReader* reader)
    {
      efd::SampleInfo info;
      // 定义样本信息对象
      eprosima::fastrtps::types::ReturnCode_t rcode = reader->take_next_sample(&_message, &info);
      // 从数据读取器中获取下一个样本，并存储在消息对象和样本信息对象中

      if (rcode == erc::ReturnCodeValue::RETCODE_OK) {
        VehicleControl control;
        control.throttle = _message.throttle();
        control.steer = _message.steer();
        control.brake = _message.brake();
        control.hand_brake = _message.hand_brake();
        control.reverse = _message.reverse();
        control.gear = _message.gear();
        control.manual_gear_shift = _message.manual_gear_shift();
        _owner->ForwardMessage(control);
      // 如果返回码为 RETCODE_OK，表示成功获取样本，将消息中的控制信息提取出来并转发给订阅者的所有者
      }
      if (rcode == erc::ReturnCodeValue::RETCODE_ERROR) {
          std::cerr << "RETCODE_ERROR" << std::endl;
      // 如果返回码为 RETCODE_ERROR，输出错误信息
      }
      if (rcode == erc::ReturnCodeValue::RETCODE_UNSUPPORTED) {
          std::cerr << "RETCODE_UNSUPPORTED" << std::endl;
      // 如果返回码为 RETCODE_UNSUPPORTED，输出错误信息
      }
      if (rcode == erc::ReturnCodeValue::RETCODE_BAD_PARAMETER) {
          std::cerr << "RETCODE_BAD_PARAMETER" << std::endl;
      // 如果返回码为 RETCODE_BAD_PARAMETER，输出错误信息
      }
      if (rcode == erc::ReturnCodeValue::RETCODE_PRECONDITION_NOT_MET) {
          std::cerr << "RETCODE_PRECONDITION_NOT_MET" << std::endl;
      // 如果返回码为 RETCODE_PRECONDITION_NOT_MET，输出错误信息
      }
      if (rcode == erc::ReturnCodeValue::RETCODE_OUT_OF_RESOURCES) {
          std::cerr << "RETCODE_OUT_OF_RESOURCES" << std::endl;
      // 如果返回码为 RETCODE_OUT_OF_RESOURCES，输出错误信息
      }
      if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ENABLED) {
          std::cerr << "RETCODE_NOT_ENABLED" << std::endl;
      // 如果返回码为 RETCODE_NOT_ENABLED，输出错误信息
      }
      if (rcode == erc::ReturnCodeValue::RETCODE_IMMUTABLE_POLICY) {
          std::cerr << "RETCODE_IMMUTABLE_POLICY" << std::endl;
      // 如果返回码为 RETCODE_IMMUTABLE_POLICY，输出错误信息
      }
      if (rcode == erc::ReturnCodeValue::RETCODE_INCONSISTENT_POLICY) {
          std::cerr << "RETCODE_INCONSISTENT_POLICY" << std::endl;
      // 如果返回码为 RETCODE_INCONSISTENT_POLICY，输出错误信息
      }
      if (rcode == erc::ReturnCodeValue::RETCODE_ALREADY_DELETED) {
          std::cerr << "RETCODE_ALREADY_DELETED" << std::endl;
      // 如果返回码为 RETCODE_ALREADY_DELETED，输出错误信息
      }
      if (rcode == erc::ReturnCodeValue::RETCODE_TIMEOUT) {
          std::cerr << "RETCODE_TIMEOUT" << std::endl;
      // 如果返回码为 RETCODE_TIMEOUT，输出错误信息
      }
      if (rcode == erc::ReturnCodeValue::RETCODE_NO_DATA) {
          std::cerr << "RETCODE_NO_DATA" << std::endl;
      // 如果返回码为 RETCODE_NO_DATA，输出错误信息
      }
      if (rcode == erc::ReturnCodeValue::RETCODE_ILLEGAL_OPERATION) {
          std::cerr << "RETCODE_ILLEGAL_OPERATION" << std::endl;
      // 如果返回码为 RETCODE_ILLEGAL_OPERATION，输出错误信息
      }
      if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ALLOWED_BY_SECURITY) {
          std::cerr << "RETCODE_NOT_ALLOWED_BY_SECURITY" << std::endl;
      // 如果返回码为 RETCODE_NOT_ALLOWED_BY_SECURITY，输出错误信息
      }
    }

    void CarlaSubscriberListener::SetOwner(CarlaEgoVehicleControlSubscriber* owner) {
        _impl->_owner = owner;
    // 设置监听器的所有者为给定的订阅者
    }

    CarlaSubscriberListener::CarlaSubscriberListener(CarlaEgoVehicleControlSubscriber* owner) :
    _impl(std::make_unique<CarlaSubscriberListenerImpl>()) {
        _impl->_owner = owner;
    // 构造函数，创建监听器实现对象，并设置所有者为给定的订阅者
    }

    CarlaSubscriberListener::~CarlaSubscriberListener() {}
    // 析构函数

}}
