#define _GLIBCXX_USE_CXX11_ABI 0

#include "CarlaEgoVehicleControlSubscriber.h"// 引入Carla自我车辆控制订阅者的头文件

#include "carla/ros2/types/CarlaEgoVehicleControl.h"// 引入Carla自我车辆控制信息的消息类型
#include "carla/ros2/types/CarlaEgoVehicleControlPubSubTypes.h"// 引入Carla自我车辆控制信息的PubSub类型
#include "carla/ros2/listeners/CarlaSubscriberListener.h"// 引入Carla订阅者监听器
// 引入Fast-DDS相关的头文件
#include <fastdds/dds/domain/DomainParticipant.hpp>// 引入域参与者类
#include <fastdds/dds/subscriber/Subscriber.hpp>// 引入订阅者类
#include <fastdds/dds/topic/Topic.hpp>// 引入主题类
#include <fastdds/dds/subscriber/DataReader.hpp> // 引入数据读取器类
#include <fastdds/dds/topic/TypeSupport.hpp>// 引入类型支持类
#include <fastdds/dds/subscriber/SampleInfo.hpp>// 引入样本信息类
// 引入Fast-DDS QOS相关的头文件
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>// 引入域参与者QOS类
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>// 引入域参与者工厂类
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>// 引入订阅者QOS类
#include <fastdds/dds/topic/qos/TopicQos.hpp>// 引入主题QOS类
// 引入Fast-RTPS属性与QOS策略相关的头文件
#include <fastrtps/attributes/ParticipantAttributes.h>// 引入参与者属性类
#include <fastrtps/qos/QosPolicies.h>// 引入QOS策略类
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>// 引入数据读取器QOS类
#include <fastdds/dds/subscriber/DataReaderListener.hpp>// 引入数据读取器监听器类

/**
 * @namespace carla::ros2
 * @brief 定义了CARLA与ROS 2集成的相关代码。
 */
namespace carla {
namespace ros2 {
    /**
 * @brief 引入并定义了一些别名，以便简化代码中的类型引用。
 *
 * - `efd` 是 `eprosima::fastdds::dds` 的别名，用于DDS（Data Distribution Service）通信。
 * - `erc` 是 `eprosima::fastrtps::types::ReturnCode_t` 的别名，用于表示Fast RTPS的返回码类型。
 */
  namespace efd = eprosima::fastdds::dds;
  using erc = eprosima::fastrtps::types::ReturnCode_t;
  /**
 * @struct CarlaEgoVehicleControlSubscriberImpl
 * @brief CarlaEgoVehicleControlSubscriber的内部实现结构体。
 *
 * 包含了DDS订阅者所需的所有成员变量，用于接收和处理CARLA的EgoVehicleControl消息。
 */
  struct CarlaEgoVehicleControlSubscriberImpl {
      /**
     * @brief DDS域参与者指针。
     */
    efd::DomainParticipant* _participant { nullptr };
    /**
    * @brief DDS订阅者指针。
    */
    efd::Subscriber* _subscriber { nullptr };
    /**
     * @brief DDS主题指针。
     */
    efd::Topic* _topic { nullptr };
    /**
     * @brief DDS数据读取器指针。
     */
    efd::DataReader* _datareader { nullptr };
    /**
     * @brief DDS类型支持，用于注册消息类型。
     */
    efd::TypeSupport _type { new carla_msgs::msg::CarlaEgoVehicleControlPubSubType() };
    /**
     * @brief 订阅者监听器，用于处理接收到的消息。
     */
    CarlaSubscriberListener _listener {nullptr};
    /**
     * @brief 存储接收到的CARLA EgoVehicleControl消息。
     */
    carla_msgs::msg::CarlaEgoVehicleControl _event {};
    /**
     * @brief 转换为内部使用的VehicleControl结构体。
     */
    VehicleControl _control {};
    /**
     * @brief 标记是否有新的消息到达。
     */
    bool _new_message {false};
    /**
     * @brief 标记订阅者是否仍然活跃。
     */
    bool _alive {true};
    /**
     * @brief 与车辆控制相关的指针（具体类型未在代码中定义）。
     */
    void* _vehicle {nullptr};
  };
  /**
 * @brief 初始化CARLA EgoVehicleControl消息的订阅者。
 *
 * @return 初始化成功返回true，否则返回false。
 */
  bool CarlaEgoVehicleControlSubscriber::Init() {
      /**
     * @brief 检查类型支持是否有效。
     */
    if (_impl->_type == nullptr) {
        std::cerr << "Invalid TypeSupport" << std::endl;
        return false;
    }
    /**
     * @brief 设置域参与者的QoS（Quality of Service）并创建域参与者。
     */
    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    pqos.name(_name);
    auto factory = efd::DomainParticipantFactory::get_instance();
    _impl->_participant = factory->create_participant(0, pqos);
    if (_impl->_participant == nullptr) {
        std::cerr << "Failed to create DomainParticipant" << std::endl;
        return false;
    }
    /**
     * @brief 注册消息类型。
     */
    _impl->_type.register_type(_impl->_participant);
    /**
     * @brief 设置订阅者的QoS并创建订阅者。
     */
    efd::SubscriberQos subqos = efd::SUBSCRIBER_QOS_DEFAULT;
    _impl->_subscriber = _impl->_participant->create_subscriber(subqos, nullptr);
    if (_impl->_subscriber == nullptr) {
      std::cerr << "Failed to create Subscriber" << std::endl;
      return false;
    }
    /**
     * @brief 设置主题的QoS并创建主题。
     */
    efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
    const std::string base { "rt/carla/" };
    const std::string publisher_type {"/vehicle_control_cmd"};
    std::string topic_name = base;
    if (!_parent.empty())
      topic_name += _parent + "/";
    topic_name += _name;
    topic_name += publisher_type;
    _impl->_topic = _impl->_participant->create_topic(topic_name, _impl->_type->getName(), tqos);
    if (_impl->_topic == nullptr) {
        std::cerr << "Failed to create Topic" << std::endl;
        return false;
    }
    /**
     * @brief 设置数据读取器的QoS并创建数据读取器。
     */
    efd::DataReaderQos rqos = efd::DATAREADER_QOS_DEFAULT;
    efd::DataReaderListener* listener = (efd::DataReaderListener*)_impl->_listener._impl.get();
    _impl->_datareader = _impl->_subscriber->create_datareader(_impl->_topic, rqos, listener);
    if (_impl->_datareader == nullptr) {
        std::cerr << "Failed to create DataReader" << std::endl;
        return false;
    }
    return true;
  }
  /**
 * @brief 读取并处理来自数据读取器的样本
 *
 * 该函数尝试从内部数据读取器中获取下一个样本，并根据返回的代码执行相应的操作。
 * 如果成功获取样本，则返回true；否则，根据错误代码打印错误信息并返回false。
 *
 * @return bool 如果成功获取样本，则返回true；否则返回false。
 */
  bool CarlaEgoVehicleControlSubscriber::Read() {
    efd::SampleInfo info;// 用于存储样本信息的结构体
    // 尝试从数据读取器中获取下一个样本
    eprosima::fastrtps::types::ReturnCode_t rcode = _impl->_datareader->take_next_sample(&_impl->_event, &info);
    // 根据返回的代码执行相应的操作
    if (rcode == erc::ReturnCodeValue::RETCODE_OK) {
        /// @todo 成功获取样本，返回true
        return true;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ERROR) {
        /// @todo 打印错误代码"RETCODE_ERROR"并返回false
        std::cerr << "RETCODE_ERROR" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_UNSUPPORTED) {
        /// @todo 打印错误代码"RETCODE_UNSUPPORTED"并返回false
        std::cerr << "RETCODE_UNSUPPORTED" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_BAD_PARAMETER) {
        /// @todo 打印错误代码"RETCODE_BAD_PARAMETER"并返回false
        std::cerr << "RETCODE_BAD_PARAMETER" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_PRECONDITION_NOT_MET) {
        /// @todo 打印错误代码"RETCODE_PRECONDITION_NOT_MET"并返回false
        std::cerr << "RETCODE_PRECONDITION_NOT_MET" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_OUT_OF_RESOURCES) {
        /// @todo 打印错误代码"RETCODE_OUT_OF_RESOURCES"并返回false
        std::cerr << "RETCODE_OUT_OF_RESOURCES" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ENABLED) {
        /// @todo 打印错误代码"RETCODE_NOT_ENABLED"并返回false
        std::cerr << "RETCODE_NOT_ENABLED" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_IMMUTABLE_POLICY) {
        /// @todo 打印错误代码"RETCODE_IMMUTABLE_POLICY"并返回false
        std::cerr << "RETCODE_IMMUTABLE_POLICY" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_INCONSISTENT_POLICY) {
        /// @todo 打印错误代码"RETCODE_INCONSISTENT_POLICY"并返回false
        std::cerr << "RETCODE_INCONSISTENT_POLICY" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ALREADY_DELETED) {
        /// @todo 打印错误代码"RETCODE_ALREADY_DELETED"并返回false
        std::cerr << "RETCODE_ALREADY_DELETED" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_TIMEOUT) {
        /// @todo 打印错误代码"RETCODE_TIMEOUT"并返回false
        std::cerr << "RETCODE_TIMEOUT" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NO_DATA) {
        /// @todo 打印错误代码"RETCODE_NO_DATA"并返回false
        std::cerr << "RETCODE_NO_DATA" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ILLEGAL_OPERATION) {
        /// @todo 打印错误代码"RETCODE_ILLEGAL_OPERATION"并返回false
        std::cerr << "RETCODE_ILLEGAL_OPERATION" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ALLOWED_BY_SECURITY) {
        /// @todo 打印错误代码"RETCODE_NOT_ALLOWED_BY_SECURITY"并返回false
        std::cerr << "RETCODE_NOT_ALLOWED_BY_SECURITY" << std::endl;
        return false;
    }
    /// @todo 打印未知错误代码"UNKNOWN"并返回false
    std::cerr << "UNKNOWN" << std::endl;
    return false;
  }
  /**
 * @brief 将车辆控制信息转发给内部实现。
 *
 * 该函数将传入的车辆控制信息（VehicleControl）保存到内部实现对象的对应成员变量中，
 * 并标记有新的消息到达。
 *
 * @param control 车辆控制信息。
 */
  void CarlaEgoVehicleControlSubscriber::ForwardMessage(VehicleControl control) {
    _impl->_control = control;
    _impl->_new_message = true;
  }
  /**
 * @brief 销毁订阅者。
 *
 * 该函数将内部实现对象的存活标志设置为false，用于通知其他部分该订阅者已经不再活跃。
 */
  void CarlaEgoVehicleControlSubscriber::DestroySubscriber() {
    _impl->_alive = false;
  }
  /**
 * @brief 获取最新的车辆控制信息。
 *
 * 该函数返回内部实现对象中保存的车辆控制信息，并将新的消息标志设置为false，
 * 表示该消息已被读取。
 *
 * @return 车辆控制信息。
 */
  VehicleControl CarlaEgoVehicleControlSubscriber::GetMessage() {
    _impl->_new_message = false;
    return _impl->_control;
  }
  /**
 * @brief 检查订阅者是否存活。
 *
 * 该函数返回内部实现对象的存活标志，用于判断订阅者是否仍然活跃。
 *
 * @return 如果订阅者存活，则返回true；否则返回false。
 */
  bool CarlaEgoVehicleControlSubscriber::IsAlive() {
    return _impl->_alive;
  }
  /**
 * @brief 检查是否有新的消息到达。
 *
 * 该函数返回内部实现对象的新消息标志，用于判断是否有新的车辆控制信息到达。
 *
 * @return 如果有新的消息到达，则返回true；否则返回false。
 */
  bool CarlaEgoVehicleControlSubscriber::HasNewMessage() {
    return _impl->_new_message;
  }
  /**
 * @brief 获取关联的车辆对象。
 *
 * 该函数返回内部实现对象中保存的车辆对象指针。
 *
 * @return 车辆对象指针。
 */
  void* CarlaEgoVehicleControlSubscriber::GetVehicle() {
    return _impl->_vehicle;
  }
  /**
 * @brief CarlaEgoVehicleControlSubscriber类的构造函数。
 *
 * 构造函数初始化内部实现对象，并设置相关属性，包括车辆对象指针、ROS节点名称和父节点名称。
 *
 * @param vehicle 车辆对象指针。
 * @param ros_name ROS节点名称。
 * @param parent 父节点名称。
 */
  CarlaEgoVehicleControlSubscriber::CarlaEgoVehicleControlSubscriber(void* vehicle, const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaEgoVehicleControlSubscriberImpl>()) {
    _impl->_listener.SetOwner(this);
    _impl->_vehicle = vehicle;
    _name = ros_name;
    _parent = parent;
  }

  CarlaEgoVehicleControlSubscriber::~CarlaEgoVehicleControlSubscriber() {
      if (!_impl)
          return;

      if (_impl->_datareader)
          _impl->_subscriber->delete_datareader(_impl->_datareader);

      if (_impl->_subscriber)
          _impl->_participant->delete_subscriber(_impl->_subscriber);

      if (_impl->_topic)
          _impl->_participant->delete_topic(_impl->_topic);

      if (_impl->_participant)
          efd::DomainParticipantFactory::get_instance()->delete_participant(_impl->_participant);
  }

  CarlaEgoVehicleControlSubscriber::CarlaEgoVehicleControlSubscriber(const CarlaEgoVehicleControlSubscriber& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    _impl->_listener.SetOwner(this);
  }

  CarlaEgoVehicleControlSubscriber& CarlaEgoVehicleControlSubscriber::operator=(const CarlaEgoVehicleControlSubscriber& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    _impl->_listener.SetOwner(this);

    return *this;
  }

  CarlaEgoVehicleControlSubscriber::CarlaEgoVehicleControlSubscriber(CarlaEgoVehicleControlSubscriber&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    _impl->_listener.SetOwner(this);
  }

  CarlaEgoVehicleControlSubscriber& CarlaEgoVehicleControlSubscriber::operator=(CarlaEgoVehicleControlSubscriber&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    _impl->_listener.SetOwner(this);

    return *this;
  }
}
}
