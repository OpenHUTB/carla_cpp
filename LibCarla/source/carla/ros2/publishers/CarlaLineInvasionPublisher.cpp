#define _GLIBCXX_USE_CXX11_ABI 0

#include "CarlaLineInvasionPublisher.h"

#include <string>

#include "carla/ros2/types/CarlaLineInvasionPubSubTypes.h"
#include "carla/ros2/listeners/CarlaListener.h"

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>

#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/qos/QosPolicies.h>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>

// 定义命名空间 carla，可能用于对相关类、函数等进行分类和组织，避免命名冲突
namespace carla {
namespace ros2 {
  namespace efd = eprosima::fastdds::dds;
  using erc = eprosima::fastrtps::types::ReturnCode_t;
 // 定义一个结构体 CarlaLineInvasionPublisherImpl，用于存储与 CarlaLineInvasionPublisher 相关的实现细节和内部状态
  struct CarlaLineInvasionPublisherImpl {
  	// 指向领域参与者对象的指针，领域参与者是 DDS 架构中的核心实体，负责协调和管理发布/订阅等操作，初始化为 nullptr
        efd::DomainParticipant* _participant { nullptr };
    efd::DomainParticipant* _participant { nullptr };
    efd::Publisher* _publisher { nullptr };
    efd::Topic* _topic { nullptr };
    efd::DataWriter* _datawriter { nullptr };
    efd::TypeSupport _type { new carla_msgs::msg::LaneInvasionEventPubSubType() };
    CarlaListener _listener {};
    carla_msgs::msg::LaneInvasionEvent _event {};
  };
// CarlaLineInvasionPublisher 类的 Init 函数定义，用于初始化相关的 DDS 实体和配置，返回 true 表示初始化成功，false 表示失败
  bool CarlaLineInvasionPublisher::Init() {
  	// 首先检查类型支持对象是否为空，如果为空则说明消息类型设置有问题，输出错误信息并返回 false
    if (_impl->_type == nullptr) {
        std::cerr << "Invalid TypeSupport" << std::endl;
        return false;
    }
// 获取默认的领域参与者服务质量配置对象，并通过调用其 name 函数设置名称为类中的 _name 成员变量（具体在构造函数中赋值）
    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    pqos.name(_name);
    // 获取领域参与者工厂的单例实例，用于后续创建领域参与者对象
    auto factory = efd::DomainParticipantFactory::get_instance();
    // 使用工厂创建领域参与者对象，传入领域 ID（这里为 0）和服务质量配置对象，如果创建失败则输出错误信息并返回 false
    _impl->_participant = factory->create_participant(0, pqos);
    if (_impl->_participant == nullptr) {
        std::cerr << "Failed to create DomainParticipant" << std::endl;
        return false;
    }
    // 使用创建好的领域参与者对象注册消息类型，确保该类型能在 DDS 系统中被正确识别和处理
    _impl->_type.register_type(_impl->_participant);
// 获取默认的发布者服务质量配置对象
    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
    // 使用领域参与者对象创建发布者对象，传入服务质量配置和空的监听器指针（可能后续可以添加监听器来监听发布相关事件），如果创建失败则输出错误信息并返回 false
    _impl->_publisher = _impl->_participant->create_publisher(pubqos, nullptr);
    if (_impl->_publisher == nullptr) {
      std::cerr << "Failed to create Publisher" << std::endl;
      return false;
    }
// 获取默认的主题服务质量配置对象
    efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
    const std::string base { "rt/carla/" };
    std::string topic_name = base;
    // 如果父名称（_parent）不为空，则将其添加到主题名称中，用于构建更具体的主题路径，增加主题的层次结构和区分度
    if (!_parent.empty())
      topic_name += _parent + "/";
    topic_name += _name;
    // 使用领域参与者对象创建主题对象，传入主题名称、消息类型名称（通过类型支持对象获取）和服务质量配置对象，如果创建失败则输出错误信息并返回 false
    _impl->_topic = _impl->_participant->create_topic(topic_name, _impl->_type->getName(), tqos);
    if (_impl->_topic == nullptr) {
        std::cerr << "Failed to create Topic" << std::endl;
        return false;
    }
// 获取默认的数据写入器服务质量配置对象
    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
    wqos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    efd::DataWriterListener* listener = (efd::DataWriterListener*)_impl->_listener._impl.get();
    _impl->_datawriter = _impl->_publisher->create_datawriter(_impl->_topic, wqos, listener);
    if (_impl->_datawriter == nullptr) {
        std::cerr << "Failed to create DataWriter" << std::endl;
        return false;
    }
    // 设置帧 ID 为类中的名称成员变量（_name），用于标识消息的来源或者所属的帧等相关信息
    _frame_id = _name;
    // 如果所有创建和配置操作都成功完成，则返回 true 表示初始化成功
    return true;
  }
// CarlaLineInvasionPublisher 类的 Publish 函数定义，用于将设置好的消息发布出去，返回 true 表示发布成功，false 表示失败
  bool CarlaLineInvasionPublisher::Publish() {
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    eprosima::fastrtps::types::ReturnCode_t rcode = _impl->_datawriter->write(&_impl->_event, instance_handle);
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
    // 如果返回码不属于上述任何已知的情况，则输出 "UNKNOWN" 表示未知错误，并返回 false
    std::cerr << "UNKNOWN" << std::endl;
    return false;
  }
// CarlaLineInvasionPublisher 类的 SetData 函数定义，用于设置要发布的消息（LaneInvasionEvent 类型）的具体内容
  void CarlaLineInvasionPublisher::SetData(int32_t seconds, uint32_t nanoseconds, const int32_t* data) {
    builtin_interfaces::msg::Time time;
    time.sec(seconds);
    time.nanosec(nanoseconds);

    std_msgs::msg::Header header;
    header.stamp(std::move(time));
    header.frame_id(_frame_id);

    _impl->_event.header(std::move(header));
    _impl->_event.crossed_lane_markings({data[0], data[1], data[2]});
  }
// CarlaLineInvasionPublisher 类的构造函数定义，接受 ROS 名称（ros_name）和父名称（parent）作为参数，用于初始化对象的相关属性
  CarlaLineInvasionPublisher::CarlaLineInvasionPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaLineInvasionPublisherImpl>()) {
    _name = ros_name;
    _parent = parent;
  }
// CarlaLineInvasionPublisher 类的析构函数定义，用于释放对象在生命周期内创建的各种资源，如 DDS 相关实体对象等
  CarlaLineInvasionPublisher::~CarlaLineInvasionPublisher() {
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
 // CarlaLineInvasionPublisher类的拷贝构造函数，用于创建一个新对象并复制另一个对象的相关成员变量和内部实现对象
  CarlaLineInvasionPublisher::CarlaLineInvasionPublisher(const CarlaLineInvasionPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
  }
// CarlaLineInvasionPublisher类的拷贝赋值运算符重载函数，用于将一个对象的状态复制到另一个对象
  CarlaLineInvasionPublisher& CarlaLineInvasionPublisher::operator=(const CarlaLineInvasionPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;

    return *this;
  }
// CarlaLineInvasionPublisher类的移动构造函数，用于通过移动语义创建新对象，将另一个对象的资源所有权转移过来
  CarlaLineInvasionPublisher::CarlaLineInvasionPublisher(CarlaLineInvasionPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
  }

  CarlaLineInvasionPublisher& CarlaLineInvasionPublisher::operator=(CarlaLineInvasionPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);

    return *this;
  }
}
}
