#define _GLIBCXX_USE_CXX11_ABI 0

#include "CarlaSemanticLidarPublisher.h"// 引入Carla语义激光雷达发布者类的声明

#include <string>// 引入字符串处理相关的标准库
// 引入CARLA ROS2桥接库中的点云数据类型和监听器类
#include "carla/ros2/types/PointCloud2PubSubTypes.h"
#include "carla/ros2/listeners/CarlaListener.h"
// 引入Fast-DDS库中的相关类和头文件
#include <fastdds/dds/domain/DomainParticipant.hpp>// 引入域参与者类
#include <fastdds/dds/publisher/Publisher.hpp>// 引入发布者类
#include <fastdds/dds/topic/Topic.hpp>// 引入话题类
#include <fastdds/dds/publisher/DataWriter.hpp>// 引入数据写入器类
#include <fastdds/dds/topic/TypeSupport.hpp>// 引入类型支持类
// 引入Fast-DDS库中与QoS（服务质量）相关的类和头文件
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>// 引入域参与者QoS类
#include <fastdds/dds/domain/DomainParticipantFactory.hpp> // 引入域参与者工厂类
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>// 引入发布者QoS类
#include <fastdds/dds/topic/qos/TopicQos.hpp>// 引入话题QoS类
// 引入Fast-RTPS库中的参与者属性和QoS策略相关的类和头文件
#include <fastrtps/attributes/ParticipantAttributes.h>// 引入参与者属性类
#include <fastrtps/qos/QosPolicies.h>  // 引入QoS策略类
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>// 引入数据写入器QoS类
#include <fastdds/dds/publisher/DataWriterListener.hpp>// 引入数据写入器监听器类

/**
 * @namespace carla::ros2
 * @brief 命名空间，包含CARLA与ROS2桥接相关的类和函数。
 */
namespace carla {
namespace ros2 {
    /**
   * @brief 引入Fast-DDS命名空间的别名。
   */
  namespace efd = eprosima::fastdds::dds;
  /**
   * @brief 引入Fast-RTPS返回码类型的别名。
   */
  using erc = eprosima::fastrtps::types::ReturnCode_t;
  /**
  * @struct CarlaSemanticLidarPublisherImpl
  * @brief Carla语义激光雷达数据发布者的内部实现结构。
  *
  * 该结构包含了Fast-DDS所需的域参与者、发布者、话题和数据写入器等成员变量，以及一个CARLA监听器和一个用于存储点云数据的成员变量。
  */
  struct CarlaSemanticLidarPublisherImpl {
    efd::DomainParticipant* _participant { nullptr };///< 域参与者指针
    efd::Publisher* _publisher { nullptr };///< 发布者指针
    efd::Topic* _topic { nullptr };///< 话题指针
    efd::DataWriter* _datawriter { nullptr };///< 数据写入器指针
    efd::TypeSupport _type { new sensor_msgs::msg::PointCloud2PubSubType() };///< 类型支持，用于点云数据
    CarlaListener _listener {};///< CARLA监听器
    sensor_msgs::msg::PointCloud2 _lidar {}; ///< 存储点云数据的成员变量
  };
  /**
   * @brief 初始化Carla语义激光雷达数据发布者。
   *
   * 该函数负责创建Fast-DDS的域参与者、发布者、话题和数据写入器，并注册类型支持。
   *
   * @return bool 如果初始化成功，则返回true；否则返回false。
   */
  bool CarlaSemanticLidarPublisher::Init() {
      // 检查类型支持是否有效
    if (_impl->_type == nullptr) {
        std::cerr << "Invalid TypeSupport" << std::endl;
        return false;
    }
    // 设置域参与者的QoS策略，并创建域参与者
    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    pqos.name(_name);
    auto factory = efd::DomainParticipantFactory::get_instance();
    _impl->_participant = factory->create_participant(0, pqos);
    if (_impl->_participant == nullptr) {
        std::cerr << "Failed to create DomainParticipant" << std::endl;
        return false;
    }
    // 注册类型支持
    _impl->_type.register_type(_impl->_participant);
    // 设置发布者的QoS策略，并创建发布者
    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
    _impl->_publisher = _impl->_participant->create_publisher(pubqos, nullptr);
    if (_impl->_publisher == nullptr) {
      std::cerr << "Failed to create Publisher" << std::endl;
      return false;
    }
    
    efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
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
    // 设置数据写入器的QoS策略，并创建数据写入器
    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
    wqos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    efd::DataWriterListener* listener = (efd::DataWriterListener*)_impl->_listener._impl.get();
    _impl->_datawriter = _impl->_publisher->create_datawriter(_impl->_topic, wqos, listener);
    if (_impl->_datawriter == nullptr) {
        std::cerr << "Failed to create DataWriter" << std::endl;
        return false;
    }
    // 设置帧ID为发布者的名称
    _frame_id = _name;
    // 初始化成功
    return true;
  }

  bool CarlaSemanticLidarPublisher::Publish() {
      /// @var instance_handle
    /// 用于存储Fast RTPS实例句柄的变量。
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    /// @var rcode
   /// 用于存储写操作返回码的变量。
    eprosima::fastrtps::types::ReturnCode_t rcode = _impl->_datawriter->write(&_impl->_lidar, instance_handle);
    /// 检查返回码，如果是RETCODE_OK，表示发布成功。
    if (rcode == erc::ReturnCodeValue::RETCODE_OK) {
        return true;
    }
    /// 返回码为RETCODE_ERROR，表示发生了错误。
    if (rcode == erc::ReturnCodeValue::RETCODE_ERROR) {
        std::cerr << "RETCODE_ERROR" << std::endl;
        return false;
    }
    /// 返回码为RETCODE_UNSUPPORTED，表示操作不受支持。
    if (rcode == erc::ReturnCodeValue::RETCODE_UNSUPPORTED) {
        std::cerr << "RETCODE_UNSUPPORTED" << std::endl;
        return false;
    }
    /// 返回码为RETCODE_BAD_PARAMETER，表示参数错误。
    if (rcode == erc::ReturnCodeValue::RETCODE_BAD_PARAMETER) {
        std::cerr << "RETCODE_BAD_PARAMETER" << std::endl;
        return false;
    }
    /// 返回码为RETCODE_PRECONDITION_NOT_MET，表示前提条件未满足。
    if (rcode == erc::ReturnCodeValue::RETCODE_PRECONDITION_NOT_MET) {
        std::cerr << "RETCODE_PRECONDITION_NOT_MET" << std::endl;
        return false;
    }
    /// 返回码为RETCODE_OUT_OF_RESOURCES，表示资源不足。
    if (rcode == erc::ReturnCodeValue::RETCODE_OUT_OF_RESOURCES) {
        std::cerr << "RETCODE_OUT_OF_RESOURCES" << std::endl;
        return false;
    }
    /// 返回码为RETCODE_NOT_ENABLED，表示功能未启用。
    if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ENABLED) {
        std::cerr << "RETCODE_NOT_ENABLED" << std::endl;
        return false;
    }
    /// 返回码为RETCODE_IMMUTABLE_POLICY，表示策略不可变。
    if (rcode == erc::ReturnCodeValue::RETCODE_IMMUTABLE_POLICY) {
        std::cerr << "RETCODE_IMMUTABLE_POLICY" << std::endl;
        return false;
    }
    /// 返回码为RETCODE_INCONSISTENT_POLICY，表示策略不一致。
    if (rcode == erc::ReturnCodeValue::RETCODE_INCONSISTENT_POLICY) {
        std::cerr << "RETCODE_INCONSISTENT_POLICY" << std::endl;
        return false;
    }
    /// 返回码为RETCODE_ALREADY_DELETED，表示对象已被删除。
    if (rcode == erc::ReturnCodeValue::RETCODE_ALREADY_DELETED) {
        std::cerr << "RETCODE_ALREADY_DELETED" << std::endl;
        return false;
    }
    /// 返回码为RETCODE_TIMEOUT，表示操作超时。
    if (rcode == erc::ReturnCodeValue::RETCODE_TIMEOUT) {
        std::cerr << "RETCODE_TIMEOUT" << std::endl;
        return false;
    }
    /// 返回码为RETCODE_NO_DATA，表示没有数据。
    if (rcode == erc::ReturnCodeValue::RETCODE_NO_DATA) {
        std::cerr << "RETCODE_NO_DATA" << std::endl;
        return false;
    }
    /// 返回码为RETCODE_ILLEGAL_OPERATION，表示非法操作。
    if (rcode == erc::ReturnCodeValue::RETCODE_ILLEGAL_OPERATION) {
        std::cerr << "RETCODE_ILLEGAL_OPERATION" << std::endl;
        return false;
    }
    /// 返回码为RETCODE_NOT_ALLOWED_BY_SECURITY，表示安全策略不允许。
    if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ALLOWED_BY_SECURITY) {
        std::cerr << "RETCODE_NOT_ALLOWED_BY_SECURITY" << std::endl;
        return false;
    }
    /// 如果返回码未知，输出UNKNOWN错误信息。
    std::cerr << "UNKNOWN" << std::endl;
    return false;
  }

void CarlaSemanticLidarPublisher::SetData(int32_t seconds, uint32_t nanoseconds, size_t elements, size_t height, size_t width, float* data) {
    float* it = data;
    float* end = &data[height * width * elements];
    for (++it; it < end; it += elements) {
        *it *= -1.0f;
    }

    std::vector<uint8_t> vector_data;
    const size_t size = height * width * sizeof(float) * elements;
    vector_data.resize(size);
    std::memcpy(&vector_data[0], &data[0], size);
    SetData(seconds, nanoseconds, height, width, std::move(vector_data));
}

void CarlaSemanticLidarPublisher::SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, std::vector<uint8_t>&& data) {
    builtin_interfaces::msg::Time time;
    time.sec(seconds);
    time.nanosec(nanoseconds);

    std_msgs::msg::Header header;
    header.stamp(std::move(time));
    header.frame_id(_frame_id);

    sensor_msgs::msg::PointField descriptor1;
    descriptor1.name("x");
    descriptor1.offset(0);
    descriptor1.datatype(sensor_msgs::msg::PointField__FLOAT32);
    descriptor1.count(1);
    sensor_msgs::msg::PointField descriptor2;
    descriptor2.name("y");
    descriptor2.offset(4);
    descriptor2.datatype(sensor_msgs::msg::PointField__FLOAT32);
    descriptor2.count(1);
    sensor_msgs::msg::PointField descriptor3;
    descriptor3.name("z");
    descriptor3.offset(8);
    descriptor3.datatype(sensor_msgs::msg::PointField__FLOAT32);
    descriptor3.count(1);
    sensor_msgs::msg::PointField descriptor4;
    descriptor4.name("cos_inc_angle");
    descriptor4.offset(12);
    descriptor4.datatype(sensor_msgs::msg::PointField__FLOAT32);
    descriptor4.count(1);
    sensor_msgs::msg::PointField descriptor5;
    descriptor5.name("object_idx");
    descriptor5.offset(16);
    descriptor5.datatype(sensor_msgs::msg::PointField__UINT32);
    descriptor5.count(1);
    sensor_msgs::msg::PointField descriptor6;
    descriptor6.name("object_tag");
    descriptor6.offset(20);
    descriptor6.datatype(sensor_msgs::msg::PointField__UINT32);
    descriptor6.count(1);

    const size_t point_size = 6 * sizeof(float);
    _impl->_lidar.header(std::move(header));
    _impl->_lidar.width(width);
    _impl->_lidar.height(height);
    _impl->_lidar.is_bigendian(false);
    _impl->_lidar.fields({descriptor1, descriptor2, descriptor3, descriptor4, descriptor5, descriptor6});
    _impl->_lidar.point_step(point_size);
    _impl->_lidar.row_step(width * point_size);
    _impl->_lidar.is_dense(false); //True if there are not invalid points
    _impl->_lidar.data(std::move(data));
  }

  CarlaSemanticLidarPublisher::CarlaSemanticLidarPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaSemanticLidarPublisherImpl>()) {
    _name = ros_name;
    _parent = parent;
  }

  CarlaSemanticLidarPublisher::~CarlaSemanticLidarPublisher() {
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

  CarlaSemanticLidarPublisher::CarlaSemanticLidarPublisher(const CarlaSemanticLidarPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
  }

  CarlaSemanticLidarPublisher& CarlaSemanticLidarPublisher::operator=(const CarlaSemanticLidarPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;

    return *this;
  }

  CarlaSemanticLidarPublisher::CarlaSemanticLidarPublisher(CarlaSemanticLidarPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
  }

  CarlaSemanticLidarPublisher& CarlaSemanticLidarPublisher::operator=(CarlaSemanticLidarPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);

    return *this;
  }
}
}
