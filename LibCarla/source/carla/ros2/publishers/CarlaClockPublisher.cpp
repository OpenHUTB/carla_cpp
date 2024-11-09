#define _GLIBCXX_USE_CXX11_ABI 0
/// @file CarlaClockPublisher.cpp
/// @brief CarlaClockPublisher 类的实现文件，负责发布CARLA的时钟信息到ROS2系统。
#include "CarlaClockPublisher.h"/// @brief 包含 CarlaClockPublisher 类的声明。

#include <string>/// @brief 包含标准字符串库，用于处理字符串数据。
// CARLA ROS2 类型支持
#include "carla/ros2/types/ClockPubSubTypes.h"/// @brief 包含 CARLA ROS2 时钟消息的类型支持。
#include "carla/ros2/listeners/CarlaListener.h"/// @brief 包含 CARLA ROS2 监听器的基类声明。
// Fast-DDS 相关头文件
#include <fastdds/dds/domain/DomainParticipant.hpp>/// @brief 包含 Fast-DDS 域参与者的类声明，用于创建、删除和管理域中的其他实体。
#include <fastdds/dds/publisher/Publisher.hpp>/// @brief 包含 Fast-DDS 发布者的类声明，用于将数据写入到指定的主题。
#include <fastdds/dds/topic/Topic.hpp>/// @brief 包含 Fast-DDS 主题的类声明，用于定义数据的类型和名称。
#include <fastdds/dds/publisher/DataWriter.hpp>/// @brief 包含 Fast-DDS 数据写入器的类声明，用于将数据写入到主题。
#include <fastdds/dds/topic/TypeSupport.hpp>/// @brief 包含 Fast-DDS 类型支持的类声明，用于为特定类型提供序列化和反序列化功能。

#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>/// @brief 包含 Fast-DDS 域参与者QoS策略的类声明。
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>/// @brief 包含 Fast-DDS 域参与者工厂的类声明，用于创建域参与者。
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>/// @brief 包含 Fast-DDS 发布者QoS策略的类声明。
#include <fastdds/dds/topic/qos/TopicQos.hpp> /// @brief 包含 Fast-DDS 主题QoS策略的类声明。
// Fast-RTPS（实时发布订阅）相关头文件
#include <fastrtps/attributes/ParticipantAttributes.h>/// @brief 包含 Fast-RTPS 参与者属性的类声明，用于配置参与者的网络属性。
#include <fastrtps/qos/QosPolicies.h>/// @brief 包含 Fast-RTPS QoS策略的类声明，用于配置发布者和订阅者的服务质量。
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>/// @brief 包含 Fast-DDS 数据写入器QoS策略的类声明。
#include <fastdds/dds/publisher/DataWriterListener.hpp>/// @brief 包含 Fast-DDS 数据写入器监听器的类声明，用于接收数据写入事件。


namespace carla {
namespace ros2 {
  namespace efd = eprosima::fastdds::dds;
  using erc = eprosima::fastrtps::types::ReturnCode_t;

  struct CarlaClockPublisherImpl {
    efd::DomainParticipant* _participant { nullptr };
    efd::Publisher* _publisher { nullptr };
    efd::Topic* _topic { nullptr };
    efd::DataWriter* _datawriter { nullptr };
    efd::TypeSupport _type { new rosgraph::msg::ClockPubSubType() };
    CarlaListener _listener {};
    rosgraph::msg::Clock _clock {};
  };

  bool CarlaClockPublisher::Init() {
    if (_impl->_type == nullptr) {
        std::cerr << "Invalid TypeSupport" << std::endl;
        return false;
    }
    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    pqos.name(_name);
    auto factory = efd::DomainParticipantFactory::get_instance();
    _impl->_participant = factory->create_participant(0, pqos);
    if (_impl->_participant == nullptr) {
        std::cerr << "Failed to create DomainParticipant" << std::endl;
        return false;
    }
    _impl->_type.register_type(_impl->_participant);

    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
    _impl->_publisher = _impl->_participant->create_publisher(pubqos, nullptr);
    if (_impl->_publisher == nullptr) {
      std::cerr << "Failed to create Publisher" << std::endl;
      return false;
    }

    efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
    const std::string topic_name { "rt/clock" };
    _impl->_topic = _impl->_participant->create_topic(topic_name, _impl->_type->getName(), tqos);
    if (_impl->_topic == nullptr) {
        std::cerr << "Failed to create Topic" << std::endl;
        return false;
    }

    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
    efd::DataWriterListener* listener = (efd::DataWriterListener*)_impl->_listener._impl.get();
    _impl->_datawriter = _impl->_publisher->create_datawriter(_impl->_topic, wqos, listener);
    if (_impl->_datawriter == nullptr) {
        std::cerr << "Failed to create DataWriter" << std::endl;
        return false;
    }
    _frame_id = _name;
    return true;
  }

  bool CarlaClockPublisher::Publish() {
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    eprosima::fastrtps::types::ReturnCode_t rcode = _impl->_datawriter->write(&_impl->_clock, instance_handle);
    if (rcode == erc::ReturnCodeValue::RETCODE_OK) {
        return true;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ERROR) {
        std::cerr << "RETCODE_ERROR" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_UNSUPPORTED) {
        std::cerr << "RETCODE_UNSUPPORTED" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_BAD_PARAMETER) {
        std::cerr << "RETCODE_BAD_PARAMETER" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_PRECONDITION_NOT_MET) {
        std::cerr << "RETCODE_PRECONDITION_NOT_MET" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_OUT_OF_RESOURCES) {
        std::cerr << "RETCODE_OUT_OF_RESOURCES" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ENABLED) {
        std::cerr << "RETCODE_NOT_ENABLED" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_IMMUTABLE_POLICY) {
        std::cerr << "RETCODE_IMMUTABLE_POLICY" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_INCONSISTENT_POLICY) {
        std::cerr << "RETCODE_INCONSISTENT_POLICY" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ALREADY_DELETED) {
        std::cerr << "RETCODE_ALREADY_DELETED" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_TIMEOUT) {
        std::cerr << "RETCODE_TIMEOUT" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NO_DATA) {
        std::cerr << "RETCODE_NO_DATA" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ILLEGAL_OPERATION) {
        std::cerr << "RETCODE_ILLEGAL_OPERATION" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ALLOWED_BY_SECURITY) {
        std::cerr << "RETCODE_NOT_ALLOWED_BY_SECURITY" << std::endl;
        return false;
    }
    std::cerr << "UNKNOWN" << std::endl;
    return false;
  }

  void CarlaClockPublisher::SetData(int32_t sec, uint32_t nanosec) {
    _impl->_clock.clock().sec(sec);
    _impl->_clock.clock().nanosec(nanosec);
  }

  CarlaClockPublisher::CarlaClockPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaClockPublisherImpl>()) {
    _name = ros_name;
    _parent = parent;
  }

  CarlaClockPublisher::~CarlaClockPublisher() {
      if (!_impl)
          return;

      if (_impl->_datawriter)
          _impl->_publisher->delete_datawriter(_impl->_datawriter);

      if (_impl->_publisher)
          _impl->_participant->delete_publisher(_impl->_publisher);

      if (_impl->_topic)
          _impl->_participant->delete_topic(_impl->_topic);

      if (_impl->_participant)
          efd::DomainParticipantFactory::get_instance()->delete_participant(_impl->_participant);
  }

  CarlaClockPublisher::CarlaClockPublisher(const CarlaClockPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
  }

  CarlaClockPublisher& CarlaClockPublisher::operator=(const CarlaClockPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;

    return *this;
  }

  CarlaClockPublisher::CarlaClockPublisher(CarlaClockPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
  }

  CarlaClockPublisher& CarlaClockPublisher::operator=(CarlaClockPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);

    return *this;
  }
}
}
