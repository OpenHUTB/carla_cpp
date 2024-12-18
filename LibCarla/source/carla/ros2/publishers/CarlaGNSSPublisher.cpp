#define _GLIBCXX_USE_CXX11_ABI 0

#include "CarlaGNSSPublisher.h"

#include <string>

#include "carla/ros2/types/NavSatFixPubSubTypes.h"
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


namespace carla {
namespace ros2 {
	// 为了方便使用Fast DDS的相关命名空间，进行别名定义
  namespace efd = eprosima::fastdds::dds;
  // 为了方便使用Fast RTPS的返回码类型，进行别名定义
  using erc = eprosima::fastrtps::types::ReturnCode_t;
// CarlaGNSSPublisher的实现类结构体，用于封装相关的内部实现细节
  struct CarlaGNSSPublisherImpl {
  	// 域参与者指针，用于参与DDS通信域，管理实体等，初始化为nullptr
    efd::DomainParticipant* _participant { nullptr };
    efd::Publisher* _publisher { nullptr };
    efd::Topic* _topic { nullptr };
    efd::DataWriter* _datawriter { nullptr };
    efd::TypeSupport _type { new sensor_msgs::msg::NavSatFixPubSubType() };
    // Carla监听器对象，用于监听相关事件等
    CarlaListener _listener {};
    // 存储导航卫星定位消息的数据结构实例，用于后续设置和发布数据
    sensor_msgs::msg::NavSatFix _nav {};
  };
// CarlaGNSSPublisher类的初始化函数，用于初始化相关的DDS实体等操作
  bool CarlaGNSSPublisher::Init() {
  	 // 检查类型支持是否有效，如果为nullptr则输出错误信息并返回false
    if (_impl->_type == nullptr) {
        std::cerr << "Invalid TypeSupport" << std::endl;
        return false;
    }
// 获取默认的域参与者QoS配置，并设置其名称为传入的_name
    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    pqos.name(_name);
    // 获取域参与者工厂的单例实例
    auto factory = efd::DomainParticipantFactory::get_instance();
    // 通过工厂创建域参与者，指定域ID为0，并传入QoS配置，如果创建失败则输出错误信息并返回false
    _impl->_participant = factory->create_participant(0, pqos);
    if (_impl->_participant == nullptr) {
        std::cerr << "Failed to create DomainParticipant" << std::endl;
        return false;
    }
    // 使用创建好的域参与者注册消息类型
    _impl->_type.register_type(_impl->_participant);
// 获取默认的发布者QoS配置
    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
    // 通过域参与者创建发布者，传入QoS配置和nullptr（表示不使用特定的监听器），如果创建失败则输出错误信息并返回false
    _impl->_publisher = _impl->_participant->create_publisher(pubqos, nullptr);
    if (_impl->_publisher == nullptr) {
      std::cerr << "Failed to create Publisher" << std::endl;
      return false;
    }
// 获取默认的主题QoS配置
    efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
    // 构建主题名称的基础部分
    const std::string base { "rt/carla/" };
    std::string topic_name = base;
    // 如果父名称不为空，则将父名称添加到主题名称中
    if (!_parent.empty())
      topic_name += _parent + "/";
      // 再添加当前的名称到主题名称中
    topic_name += _name;
    // 通过域参与者创建主题，指定主题名称、消息类型名称以及QoS配置，如果创建失败则输出错误信息并返回false
    _impl->_topic = _impl->_participant->create_topic(topic_name, _impl->_type->getName(), tqos);
    if (_impl->_topic == nullptr) {
        std::cerr << "Failed to create Topic" << std::endl;
        return false;
    }
// 获取默认的数据写入器QoS配置
    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
    // 设置数据写入器的历史内存策略为预分配可重分配模式
    wqos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    // 获取数据写入器监听器对象（从Carla监听器中获取其内部实现指针并进行类型转换）
    efd::DataWriterListener* listener = (efd::DataWriterListener*)_impl->_listener._impl.get();
    // 通过发布者创建数据写入器，指定主题、QoS配置以及监听器，如果创建失败则输出错误信息并返回false
    _impl->_datawriter = _impl->_publisher->create_datawriter(_impl->_topic, wqos, listener);
    if (_impl->_datawriter == nullptr) {
        std::cerr << "Failed to create DataWriter" << std::endl;
        return false;
    }
    // 设置帧ID为当前名称，通常用于标识消息所属的帧等情况
    _frame_id = _name;
    return true;
  }
// CarlaGNSSPublisher类的发布函数，用于将设置好的数据通过数据写入器发布出去
  bool CarlaGNSSPublisher::Publish() {
  	// 实例句柄，用于在发布数据时标识实例等情况（具体根据Fast DDS内部机制使用）
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    // 调用数据写入器的write函数将导航卫星定位消息数据写入，获取返回码
    eprosima::fastrtps::types::ReturnCode_t rcode = _impl->_datawriter->write(&_impl->_nav, instance_handle);
    // 如果返回码表示操作成功，则返回true
    if (rcode == erc::ReturnCodeValue::RETCODE_OK) {
        return true;
    }
    // 以下是针对各种可能的错误返回码情况，分别输出相应的错误信息并返回false
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
    // 如果返回码不属于上述已知的情况，则输出未知返回码的错误信息并返回false
    std::cerr << "UNKNOWN return code: " << rcode() << std::endl;
    return false;
  }
// 设置要发布的导航卫星定位消息的数据函数
  void CarlaGNSSPublisher::SetData(int32_t seconds, uint32_t nanoseconds, const double* data) {
  	// 创建一个时间结构体实例，用于设置消息的时间戳
    builtin_interfaces::msg::Time time;
    // 设置时间的秒数部分
    time.sec(seconds);
    // 设置时间的纳秒部分
    time.nanosec(nanoseconds);
// 创建一个消息头结构体实例，用于设置消息的头部信息
    std_msgs::msg::Header header;
    header.stamp(std::move(time));
    header.frame_id(_frame_id);
// 设置导航卫星定位消息的头部信息为上面构建好的消息头结构体实例（通过移动语义传递）
    _impl->_nav.header(std::move(header));
    _impl->_nav.latitude(*data++);
    _impl->_nav.longitude(*data++);
    _impl->_nav.altitude(*data++);
  }
// CarlaGNSSPublisher类的构造函数，用于初始化对象，传入ROS名称和父名称
  CarlaGNSSPublisher::CarlaGNSSPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaGNSSPublisherImpl>()) {
    _name = ros_name;
    _parent = parent;
  }
// CarlaGNSSPublisher类的析构函数，用于释放相关的DDS资源
  CarlaGNSSPublisher::~CarlaGNSSPublisher() {
// 如果内部实现指针为空，则直接返回，不进行资源释放操作
      if (!_impl)
          return;
// 如果数据写入器存在，则通过发布者删除数据写入器
      if (_impl->_datawriter)
          _impl->_publisher->delete_datawriter(_impl->_datawriter);
 // 如果发布者存在，则通过域参与者删除发布者
      if (_impl->_publisher)
          _impl->_participant->delete_publisher(_impl->_publisher);
// 如果主题存在，则通过域参与者删除主题
      if (_impl->_topic)
          _impl->_participant->delete_topic(_impl->_topic);
// 如果域参与者存在，则通过域参与者工厂删除域参与者
      if (_impl->_participant)
          efd::DomainParticipantFactory::get_instance()->delete_participant(_impl->_participant);
  }
// CarlaGNSSPublisher类的拷贝构造函数
  CarlaGNSSPublisher::CarlaGNSSPublisher(const CarlaGNSSPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
  }
// CarlaGNSSPublisher类的拷贝赋值运算符重载函数
  CarlaGNSSPublisher& CarlaGNSSPublisher::operator=(const CarlaGNSSPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;

    return *this;
  }
// CarlaGNSSPublisher类的移动构造函数
  CarlaGNSSPublisher::CarlaGNSSPublisher(CarlaGNSSPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
  }
// CarlaGNSSPublisher类的移动赋值运算符重载函数
  CarlaGNSSPublisher& CarlaGNSSPublisher::operator=(CarlaGNSSPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);

    return *this;
  }
}
}
