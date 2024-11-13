#define _GLIBCXX_USE_CXX11_ABI 0

#include "CarlaDVSCameraPublisher.h"// 引入CarlaDVS相机发布器的头文件

#include <string>// 引入字符串处理功能
// 引入CARLA传感器数据中的DVS事件类型
#include "carla/sensor/data/DVSEvent.h"
// 引入CARLA ROS2类型的发布/订阅类型定义
#include "carla/ros2/types/ImagePubSubTypes.h"
#include "carla/ros2/types/CameraInfoPubSubTypes.h"
#include "carla/ros2/types/PointCloud2PubSubTypes.h"
// 引入CARLA ROS2监听器接口
#include "carla/ros2/listeners/CarlaListener.h"
// 引入Fast-DDS的域参与者、发布者、主题和数据写入器等核心组件
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
// 引入Fast-DDS的QoS策略定义
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
// 引入Fast-RTPS的参与者属性和QoS策略
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/qos/QosPolicies.h>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>

/**
 * @namespace carla::ros2
 * @brief CARLA的ROS2集成命名空间，包含DVS相机、相机信息和点云数据的发布者实现。
 */
namespace carla {
namespace ros2 {
    /**
  * @brief 引入Fast-DDS命名空间的别名，简化代码中的引用。
  */
  namespace efd = eprosima::fastdds::dds;
  /**
   * @brief 引入Fast-RTPS返回代码类型的别名。
   */
  using erc = eprosima::fastrtps::types::ReturnCode_t;
  /**
  * @struct CarlaDVSCameraPublisherImpl
  * @brief DVS相机数据发布者的内部实现结构。
  */
  struct CarlaDVSCameraPublisherImpl {
      /**
    * @brief Fast-DDS域参与者指针。
    */
    efd::DomainParticipant* _participant { nullptr };
    /**
     * @brief Fast-DDS发布者指针。
     */
    efd::Publisher* _publisher { nullptr };
    /**
     * @brief Fast-DDS主题指针。
     */
    efd::Topic* _topic { nullptr };
    /**
     * @brief Fast-DDS数据写入器指针。
     */
    efd::DataWriter* _datawriter { nullptr };
    /**
     * @brief Fast-DDS类型支持，用于图像数据。
     */
    efd::TypeSupport _type { new sensor_msgs::msg::ImagePubSubType() };
    /**
     * @brief CARLA监听器实例，用于处理回调。
     */
    CarlaListener _listener {};
    /**
     * @brief 待发布的图像数据。
     */
    sensor_msgs::msg::Image _image {};
  };
  /**
   * @struct CarlaCameraInfoPublisherImpl
   * @brief 相机信息数据发布者的内部实现结构。
   */
  struct CarlaCameraInfoPublisherImpl {
      /**
     * @brief Fast-DDS域参与者指针。
     */
    efd::DomainParticipant* _participant { nullptr };
    /**
     * @brief Fast-DDS发布者指针。
     */
    efd::Publisher* _publisher { nullptr };
    /**
     * @brief Fast-DDS主题指针。
     */
    efd::Topic* _topic { nullptr };
    /**
     * @brief Fast-DDS数据写入器指针。
     */
    efd::DataWriter* _datawriter { nullptr };
    /**
     * @brief Fast-DDS类型支持，用于相机信息数据。
     */
    efd::TypeSupport _type { new sensor_msgs::msg::CameraInfoPubSubType() };
    /**
     * @brief CARLA监听器实例，用于处理回调。
     */
    CarlaListener _listener {};
    /**
     * @brief 初始化标志。
     */
    bool _init {false};
    /**
     * @brief 待发布的相机信息数据。
     */
    sensor_msgs::msg::CameraInfo _ci {};
  };
  /**
  * @struct CarlaPointCloudPublisherImpl
  * @brief 点云数据发布者的内部实现结构。
  */
  struct CarlaPointCloudPublisherImpl {
   /**
     * @brief Fast-DDS域参与者指针。
     */
    efd::DomainParticipant* _participant { nullptr };
    /**
    * @brief Fast-DDS发布者指针。
    */
    efd::Publisher* _publisher { nullptr };
    /**
     * @brief Fast-DDS主题指针。
     */
    efd::Topic* _topic { nullptr };
    /**
     * @brief Fast-DDS数据写入器指针。
     */
    efd::DataWriter* _datawriter { nullptr };
    /**
     * @brief Fast-DDS类型支持，用于点云数据。
     */
    efd::TypeSupport _type { new sensor_msgs::msg::PointCloud2PubSubType() };
    /**
     * @brief CARLA监听器实例，用于处理回调。
     */
    CarlaListener _listener {};
    /**
     * @brief 待发布的点云数据。
     */
    sensor_msgs::msg::PointCloud2 _pc {};
  };
  /**
 * @brief 检查是否已初始化
 *
 * @return true 如果已经初始化，否则返回false
 */
  bool CarlaDVSCameraPublisher::HasBeenInitialized() const {
    return _info->_init;
  }
  /**
 * @brief 初始化相机信息数据
 *
 * @param x_offset X轴偏移量
 * @param y_offset Y轴偏移量
 * @param height 图像高度
 * @param width 图像宽度
 * @param fov 视野角度（以弧度为单位）
 * @param do_rectify 是否进行校正
 */
  void CarlaDVSCameraPublisher::InitInfoData(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, float fov, bool do_rectify) {
    _info->_ci = std::move(sensor_msgs::msg::CameraInfo(height, width, fov));
    SetInfoRegionOfInterest(x_offset, y_offset, height, width, do_rectify);
    _info->_init = true;
  }
  /**
 * @brief 初始化CarlaDVSCameraPublisher
 *
 * @return true 如果初始化成功，否则返回false
 */
  bool CarlaDVSCameraPublisher::Init() {
    return InitImage() && InitInfo() && InitPointCloud();
  }
  /**
 * @brief 初始化图像发布相关资源
 *
 * @return true 如果图像发布初始化成功，否则返回false
 */
  bool CarlaDVSCameraPublisher::InitImage() {
    if (_impl->_type == nullptr) {
        std::cerr << "Invalid TypeSupport" << std::endl;
        return false;
    }

    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    pqos.name(_name);
    auto factory = efd::DomainParticipantFactory::get_instance();
    _impl->_participant = factory->create_participant(0, pqos);
    if (_impl->_participant == nullptr) {
        std::cerr << "Failed to create DomainParticipant" << std::endl;
        return false;
    }
    _impl->_type.register_type(_impl->_participant);

    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
    _impl->_publisher = _impl->_participant->create_publisher(pubqos, nullptr);
    if (_impl->_publisher == nullptr) {
      std::cerr << "Failed to create Publisher" << std::endl;
      return false;
    }

    efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
    const std::string publisher_type {"/image"};
    const std::string base { "rt/carla/" };
    std::string topic_name = base;
    if (!_parent.empty())
      topic_name += _parent + "/";
    topic_name += _name;
    topic_name += publisher_type;
    _impl->_topic = _impl->_participant->create_topic(topic_name, _impl->_type->getName(), tqos);
    if (_impl->_topic == nullptr) {
        std::cerr << "Failed to create Topic" << std::endl;
        return false;
    }

    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
    wqos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    efd::DataWriterListener* listener = (efd::DataWriterListener*)_impl->_listener._impl.get();
    _impl->_datawriter = _impl->_publisher->create_datawriter(_impl->_topic, wqos, listener);
    if (_impl->_datawriter == nullptr) {
        std::cerr << "Failed to create DataWriter" << std::endl;
        return false;
    }
    _frame_id = _name;
    return true;
  }
  /**
 * @brief 初始化相机信息发布相关资源
 *
 * @return true 如果相机信息初始化成功，否则返回false
 */
  bool CarlaDVSCameraPublisher::InitInfo() {
    if (_info->_type == nullptr) {
        std::cerr << "Invalid TypeSupport" << std::endl;
        return false;
    }
    /// 设置DomainParticipant的QoS（Quality of Service）策略为默认值，并设置其名称
    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    pqos.name(_name);
    /// 获取DomainParticipantFactory的实例
    auto factory = efd::DomainParticipantFactory::get_instance();
    _info->_participant = factory->create_participant(0, pqos);
    /// 创建DomainParticipant
    if (_info->_participant == nullptr) {
        /// 如果创建DomainParticipant失败，则输出错误信息并返回false
        std::cerr << "Failed to create DomainParticipant" << std::endl;
        return false;
    }
    /// 注册数据类型
    _info->_type.register_type(_info->_participant);
    /// 设置Publisher的QoS策略为默认值
    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
    /// 创建Publisher
    _info->_publisher = _info->_participant->create_publisher(pubqos, nullptr);
    if (_info->_publisher == nullptr) {
        /// 如果创建Publisher失败，则输出错误信息并返回false
      std::cerr << "Failed to create Publisher" << std::endl;
      return false;
    }
    /// 设置Topic的QoS策略为默认值
    efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
    /// 构建Topic的名称
    const std::string publisher_type {"/camera_info"};
    const std::string base { "rt/carla/" };
    std::string topic_name = base;
    if (!_parent.empty())
      topic_name += _parent + "/";
    topic_name += _name;
    topic_name += publisher_type;
    /// 创建Topic
    _info->_topic = _info->_participant->create_topic(topic_name, _info->_type->getName(), tqos);
    if (_info->_topic == nullptr) {
        /// 如果创建Topic失败，则输出错误信息并返回false
        std::cerr << "Failed to create Topic" << std::endl;
        return false;
    }
    /// 设置DataWriter的QoS策略为默认值
    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
    /// 获取DataWriterListener的实例
    efd::DataWriterListener* listener = (efd::DataWriterListener*)_info->_listener._impl.get();
    /// 创建DataWriter
    _info->_datawriter = _info->_publisher->create_datawriter(_info->_topic, wqos, listener);
    if (_info->_datawriter == nullptr) {
        /// 如果创建DataWriter失败，则输出错误信息并返回false
        std::cerr << "Failed to create DataWriter" << std::endl;
        return false;
    }
    /// 设置帧ID为节点名称
    _frame_id = _name;
    /// 所有组件都成功创建，返回true
    return true;
  }
  /**
 * @brief 初始化点云发布相关资源
 *
 * @return true 如果点云初始化成功，否则返回false
 */
  bool CarlaDVSCameraPublisher::InitPointCloud() {
    if (_point_cloud->_type == nullptr) {
        std::cerr << "Invalid TypeSupport" << std::endl;
        return false;
    }
    /// 设置DomainParticipant的QoS策略
    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    pqos.name(_name);
    /// 获取DomainParticipantFactory的实例
    auto factory = efd::DomainParticipantFactory::get_instance();
    /// 创建DomainParticipant
    _point_cloud->_participant = factory->create_participant(0, pqos);
    if (_point_cloud->_participant == nullptr) {
        std::cerr << "Failed to create DomainParticipant" << std::endl;
        return false;
    }
    /// 注册类型到DomainParticipant
    _point_cloud->_type.register_type(_point_cloud->_participant);
    /// 设置Publisher的QoS策略
    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
    _point_cloud->_publisher = _point_cloud->_participant->create_publisher(pubqos, nullptr);
    if (_point_cloud->_publisher == nullptr) {
      std::cerr << "Failed to create Publisher" << std::endl;
      return false;
    }
    /// 设置Topic的QoS策略
    efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
    const std::string publisher_type {"/point_cloud"};
    const std::string base { "rt/carla/" };
    std::string topic_name = base;
    if (!_parent.empty())
      topic_name += _parent + "/";
    topic_name += _name;
    topic_name += publisher_type;
    _point_cloud->_topic = _point_cloud->_participant->create_topic(topic_name, _point_cloud->_type->getName(), tqos);
    if (_point_cloud->_topic == nullptr) {
        std::cerr << "Failed to create Topic" << std::endl;
        return false;
    }
    /// 设置DataWriter的QoS策略，并指定历史内存策略为预分配并允许重新分配
    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
    wqos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    efd::DataWriterListener* listener = (efd::DataWriterListener*)_point_cloud->_listener._impl.get();
    _point_cloud->_datawriter = _point_cloud->_publisher->create_datawriter(_point_cloud->_topic, wqos, listener);
    if (_point_cloud->_datawriter == nullptr) {
        std::cerr << "Failed to create DataWriter" << std::endl;
        return false;
    }
    /// 设置帧ID为参与者名称
    _frame_id = _name;
    return true;
  }
  /// @brief 发布图像、信息和点云数据。
    /// 
    /// 此方法会依次调用 PublishImage()、PublishInfo() 和 PublishPointCloud() 方法，
    /// 只有当这三个方法都返回 true 时，此方法才返回 true。
    /// 
    /// @return bool 如果图像、信息和点云数据都成功发布，则返回 true；否则返回 false。
  bool CarlaDVSCameraPublisher::Publish() {
    return PublishImage() && PublishInfo() && PublishPointCloud();
  }
  /// @brief 发布图像数据。
   /// 
   /// 使用 DataWriter 将图像数据发布到 DDS 网络中。
   /// 根据返回的 ReturnCode 判断发布是否成功，并输出相应的错误信息。
   /// 
   /// @return bool 如果图像数据成功发布，则返回 true；否则返回 false。
  bool CarlaDVSCameraPublisher::PublishImage() {
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    eprosima::fastrtps::types::ReturnCode_t rcode = _impl->_datawriter->write(& _impl->_image, instance_handle);
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
  /// @brief 发布信息数据。
   /// 
   /// 使用 DataWriter 将信息数据发布到 DDS 网络中。
   /// 根据返回的 ReturnCode 判断发布是否成功，并输出相应的错误信息。
   /// 
   /// @return bool 如果信息数据成功发布，则返回 true；否则返回 false。
  bool CarlaDVSCameraPublisher::PublishInfo() {
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    eprosima::fastrtps::types::ReturnCode_t rcode = _info->_datawriter->write(& _info->_ci, instance_handle);
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
  /// @brief 发布点云数据。
    /// 
    /// 使用 DataWriter 将点云数据发布到 DDS 网络中。
    /// 根据返回的 ReturnCode 判断发布是否成功，并输出相应的错误信息。
    /// 
    /// @return bool 如果点云数据成功发布，则返回 true；否则返回 false。
  bool CarlaDVSCameraPublisher::PublishPointCloud() {
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    eprosima::fastrtps::types::ReturnCode_t rcode = _point_cloud->_datawriter->write(&_point_cloud->_pc, instance_handle);
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
  /**
 * @brief 设置图像数据，用于DVS（动态视觉传感器）相机发布者。
 *
 * 该函数接收时间戳、图像元素数量、高度、宽度以及图像数据，并将这些数据转换为BGR8格式的图像数据，
 * 然后调用SetData函数来设置图像数据。
 *
 * @param seconds 时间戳的秒部分
 * @param nanoseconds 时间戳的纳秒部分
 * @param elements 图像中的事件元素数量
 * @param height 图像的高度
 * @param width 图像的宽度
 * @param data 指向图像数据的指针，数据格式为DVS事件
 */
  void CarlaDVSCameraPublisher::SetImageData(int32_t seconds, uint32_t nanoseconds, size_t elements, size_t height, size_t width, const uint8_t* data) {
    std::vector<uint8_t> im_data;
    const size_t im_size = width * height * 3;// 计算图像数据的大小（BGR格式）
    im_data.resize(im_size);// 调整向量大小以适应图像数据
    carla::sensor::data::DVSEvent* vec_event = (carla::sensor::data::DVSEvent*)&data[0];
    for (size_t i = 0; i < elements; ++i, ++vec_event) {
        // 计算每个事件在图像数据中的索引位置，根据事件的x, y坐标和极性（pol）
        size_t index = (vec_event->y * width + vec_event->x) * 3 + (static_cast<int>(vec_event->pol) * 2);
        im_data[index] = 255;// 设置对应位置的像素值为255（假设为亮像素）
    }
    // 调用SetData函数来设置图像数据
    SetData(seconds, nanoseconds, height, width, std::move(im_data));
  }
  /**
 * @brief 设置图像数据，包括时间戳、图像尺寸和图像数据。
 *
 * 该函数接收时间戳、图像高度、图像宽度和图像数据，并设置图像消息的相关字段，
 * 包括时间戳、帧ID、图像尺寸、编码方式以及图像数据。
 *
 * @param seconds 时间戳的秒部分
 * @param nanoseconds 时间戳的纳秒部分
 * @param height 图像的高度
 * @param width 图像的宽度
 * @param data 图像数据的右值引用，用于移动语义
 */
  void CarlaDVSCameraPublisher::SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, std::vector<uint8_t>&& data) {
    builtin_interfaces::msg::Time time;
    time.sec(seconds);
    time.nanosec(nanoseconds);

    std_msgs::msg::Header header;
    header.stamp(std::move(time));// 设置时间戳
    header.frame_id(_frame_id);// 设置帧ID
    _impl->_image.header(header);// 设置图像消息的头信息
    _info->_ci.header(header);// 设置相机信息消息的头信息
    _point_cloud->_pc.header(header);// 设置点云消息的头信息

    _impl->_image.width(width); // 设置图像宽度
    _impl->_image.height(height);// 设置图像高度
    _impl->_image.encoding("bgr8"); //taken from the list of strings in include/sensor_msgs/image_encodings.h // 设置图像编码方式为BGR8
    _impl->_image.is_bigendian(0);// 设置图像数据是否为大端模式（0表示小端）
    _impl->_image.step(_impl->_image.width() * sizeof(uint8_t) * 3);// 设置图像每行的字节数
    _impl->_image.data(std::move(data)); //https://github.com/eProsima/Fast-DDS/issues/2330  // 设置图像数据
  }
  /**
 * @brief 设置相机信息数据的时间戳和帧ID。
 *
 * 该函数接收时间戳，并设置相机信息消息的时间戳和帧ID。
 *
 * @param seconds 时间戳的秒部分
 * @param nanoseconds 时间戳的纳秒部分
 */
  void CarlaDVSCameraPublisher::SetCameraInfoData(int32_t seconds, uint32_t nanoseconds) {
    builtin_interfaces::msg::Time time;
    time.sec(seconds);
    time.nanosec(nanoseconds);

    std_msgs::msg::Header header;
    header.stamp(std::move(time));// 设置时间戳
    header.frame_id(_frame_id);// 设置帧ID
  }

  void CarlaDVSCameraPublisher::SetInfoRegionOfInterest( uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, bool do_rectify) {
    sensor_msgs::msg::RegionOfInterest roi;
    roi.x_offset(x_offset);
    roi.y_offset(y_offset);
    roi.height(height);
    roi.width(width);
    roi.do_rectify(do_rectify);
    _info->_ci.roi(roi);
  }

  void CarlaDVSCameraPublisher::SetPointCloudData(size_t height, size_t width, size_t elements, const uint8_t* data) {

    std::vector<uint8_t> vector_data;
    const size_t size = height * width;
    vector_data.resize(size);
    std::memcpy(&vector_data[0], &data[0], size);

    sensor_msgs::msg::PointField descriptor1;
    descriptor1.name("x");
    descriptor1.offset(0);
    descriptor1.datatype(sensor_msgs::msg::PointField__UINT16);
    descriptor1.count(1);
    sensor_msgs::msg::PointField descriptor2;
    descriptor2.name("y");
    descriptor2.offset(2);
    descriptor2.datatype(sensor_msgs::msg::PointField__UINT16);
    descriptor2.count(1);
    sensor_msgs::msg::PointField descriptor3;
    descriptor3.name("t");
    descriptor3.offset(4);
    descriptor3.datatype(sensor_msgs::msg::PointField__FLOAT64);
    descriptor3.count(1);
    sensor_msgs::msg::PointField descriptor4;
    descriptor3.name("pol");
    descriptor3.offset(12);
    descriptor3.datatype(sensor_msgs::msg::PointField__INT8);
    descriptor3.count(1);

    const size_t point_size = sizeof(carla::sensor::data::DVSEvent);
    _point_cloud->_pc.width(width);
    _point_cloud->_pc.height(height);
    _point_cloud->_pc.is_bigendian(false);
    _point_cloud->_pc.fields({descriptor1, descriptor2, descriptor3, descriptor4});
    _point_cloud->_pc.point_step(point_size);
    _point_cloud->_pc.row_step(width * point_size);
    _point_cloud->_pc.is_dense(false); //True if there are not invalid points
    _point_cloud->_pc.data(std::move(vector_data));
  }

  CarlaDVSCameraPublisher::CarlaDVSCameraPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaDVSCameraPublisherImpl>()),
  _info(std::make_shared<CarlaCameraInfoPublisherImpl>()),
  _point_cloud(std::make_shared<CarlaPointCloudPublisherImpl>()) {
    _name = ros_name;
    _parent = parent;
  }

  CarlaDVSCameraPublisher::~CarlaDVSCameraPublisher() {
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

      if (!_info)
          return;

      if (_info->_datawriter)
          _info->_publisher->delete_datawriter(_info->_datawriter);

      if (_info->_publisher)
          _info->_participant->delete_publisher(_info->_publisher);

      if (_info->_topic)
          _info->_participant->delete_topic(_info->_topic);

      if (_info->_participant)
          efd::DomainParticipantFactory::get_instance()->delete_participant(_info->_participant);

      if (!_point_cloud)
          return;

      if (_point_cloud->_datawriter)
          _point_cloud->_publisher->delete_datawriter(_point_cloud->_datawriter);

      if (_point_cloud->_publisher)
          _point_cloud->_participant->delete_publisher(_point_cloud->_publisher);

      if (_point_cloud->_topic)
          _point_cloud->_participant->delete_topic(_point_cloud->_topic);

      if (_point_cloud->_participant)
          efd::DomainParticipantFactory::get_instance()->delete_participant(_point_cloud->_participant);
  }

  CarlaDVSCameraPublisher::CarlaDVSCameraPublisher(const CarlaDVSCameraPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    _info = other._info;
    _point_cloud = other._point_cloud;
  }

  CarlaDVSCameraPublisher& CarlaDVSCameraPublisher::operator=(const CarlaDVSCameraPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    _info = other._info;
    _point_cloud = other._point_cloud;

    return *this;
  }

  CarlaDVSCameraPublisher::CarlaDVSCameraPublisher(CarlaDVSCameraPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    _info = std::move(other._info);
    _point_cloud = std::move(other._point_cloud);
  }

  CarlaDVSCameraPublisher& CarlaDVSCameraPublisher::operator=(CarlaDVSCameraPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    _info = std::move(other._info);
    _point_cloud = std::move(other._point_cloud);

    return *this;
  }
}
}
