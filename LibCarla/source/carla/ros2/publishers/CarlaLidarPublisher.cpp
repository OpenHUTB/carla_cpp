#define _GLIBCXX_USE_CXX11_ABI 0

#include "CarlaLidarPublisher.h"// 包含 CarlaLidarPublisher 类的声明

#include <string>// 包含字符串处理功能
// 包含 CARLA ROS2 桥接所需的类型定义和监听器类
#include "carla/ros2/types/PointCloud2PubSubTypes.h"
#include "carla/ros2/listeners/CarlaListener.h"
// 包含 FastDDS 相关的头文件，用于 DDS 通信
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
// 包含 FastDDS QoS（服务质量）配置相关的头文件
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
// 包含 FastRTPS（FastDDS 的底层实现）的参与者属性和 QoS 策略相关的头文件
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/qos/QosPolicies.h>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
/**
 * @namespace carla::ros2
 * @brief 命名空间，包含CARLA与ROS2集成相关的类和函数。
 */
namespace carla {
namespace ros2 {
    /**
       * @brief 命名空间别名，简化eprosima::fastdds::dds的引用。
       */
  namespace efd = eprosima::fastdds::dds;
  /**
   * @brief 类型别名，简化eprosima::fastrtps::types::ReturnCode_t的引用。
   */
  using erc = eprosima::fastrtps::types::ReturnCode_t;
  /**
   * @struct CarlaLidarPublisherImpl
   * @brief CarlaLidarPublisher的内部实现结构体，封装了DDS通信所需的资源。
   */
  struct CarlaLidarPublisherImpl {
      /**
     * @brief DDS域参与者指针。
     */
    efd::DomainParticipant* _participant { nullptr };
    /**
     * @brief DDS发布者指针。
     */
    efd::Publisher* _publisher { nullptr };
    /**
     * @brief DDS主题指针。
     */
    efd::Topic* _topic { nullptr };
    /**
     * @brief DDS数据写入器指针。
     */
    efd::DataWriter* _datawriter { nullptr };
    /**
     * @brief DDS类型支持，用于PointCloud2消息。
     */
    efd::TypeSupport _type { new sensor_msgs::msg::PointCloud2PubSubType() };
    /**
     * @brief CARLA监听器，用于接收CARLA模拟器的数据。
     */
    CarlaListener _listener {};
    /**
     * @brief 存储激光雷达数据的PointCloud2消息。
     */
    sensor_msgs::msg::PointCloud2 _lidar {};
  };
  /**
   * @brief 初始化CarlaLidarPublisher。
   *
   * 该函数负责初始化DDS通信所需的资源，包括域参与者、发布者、主题和数据写入器。
   *
   * @return 初始化成功返回true，否则返回false。
   */
  bool CarlaLidarPublisher::Init() {
      // 检查类型支持是否有效
    if (_impl->_type == nullptr) {
        std::cerr << "Invalid TypeSupport" << std::endl;
        return false;
    }
    // 设置域参与者的QoS策略
    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    pqos.name(_name);
    // 创建域参与者
    auto factory = efd::DomainParticipantFactory::get_instance();
    _impl->_participant = factory->create_participant(0, pqos);
    if (_impl->_participant == nullptr) {
        std::cerr << "Failed to create DomainParticipant" << std::endl;
        return false;
    }
    // 注册类型支持
    _impl->_type.register_type(_impl->_participant);
    // 设置发布者的QoS策略
    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
    _impl->_publisher = _impl->_participant->create_publisher(pubqos, nullptr);
    if (_impl->_publisher == nullptr) {
      std::cerr << "Failed to create Publisher" << std::endl;
      return false;
    }
    // 设置主题的QoS策略
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
    // 设置数据写入器的QoS策略
    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
    wqos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    // 创建数据写入器，并传入自定义的监听器
    efd::DataWriterListener* listener = (efd::DataWriterListener*)_impl->_listener._impl.get();
    _impl->_datawriter = _impl->_publisher->create_datawriter(_impl->_topic, wqos, listener);
    if (_impl->_datawriter == nullptr) {
        std::cerr << "Failed to create DataWriter" << std::endl;
        return false;
    }
    // 设置帧ID为发布者的名称
    _frame_id = _name;
    return true;
  }

  bool CarlaLidarPublisher::Publish() {
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    eprosima::fastrtps::types::ReturnCode_t rcode = _impl->_datawriter->write(&_impl->_lidar, instance_handle);
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


void CarlaLidarPublisher::SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, float* data) {
    float* it = data;
    float* end = &data[height * width];
    for (++it; it < end; it += 4) {
        *it *= -1.0f;
    }
    std::vector<uint8_t> vector_data;
    const size_t size = height * width * sizeof(float);
    vector_data.resize(size);
    std::memcpy(&vector_data[0], &data[0], size);
    SetData(seconds, nanoseconds, height, width, std::move(vector_data));
  }

  void CarlaLidarPublisher::SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, std::vector<uint8_t>&& data) {
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
    descriptor4.name("intensity");
    descriptor4.offset(12);
    descriptor4.datatype(sensor_msgs::msg::PointField__FLOAT32);
    descriptor4.count(1);

    const size_t point_size = 4 * sizeof(float);
    _impl->_lidar.header(std::move(header));
    _impl->_lidar.width(width / 4);
    _impl->_lidar.height(height);
    _impl->_lidar.is_bigendian(false);
    _impl->_lidar.fields({descriptor1, descriptor2, descriptor3, descriptor4});
    _impl->_lidar.point_step(point_size);
    _impl->_lidar.row_step(width * sizeof(float));
    _impl->_lidar.is_dense(false); //True if there are not invalid points
    _impl->_lidar.data(std::move(data));
  }

  CarlaLidarPublisher::CarlaLidarPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaLidarPublisherImpl>()) {
    _name = ros_name;
    _parent = parent;
  }

  CarlaLidarPublisher::~CarlaLidarPublisher() {
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

  CarlaLidarPublisher::CarlaLidarPublisher(const CarlaLidarPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
  }

  CarlaLidarPublisher& CarlaLidarPublisher::operator=(const CarlaLidarPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;

    return *this;
  }

  CarlaLidarPublisher::CarlaLidarPublisher(CarlaLidarPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
  }

  CarlaLidarPublisher& CarlaLidarPublisher::operator=(CarlaLidarPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);

    return *this;
  }
}
}
