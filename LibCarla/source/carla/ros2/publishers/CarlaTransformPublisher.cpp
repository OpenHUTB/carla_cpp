#define _GLIBCXX_USE_CXX11_ABI 0

#include "CarlaTransformPublisher.h"// 包含CarlaTransformPublisher类的声明

#include <string>// 包含字符串处理功能
// 包含CARLA ROS2类型定义和监听器类
#include "carla/ros2/types/TFMessagePubSubTypes.h"
#include "carla/ros2/listeners/CarlaListener.h"
// 包含Fast-DDS（eProsima Fast RTPS的C++ API）的相关头文件
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
// 包含Fast-DDS的QoS（服务质量）策略配置相关的头文件
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
// 包含Fast-RTPS的基础配置和QoS策略相关的头文件
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/qos/QosPolicies.h>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>

/**
 * @namespace carla::ros2
 * @brief 命名空间，包含CARLA与ROS2桥接相关的类和功能。
 */
namespace carla {
namespace ros2 {
    /// 引入eprosima::fastdds::dds命名空间，并为其设置别名efd。
  namespace efd = eprosima::fastdds::dds;
  /// 为eprosima::fastrtps::types::ReturnCode_t类型设置别名erc。
  using erc = eprosima::fastrtps::types::ReturnCode_t;
  /**
   * @struct CarlaTransformPublisherImpl
   * @brief CarlaTransformPublisher的内部实现结构体。
   *
   * 该结构体包含了Fast-DDS相关的资源指针，以及用于发布变换信息的成员变量。
   */
  struct CarlaTransformPublisherImpl {
      /// Fast-DDS的DomainParticipant指针。
    efd::DomainParticipant* _participant { nullptr };
    /// Fast-DDS的Publisher指针。
    efd::Publisher* _publisher { nullptr };
    /// Fast-DDS的Topic指针。
    efd::Topic* _topic { nullptr };
    /// Fast-DDS的DataWriter指针。
    efd::DataWriter* _datawriter { nullptr };
    /// Fast-DDS的TypeSupport，用于注册TFMessage类型。
    efd::TypeSupport _type { new tf2_msgs::msg::TFMessagePubSubType() };
    /// CarlaListener对象，用于监听CARLA的消息。
    CarlaListener _listener {};
    /// 存储要发布的TFMessage对象。
    tf2_msgs::msg::TFMessage _transform {};
    /// 上次的位置信息（x, y, z）。
    float last_translation[3] = {0.0f};
    /// 上次的旋转信息（roll, pitch, yaw，但通常使用四元数表示旋转更为准确）。
    float last_rotation[3] = {0.0f};
    /// 位置信息的ROS2消息表示。
    geometry_msgs::msg::Vector3 vec_translation;
    /// 旋转信息的ROS2消息表示（使用四元数）。
    geometry_msgs::msg::Quaternion vec_rotation;
  };
  /**
 * @brief 初始化CarlaTransformPublisher对象
 *
 * 该函数负责初始化CarlaTransformPublisher对象，包括设置DomainParticipant、Publisher、Topic和DataWriter。
 * 如果在初始化过程中遇到任何错误，函数将输出错误信息并返回false。
 *
 * @return bool 如果初始化成功，则返回true；否则返回false。
 */
  bool CarlaTransformPublisher::Init() {
      /**
     * 检查_type是否为nullptr。如果是，则表示TypeSupport无效，输出错误信息并返回false。
     */
    if (_impl->_type == nullptr) {
        std::cerr << "Invalid TypeSupport" << std::endl;
        return false;
    }
    /**
     * 设置DomainParticipant的QoS策略，并使用默认QoS创建一个DomainParticipant。
     * 设置DomainParticipant的名称为_name。
     */
    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    pqos.name(_name);
    auto factory = efd::DomainParticipantFactory::get_instance();
    _impl->_participant = factory->create_participant(0, pqos);
    /**
     * 如果DomainParticipant创建失败，则输出错误信息并返回false。
     */
    if (_impl->_participant == nullptr) {
        std::cerr << "Failed to create DomainParticipant" << std::endl;
        return false;
    }
    /**
    * 使用_type注册类型到DomainParticipant。
    */
    _impl->_type.register_type(_impl->_participant);
    /**
     * 设置Publisher的QoS策略，并使用默认QoS创建一个Publisher。
     */
    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
    _impl->_publisher = _impl->_participant->create_publisher(pubqos, nullptr);
    /**
     * 如果Publisher创建失败，则输出错误信息并返回false。
     */
    if (_impl->_publisher == nullptr) {
      std::cerr << "Failed to create Publisher" << std::endl;
      return false;
    }
    /**
    * 设置Topic的QoS策略，并创建一个名为"rt/tf"的Topic。
    */
    efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
    const std::string topic_name { "rt/tf" };
    _impl->_topic = _impl->_participant->create_topic(topic_name, _impl->_type->getName(), tqos);
    /**
     * 如果Topic创建失败，则输出错误信息并返回false。
     */
    if (_impl->_topic == nullptr) {
        std::cerr << "Failed to create Topic" << std::endl;
        return false;
    }
    /**
     * 设置DataWriter的QoS策略，并创建一个DataWriter。
     * 将history_memory_policy设置为PREALLOCATED_WITH_REALLOC_MEMORY_MODE。
     */
    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
    wqos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    efd::DataWriterListener* listener = (efd::DataWriterListener*)_impl->_listener._impl.get();
    _impl->_datawriter = _impl->_publisher->create_datawriter(_impl->_topic, wqos, listener);
    /**
     * 如果DataWriter创建失败，则输出错误信息并返回false。
     */
    if (_impl->_datawriter == nullptr) {
        std::cerr << "Failed to create DataWriter" << std::endl;
        return false;
    }
    /**
     * 设置_frame_id为_name。
     */
    _frame_id = _name;
    /**
     * 初始化成功，返回true。
     */
    return true;
  }

  bool CarlaTransformPublisher::Publish() {
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    eprosima::fastrtps::types::ReturnCode_t rcode = _impl->_datawriter->write(&_impl->_transform, instance_handle);
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

  void CarlaTransformPublisher::SetData(int32_t seconds, uint32_t nanoseconds, const float* translation, const float* rotation) {

    int same_translation = std::memcmp(translation, _impl->last_translation, sizeof(float) * 3);
    int same_rotation = std::memcmp(rotation, _impl->last_rotation, sizeof(float) * 3);
    if (same_translation != 0 || same_rotation != 0) {
        std::memcpy(_impl->last_translation, translation, sizeof(float) * 3);
        std::memcpy(_impl->last_rotation, rotation, sizeof(float) * 3);

        const float tx = *translation++;
        const float ty = *translation++;
        const float tz = *translation++;

        const float rx = ((*rotation++) * -1.0f) * (M_PIf32 / 180.0f);
        const float ry = ((*rotation++) * -1.0f) * (M_PIf32 / 180.0f);
        const float rz = *rotation++ * (M_PIf32 / 180.0f);

        const float cr = cosf(rz * 0.5f);
        const float sr = sinf(rz * 0.5f);
        const float cp = cosf(rx * 0.5f);
        const float sp = sinf(rx * 0.5f);
        const float cy = cosf(ry * 0.5f);
        const float sy = sinf(ry * 0.5f);

        _impl->vec_translation.x(tx);
        _impl->vec_translation.y(-ty);
        _impl->vec_translation.z(tz);

        _impl->vec_rotation.w(cr * cp * cy + sr * sp * sy);
        _impl->vec_rotation.x(sr * cp * cy - cr * sp * sy);
        _impl->vec_rotation.y(cr * sp * cy + sr * cp * sy);
        _impl->vec_rotation.z(cr * cp * sy - sr * sp * cy);
    }

    builtin_interfaces::msg::Time time;
    time.sec(seconds);
    time.nanosec(nanoseconds);

    std_msgs::msg::Header header;
    header.stamp(std::move(time));
    header.frame_id(_parent);

    geometry_msgs::msg::Transform t;
    t.rotation(_impl->vec_rotation);
    t.translation(_impl->vec_translation);

    geometry_msgs::msg::TransformStamped ts;
    ts.header(std::move(header));
    ts.transform(std::move(t));
    ts.child_frame_id(_frame_id);
    _impl->_transform.transforms({ts});
  }

  CarlaTransformPublisher::CarlaTransformPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaTransformPublisherImpl>()) {
    _name = ros_name;
    _parent = parent;
  }

  CarlaTransformPublisher::~CarlaTransformPublisher() {
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

  CarlaTransformPublisher::CarlaTransformPublisher(const CarlaTransformPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
  }

  CarlaTransformPublisher& CarlaTransformPublisher::operator=(const CarlaTransformPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;

    return *this;
  }

  CarlaTransformPublisher::CarlaTransformPublisher(CarlaTransformPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
  }

  CarlaTransformPublisher& CarlaTransformPublisher::operator=(CarlaTransformPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);

    return *this;
  }
}
}
