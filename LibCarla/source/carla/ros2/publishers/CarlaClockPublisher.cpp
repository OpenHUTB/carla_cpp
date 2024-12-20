#define _GLIBCXX_USE_CXX11_ABI 0
//定义宏，设置使用的C++11 ABI版本为0
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

/**
 * @namespace carla::ros2
 * @brief 此命名空间包含了CARLA与ROS 2集成的相关功能。
 */
namespace carla {
namespace ros2 {
    /**
     * @namespace efd
     * @brief 别名，代表eprosima::fastdds::dds命名空间，用于简化Fast DDS相关类型的引用。
     */
  namespace efd = eprosima::fastdds::dds;
  /**
     * @using erc
     * @brief 别名，代表eprosima::fastrtps::types::ReturnCode_t类型，用于简化返回码类型的引用。
     */
  using erc = eprosima::fastrtps::types::ReturnCode_t;
  /**
     * @struct CarlaClockPublisherImpl
     * @brief CarlaClockPublisher的内部实现结构体，用于封装与Fast DDS相关的资源。
     */
  struct CarlaClockPublisherImpl {
      /**
         * @brief 指向DomainParticipant的指针，用于管理DDS域中的参与者。
         */
    efd::DomainParticipant* _participant { nullptr };
    /**
         * @brief 指向Publisher的指针，用于发布数据。
         */
    efd::Publisher* _publisher { nullptr };
    /**
         * @brief 指向Topic的指针，用于定义数据的主题。
         */
    efd::Topic* _topic { nullptr };
    /**
         * @brief 指向DataWriter的指针，用于写入数据到指定的Topic。
         */
    efd::DataWriter* _datawriter { nullptr };
    /**
         * @brief TypeSupport对象，用于注册和识别消息类型。
         */
    efd::TypeSupport _type { new rosgraph::msg::ClockPubSubType() };
    /**
         * @brief CarlaListener对象，用于监听DDS事件。
         */
    CarlaListener _listener {};
    /**
         * @brief 存储Clock消息的变量。
         */
    rosgraph::msg::Clock _clock {};
  };
  /**
     * @brief 初始化CarlaClockPublisher对象。
     *
     * 此函数负责创建并配置DDS中的DomainParticipant、Publisher、Topic和DataWriter。
     *
     * @return 如果初始化成功，则返回true；否则返回false。
     */
  bool CarlaClockPublisher::Init() {
      // 检查TypeSupport是否有效
    if (_impl->_type == nullptr) {
        std::cerr << "Invalid TypeSupport" << std::endl;
        return false;
    }
    // 设置DomainParticipant的QoS策略为默认值，并设置其名称为_name，然后创建DomainParticipant
    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    pqos.name(_name);
    auto factory = efd::DomainParticipantFactory::get_instance();
    _impl->_participant = factory->create_participant(0, pqos);
    if (_impl->_participant == nullptr) {
        std::cerr << "Failed to create DomainParticipant" << std::endl;
        return false;
    }
// 如果创建DomainParticipant失败，则输出错误信息并返回false
    // 注册消息类型到DomainParticipant
    _impl->_type.register_type(_impl->_participant);
    // 设置Publisher的QoS策略并创建Publisher
    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
    _impl->_publisher = _impl->_participant->create_publisher(pubqos, nullptr);
    if (_impl->_publisher == nullptr) {
      std::cerr << "Failed to create Publisher" << std::endl;
      return false;
 // 如果创建Publisher失败，则输出错误信息并返回false
    }
   // 设置Topic的QoS策略为默认值，定义主题名称为"rt/clock"，然后创建Topic
    efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
    const std::string topic_name { "rt/clock" };
    _impl->_topic = _impl->_participant->create_topic(topic_name, _impl->_type->getName(), tqos);
    if (_impl->_topic == nullptr) {
        std::cerr << "Failed to create Topic" << std::endl;
        return false;
// 如果创建Topic失败，则输出错误信息并返回false
    }
    // 设置DataWriter的QoS策略为默认值，获取对应的监听器，然后创建DataWriter
    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
    efd::DataWriterListener* listener = (efd::DataWriterListener*)_impl->_listener._impl.get();
    _impl->_datawriter = _impl->_publisher->create_datawriter(_impl->_topic, wqos, listener);
    if (_impl->_datawriter == nullptr) {
        std::cerr << "Failed to create DataWriter" << std::endl;
        return false;
// 如果创建DataWriter失败，则输出错误信息并返回false
    }
    // 设置帧ID
    _frame_id = _name;
    return true;
  }
  /**
 * @brief 发布时钟数据的方法
 *
 * 该方法尝试通过Fast-RTPS发布时钟数据。
 * 如果发布成功，则返回true；否则，根据返回码输出相应的错误信息，并返回false。
 *
 * @return bool 发布成功返回true，否则返回false。
 */
  bool CarlaClockPublisher::Publish() {
// 创建一个实例句柄，用于标识数据实例（在Fast-RTPS相关操作中使用）
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
// 通过数据写入器尝试写入时钟数据，获取返回码
    eprosima::fastrtps::types::ReturnCode_t rcode = _impl->_datawriter->write(&_impl->_clock, instance_handle);
     // 根据返回码判断是否发布成功，如果返回码为RETCODE_OK表示发布成功，直接返回true
    if (rcode == erc::ReturnCodeValue::RETCODE_OK) {
        return true;
    }
// 如果返回码为RETCODE_ERROR，表示出现错误，输出相应错误信息并返回false
    if (rcode == erc::ReturnCodeValue::RETCODE_ERROR) {
        std::cerr << "RETCODE_ERROR" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_UNSUPPORTED) {
        std::cerr << "RETCODE_UNSUPPORTED" << std::endl;
        return false;
 // 如果返回码为RETCODE_UNSUPPORTED，表示操作不被支持，输出相应错误信息并返回false
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_BAD_PARAMETER) {
        std::cerr << "RETCODE_BAD_PARAMETER" << std::endl;
        return false;
// 如果返回码为RETCODE_BAD_PARAMETER，表示参数错误，输出相应错误信息并返回false
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_PRECONDITION_NOT_MET) {
        std::cerr << "RETCODE_PRECONDITION_NOT_MET" << std::endl;
        return false;
// 如果返回码为RETCODE_PRECONDITION_NOT_MET，表示前置条件未满足，输出相应错误信息并返回false
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_OUT_OF_RESOURCES) {
        std::cerr << "RETCODE_OUT_OF_RESOURCES" << std::endl;
        return false;
// 如果返回码为RETCODE_OUT_OF_RESOURCES，表示资源不足，输出相应错误信息并返回false
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ENABLED) {
        std::cerr << "RETCODE_NOT_ENABLED" << std::endl;
        return false;
// 如果返回码为RETCODE_NOT_ENABLED，表示未启用相关功能，输出相应错误信息并返回false
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_IMMUTABLE_POLICY) {
        std::cerr << "RETCODE_IMMUTABLE_POLICY" << std::endl;
        return false;
 // 如果返回码为RETCODE_IMMUTABLE_POLICY，表示策略不可变，输出相应错误信息并返回false
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_INCONSISTENT_POLICY) {
        std::cerr << "RETCODE_INCONSISTENT_POLICY" << std::endl;
        return false;
// 如果返回码为RETCODE_INCONSISTENT_POLICY，表示策略不一致，输出相应错误信息并返回false
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ALREADY_DELETED) {
        std::cerr << "RETCODE_ALREADY_DELETED" << std::endl;
        return false;
// 如果返回码为RETCODE_ALREADY_DELETED，表示已经被删除，输出相应错误信息并返回false
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_TIMEOUT) {
        std::cerr << "RETCODE_TIMEOUT" << std::endl;
        return false;
 // 如果返回码为RETCODE_TIMEOUT，表示操作超时，输出相应错误信息并返回false
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NO_DATA) {
        std::cerr << "RETCODE_NO_DATA" << std::endl;
        return false;
// 如果返回码为RETCODE_NO_DATA，表示没有数据，输出相应错误信息并返回false
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ILLEGAL_OPERATION) {
        std::cerr << "RETCODE_ILLEGAL_OPERATION" << std::endl;
        return false;
// 如果返回码为RETCODE_ILLEGAL_OPERATION，表示非法操作，输出相应错误信息并返回false
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ALLOWED_BY_SECURITY) {
        std::cerr << "RETCODE_NOT_ALLOWED_BY_SECURITY" << std::endl;
        return false;
 // 如果返回码为RETCODE_NOT_ALLOWED_BY_SECURITY，表示安全策略不允许，输出相应错误信息并返回false
    }
    std::cerr << "UNKNOWN" << std::endl;
    return false;
  }
  /**
 * @brief 设置时钟数据的方法
 *
 * 该方法用于设置时钟数据的秒和纳秒部分。
 *
 * @param sec 秒部分
 * @param nanosec 纳秒部分
 */
  void CarlaClockPublisher::SetData(int32_t sec, uint32_t nanosec) {
    _impl->_clock.clock().sec(sec);
    _impl->_clock.clock().nanosec(nanosec);
  }
  /**
 * @brief CarlaClockPublisher的构造函数
 *
 * 初始化CarlaClockPublisher对象，包括设置名称、父对象，并创建实现类的实例。
 *
 * @param ros_name ROS节点名称
 * @param parent 父对象名称
 */
  CarlaClockPublisher::CarlaClockPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaClockPublisherImpl>()) {
    _name = ros_name;
    _parent = parent;
  }
  /**
 * @brief CarlaClockPublisher的析构函数
 *
 * 释放CarlaClockPublisher对象占用的资源，包括删除数据写入器、发布者、主题和参与者。
 */
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
  /**
 * @brief CarlaClockPublisher的拷贝构造函数
 *
 * 创建一个新的CarlaClockPublisher对象，作为现有对象的副本。
 *
 * @param other 现有对象
 */
  CarlaClockPublisher::CarlaClockPublisher(const CarlaClockPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
  }
  /**
 * @brief 赋值运算符重载
 *
 * 将现有对象的值赋给当前对象。
 *
 * @param other 现有对象
 * @return CarlaClockPublisher& 当前对象的引用
 */
  CarlaClockPublisher& CarlaClockPublisher::operator=(const CarlaClockPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;

    return *this;
  }
  /**
 * @brief CarlaClockPublisher的移动构造函数
 *
 * 创建一个新的CarlaClockPublisher对象，通过移动现有对象的资源来初始化。
 *
 * @param other 现有对象
 */
  CarlaClockPublisher::CarlaClockPublisher(CarlaClockPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
  }
  /**
 * @brief 移动赋值运算符重载
 *
 * 通过移动现有对象的资源来更新当前对象。
 *
 * @param other 现有对象
 * @return CarlaClockPublisher& 当前对象的引用
 */
  CarlaClockPublisher& CarlaClockPublisher::operator=(CarlaClockPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);

    return *this;
  }
}
}
