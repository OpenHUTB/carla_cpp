#define _GLIBCXX_USE_CXX11_ABI 0

#include "CarlaIMUPublisher.h"

#include <string>

#include "carla/ros2/types/ImuPubSubTypes.h"
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
// eprosima::fastdds::dds 是 Fast DDS（Data Distribution Service）相关的命名空间，
// 它提供了数据发布和订阅所需的基本类型和功能。
  namespace efd = eprosima::fastdds::dds; // efd 是 eprosima::fastdds::dds 的别名
  using erc = eprosima::fastrtps::types::ReturnCode_t;// erc 是 Fast RTPS 中返回码类型的别名


  // CarlaIMUPublisherImpl 结构体，封装了所有与 IMU 数据发布器相关的成员。
  struct CarlaIMUPublisherImpl {
    efd::DomainParticipant* _participant { nullptr };// Fast DDS 中的 DomainParticipant，用于参与 DDS 域
    efd::Publisher* _publisher { nullptr }; // 用于发布数据的 Publisher
    efd::Topic* _topic { nullptr }; // 发布数据的 Topic
    efd::DataWriter* _datawriter { nullptr };// 数据写入器，用于将数据写入 Topic
    efd::TypeSupport _type { new sensor_msgs::msg::ImuPubSubType() };// 数据类型支持对象，定义 IMU 数据类型
    CarlaListener _listener {};// 用于监听 DataWriter 事件的监听器
    sensor_msgs::msg::Imu _imu {};
  };

  // CarlaIMUPublisher 类的 Init 函数，用于初始化发布器
  bool CarlaIMUPublisher::Init() {
      // 检查 TypeSupport 是否有效，确保 IMU 数据类型正确注册
    if (_impl->_type == nullptr) {
        std::cerr << "Invalid TypeSupport" << std::endl;
        return false;// 如果无效，输出错误信息并返回 false
    }

    // 设置 DomainParticipant 的 QoS（质量服务）
    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    pqos.name(_name);
    // 获取 DomainParticipantFactory 的实例，创建一个 DomainParticipant
    auto factory = efd::DomainParticipantFactory::get_instance();
    _impl->_participant = factory->create_participant(0, pqos);
    if (_impl->_participant == nullptr) {
        std::cerr << "Failed to create DomainParticipant" << std::endl;
        return false;
    }
    // 注册 IMU 数据类型
    _impl->_type.register_type(_impl->_participant);

    // 设置 Publisher 的 QoS
    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
    _impl->_publisher = _impl->_participant->create_publisher(pubqos, nullptr);
    if (_impl->_publisher == nullptr) { // 如果有父类名称，则追加
      std::cerr << "Failed to create Publisher" << std::endl;
      return false;
    }

    // 设置 Topic 的 QoS
    efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
    // 设置 Topic 名称，基于父类和当前类的名称构建完整 Topic 名称
    const std::string base { "rt/carla/" };
    std::string topic_name = base;
    if (!_parent.empty())
      topic_name += _parent + "/";
    topic_name += _name;
    _impl->_topic = _impl->_participant->create_topic(topic_name, _impl->_type->getName(), tqos);
    if (_impl->_topic == nullptr) {
        std::cerr << "Failed to create Topic" << std::endl;
        return false;
    }

    // 设置 DataWriter 的 QoS
    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
    wqos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    efd::DataWriterListener* listener = (efd::DataWriterListener*)_impl->_listener._impl.get();// 创建 DataWriter 的监听器对象
    // 创建 DataWriter
    _impl->_datawriter = _impl->_publisher->create_datawriter(_impl->_topic, wqos, listener);
    if (_impl->_datawriter == nullptr) {
        std::cerr << "Failed to create DataWriter" << std::endl;
        return false;
    }
    _frame_id = _name;
    return true;
  }

  bool CarlaIMUPublisher::Publish() {
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    eprosima::fastrtps::types::ReturnCode_t rcode = _impl->_datawriter->write(&_impl->_imu, instance_handle);
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

  void CarlaIMUPublisher::SetData(int32_t seconds, uint32_t nanoseconds, float* pAccelerometer, float* pGyroscope, float compass) {
    geometry_msgs::msg::Vector3 gyroscope;
    geometry_msgs::msg::Vector3 linear_acceleration;
    const float ax = *pAccelerometer++;
    const float ay = *pAccelerometer++;
    const float az = *pAccelerometer++;
    linear_acceleration.x(ax);
    linear_acceleration.y(ay);
    linear_acceleration.z(az);
    const float gx = *pGyroscope++;
    const float gy = *pGyroscope++;
    const float gz = *pGyroscope++;
    gyroscope.x(gx);
    gyroscope.y(gy);
    gyroscope.z(gz);

    builtin_interfaces::msg::Time time;
    time.sec(seconds);
    time.nanosec(nanoseconds);

    std_msgs::msg::Header header;
    header.stamp(std::move(time));
    header.frame_id(_frame_id);

    geometry_msgs::msg::Quaternion orientation;

    const float rx = 0.0f;                           // pitch
    const float ry = (M_PIf32 / 2.0f) - compass;     // yaw
    const float rz = 0.0f;                           // roll

    const float cr = cosf(rz * 0.5f);
    const float sr = sinf(rz * 0.5f);
    const float cp = cosf(rx * 0.5f);
    const float sp = sinf(rx * 0.5f);
    const float cy = cosf(ry * 0.5f);
    const float sy = sinf(ry * 0.5f);

    orientation.w(cr * cp * cy + sr * sp * sy);
    orientation.x(sr * cp * cy - cr * sp * sy);
    orientation.y(cr * sp * cy + sr * cp * sy);
    orientation.z(cr * cp * sy - sr * sp * cy);

    _impl->_imu.header(std::move(header));
    _impl->_imu.orientation(orientation);
    _impl->_imu.angular_velocity(gyroscope);
    _impl->_imu.linear_acceleration(linear_acceleration);
  }

  CarlaIMUPublisher::CarlaIMUPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaIMUPublisherImpl>()) {
    _name = ros_name;
    _parent = parent;
  }

  CarlaIMUPublisher::~CarlaIMUPublisher() {
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

  CarlaIMUPublisher::CarlaIMUPublisher(const CarlaIMUPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
  }

  CarlaIMUPublisher& CarlaIMUPublisher::operator=(const CarlaIMUPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;

    return *this;
  }

  CarlaIMUPublisher::CarlaIMUPublisher(CarlaIMUPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
  }

  CarlaIMUPublisher& CarlaIMUPublisher::operator=(CarlaIMUPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);

    return *this;
  }
}
}
