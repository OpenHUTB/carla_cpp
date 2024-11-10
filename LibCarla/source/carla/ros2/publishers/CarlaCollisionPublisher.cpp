#define _GLIBCXX_USE_CXX11_ABI 0

#include "CarlaCollisionPublisher.h"// 包含Carla碰撞事件发布者类的声明

#include <string>// 包含字符串处理相关的功能
// 包含Carla ROS 2类型定义和监听器相关的头文件
#include "carla/ros2/types/CarlaCollisionEventPubSubTypes.h"
#include "carla/ros2/listeners/CarlaListener.h"
// 包含Fast-DDS（eProsima Fast RTPS的C++ API）相关的头文件
#include <fastdds/dds/domain/DomainParticipant.hpp>// 包含域参与者相关的类
#include <fastdds/dds/publisher/Publisher.hpp>// 包含发布者相关的类
#include <fastdds/dds/topic/Topic.hpp>// 包含主题相关的类
#include <fastdds/dds/publisher/DataWriter.hpp> // 包含数据写入器相关的类
#include <fastdds/dds/topic/TypeSupport.hpp>// 包含类型支持相关的类
// 包含Fast-DDS QOS策略相关的头文件
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp> // 包含域参与者QOS策略相关的类
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>// 包含域参与者工厂相关的类
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>// 包含发布者QOS策略相关的类
#include <fastdds/dds/topic/qos/TopicQos.hpp>// 包含主题QOS策略相关的类
// 包含Fast RTPS属性相关的头文件
#include <fastrtps/attributes/ParticipantAttributes.h>// 包含参与者属性相关的类
#include <fastrtps/qos/QosPolicies.h>// 包含QOS策略相关的类
// 包含Fast-DDS数据写入器QOS策略和监听器相关的头文件
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>// 包含数据写入器QOS策略相关的类
#include <fastdds/dds/publisher/DataWriterListener.hpp>// 包含数据写入器监听器相关的类

/**
 * @namespace carla::ros2
 * @brief 命名空间，用于组织CARLA与ROS2相关的代码。
 */

 /**
  * @brief 命名空间别名定义
  * @details 为eprosima::fastdds::dds命名空间定义别名efd，为eprosima::fastrtps::types::ReturnCode_t类型定义别名erc，方便后续使用。
  */
namespace carla {
namespace ros2 {

  namespace efd = eprosima::fastdds::dds;
  using erc = eprosima::fastrtps::types::ReturnCode_t;
  /**
 * @struct CarlaCollisionPublisherImpl
 * @brief CarlaCollisionPublisher的内部实现结构体
 *
 * 该结构体包含了与DDS通信相关的成员变量，用于管理CARLA碰撞事件的发布。
 */
  struct CarlaCollisionPublisherImpl {
      /**
     * @brief DDS域参与者指针
     * @details 指向一个DDS域参与者的指针，用于管理通信域。
     */
    efd::DomainParticipant* _participant { nullptr };
    /**
     * @brief DDS发布者指针
     * @details 指向一个DDS发布者的指针，用于发布消息。
     */
    efd::Publisher* _publisher { nullptr };
    /**
     * @brief DDS主题指针
     * @details 指向一个DDS主题的指针，用于定义消息的类型和名称。
     */
    efd::Topic* _topic { nullptr };
    /**
     * @brief DDS数据写入器指针
     * @details 指向一个DDS数据写入器的指针，用于写入数据到主题。
     */
    efd::DataWriter* _datawriter { nullptr };
    /**
     * @brief DDS类型支持
     * @details 用于注册和管理消息类型的支持对象，这里使用carla_msgs::msg::CarlaCollisionEventPubSubType。
     */
    efd::TypeSupport _type { new carla_msgs::msg::CarlaCollisionEventPubSubType() };
    /**
     * @brief Carla监听器
     * @details 用于监听DDS通信事件的监听器对象。
     */
    CarlaListener _listener {};
    /**
     * @brief CARLA碰撞事件消息
     * @details 存储要发布的CARLA碰撞事件消息的数据结构。
     */
    carla_msgs::msg::CarlaCollisionEvent _event {};
  };
  /**
 * @brief 初始化CarlaCollisionPublisher
 * @return bool 初始化成功返回true，失败返回false
 *
 * 该函数负责初始化CarlaCollisionPublisher，包括创建DDS域参与者、发布者、主题和数据写入器，并注册消息类型。
 */
  bool CarlaCollisionPublisher::Init() {
      /**
     * @brief 检查类型支持是否有效
     * @details 如果_type为nullptr，则打印错误信息并返回false。
     */
    if (_impl->_type == nullptr) {
        std::cerr << "Invalid TypeSupport" << std::endl;
        return false;
    }
    /**
     * @brief 设置域参与者QoS策略
     * @details 使用默认的域参与者QoS策略，并设置名称。
     */
    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    pqos.name(_name);
    /**
     * @brief 创建域参与者
     * @details 使用DomainParticipantFactory创建域参与者，如果失败则打印错误信息并返回false。
     */
    auto factory = efd::DomainParticipantFactory::get_instance();
    _impl->_participant = factory->create_participant(0, pqos);
    if (_impl->_participant == nullptr) {
        std::cerr << "Failed to create DomainParticipant" << std::endl;
        return false;
    }
    /**
     * @brief 注册类型支持
     * @details 使用类型支持对象注册消息类型到域参与者。
     */
    _impl->_type.register_type(_impl->_participant);
    /**
     * @brief 设置发布者QoS策略
     * @details 使用默认的发布者QoS策略。
     */
    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
    /**
     * @brief 创建发布者
     * @details 在域参与者中创建一个发布者，如果失败则打印错误信息并返回false。
     */
    _impl->_publisher = _impl->_participant->create_publisher(pubqos, nullptr);
    if (_impl->_publisher == nullptr) {
      std::cerr << "Failed to create Publisher" << std::endl;
      return false;
    }
    /**
     * @brief 设置主题QoS策略
     * @details 使用默认的主题QoS策略。
     */
    efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
    /**
     * @brief 构建主题名称
     * @details 根据_name和_parent成员变量构建主题名称。
     */
    const std::string base { "rt/carla/" };
    std::string topic_name = base;
    if (!_parent.empty())
      topic_name += _parent + "/";
    topic_name += _name;
    /**
     * @brief 创建主题
     * @details 在域参与者中创建一个主题，如果失败则打印错误信息并返回false。
     */
    _impl->_topic = _impl->_participant->create_topic(topic_name, _impl->_type->getName(), tqos);
    if (_impl->_topic == nullptr) {
        std::cerr << "Failed to create Topic" << std::endl;
        return false;
    }
    /**
     * @brief 设置数据写入器QoS策略
     * @details 使用默认的数据写入器QoS策略，并设置历史内存策略为预分配并允许重新分配。
     */
    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
    wqos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    /**
     * @brief 创建数据写入器
     * @details 在发布者中创建一个数据写入器，并关联监听器，如果失败则打印错误信息并返回false。
     */
    efd::DataWriterListener* listener = (efd::DataWriterListener*)_impl->_listener._impl.get();
    _impl->_datawriter = _impl->_publisher->create_datawriter(_impl->_topic, wqos, listener);
    if (_impl->_datawriter == nullptr) {
        std::cerr << "Failed to create DataWriter" << std::endl;
        return false;
    }
    /**
    * @brief 设置帧ID
    * @details 将_frame_id设置为_name的值。
    */
    _frame_id = _name;
    /**
     * @brief 初始化成功
     * @details 初始化所有DDS组件成功后返回true。
     */
    return true;
  }
  /**
 * @brief 发布碰撞数据
 *
 * 此函数尝试通过Fast-RTPS发布碰撞事件数据。
 * 根据返回码（ReturnCode_t）判断发布是否成功，并输出相应的错误信息。
 *
 * @return bool 如果发布成功返回true，否则返回false。
 */
  bool CarlaCollisionPublisher::Publish() {
      /// @brief 用于存储Fast-RTPS实例句柄的变量
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    /// @brief 调用DataWriter的write方法尝试发布数据，并获取返回码
    eprosima::fastrtps::types::ReturnCode_t rcode = _impl->_datawriter->write(&_impl->_event, instance_handle);
    /// @brief 如果返回码为RETCODE_OK，表示发布成功
    if (rcode == erc::ReturnCodeValue::RETCODE_OK) {
        return true;
    }
    /// @brief 处理各种可能的错误返回码，并输出相应的错误信息
    /// @note 以下分支处理不同的错误情况
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
    /// @brief 如果返回码未知，则输出"UNKNOWN"错误信息
    std::cerr << "UNKNOWN" << std::endl;
    return false;
  }

void CarlaCollisionPublisher::SetData(int32_t seconds, uint32_t nanoseconds, uint32_t actor_id, float x, float y, float z) {
    std::vector<float> vector_data ;
    SetData(seconds, nanoseconds, actor_id, {x, y, z});
  }

  void CarlaCollisionPublisher::SetData(int32_t seconds, uint32_t nanoseconds, uint32_t actor_id, std::vector<float>&& data) {
    builtin_interfaces::msg::Time time;
    time.sec(seconds);
    time.nanosec(nanoseconds);

    std_msgs::msg::Header header;
    header.stamp(std::move(time));
    header.frame_id(_frame_id);

    geometry_msgs::msg::Vector3 impulse;
    impulse.x(data[0]);
    impulse.y(data[1]);
    impulse.z(data[2]);
    _impl->_event.header(std::move(header));
    _impl->_event.other_actor_id(actor_id);
    _impl->_event.normal_impulse(impulse);
  }

  CarlaCollisionPublisher::CarlaCollisionPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaCollisionPublisherImpl>()) {
    _name = ros_name;
    _parent = parent;
  }

  CarlaCollisionPublisher::~CarlaCollisionPublisher() {
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

  CarlaCollisionPublisher::CarlaCollisionPublisher(const CarlaCollisionPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
  }

  CarlaCollisionPublisher& CarlaCollisionPublisher::operator=(const CarlaCollisionPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;

    return *this;
  }

  CarlaCollisionPublisher::CarlaCollisionPublisher(CarlaCollisionPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
  }

  CarlaCollisionPublisher& CarlaCollisionPublisher::operator=(CarlaCollisionPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);

    return *this;
  }
}
}
