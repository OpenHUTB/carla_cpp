#define _GLIBCXX_USE_CXX11_ABI 0

#include "CarlaRadarPublisher.h"

#include <string>

#include "carla/sensor/data/RadarData.h"/// @brief 包含CARLA雷达数据结构的头文件。
#include "carla/ros2/types/PointCloud2PubSubTypes.h"/// @brief 包含ROS 2点云发布/订阅类型的头文件。
#include "carla/ros2/listeners/CarlaListener.h"/// @brief 包含CARLA监听器类的头文件。

#include <fastdds/dds/domain/DomainParticipant.hpp>/// @brief 包含Fast-DDS域参与者的头文件。
#include <fastdds/dds/publisher/Publisher.hpp> /// @brief 包含Fast-DDS发布者的头文件。
#include <fastdds/dds/topic/Topic.hpp> /// @brief 包含Fast-DDS主题的头文件。
#include <fastdds/dds/publisher/DataWriter.hpp>/// @brief 包含Fast-DDS数据写入器的头文件。
#include <fastdds/dds/topic/TypeSupport.hpp>/// @brief 包含Fast-DDS类型支持的头文件。

#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>/// @brief 包含Fast-DDS域参与者服务质量（QoS）的头文件。
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>/// @brief 包含Fast-DDS域参与者工厂的头文件。
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>/// @brief 包含Fast-DDS发布者服务质量（QoS）的头文件。
#include <fastdds/dds/topic/qos/TopicQos.hpp>/// @brief 包含Fast-DDS主题服务质量（QoS）的头文件。

#include <fastrtps/attributes/ParticipantAttributes.h>/// @brief 包含Fast-RTPS参与者属性的头文件。
#include <fastrtps/qos/QosPolicies.h>/// @brief 包含Fast-RTPS服务质量（QoS）策略的头文件。
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>/// @brief 包含Fast-DDS数据写入器服务质量（QoS）的头文件。
#include <fastdds/dds/publisher/DataWriterListener.hpp>/// @brief 包含Fast-DDS数据写入器监听器的头文件。

/**
 * @namespace carla::ros2
 * @brief 包含CARLA与ROS 2集成相关功能的命名空间。
 */
namespace carla {
namespace ros2 {
    /**
  * @brief 简化eprosima::fastdds::dds命名空间的别名，以便代码更简洁。
  */
  namespace efd = eprosima::fastdds::dds;
  /**
   * @brief 简化eprosima::fastrtps::types::ReturnCode_t类型的别名，用于错误码返回。
   */
  using erc = eprosima::fastrtps::types::ReturnCode_t;
  /**
   * @struct CarlaRadarPublisherImpl
   * @brief CarlaRadarPublisher的内部实现结构体，封装了Fast-DDS发布雷达数据所需的对象。
   */
  struct CarlaRadarPublisherImpl {
      /**
     * @brief Fast-DDS域参与者对象指针。
     */
    efd::DomainParticipant* _participant { nullptr };
    /**
     * @brief Fast-DDS发布者对象指针。
     */
    efd::Publisher* _publisher { nullptr };
    /**
     * @brief Fast-DDS主题对象指针。
     */
    efd::Topic* _topic { nullptr };
    /**
     * @brief Fast-DDS数据写入器对象指针。
     */
    efd::DataWriter* _datawriter { nullptr };
    /**
     * @brief Fast-DDS类型支持对象，用于注册ROS 2点云消息类型。
     */
    efd::TypeSupport _type { new sensor_msgs::msg::PointCloud2PubSubType() };
    /**
    * @brief CARLA监听器对象，用于接收来自CARLA的雷达数据。
    */
    CarlaListener _listener {};
    /**
     * @brief 存储雷达数据的ROS 2点云消息对象。
     */
    sensor_msgs::msg::PointCloud2 _radar {};
  };
  /**
   * @struct RadarDetectionWithPosition
   * @brief 包含雷达检测及其位置信息的结构体。
   */
  struct RadarDetectionWithPosition {
      /**
     * @brief 检测到的目标在X轴上的位置。
     */
    float x;
    /**
     * @brief 检测到的目标在Y轴上的位置。
     */
    float y;
    /**
     * @brief 检测到的目标在Z轴上的位置。
     */
    float z;
    /**
     * @brief CARLA雷达检测数据。
     */
    carla::sensor::data::RadarDetection detection;
  };
  /**
   * @brief 初始化CarlaRadarPublisher对象。
   *
   * 此函数负责创建Fast-DDS域参与者、发布者、主题和数据写入器，并注册ROS 2点云消息类型。
   *
   * @return 如果初始化成功，则返回true；否则返回false。
   */
  bool CarlaRadarPublisher::Init() {
      /**
   * 检查类型支持是否有效。如果_impl->_type为nullptr，表示类型支持无效，打印错误信息并返回false。
   */
    if (_impl->_type == nullptr) {
        std::cerr << "Invalid TypeSupport" << std::endl;
        return false;
    }
    /**
   * 设置域参与者（DomainParticipant）的质量服务（Qos）参数，使用默认值，并设置名称。
   */
    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    pqos.name(_name); // 设置域参与者的名称
    auto factory = efd::DomainParticipantFactory::get_instance();
    _impl->_participant = factory->create_participant(0, pqos);
    /**
   * 检查域参与者是否创建成功。如果为nullptr，表示创建失败，打印错误信息并返回false。
   */
    if (_impl->_participant == nullptr) {
        std::cerr << "Failed to create DomainParticipant" << std::endl;
        return false;
    }
    /**
   * 注册类型支持到域参与者。
   */
    _impl->_type.register_type(_impl->_participant);// 注册类型支持
    /**
   * 设置发布者（Publisher）的质量服务（Qos）参数，使用默认值。
   */
    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
    /**
   * 创建发布者。
   */
    _impl->_publisher = _impl->_participant->create_publisher(pubqos, nullptr);
    /**
   * 检查发布者是否创建成功。如果为nullptr，表示创建失败，打印错误信息并返回false。
   */
    if (_impl->_publisher == nullptr) {
      std::cerr << "Failed to create Publisher" << std::endl;
      return false;
    }
    /**
   * 设置主题（Topic）的质量服务（Qos）参数，使用默认值。
   */
    efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
    /**
   * 构造主题名称，根据基础名称"rt/carla/"和可能的父级名称以及本对象的名称。
   */
    const std::string base { "rt/carla/" };
    std::string topic_name = base;
    if (!_parent.empty())
      topic_name += _parent + "/";
    topic_name += _name;
    /**
   * 创建主题。
   */
    _impl->_topic = _impl->_participant->create_topic(topic_name, _impl->_type->getName(), tqos);
    /**
   * 检查主题是否创建成功。如果为nullptr，表示创建失败，打印错误信息并返回false。
   */
    if (_impl->_topic == nullptr) {
        std::cerr << "Failed to create Topic" << std::endl;
        return false;
    }
    /**
   * 设置数据写入器（DataWriter）的质量服务（Qos）参数，使用默认值，并设置历史内存策略为预分配并重新分配模式。
   */
    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
    wqos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    /**
  * 获取数据写入器监听器实例。
  */
    efd::DataWriterListener* listener = (efd::DataWriterListener*)_impl->_listener._impl.get();
    /**
   * 创建数据写入器。
   */
    _impl->_datawriter = _impl->_publisher->create_datawriter(_impl->_topic, wqos, listener);
    /**
   * 检查数据写入器是否创建成功。如果为nullptr，表示创建失败，打印错误信息并返回false。
   */
    if (_impl->_datawriter == nullptr) {
        std::cerr << "Failed to create DataWriter" << std::endl;
        return false;
    }
    // 设置帧ID
    _frame_id = _name;
    /**
   * 初始化成功，返回true。
   */
    return true;
  }
  /**
 * @brief 发布雷达数据
 *
 * 此函数尝试通过Fast-RTPS发布雷达数据。根据返回代码，函数会返回不同的结果，并在控制台上打印相应的错误信息。
 *
 * @return true 如果数据成功发布
 * @return false 如果数据发布失败，并根据返回代码打印错误信息
 */
  bool CarlaRadarPublisher::Publish() {
      /// Fast-RTPS实例句柄
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    /// 尝试写入雷达数据到数据写入器，并获取返回代码
    eprosima::fastrtps::types::ReturnCode_t rcode = _impl->_datawriter->write(&_impl->_radar, instance_handle);
    /// 根据返回代码处理不同的结果
    if (rcode == erc::ReturnCodeValue::RETCODE_OK) {
        /// 数据成功发布
        return true;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ERROR) {
        /// 通用错误
        std::cerr << "RETCODE_ERROR" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_UNSUPPORTED) {
        /// 操作不支持
        std::cerr << "RETCODE_UNSUPPORTED" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_BAD_PARAMETER) {
        /// 参数错误
        std::cerr << "RETCODE_BAD_PARAMETER" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_PRECONDITION_NOT_MET) {
        /// 先决条件未满足
        std::cerr << "RETCODE_PRECONDITION_NOT_MET" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_OUT_OF_RESOURCES) {
        /// 资源不足
        std::cerr << "RETCODE_OUT_OF_RESOURCES" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ENABLED) {
        /// 功能未启用
        std::cerr << "RETCODE_NOT_ENABLED" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_IMMUTABLE_POLICY) {
        /// 不可变策略
        std::cerr << "RETCODE_IMMUTABLE_POLICY" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_INCONSISTENT_POLICY) {
        /// 策略不一致
        std::cerr << "RETCODE_INCONSISTENT_POLICY" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ALREADY_DELETED) {
        /// 已被删除
        std::cerr << "RETCODE_ALREADY_DELETED" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_TIMEOUT) {
        /// 超时
        std::cerr << "RETCODE_TIMEOUT" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NO_DATA) {
        /// 无数据
        std::cerr << "RETCODE_NO_DATA" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ILLEGAL_OPERATION) {
        /// 非法操作
        std::cerr << "RETCODE_ILLEGAL_OPERATION" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ALLOWED_BY_SECURITY) {
        /// 安全策略不允许
        std::cerr << "RETCODE_NOT_ALLOWED_BY_SECURITY" << std::endl;
        return false;
    }
    /// 未知错误
    std::cerr << "UNKNOWN" << std::endl;
    return false;
  }

void CarlaRadarPublisher::SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, size_t elements, const uint8_t* data) {

    std::vector<uint8_t> vector_data;
    const size_t size = elements * sizeof(RadarDetectionWithPosition);
    vector_data.resize(size);
    RadarDetectionWithPosition* radar_data = (RadarDetectionWithPosition*)&vector_data[0];
    carla::sensor::data::RadarDetection* detection_data = (carla::sensor::data::RadarDetection*)data;
    for (size_t i = 0; i < elements; ++i, ++radar_data, ++detection_data) {
      radar_data->x = detection_data->depth * cosf(detection_data->azimuth) * cosf(-detection_data->altitude);
      radar_data->y = detection_data->depth * sinf(-detection_data->azimuth) * cosf(detection_data->altitude);
      radar_data->z = detection_data->depth * sinf(detection_data->altitude);
      radar_data->detection = *detection_data;
    }

    SetData(seconds, nanoseconds, height, width, elements, std::move(vector_data));
  }

  void CarlaRadarPublisher::SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, size_t elements, std::vector<uint8_t>&& data) {
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
    descriptor4.name("velocity");
    descriptor4.offset(12);
    descriptor4.datatype(sensor_msgs::msg::PointField__FLOAT32);
    descriptor4.count(1);
    sensor_msgs::msg::PointField descriptor5;
    descriptor5.name("azimuth");
    descriptor5.offset(16);
    descriptor5.datatype(sensor_msgs::msg::PointField__FLOAT32);
    descriptor5.count(1);
    sensor_msgs::msg::PointField descriptor6;
    descriptor6.name("altitude");
    descriptor6.offset(20);
    descriptor6.datatype(sensor_msgs::msg::PointField__FLOAT32);
    descriptor6.count(1);
    sensor_msgs::msg::PointField descriptor7;
    descriptor7.name("depth");
    descriptor7.offset(24);
    descriptor7.datatype(sensor_msgs::msg::PointField__FLOAT32);
    descriptor7.count(1);

    const size_t point_size = sizeof(RadarDetectionWithPosition);
    _impl->_radar.header(std::move(header));
    _impl->_radar.width(elements);
    _impl->_radar.height(height);
    _impl->_radar.is_bigendian(false);
    _impl->_radar.fields({descriptor1, descriptor2, descriptor3, descriptor4, descriptor5, descriptor6, descriptor7});
    _impl->_radar.point_step(point_size);
    _impl->_radar.row_step(elements * point_size);
    _impl->_radar.is_dense(false);
    _impl->_radar.data(std::move(data));
  }

  CarlaRadarPublisher::CarlaRadarPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaRadarPublisherImpl>()) {
    _name = ros_name;
    _parent = parent;
  }

  CarlaRadarPublisher::~CarlaRadarPublisher() {
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

  CarlaRadarPublisher::CarlaRadarPublisher(const CarlaRadarPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
  }

  CarlaRadarPublisher& CarlaRadarPublisher::operator=(const CarlaRadarPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;

    return *this;
  }

  CarlaRadarPublisher::CarlaRadarPublisher(CarlaRadarPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
  }

  CarlaRadarPublisher& CarlaRadarPublisher::operator=(CarlaRadarPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);

    return *this;
  }
}
}
