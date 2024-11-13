#define _GLIBCXX_USE_CXX11_ABI 0

#include "CarlaLineInvasionPublisher.h"

#include <string>
// 包含Carla相关的消息类型定义
#include "carla/ros2/types/CarlaLineInvasionPubSubTypes.h"
// 包含Carla监听器相关头文件
#include "carla/ros2/listeners/CarlaListener.h"
// 包含Fast DDS相关的头文件，用于DDS通信的各种功能实现
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
// 包含Fast DDS相关的QoS（服务质量）设置头文件
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
// 包含Fast RTPS相关的属性和QoS策略头文件
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/qos/QosPolicies.h>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>


namespace carla {
namespace ros2 {
	// 为了方便使用，给eprosima::fastdds::dds命名空间起别名efd
  namespace efd = eprosima::fastdds::dds;
  // 给eprosima::fastrtps::types::ReturnCode_t起别名erc，用于表示返回码类型
  using erc = eprosima::fastrtps::types::ReturnCode_t;
  // 定义CarlaLineInvasionPublisherImpl结构体，用于存储发布者相关的各种对象指针和数据
  struct CarlaLineInvasionPublisherImpl {
  	// 域参与者指针，用于参与DDS通信域，初始化为nullptr
    efd::DomainParticipant* _participant { nullptr };
    // 发布者指针，用于发布数据，初始化为nullptr
    efd::Publisher* _publisher { nullptr };
    // 主题指针，用于标识发布数据的主题，初始化为nullptr
    efd::Topic* _topic { nullptr };
    // 数据写入器指针，用于将数据写入到主题，初始化为nullptr
    efd::DataWriter* _datawriter { nullptr };
    // 类型支持对象，用于注册消息类型，这里是Carla的车道入侵事件消息类型
    efd::TypeSupport _type { new carla_msgs::msg::LaneInvasionEventPubSubType() };
    // Carla监听器对象，用于监听相关事件
    CarlaListener _listener {};
    // 存储车道入侵事件的数据结构，初始化为默认值
    carla_msgs::msg::LaneInvasionEvent _event {};
  };
// CarlaLineInvasionPublisher类的初始化函数，用于初始化发布者相关的各种对象
  bool CarlaLineInvasionPublisher::Init() {
  	// 检查类型支持对象是否为空，如果为空则输出错误信息并返回false
    if (_impl->_type == nullptr) {
        std::cerr << "Invalid TypeSupport" << std::endl;
        return false;
    }
// 获取默认的域参与者QoS设置，并设置其名称为传入的_name
    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    pqos.name(_name);
    // 获取域参与者工厂的单例实例
    auto factory = efd::DomainParticipantFactory::get_instance();
    // 使用工厂创建域参与者对象，传入域ID和QoS设置，如果创建失败则输出错误信息并返回false
    _impl->_participant = factory->create_participant(0, pqos);
    if (_impl->_participant == nullptr) {
        std::cerr << "Failed to create DomainParticipant" << std::endl;
        return false;
    }
    // 在创建的域参与者上注册消息类型
    _impl->_type.register_type(_impl->_participant);
// 获取默认的发布者QoS设置
    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
    // 使用域参与者创建发布者对象，传入QoS设置和空的监听器指针，如果创建失败则输出错误信息并返回false
    _impl->_publisher = _impl->_participant->create_publisher(pubqos, nullptr);
    if (_impl->_publisher == nullptr) {
      std::cerr << "Failed to create Publisher" << std::endl;
      return false;
    }
// 获取默认的主题QoS设置
    efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
    const std::string base { "rt/carla/" };
    std::string topic_name = base;
    // 如果父名称不为空，则将父名称添加到主题名称中
    if (!_parent.empty())
      topic_name += _parent + "/";
      // 再添加自身名称到主题名称中
    topic_name += _name;
    // 使用域参与者创建主题对象，传入主题名称、消息类型名称和QoS设置，如果创建失败则输出错误信息并返回false
    _impl->_topic = _impl->_participant->create_topic(topic_name, _impl->_type->getName(), tqos);
    if (_impl->_topic == nullptr) {
        std::cerr << "Failed to create Topic" << std::endl;
        return false;
    }
// 获取默认的数据写入器QoS设置，并设置其内存策略为预分配并可重新分配模式
    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
    wqos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
     // 获取监听器对象的指针，用于数据写入器的监听器设置
    efd::DataWriterListener* listener = (efd::DataWriterListener*)_impl->_listener._impl.get();
    // 使用发布者创建数据写入器对象，传入主题、QoS设置和监听器指针，如果创建失败则输出错误信息并返回false
    _impl->_datawriter = _impl->_publisher->create_datawriter(_impl->_topic, wqos, listener);
    if (_impl->_datawriter == nullptr) {
        std::cerr << "Failed to create DataWriter" << std::endl;
        return false;
    }
    // 设置帧ID为自身名称
    _frame_id = _name;
    return true;
  }
// CarlaLineInvasionPublisher类的发布函数，用于将数据发布到主题
  bool CarlaLineInvasionPublisher::Publish() {
  	// 定义实例句柄，用于标识数据实例
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    // 调用数据写入器的write函数将事件数据写入，获取返回码
    eprosima::fastrtps::types::ReturnCode_t rcode = _impl->_datawriter->write(&_impl->_event, instance_handle);
    // 如果返回码为RETCODE_OK，表示写入成功，返回true
    if (rcode == erc::ReturnCodeValue::RETCODE_OK) {
        return true;
    }
     // 根据不同的返回码值，输出相应的错误信息并返回false
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
// 设置要发布的车道入侵事件数据的函数
  void CarlaLineInvasionPublisher::SetData(int32_t seconds, uint32_t nanoseconds, const int32_t* data) {
  	// 创建一个时间结构体，并设置秒和纳秒的值
    builtin_interfaces::msg::Time time;
    time.sec(seconds);
    time.nanosec(nanoseconds);
// 创建一个消息头结构体，设置时间戳和帧ID
    std_msgs::msg::Header header;
    header.stamp(std::move(time));
    header.frame_id(_frame_id);
// 设置车道入侵事件的消息头
    _impl->_event.header(std::move(header));
    // 设置车道入侵事件中跨越车道标记的数据
    _impl->_event.crossed_lane_markings({data[0], data[1], data[2]});
  }
// CarlaLineInvasionPublisher类的构造函数，用于初始化对象的成员变量
  CarlaLineInvasionPublisher::CarlaLineInvasionPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaLineInvasionPublisherImpl>()) {
    _name = ros_name;
    _parent = parent;
  }
// CarlaLineInvasionPublisher类的析构函数，用于释放相关资源
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
// 拷贝构造函数，用于复制另一个CarlaLineInvasionPublisher对象的成员变量
  CarlaLineInvasionPublisher::CarlaLineInvasionPublisher(const CarlaLineInvasionPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
  }
// 拷贝赋值运算符重载函数，用于将另一个CarlaLineInvasionPublisher对象的成员变量赋值给当前对象
  CarlaLineInvasionPublisher& CarlaLineInvasionPublisher::operator=(const CarlaLineInvasionPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;

    return *this;
  }
// 移动构造函数，用于移动另一个CarlaLineInvasionPublisher对象的资源到当前对象
  CarlaLineInvasionPublisher::CarlaLineInvasionPublisher(CarlaLineInvasionPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
  }
// 移动赋值运算符重载函数，用于将另一个CarlaLineInvasionPublisher对象的资源移动赋值给当前对象
  CarlaLineInvasionPublisher& CarlaLineInvasionPublisher::operator=(CarlaLineInvasionPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);

    return *this;
  }
}
}
