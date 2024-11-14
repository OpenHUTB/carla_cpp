#define _GLIBCXX_USE_CXX11_ABI 0

#include "CarlaNormalsCameraPublisher.h"// 包含CarlaNormalsCameraPublisher类的声明

#include <string>// 包含字符串类
// 包含Carla ROS2类型定义
#include "carla/ros2/types/ImagePubSubTypes.h"
#include "carla/ros2/types/CameraInfoPubSubTypes.h"
#include "carla/ros2/listeners/CarlaListener.h"// 包含Carla监听器类
// 包含Fast-DDS相关头文件
#include <fastdds/dds/domain/DomainParticipant.hpp>// 域参与者类
#include <fastdds/dds/publisher/Publisher.hpp> // 发布者类
#include <fastdds/dds/topic/Topic.hpp>// 主题类
#include <fastdds/dds/publisher/DataWriter.hpp>// 数据写入器类
#include <fastdds/dds/topic/TypeSupport.hpp>// 类型支持类
// 包含Fast-DDS QOS策略相关头文件
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>// 域参与者QOS策略
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>// 域参与者工厂类
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>// 发布者QOS策略
#include <fastdds/dds/topic/qos/TopicQos.hpp>// 主题QOS策略
// 包含RTPS属性与QOS策略的头文件
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/qos/QosPolicies.h>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>// 数据写入器QOS策略
#include <fastdds/dds/publisher/DataWriterListener.hpp>// 数据写入器监听器类

/**
 * @namespace carla::ros2
 * @brief 命名空间，包含CARLA与ROS2集成的相关功能。
 */

 /**
  * @brief 别名定义，简化eprosima::fastdds::dds命名空间的引用。
  */
namespace carla {
namespace ros2 {

  namespace efd = eprosima::fastdds::dds;
  /**
 * @brief 类型别名，简化eprosima::fastrtps::types::ReturnCode_t的引用。
 */
  using erc = eprosima::fastrtps::types::ReturnCode_t;
  /**
 * @struct CarlaNormalsCameraPublisherImpl
 * @brief CarlaNormalsCameraPublisher的内部实现结构体。
 */
  struct CarlaNormalsCameraPublisherImpl {
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
     * @brief DDS类型支持，用于Image消息。
     */
    efd::TypeSupport _type { new sensor_msgs::msg::ImagePubSubType() };
    /**
     * @brief CarlaListener实例，用于监听DDS事件。
     */
    CarlaListener _listener {};
    /**
     * @brief 存储的Image消息。
     */
    sensor_msgs::msg::Image _image {};
  };
  /**
 * @struct CarlaCameraInfoPublisherImpl
 * @brief CarlaCameraInfoPublisher的内部实现结构体。
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
     * @brief DDS类型支持，用于CameraInfo消息。
     */
    efd::TypeSupport _type { new sensor_msgs::msg::CameraInfoPubSubType() };
    /**
     * @brief CarlaListener实例，用于监听DDS事件。
     */
    CarlaListener _listener {};
    /**
     * @brief 初始化标志。
     */
    bool _init {false};
    /**
     * @brief 存储的CameraInfo消息。
     */
    sensor_msgs::msg::CameraInfo _info {};
  };
  /**
 * @brief 检查CarlaNormalsCameraPublisher是否已初始化。
 *
 * @return true 如果已初始化，否则返回false。
 */
  bool CarlaNormalsCameraPublisher::HasBeenInitialized() const {
    return _impl_info->_init;
  }
  /**
 * @brief 初始化CameraInfo数据。
 *
 * @param x_offset X轴偏移量。
 * @param y_offset Y轴偏移量。
 * @param height 图像高度。
 * @param width 图像宽度。
 * @param fov 视野角度。
 * @param do_rectify 是否进行校正。
 */
  void CarlaNormalsCameraPublisher::InitInfoData(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, float fov, bool do_rectify) {
    _impl_info->_info = std::move(sensor_msgs::msg::CameraInfo(height, width, fov));
    SetInfoRegionOfInterest(x_offset, y_offset, height, width, do_rectify);
    _impl_info->_init = true;
  }
  /**
 * @brief 初始化CarlaNormalsCameraPublisher。
 *
 * @return true 如果初始化成功，否则返回false。
 */
  bool CarlaNormalsCameraPublisher::Init() {
    return InitImage() && InitInfo();
  }
  /**
 * @brief 初始化图像发布者。
 *
 * @return true 如果初始化成功，否则返回false。
 */
  bool CarlaNormalsCameraPublisher::InitImage() {
      // 检查类型支持是否有效
    if (_impl->_type == nullptr) {
        std::cerr << "Invalid TypeSupport" << std::endl;// 打印错误信息：无效的类型支持
        return false;// 返回false表示初始化失败
    }
    // 设置DomainParticipant的QoS策略，并创建DomainParticipant
    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    pqos.name(_name);// 设置DomainParticipant的名称
    auto factory = efd::DomainParticipantFactory::get_instance();
    _impl->_participant = factory->create_participant(0, pqos);// 创建DomainParticipant
    if (_impl->_participant == nullptr) {
        std::cerr << "Failed to create DomainParticipant" << std::endl;// 打印错误信息：创建DomainParticipant失败
        return false;// 返回false表示初始化失败
    }
    _impl->_type.register_type(_impl->_participant);// 注册类型到DomainParticipant
    // 设置Publisher的QoS策略，并创建Publisher
    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
    _impl->_publisher = _impl->_participant->create_publisher(pubqos, nullptr);// 创建Publisher
    if (_impl->_publisher == nullptr) {
      std::cerr << "Failed to create Publisher" << std::endl;// 打印错误信息：创建Publisher失败
      return false;// 返回false表示初始化失败
    }
    // 设置Topic的QoS策略，并创建Topic
    efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
    const std::string publisher_type {"/image"};// 定义Topic的类型后缀，表示这是一个图像发布者
    const std::string base { "rt/carla/" };// 定义Topic的基础路径
    std::string topic_name = base;// 初始化Topic名称
    if (!_parent.empty())// 如果父路径不为空
      topic_name += _parent + "/";// 添加父路径到Topic名称
    topic_name += _name;// 添加名称到Topic名称
    topic_name += publisher_type;// 添加类型后缀到Topic名称
    _impl->_topic = _impl->_participant->create_topic(topic_name, _impl->_type->getName(), tqos);
    if (_impl->_topic == nullptr) {
        std::cerr << "Failed to create Topic" << std::endl;// 打印错误信息：创建Topic失败
        return false;// 返回false表示初始化失败
    }
    // 设置DataWriter的QoS策略，并创建DataWriter
    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
    wqos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;// 设置历史内存策略为预分配并允许重新分配
    efd::DataWriterListener* listener = (efd::DataWriterListener*)_impl->_listener._impl.get();// 获取DataWriter监听器
    _impl->_datawriter = _impl->_publisher->create_datawriter(_impl->_topic, wqos, listener);// 创建DataWriter
    if (_impl->_datawriter == nullptr) {
        std::cerr << "Failed to create DataWriter" << std::endl;// 打印错误信息：创建DataWriter失败
        return false;// 返回false表示初始化失败
    }
    // 设置帧ID为名称
    _frame_id = _name;
    return true;// 返回true表示初始化成功
  }
  /**
 * @brief 初始化CameraInfo发布者。
 *
 * @return true 如果初始化成功，否则返回false。
 */
  bool CarlaNormalsCameraPublisher::InitInfo() {
      // 检查类型支持是否有效
    if (_impl_info->_type == nullptr) {
        std::cerr << "Invalid TypeSupport" << std::endl;// 打印错误信息
        return false;// 返回false表示初始化失败
    }
    // 设置DomainParticipant的QoS策略，并创建DomainParticipant
    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    pqos.name(_name);// 设置DomainParticipant的名称
    auto factory = efd::DomainParticipantFactory::get_instance();
    _impl_info->_participant = factory->create_participant(0, pqos);// 创建DomainParticipant
    if (_impl_info->_participant == nullptr) {
        std::cerr << "Failed to create DomainParticipant" << std::endl;// 打印错误信息：创建DomainParticipant失败
        return false;// 返回false表示初始化失败
    }
    _impl_info->_type.register_type(_impl_info->_participant);// 注册类型到DomainParticipant
    // 设置Publisher的QoS策略，并创建Publisher
    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
    _impl_info->_publisher = _impl_info->_participant->create_publisher(pubqos, nullptr);// 创建Publisher
    if (_impl_info->_publisher == nullptr) {
      std::cerr << "Failed to create Publisher" << std::endl;// 打印错误信息：创建Publisher失败
      return false;// 返回false表示初始化失败
    }
    // 设置Topic的QoS策略，并创建Topic
    efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
    const std::string publisher_type {"/camera_info"};// 定义Topic类型后缀
    const std::string base { "rt/carla/" };// 定义Topic的基础路径
    std::string topic_name = base;// 初始化Topic名称
    if (!_parent.empty())// 如果父路径不为空
      topic_name += _parent + "/"; // 添加父路径到Topic名称
    topic_name += _name;// 添加名称到Topic名称
    topic_name += publisher_type;// 添加类型后缀到Topic名称
    _impl_info->_topic = _impl_info->_participant->create_topic(topic_name, _impl_info->_type->getName(), tqos);// 创建Topic
    if (_impl_info->_topic == nullptr) {
        std::cerr << "Failed to create Topic" << std::endl;// 打印错误信息：创建Topic失败
        return false;// 返回false表示初始化失败
    }// 设置DataWriter的QoS策略，并创建DataWriter
    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
    efd::DataWriterListener* listener = (efd::DataWriterListener*)_impl_info->_listener._impl.get();// 获取DataWriter监听器
    _impl_info->_datawriter = _impl_info->_publisher->create_datawriter(_impl_info->_topic, wqos, listener);// 创建DataWriter
    if (_impl_info->_datawriter == nullptr) {
        std::cerr << "Failed to create DataWriter" << std::endl;// 打印错误信息：创建DataWriter失败
        return false;// 返回false表示初始化失败
    }
    // 设置帧ID为名称
    _frame_id = _name;
    return true;// 返回true表示初始化成功
  }

  bool CarlaNormalsCameraPublisher::Publish() {
    return PublishImage() && PublishInfo();
  }

  bool CarlaNormalsCameraPublisher::PublishImage() {
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

  bool CarlaNormalsCameraPublisher::PublishInfo() {
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

  void CarlaNormalsCameraPublisher::SetImageData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, const uint8_t* data) {    std::vector<uint8_t> vector_data;
    const size_t size = height * width * 4;
    vector_data.resize(size);
    std::memcpy(&vector_data[0], &data[0], size);
    SetData(seconds, nanoseconds,height, width, std::move(vector_data));
  }

  void CarlaNormalsCameraPublisher::SetInfoRegionOfInterest( uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, bool do_rectify) {
    sensor_msgs::msg::RegionOfInterest roi;
    roi.x_offset(x_offset);
    roi.y_offset(y_offset);
    roi.height(height);
    roi.width(width);
    roi.do_rectify(do_rectify);
    _impl_info->_info.roi(roi);
  }

  void CarlaNormalsCameraPublisher::SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, std::vector<uint8_t>&& data) {
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

  void CarlaNormalsCameraPublisher::SetCameraInfoData(int32_t seconds, uint32_t nanoseconds) {
    builtin_interfaces::msg::Time time;
    time.sec(seconds);
    time.nanosec(nanoseconds);

    std_msgs::msg::Header header;
    header.stamp(std::move(time));
    header.frame_id(_frame_id);
    _impl_info->_info.header(header);
  }

  CarlaNormalsCameraPublisher::CarlaNormalsCameraPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaNormalsCameraPublisherImpl>()),
  _impl_info(std::make_shared<CarlaCameraInfoPublisherImpl>()) {
    _name = ros_name;
    _parent = parent;
  }

  CarlaNormalsCameraPublisher::~CarlaNormalsCameraPublisher() {
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

  CarlaNormalsCameraPublisher::CarlaNormalsCameraPublisher(const CarlaNormalsCameraPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    _impl_info = other._impl_info;
  }

  CarlaNormalsCameraPublisher& CarlaNormalsCameraPublisher::operator=(const CarlaNormalsCameraPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    _impl_info = other._impl_info;

    return *this;
  }

  CarlaNormalsCameraPublisher::CarlaNormalsCameraPublisher(CarlaNormalsCameraPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    _impl_info = std::move(other._impl_info);

  }

  CarlaNormalsCameraPublisher& CarlaNormalsCameraPublisher::operator=(CarlaNormalsCameraPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    _impl_info = std::move(other._impl_info);

    return *this;
  }
}
}
