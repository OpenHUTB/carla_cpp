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
  /// @brief 发布激光雷达数据
/// 
/// 该函数尝试通过DDS（数据分发服务）发布激光雷达数据。
/// 根据返回码，函数会返回发布是否成功。
/// 
/// @return 发布成功返回true，否则返回false。
  bool CarlaLidarPublisher::Publish() {
      /// @brief DDS实例句柄
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    /// @brief 写入数据并获取返回码
    /// 
    /// 尝试将激光雷达数据写入DDS，并获取操作返回码。
    eprosima::fastrtps::types::ReturnCode_t rcode = _impl->_datawriter->write(&_impl->_lidar, instance_handle);
    /// @brief 根据返回码处理结果
    if (rcode == erc::ReturnCodeValue::RETCODE_OK) {
        /// @details 操作成功，返回true
        return true;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ERROR) {
        /// @details 发生错误，打印错误信息并返回false
        std::cerr << "RETCODE_ERROR" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_UNSUPPORTED) {
        /// @details 操作不支持，打印错误信息并返回false
        std::cerr << "RETCODE_UNSUPPORTED" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_BAD_PARAMETER) {
        /// @details 参数错误，打印错误信息并返回false
        std::cerr << "RETCODE_BAD_PARAMETER" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_PRECONDITION_NOT_MET) {
        /// @details 前置条件未满足，打印错误信息并返回false
        std::cerr << "RETCODE_PRECONDITION_NOT_MET" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_OUT_OF_RESOURCES) {
        /// @details 资源不足，打印错误信息并返回false
        std::cerr << "RETCODE_OUT_OF_RESOURCES" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ENABLED) {
        /// @details 功能未启用，打印错误信息并返回false
        std::cerr << "RETCODE_NOT_ENABLED" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_IMMUTABLE_POLICY) {
        /// @details 策略不可变，打印错误信息并返回false
        std::cerr << "RETCODE_IMMUTABLE_POLICY" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_INCONSISTENT_POLICY) {
        /// @details 策略不一致，打印错误信息并返回false
        std::cerr << "RETCODE_INCONSISTENT_POLICY" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ALREADY_DELETED) {
        /// @details 对象已被删除，打印错误信息并返回false
        std::cerr << "RETCODE_ALREADY_DELETED" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_TIMEOUT) {
        /// @details 操作超时，打印错误信息并返回false
        std::cerr << "RETCODE_TIMEOUT" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NO_DATA) {
        /// @details 无数据，打印错误信息并返回false
        std::cerr << "RETCODE_NO_DATA" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ILLEGAL_OPERATION) {
        /// @details 非法操作，打印错误信息并返回false
        std::cerr << "RETCODE_ILLEGAL_OPERATION" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ALLOWED_BY_SECURITY) {
        /// @details 安全策略不允许，打印错误信息并返回false
        std::cerr << "RETCODE_NOT_ALLOWED_BY_SECURITY" << std::endl;
        return false;
    }
    /// @details 未知错误，打印未知错误信息并返回false
    std::cerr << "UNKNOWN" << std::endl;
    return false;
  }

  /**
 * @brief 设置激光雷达数据，处理并转换数据类型
 *
 * 该函数接收激光雷达的原始浮点数据，将其中的某些值取反，然后转换为字节向量，
 * 并调用另一个重载的 SetData 函数来设置激光雷达数据。
 *
 * @param seconds 时间戳的秒部分
 * @param nanoseconds 时间戳的纳秒部分
 * @param height 数据的高度（行数）
 * @param width 数据的宽度（列数），假设每个点包含4个浮数值（x, y, z, intensity）
 * @param data 指向浮点数据数组的指针
 */
void CarlaLidarPublisher::SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, float* data) {
    float* it = data;
    float* end = &data[height * width];
    for (++it; it < end; it += 4) {
        *it *= -1.0f;// 将y值取反（假设data[1]是y值）
    }
    std::vector<uint8_t> vector_data;
    const size_t size = height * width * sizeof(float);
    vector_data.resize(size);
    std::memcpy(&vector_data[0], &data[0], size);// 将浮点数据复制到字节向量中
    // 调用重载的SetData函数来设置处理后的数据
    SetData(seconds, nanoseconds, height, width, std::move(vector_data));
  }
/**
 * @brief 设置激光雷达数据
 *
 * 该函数接收时间戳、数据的高度和宽度，以及处理后的字节数据，
 * 然后设置激光雷达消息的各个字段。
 *
 * @param seconds 时间戳的秒部分
 * @param nanoseconds 时间戳的纳秒部分
 * @param height 数据的高度（行数）
 * @param width 数据的宽度（每行点数），注意这里已经是处理后的宽度（原始宽度的1/4）
 * @param data 包含处理后的激光雷达数据的字节向量，按点（x, y, z, intensity）组织
 */
  void CarlaLidarPublisher::SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, std::vector<uint8_t>&& data) {
    builtin_interfaces::msg::Time time;
    time.sec(seconds);
    time.nanosec(nanoseconds);

    std_msgs::msg::Header header;
    header.stamp(std::move(time));
    header.frame_id(_frame_id);// 设置帧ID
    // 设置点云数据的描述字段
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

    const size_t point_size = 4 * sizeof(float);// 每个点的大小（字节）
    _impl->_lidar.header(std::move(header));// 设置消息头
    _impl->_lidar.width(width / 4);// 设置宽度（每行点数）
    _impl->_lidar.height(height);// 设置高度（行数）
    _impl->_lidar.is_bigendian(false);// 设置字节序
    _impl->_lidar.fields({descriptor1, descriptor2, descriptor3, descriptor4});// 设置点字段描述
    _impl->_lidar.point_step(point_size);// 设置每个点的步长
    _impl->_lidar.row_step(width * sizeof(float));// 设置每行的步长
    _impl->_lidar.is_dense(false); // 设置是否稠密（True表示没有无效点）
    _impl->_lidar.data(std::move(data));// 设置点云数据
  }
  /**
 * @brief CarlaLidarPublisher 类的构造函数
 *
 * 初始化 CarlaLidarPublisher 实例，并创建 CarlaLidarPublisherImpl 实现类的共享指针
 *
 * @param ros_name ROS 节点名称
 * @param parent 父节点的名称
 */
  CarlaLidarPublisher::CarlaLidarPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaLidarPublisherImpl>()) {
    _name = ros_name;///< ROS 节点名称
    _parent = parent;///< 父节点名称
  }
  /**
 * @brief CarlaLidarPublisher 类的析构函数
 *
 * 释放所有分配的资源，包括数据写入器、发布者、主题和参与者
 */
  CarlaLidarPublisher::~CarlaLidarPublisher() {
      if (!_impl)
          return;

      if (_impl->_datawriter)
          _impl->_publisher->delete_datawriter(_impl->_datawriter);///< 删除数据写入器

      if (_impl->_publisher)
          _impl->_participant->delete_publisher(_impl->_publisher);///< 删除发布者

      if (_impl->_topic)
          _impl->_participant->delete_topic(_impl->_topic); ///< 删除主题

      if (_impl->_participant)
          efd::DomainParticipantFactory::get_instance()->delete_participant(_impl->_participant); ///< 删除参与者
  }
  /**
 * @brief CarlaLidarPublisher 类的拷贝构造函数
 *
 * 使用另一个 CarlaLidarPublisher 实例初始化新实例
 *
 * @param other 要拷贝的 CarlaLidarPublisher 实例
 */
  CarlaLidarPublisher::CarlaLidarPublisher(const CarlaLidarPublisher& other) {
    _frame_id = other._frame_id;///< 拷贝帧 ID
    _name = other._name;///< 拷贝 ROS 节点名称
    _parent = other._parent;///< 拷贝父节点名称
    _impl = other._impl;///< 共享实现类的指针
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
