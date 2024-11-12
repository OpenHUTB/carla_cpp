#define _GLIBCXX_USE_CXX11_ABI 0

#include "CarlaDepthCameraPublisher.h"// 引入Carla深度相机发布者类的声明

#include <string>// 引入字符串处理相关的功能
// 引入CARLA ROS 2桥接器中定义的图像和相机信息类型支持
#include "carla/ros2/types/ImagePubSubTypes.h"
#include "carla/ros2/types/CameraInfoPubSubTypes.h"
// 引入CARLA ROS 2桥接器中定义的监听器类，用于处理CARLA仿真环境中的事件
#include "carla/ros2/listeners/CarlaListener.h"
// 引入Fast-DDS库中的相关类和类型定义
#include <fastdds/dds/domain/DomainParticipant.hpp>// 引入域参与者类
#include <fastdds/dds/publisher/Publisher.hpp>// 引入发布者类
#include <fastdds/dds/topic/Topic.hpp>// 引入主题类
#include <fastdds/dds/publisher/DataWriter.hpp>// 引入数据写入器类
#include <fastdds/dds/topic/TypeSupport.hpp>// 引入类型支持类
// 引入Fast-DDS库中的QoS配置相关的类和类型定义
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>// 引入域参与者QoS配置类
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>// 引入域参与者工厂类
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>// 引入发布者QoS配置类
#include <fastdds/dds/topic/qos/TopicQos.hpp>// 引入主题QoS配置类
// 引入Fast-RTPS库中的参与者属性和QoS策略相关的类和类型定义
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/qos/QosPolicies.h>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>// 引入数据写入器QoS配置类
#include <fastdds/dds/publisher/DataWriterListener.hpp>// 引入数据写入器监听器类

/**
 * @namespace carla::ros2
 * @brief 命名空间，用于封装CARLA与ROS 2之间的集成代码。
 */

 /**
  * @brief 命名空间别名定义，简化eprosima::fastdds::dds的引用。
  */
namespace carla {
namespace ros2 {

  namespace efd = eprosima::fastdds::dds;
  /**
 * @brief 类型别名定义，简化eprosima::fastrtps::types::ReturnCode_t的引用。
 */
  using erc = eprosima::fastrtps::types::ReturnCode_t;
  /**
 * @struct CarlaDepthCameraPublisherImpl
 * @brief CARLA深度相机发布者实现结构体。
 *
 * 该结构体包含了与DDS（Data Distribution Service）相关的成员变量，用于发布深度图像数据。
 */
  struct CarlaDepthCameraPublisherImpl {
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
     * @brief DDS类型支持，用于深度图像消息。
     */
    efd::TypeSupport _type { new sensor_msgs::msg::ImagePubSubType() };
    /**
     * @brief CARLA监听器实例。
     */
    CarlaListener _listener {};
    /**
     * @brief 深度图像消息实例。
     */
    sensor_msgs::msg::Image _image {};
  };
  /**
 * @struct CarlaCameraInfoPublisherImpl
 * @brief CARLA相机信息发布者实现结构体。
 *
 * 该结构体包含了与DDS相关的成员变量，用于发布相机信息数据。
 */
  struct CarlaCameraInfoPublisherImpl {
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
     * @brief DDS类型支持，用于相机信息消息。
     */
    efd::TypeSupport _type { new sensor_msgs::msg::CameraInfoPubSubType() };
    /**
     * @brief CARLA监听器实例。
     */
    CarlaListener _listener {};
    /**
     * @brief 初始化标志。
     */
    bool _init {false};
    /**
     * @brief 相机信息消息实例。
     */
    sensor_msgs::msg::CameraInfo _info {};
  };
  /**
 * @brief 检查深度相机发布者是否已初始化。
 *
 * @return true 如果已初始化，否则返回false。
 */
  bool CarlaDepthCameraPublisher::HasBeenInitialized() const {
    return _impl_info->_init;
  }
  /**
 * @brief 初始化深度相机信息数据。
 *
 * @param x_offset X轴偏移量。
 * @param y_offset Y轴偏移量。
 * @param height 图像高度。
 * @param width 图像宽度。
 * @param fov 视野角度。
 * @param do_rectify 是否进行校正。
 */
  void CarlaDepthCameraPublisher::InitInfoData(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, float fov, bool do_rectify) {
    _impl_info->_info = std::move(sensor_msgs::msg::CameraInfo(height, width, fov));
    SetInfoRegionOfInterest(x_offset, y_offset, height, width, do_rectify);
    _impl_info->_init = true;
  }
  /**
 * @brief 初始化深度相机发布者。
 *
 * @return true 如果初始化成功，否则返回false。
 */
  bool CarlaDepthCameraPublisher::Init() {
    return InitImage() && InitInfo();
  }
  /**
 * @brief 初始化深度图像数据。
 *
 * @return true 如果初始化成功，否则返回false。
 */
  bool CarlaDepthCameraPublisher::InitImage() {
    if (_impl->_type == nullptr) {
        /**
         * @brief 输出错误信息，表示类型支持无效。
         */
        std::cerr << "Invalid TypeSupport" << std::endl;
        return false;
    }
    // 设置域参与者的QoS策略，并为其命名
    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    pqos.name(_name);
    auto factory = efd::DomainParticipantFactory::get_instance();
    _impl->_participant = factory->create_participant(0, pqos);
    if (_impl->_participant == nullptr) {
        std::cerr << "Failed to create DomainParticipant" << std::endl;
        return false;
    }
    // 在域参与者中注册类型
    _impl->_type.register_type(_impl->_participant);
    // 设置发布者的QoS策略，并创建发布者
    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
    _impl->_publisher = _impl->_participant->create_publisher(pubqos, nullptr);
    if (_impl->_publisher == nullptr) {
      std::cerr << "Failed to create Publisher" << std::endl;
      return false;
    }
    // 设置主题的QoS策略，并创建主题
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
    // 设置数据写入器的QoS策略，并创建数据写入器
    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
    wqos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    efd::DataWriterListener* listener = (efd::DataWriterListener*)_impl->_listener._impl.get();
    _impl->_datawriter = _impl->_publisher->create_datawriter(_impl->_topic, wqos, listener);
    if (_impl->_datawriter == nullptr) {
        std::cerr << "Failed to create DataWriter" << std::endl;
        return false;
    }
    // 设置帧ID
    _frame_id = _name;
    return true;
  }
  /**
 * @brief 初始化相机信息发布者的DDS相关组件。
 *
 * 该函数负责为相机信息创建DDS域参与者、发布者、主题和数据写入器，并处理可能出现的错误。
 *
 * @return true 如果所有组件都成功创建，否则返回false。
 */
  bool CarlaDepthCameraPublisher::InitInfo() {
      // 检查类型支持是否有效
    if (_impl_info->_type == nullptr) {
        std::cerr << "Invalid TypeSupport" << std::endl;
        return false;
    }
    // 设置域参与者的QoS策略，并为其命名
    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    pqos.name(_name);
    auto factory = efd::DomainParticipantFactory::get_instance();
    _impl_info->_participant = factory->create_participant(0, pqos);
    if (_impl_info->_participant == nullptr) {
        std::cerr << "Failed to create DomainParticipant" << std::endl;
        return false;
    }
    // 在域参与者中注册类型
    _impl_info->_type.register_type(_impl_info->_participant);
    // 设置发布者的QoS策略，并创建发布者
    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
    _impl_info->_publisher = _impl_info->_participant->create_publisher(pubqos, nullptr);
    if (_impl_info->_publisher == nullptr) {
      std::cerr << "Failed to create Publisher" << std::endl;
      return false;
    }
    // 设置主题的QoS策略，并创建主题
    efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
    const std::string publisher_type {"/camera_info"};
    const std::string base { "rt/carla/" };
    std::string topic_name = base;
    if (!_parent.empty())
      topic_name += _parent + "/";
    topic_name += _name;
    topic_name += publisher_type;
    _impl_info->_topic = _impl_info->_participant->create_topic(topic_name, _impl_info->_type->getName(), tqos);
    if (_impl_info->_topic == nullptr) {
        std::cerr << "Failed to create Topic" << std::endl;
        return false;
    }
    // 设置数据写入器的QoS策略（使用默认值），并创建数据写入器
    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
    efd::DataWriterListener* listener = (efd::DataWriterListener*)_impl_info->_listener._impl.get();
    _impl_info->_datawriter = _impl_info->_publisher->create_datawriter(_impl_info->_topic, wqos, listener);
    if (_impl_info->_datawriter == nullptr) {
        std::cerr << "Failed to create DataWriter" << std::endl;
        return false;
    }
    // 设置帧ID
    _frame_id = _name;
    return true;
  }
  /**
 * @brief 发布深度图像和相机信息。
 *
 * 该函数负责调用其他函数来发布深度图像和相机信息。
 *
 * @return true 如果深度图像和相机信息都成功发布，否则返回false。
 */
  bool CarlaDepthCameraPublisher::Publish() {
      // 发布深度图像和相机信息，并返回结果
    return PublishImage() && PublishInfo();
  }

  bool CarlaDepthCameraPublisher::PublishImage() {
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    eprosima::fastrtps::types::ReturnCode_t rcode = _impl->_datawriter->write(&_impl->_image, instance_handle);
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

  bool CarlaDepthCameraPublisher::PublishInfo() {
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    eprosima::fastrtps::types::ReturnCode_t rcode = _impl_info->_datawriter->write(&_impl_info->_info, instance_handle);
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

  void CarlaDepthCameraPublisher::SetImageData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, const uint8_t* data) {    std::vector<uint8_t> vector_data;
    const size_t size = height * width * 4;
    vector_data.resize(size);
    std::memcpy(&vector_data[0], &data[0], size);
    SetData(seconds, nanoseconds,height, width, std::move(vector_data));
  }

  void CarlaDepthCameraPublisher::SetInfoRegionOfInterest( uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, bool do_rectify) {
    sensor_msgs::msg::RegionOfInterest roi;
    roi.x_offset(x_offset);
    roi.y_offset(y_offset);
    roi.height(height);
    roi.width(width);
    roi.do_rectify(do_rectify);
    _impl_info->_info.roi(roi);
  }

  void CarlaDepthCameraPublisher::SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, std::vector<uint8_t>&& data) {
    builtin_interfaces::msg::Time time;
    time.sec(seconds);
    time.nanosec(nanoseconds);

    std_msgs::msg::Header header;
    header.stamp(std::move(time));
    header.frame_id(_frame_id);

    _impl->_image.header(std::move(header));
    _impl->_image.width(width);
    _impl->_image.height(height);
    _impl->_image.encoding("bgra8");
    _impl->_image.is_bigendian(0);
    _impl->_image.step(_impl->_image.width() * sizeof(uint8_t) * 4);
    _impl->_image.data(std::move(data)); //https://github.com/eProsima/Fast-DDS/issues/2330
  }

  void CarlaDepthCameraPublisher::SetCameraInfoData(int32_t seconds, uint32_t nanoseconds) {
    builtin_interfaces::msg::Time time;
    time.sec(seconds);
    time.nanosec(nanoseconds);

    std_msgs::msg::Header header;
    header.stamp(std::move(time));
    header.frame_id(_frame_id);
    _impl_info->_info.header(header);
  }

  CarlaDepthCameraPublisher::CarlaDepthCameraPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaDepthCameraPublisherImpl>()),
  _impl_info(std::make_shared<CarlaCameraInfoPublisherImpl>()) {
    _name = ros_name;
    _parent = parent;
  }

  CarlaDepthCameraPublisher::~CarlaDepthCameraPublisher() {
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

      if (!_impl_info)
        return;

      if (_impl_info->_datawriter)
          _impl_info->_publisher->delete_datawriter(_impl_info->_datawriter);

      if (_impl_info->_publisher)
          _impl_info->_participant->delete_publisher(_impl_info->_publisher);

      if (_impl_info->_topic)
          _impl_info->_participant->delete_topic(_impl_info->_topic);

      if (_impl_info->_participant)
          efd::DomainParticipantFactory::get_instance()->delete_participant(_impl_info->_participant);
  }

  CarlaDepthCameraPublisher::CarlaDepthCameraPublisher(const CarlaDepthCameraPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    _impl_info = other._impl_info;
  }

  CarlaDepthCameraPublisher& CarlaDepthCameraPublisher::operator=(const CarlaDepthCameraPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    _impl_info = other._impl_info;

    return *this;
  }

  CarlaDepthCameraPublisher::CarlaDepthCameraPublisher(CarlaDepthCameraPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    _impl_info = std::move(other._impl_info);

  }

  CarlaDepthCameraPublisher& CarlaDepthCameraPublisher::operator=(CarlaDepthCameraPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    _impl_info = std::move(other._impl_info);

    return *this;
  }
}
}
