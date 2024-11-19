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
  /**
 * @brief 发布Carla的变换信息
 *
 * 该函数负责使用DataWriter发布_impl->_transform中的数据。根据返回码（ReturnCode_t）处理不同的错误情况，
 * 并输出相应的错误信息。如果发布成功，则返回true；否则返回false。
 *
 * @return bool 发布成功返回true，否则返回false。
 */
  bool CarlaTransformPublisher::Publish() {
      /**
     * 声明一个InstanceHandle_t类型的变量instance_handle，用于接收write方法的返回值。
     */
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    eprosima::fastrtps::types::ReturnCode_t rcode = _impl->_datawriter->write(&_impl->_transform, instance_handle);
    /**
     * 调用DataWriter的write方法发布_impl->_transform数据，并获取返回码。
     */
     /**
      * 根据返回码处理不同的错误情况。
      */
    if (rcode == erc::ReturnCodeValue::RETCODE_OK) {
        /**
             * 发布成功，返回true。
             */
        return true;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ERROR) {
        /// @brief 通用错误
        std::cerr << "RETCODE_ERROR" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_UNSUPPORTED) {
        /// @brief 请求的操作不受支持
        std::cerr << "RETCODE_UNSUPPORTED" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_BAD_PARAMETER) {
        /// @brief 传递给函数的参数无效
        std::cerr << "RETCODE_BAD_PARAMETER" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_PRECONDITION_NOT_MET) {
        /// @brief 操作的前置条件未满足
        std::cerr << "RETCODE_PRECONDITION_NOT_MET" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_OUT_OF_RESOURCES) {
        /// @brief 系统资源不足，无法完成操作
        std::cerr << "RETCODE_OUT_OF_RESOURCES" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ENABLED) {
        /// @brief 实体未启用，无法执行操作
        std::cerr << "RETCODE_NOT_ENABLED" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_IMMUTABLE_POLICY) {
        /// @brief 试图更改不可变的QoS策略
        std::cerr << "RETCODE_IMMUTABLE_POLICY" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_INCONSISTENT_POLICY) {
        /// @brief QoS策略不一致，无法设置
        std::cerr << "RETCODE_INCONSISTENT_POLICY" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ALREADY_DELETED) {
        /// @brief 实体已被删除
        std::cerr << "RETCODE_ALREADY_DELETED" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_TIMEOUT) {
        /// @brief 操作超时
        std::cerr << "RETCODE_TIMEOUT" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NO_DATA) {
        /// @brief 请求的数据不存在
        std::cerr << "RETCODE_NO_DATA" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ILLEGAL_OPERATION) {
        /// @brief 执行了非法操作
        std::cerr << "RETCODE_ILLEGAL_OPERATION" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ALLOWED_BY_SECURITY) {
        /// @brief 安全策略不允许执行此操作
        std::cerr << "RETCODE_NOT_ALLOWED_BY_SECURITY" << std::endl;
        return false;
    }
    /// @brief 如果返回码不是预定义的任何值，则输出"UNKNOWN"错误信息
    std::cerr << "UNKNOWN" << std::endl;
    return false;
  }
  /**
 * @brief 设置CarlaTransformPublisher的数据
 *
 * 此函数用于更新位置和时间信息，并计算四元数旋转。
 * 如果传入的位置或旋转与上一次的不同，则更新内部状态，并计算新的四元数旋转。
 *
 * @param seconds 时间戳的秒部分
 * @param nanoseconds 时间戳的纳秒部分
 * @param translation 指向包含x, y, z位置信息的浮点数组的指针
 * @param rotation 指向包含绕x, y, z轴旋转角度（以度为单位）的浮点数组的指针
 */
  void CarlaTransformPublisher::SetData(int32_t seconds, uint32_t nanoseconds, const float* translation, const float* rotation) {
      // 比较传入的位置和旋转与上一次的是否相同
    int same_translation = std::memcmp(translation, _impl->last_translation, sizeof(float) * 3);
    int same_rotation = std::memcmp(rotation, _impl->last_rotation, sizeof(float) * 3);
    // 如果位置或旋转有变化，则更新内部状态
    if (same_translation != 0 || same_rotation != 0) {
        std::memcpy(_impl->last_translation, translation, sizeof(float) * 3);
        std::memcpy(_impl->last_rotation, rotation, sizeof(float) * 3);
        // 从数组中解包位置信息
        const float tx = *translation++;
        const float ty = *translation++;
        const float tz = *translation++;
        // 从数组中解包旋转信息，并转换为弧度
        const float rx = ((*rotation++) * -1.0f) * (M_PIf32 / 180.0f);
        const float ry = ((*rotation++) * -1.0f) * (M_PIf32 / 180.0f);
        const float rz = *rotation++ * (M_PIf32 / 180.0f);
        // 计算四元数的组成部分
        const float cr = cosf(rz * 0.5f);
        const float sr = sinf(rz * 0.5f);
        const float cp = cosf(rx * 0.5f);
        const float sp = sinf(rx * 0.5f);
        const float cy = cosf(ry * 0.5f);
        const float sy = sinf(ry * 0.5f);
        // 更新位置信息（注意y轴方向取反）
        _impl->vec_translation.x(tx);
        _impl->vec_translation.y(-ty);
        _impl->vec_translation.z(tz);
        // 更新四元数旋转信息
        _impl->vec_rotation.w(cr * cp * cy + sr * sp * sy);
        _impl->vec_rotation.x(sr * cp * cy - cr * sp * sy);
        _impl->vec_rotation.y(cr * sp * cy + sr * cp * sy);
        _impl->vec_rotation.z(cr * cp * sy - sr * sp * cy);
    }
    // 设置时间戳
    builtin_interfaces::msg::Time time;
    time.sec(seconds);
    time.nanosec(nanoseconds);
    // 创建消息头并设置时间戳和帧ID
    std_msgs::msg::Header header;
    header.stamp(std::move(time));
    header.frame_id(_parent);
    // 创建Transform消息并设置旋转和位置
    geometry_msgs::msg::Transform t;
    t.rotation(_impl->vec_rotation);
    t.translation(_impl->vec_translation);
    // 创建TransformStamped消息并设置头信息、Transform和子帧ID
    geometry_msgs::msg::TransformStamped ts;
    ts.header(std::move(header));
    ts.transform(std::move(t));
    ts.child_frame_id(_frame_id);
    // 更新内部存储的Transform集合
    _impl->_transform.transforms({ts});
  }
  /**
 * @brief CarlaTransformPublisher 类的构造函数
 *
 * 初始化 CarlaTransformPublisher 对象，并设置 ROS 名称和父帧ID。
 *
 * @param ros_name ROS 节点名称
 * @param parent 父帧ID
 */
  CarlaTransformPublisher::CarlaTransformPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaTransformPublisherImpl>()) {
    _name = ros_name;
    _parent = parent;
  }
  /**
 * @brief CarlaTransformPublisher 类的析构函数
 *
 * 清理资源，删除与 DDS（Data Distribution Service）相关的对象。
 */
  CarlaTransformPublisher::~CarlaTransformPublisher() {
      if (!_impl)
          return;
      // 删除 DataWriter
      if (_impl->_datawriter)
          _impl->_publisher->delete_datawriter(_impl->_datawriter);
      // 删除 Publisher
      if (_impl->_publisher)
          _impl->_participant->delete_publisher(_impl->_publisher);
      // 删除 Topic
      if (_impl->_topic)
          _impl->_participant->delete_topic(_impl->_topic);
      // 删除 Participant
      if (_impl->_participant)
          efd::DomainParticipantFactory::get_instance()->delete_participant(_impl->_participant);
  }
  /**
 * @brief CarlaTransformPublisher 类的拷贝构造函数
 *
 * 创建一个与现有对象相同的 CarlaTransformPublisher 对象。
 * 注意：这里浅拷贝了 _impl 指针，假设 CarlaTransformPublisherImpl 类是正确管理其生命周期的。
 *
 * @param other 要拷贝的对象
 */
  CarlaTransformPublisher::CarlaTransformPublisher(const CarlaTransformPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;// 浅拷贝 _impl 指针
  }
  /**
 * @brief 赋值运算符重载
 *
 * 将现有对象的值赋给另一个 CarlaTransformPublisher 对象。
 * 注意：这里浅拷贝了 _impl 指针。
 *
 * @param other 要赋值的对象
 * @return 引用到当前对象
 */
  CarlaTransformPublisher& CarlaTransformPublisher::operator=(const CarlaTransformPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;// 浅拷贝 _impl 指针

    return *this;
  }
  /**
 * @brief CarlaTransformPublisher 类的移动构造函数
 *
 * 创建一个新的 CarlaTransformPublisher 对象，并将现有对象的资源移动到新对象中。
 *
 * @param other 要移动的对象
 */
  CarlaTransformPublisher::CarlaTransformPublisher(CarlaTransformPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);// 移动 _impl 指针
  }
  /**
 * @brief 移动赋值运算符重载
 *
 * 将现有对象的资源移动到另一个 CarlaTransformPublisher 对象。
 *
 * @param other 要移动赋值的对象
 * @return 引用到当前对象
 */
  CarlaTransformPublisher& CarlaTransformPublisher::operator=(CarlaTransformPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);// 移动 _impl 指针

    return *this;
  }
}
}
