#define _GLIBCXX_USE_CXX11_ABI 0

#include "CarlaSSCameraPublisher.h"// 引入CarlaSSCameraPublisher类的声明

#include <string>// 引入标准字符串库
// 引入CARLA ROS2桥接器中定义的图像和相机信息类型的PubSubTypes
#include "carla/ros2/types/ImagePubSubTypes.h"
#include "carla/ros2/types/CameraInfoPubSubTypes.h"
#include "carla/ros2/listeners/CarlaListener.h"// 引入CARLA ROS2监听器基类
// 引入Fast-DDS（eProsima Fast RTPS的C++ API封装）相关的头文件
#include <fastdds/dds/domain/DomainParticipant.hpp> // DomainParticipant类，用于创建、删除和管理RTPS实体
#include <fastdds/dds/publisher/Publisher.hpp>// Publisher类，用于发布数据
#include <fastdds/dds/topic/Topic.hpp>// Topic类，用于定义数据的主题
#include <fastdds/dds/publisher/DataWriter.hpp>// DataWriter类，用于写入数据到指定的主题
#include <fastdds/dds/topic/TypeSupport.hpp> // TypeSupport类，用于支持特定类型的数据
// 引入Fast-DDS中与质量服务（QoS）相关的头文件
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>// DomainParticipantQos类，用于配置DomainParticipant的QoS策略
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>// DomainParticipantFactory类，用于创建DomainParticipant实例
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>// PublisherQos类，用于配置Publisher的QoS策略
#include <fastdds/dds/topic/qos/TopicQos.hpp>// TopicQos类，用于配置Topic的QoS策略
// 引入Fast-RTPS（eProsima Fast RTPS库）中的头文件，用于配置参与者属性和QoS策略
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/qos/QosPolicies.h>
// 引入Fast-DDS中DataWriter的QoS配置和数据写入监听器
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>// DataWriterQos类，用于配置DataWriter的QoS策略
#include <fastdds/dds/publisher/DataWriterListener.hpp>// DataWriterListener类，用于监听DataWriter的事件
/**
 * @namespace carla::ros2
 * @brief 此命名空间包含了CARLA与ROS2集成相关的类和函数。
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
   * @struct CarlaSSCameraPublisherImpl
   * @brief CarlaSSCameraPublisher的内部实现结构体，包含Fast-DDS相关的资源和配置。
   */
  struct CarlaSSCameraPublisherImpl {
      /**
     * @brief Fast-DDS的DomainParticipant指针，用于管理RTPS实体。
     */
    efd::DomainParticipant* _participant { nullptr };
    /**
     * @brief Fast-DDS的Publisher指针，用于发布数据。
     */
    efd::Publisher* _publisher { nullptr };
    /**
     * @brief Fast-DDS的Topic指针，定义了数据的主题。
     */
    efd::Topic* _topic { nullptr };
    /**
     * @brief Fast-DDS的DataWriter指针，用于写入数据到指定的主题。
     */
    efd::DataWriter* _datawriter { nullptr };
    /**
    * @brief Fast-DDS的TypeSupport对象，支持特定类型的数据（这里是sensor_msgs::msg::Image）。
    */
    efd::TypeSupport _type { new sensor_msgs::msg::ImagePubSubType() };
    /**
     * @brief CarlaListener对象，用于监听CARLA的事件。
     */
    CarlaListener _listener {};
    /**
     * @brief 存储的图像数据。
     */
    sensor_msgs::msg::Image _image {};
  };
  /**
   * @struct CarlaCameraInfoPublisherImpl
   * @brief CarlaCameraInfoPublisher的内部实现结构体，包含Fast-DDS相关的资源和配置。
   */
  struct CarlaCameraInfoPublisherImpl {
      /**
     * @brief Fast-DDS的DomainParticipant指针，用于管理RTPS实体。
     */
    efd::DomainParticipant* _participant { nullptr };
    /**
     * @brief Fast-DDS的Publisher指针，用于发布数据。
     */
    efd::Publisher* _publisher { nullptr };
    /**
     * @brief Fast-DDS的Topic指针，定义了数据的主题。
     */
    efd::Topic* _topic { nullptr };
    /**
     * @brief Fast-DDS的DataWriter指针，用于写入数据到指定的主题。
     */
    efd::DataWriter* _datawriter { nullptr };
    /**
     * @brief Fast-DDS的TypeSupport对象，支持特定类型的数据（这里是sensor_msgs::msg::CameraInfo）。
     */
    efd::TypeSupport _type { new sensor_msgs::msg::CameraInfoPubSubType() };
    /**
     * @brief CarlaListener对象，用于监听CARLA的事件。
     */
    CarlaListener _listener {};
    /**
     * @brief 标记是否已初始化。
     */
    bool _init {false};
    /**
     * @brief 存储的相机信息数据。
     */
    sensor_msgs::msg::CameraInfo _info {};
  };
  /**
   * @brief 检查CarlaSSCameraPublisher是否已初始化。
   *
   * @return 如果已初始化，则返回true；否则返回false。
   */
  bool CarlaSSCameraPublisher::HasBeenInitialized() const {
    return _impl_info->_init;
  }
  /**
 * @brief 初始化相机信息数据
 *
 * 该函数用于初始化相机的信息数据，包括高度、宽度、视场角（FOV），并设置感兴趣区域（ROI）。
 *
 * @param x_offset  感兴趣区域的X轴偏移量
 * @param y_offset  感兴趣区域的Y轴偏移量
 * @param height    相机图像的高度
 * @param width     相机图像的宽度
 * @param fov       相机的视场角（FOV）
 * @param do_rectify 是否对图像进行校正
 */
  void CarlaSSCameraPublisher::InitInfoData(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, float fov, bool do_rectify) {
    _impl_info->_info = std::move(sensor_msgs::msg::CameraInfo(height, width, fov));
    SetInfoRegionOfInterest(x_offset, y_offset, height, width, do_rectify);
    _impl_info->_init = true;
  }
  /**
 * @brief 初始化相机发布者
 *
 * 该函数用于初始化相机发布者，包括图像和信息的初始化。
 *
 * @return 初始化是否成功，成功返回true，失败返回false
 */
  bool CarlaSSCameraPublisher::Init() {
    return InitImage() && InitInfo();
  }
  /**
 * @brief 初始化相机图像
 *
 * 该函数用于初始化相机图像的发布，包括创建DomainParticipant、Publisher、Topic和DataWriter。
 *
 * @return 初始化是否成功，成功返回true，失败返回false
 */
  bool CarlaSSCameraPublisher::InitImage() {
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
 * @brief 初始化CarlaSSCameraPublisher的信息
 *
 * 此函数负责初始化CarlaSSCameraPublisher所需的各种组件，包括DomainParticipant, Publisher, Topic和DataWriter。
 * 如果任何步骤失败，函数将输出错误信息并返回false。
 *
 * @return bool 如果初始化成功返回true，否则返回false。
 */
  bool CarlaSSCameraPublisher::InitInfo() {
      /**
    * 检查_impl_info->_type是否为nullptr，如果是，则输出错误信息并返回false。
    */
    if (_impl_info->_type == nullptr) {
        std::cerr << "Invalid TypeSupport" << std::endl;
        return false;
    }
    /**
    * 设置DomainParticipant的QoS参数为默认值，并设置其名称。
    */
    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    pqos.name(_name);
    /**
    * 获取DomainParticipantFactory的实例，并创建一个DomainParticipant。
    */
    auto factory = efd::DomainParticipantFactory::get_instance();
    _impl_info->_participant = factory->create_participant(0, pqos);
    /**
    * 如果DomainParticipant创建失败，输出错误信息并返回false。
    */
    if (_impl_info->_participant == nullptr) {
        std::cerr << "Failed to create DomainParticipant" << std::endl;
        return false;
    }
    /**
    * 注册类型到DomainParticipant。
    */
    _impl_info->_type.register_type(_impl_info->_participant);
    /**
    * 设置Publisher的QoS参数为默认值，并创建一个Publisher。
    */
    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
    _impl_info->_publisher = _impl_info->_participant->create_publisher(pubqos, nullptr);
    /**
    * 如果Publisher创建失败，输出错误信息并返回false。
    */
    if (_impl_info->_publisher == nullptr) {
      std::cerr << "Failed to create Publisher" << std::endl;
      return false;
    }
    /**
    * 设置Topic的QoS参数为默认值，并构造Topic名称。
    */
    efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
    const std::string publisher_type {"/camera_info"};
    const std::string base { "rt/carla/" };
    std::string topic_name = base;
    if (!_parent.empty())
      topic_name += _parent + "/";
    topic_name += _name;
    topic_name += publisher_type;
    /**
    * 创建一个Topic。
    */
    _impl_info->_topic = _impl_info->_participant->create_topic(topic_name, _impl_info->_type->getName(), tqos);
    /**
    * 如果Topic创建失败，输出错误信息并返回false。
    */
    if (_impl_info->_topic == nullptr) {
        std::cerr << "Failed to create Topic" << std::endl;
        return false;
    }
    /**
    * 设置DataWriter的QoS参数为默认值，并创建一个DataWriter。
    */
    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
    efd::DataWriterListener* listener = (efd::DataWriterListener*)_impl_info->_listener._impl.get();
    _impl_info->_datawriter = _impl_info->_publisher->create_datawriter(_impl_info->_topic, wqos, listener);
    /**
   * 如果DataWriter创建失败，输出错误信息并返回false。
   */
    if (_impl_info->_datawriter == nullptr) {
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
 * @brief 发布图像和信息
 *
 * 此函数调用PublishImage和PublishInfo函数来发布图像和相关信息。
 * 如果两个函数都成功返回true，则此函数也返回true；否则返回false。
 *
 * @return bool 如果发布成功返回true，否则返回false。
 */
  bool CarlaSSCameraPublisher::Publish() {
    return PublishImage() && PublishInfo();
  }

  bool CarlaSSCameraPublisher::PublishImage() {
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

  bool CarlaSSCameraPublisher::PublishInfo() {
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

  void CarlaSSCameraPublisher::SetImageData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, const uint8_t* data) {
    std::vector<uint8_t> vector_data;
    const size_t size = height * width * 4;
    vector_data.resize(size);
    std::memcpy(&vector_data[0], &data[0], size);
    SetData(seconds, nanoseconds, height, width, std::move(vector_data));
  }

  void CarlaSSCameraPublisher::SetInfoRegionOfInterest( uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, bool do_rectify) {
    sensor_msgs::msg::RegionOfInterest roi;
    roi.x_offset(x_offset);
    roi.y_offset(y_offset);
    roi.height(height);
    roi.width(width);
    roi.do_rectify(do_rectify);
    _impl_info->_info.roi(roi);
  }

  void CarlaSSCameraPublisher::SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, std::vector<uint8_t>&& data) {
    builtin_interfaces::msg::Time time;
    time.sec(seconds);
    time.nanosec(nanoseconds);

    std_msgs::msg::Header header;
    header.stamp(std::move(time));
    header.frame_id(_frame_id);

    _impl->_image.header(std::move(header));
    _impl->_image.width(width);
    _impl->_image.height(height);
    _impl->_image.encoding("bgra8"); //taken from the list of strings in include/sensor_msgs/image_encodings.h
    _impl->_image.is_bigendian(0);
    _impl->_image.step(_impl->_image.width() * sizeof(uint8_t) * 4);
    _impl->_image.data(std::move(data)); //https://github.com/eProsima/Fast-DDS/issues/2330
  }

  void CarlaSSCameraPublisher::SetCameraInfoData(int32_t seconds, uint32_t nanoseconds) {
    builtin_interfaces::msg::Time time;
    time.sec(seconds);
    time.nanosec(nanoseconds);

    std_msgs::msg::Header header;
    header.stamp(std::move(time));
    header.frame_id(_frame_id);
    _impl_info->_info.header(header);
  }

  CarlaSSCameraPublisher::CarlaSSCameraPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaSSCameraPublisherImpl>()),
  _impl_info(std::make_shared<CarlaCameraInfoPublisherImpl>()) {
    _name = ros_name;
    _parent = parent;
  }

  CarlaSSCameraPublisher::~CarlaSSCameraPublisher() {
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

  CarlaSSCameraPublisher::CarlaSSCameraPublisher(const CarlaSSCameraPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    _impl_info = other._impl_info;
  }

  CarlaSSCameraPublisher& CarlaSSCameraPublisher::operator=(const CarlaSSCameraPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    _impl_info = other._impl_info;

    return *this;
  }

  CarlaSSCameraPublisher::CarlaSSCameraPublisher(CarlaSSCameraPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    _impl_info = std::move(other._impl_info);
  }

  CarlaSSCameraPublisher& CarlaSSCameraPublisher::operator=(CarlaSSCameraPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    _impl_info = std::move(other._impl_info);

    return *this;
  }
}
}
