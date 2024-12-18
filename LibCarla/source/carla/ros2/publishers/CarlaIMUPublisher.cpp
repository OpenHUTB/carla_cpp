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
// 为了方便使用 eprosima::fastdds::dds 命名空间，使用别名 efd 来指代它
  namespace efd = eprosima::fastdds::dds;
// 使用别名 erc 来指代 eprosima::fastrtps::types::ReturnCode_t 类型，该类型通常用于表示操作的返回码
  using erc = eprosima::fastrtps::types::ReturnCode_t;
// 定义一个名为 CarlaIMUPublisherImpl 的结构体，用于封装与 Carla IMU 发布者相关的内部实现细节

  struct CarlaIMUPublisherImpl {
// 指向 DDS 领域参与者的指针，用于参与 DDS 网络通信，初始化为 nullptr
    efd::DomainParticipant* _participant { nullptr };
 // 指向 DDS 发布者的指针，用于发布数据，初始化为 nullptr
    efd::Publisher* _publisher { nullptr };
// 指向 DDS 主题的指针，用于定义发布的数据主题，初始化为 nullptr
    efd::Topic* _topic { nullptr };
// 指向 DDS 数据写入器的指针，用于实际将数据写入 DDS 网络，初始化为 nullptr
    efd::DataWriter* _datawriter { nullptr };
// 用于支持要发布的数据类型，这里实例化为 sensor_msgs::msg::ImuPubSubType 类型，用于 IMU 数据类型支持
    efd::TypeSupport _type { new sensor_msgs::msg::ImuPubSubType() };
// Carla 监听器对象，可能用于监听发布过程中的一些事件等情况
    CarlaListener _listener {};
// 用于存储 IMU 数据的消息对象，初始化为默认状态
    sensor_msgs::msg::Imu _imu {};
  };
// CarlaIMUPublisher 类的初始化函数，用于初始化发布者相关的各种 DDS 对象等资源
  bool CarlaIMUPublisher::Init() {
    if (_impl->_type == nullptr) {
        std::cerr << "Invalid TypeSupport" << std::endl;
        return false;// 检查类型支持对象是否为空，如果为空则说明类型支持设置不正确，输出错误信息并返回 false
    }
// 获取默认的 DDS 领域参与者 QoS 配置，并赋值给 pqos 对象，后续可基于此进行个性化配置
    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;// 获取默认的 DDS 领域参与者 QoS 配置，并赋值给 pqos 对象
    pqos.name(_name);// 设置领域参与者的名称为类成员变量 _name 存储的值
    // 获取 DDS 领域参与者工厂的单例实例，用于创建领域参与者对象
    auto factory = efd::DomainParticipantFactory::get_instance();
    // 使用工厂创建一个领域参与者对象，传入领域 ID（这里为 0）和配置好的 QoS 对象 pqos，若创建失败返回 nullptr
    _impl->_participant = factory->create_participant(0, pqos);
    if (_impl->_participant == nullptr) {
        std::cerr << "Failed to create DomainParticipant" << std::endl;
        return false;
    }// // 使用创建好的领域参与者注册要发布的数据类型，确保 DDS 网络知道如何处理该类型的数据
    _impl->_type.register_type(_impl->_participant);
// 获取默认的 DDS 发布者 QoS 配置，并赋值给 pubqos 对象
    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
    // 使用领域参与者创建一个 DDS 发布者，使用前面设置的 QoS 配置 pubqos，监听器为 nullptr
    _impl->_publisher = _impl->_participant->create_publisher(pubqos, nullptr);
    // 如果创建发布者失败（返回的指针为 nullptr），输出错误信息并返回 false
    if (_impl->_publisher == nullptr) {
      std::cerr << "Failed to create Publisher" << std::endl;
      return false;
    }
 // 获取默认的 DDS 主题 QoS 配置，并赋值给 tqos 对象
    efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
    // 定义一个基础的主题名称字符串前缀
    const std::string base { "rt/carla/" };
    // 初始化主题名称字符串为基础前缀
    std::string topic_name = base;
    if (!_parent.empty())
      topic_name += _parent + "/";
    topic_name += _name;
    _impl->_topic = _impl->_participant->create_topic(topic_name, _impl->_type->getName(), tqos); // 如果创建主题失败（返回的指针为 nullptr），输出错误信息并返回 false
    if (_impl->_topic == nullptr) {
        std::cerr << "Failed to create Topic" << std::endl;
        return false;
    }

    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
     // 设置数据写入器的历史内存策略为预分配并可重新分配内存模式，用于管理数据写入的内存相关配置
    wqos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
     // 获取 Carla 监听器内部实现对象的指针（进行了类型转换），用于传递给数据写入器
    efd::DataWriterListener* listener = (efd::DataWriterListener*)_impl->_listener._impl.get();
    _impl->_datawriter = _impl->_publisher->create_datawriter(_impl->_topic, wqos, listener);
    // 如果创建数据写入器失败（返回的指针为 nullptr），输出错误信息并返回 false
    if (_impl->_datawriter == nullptr) {
        std::cerr << "Failed to create DataWriter" << std::endl;
        return false;
    }
    _frame_id = _name;
    return true;
  }
 // CarlaIMUPublisher 类的发布函数，用于将存储在内部的 IMU 数据发布出去
  bool CarlaIMUPublisher::Publish() {
    // 定义一个实例句柄对象，用于在 DDS 中标识要发布的数据实例（具体用法与 DDS 内部机制相关）
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    // 调用数据写入器的 write 方法尝试将内部存储的 IMU 数据（_impl->_imu）写入到 DDS 网络中，返回操作结果码
    eprosima::fastrtps::types::ReturnCode_t rcode = _impl->_datawriter->write(&_impl->_imu, instance_handle);
    // 如果返回码表示操作成功（RETCODE_OK），则返回 true，表示发布成功
    if (rcode == erc::ReturnCodeValue::RETCODE_OK) {
        return true;
    }
    // 如果返回码表示发生错误（RETCODE_ERROR），输出错误信息并返回 false，表示发布失败
    if (rcode == erc::ReturnCodeValue::RETCODE_ERROR) {
        std::cerr << "RETCODE_ERROR" << std::endl;
        return false;
    }
   // 如果返回码表示操作不被支持（RETCODE_UNSUPPORTED），输出错误信息并返回 false，表示发布失败
    if (rcode == erc::ReturnCodeValue::RETCODE_UNSUPPORTED) {
        std::cerr << "RETCODE_UNSUPPORTED" << std::endl;
        return false;
    }
    // 如果返回码表示参数错误（RETCODE_BAD_PARAMETER），输出错误信息并返回 false，表示发布失败
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
 // 设置 IMU 数据的函数，根据传入的参数填充内部的 IMU 消息对象的各个字段
  void CarlaIMUPublisher::SetData(int32_t seconds, uint32_t nanoseconds, float* pAccelerometer, float* pGyroscope, float compass) {
    // 定义一个用于存储陀螺仪数据的向量对象
    geometry_msgs::msg::Vector3 gyroscope;
    // 定义一个用于存储线性加速度数据的向量对象
    geometry_msgs::msg::Vector3 linear_acceleration;
    // 从传入的加速度计数据指针中依次取出 x、y、z 方向的加速度值，并设置到线性加速度向量对象中
    const float ax = *pAccelerometer++;
    const float ay = *pAccelerometer++;
    const float az = *pAccelerometer++;
    linear_acceleration.x(ax);
    linear_acceleration.y(ay);
    linear_acceleration.z(az);
    // 从传入的陀螺仪数据指针中依次取出 x、y、z 方向的角速度值，并设置到陀螺仪向量对象中
    const float gx = *pGyroscope++;
    const float gy = *pGyroscope++;
    const float gz = *pGyroscope++;
    gyroscope.x(gx);
    gyroscope.y(gy);
    gyroscope.z(gz);
// 创建一个时间消息对象，用于存储时间戳信息
    builtin_interfaces::msg::Time time;
    time.sec(seconds);
    time.nanosec(nanoseconds);
 // 创建一个消息头对象，用于存储消息的一些通用元数据，如时间戳、帧 ID 等
    std_msgs::msg::Header header;
    header.stamp(std::move(time));
    header.frame_id(_frame_id);
// 定义一个用于存储姿态（四元数表示）的对象
    geometry_msgs::msg::Quaternion orientation;
// 设置俯仰角（这里固定为 0.0f）
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
