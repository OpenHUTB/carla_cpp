#define _GLIBCXX_USE_CXX11_ABI 0
// 包含Carla RGBCamera发布者相关的头文件，推测其中定义了CarlaRGBCameraPublisher类的声明等内容
#include "CarlaRGBCameraPublisher.h"
// 引入C++标准库中的字符串头文件，用于处理字符串相关操作
#include <string>
// 引入Carla项目中ROS2相关的图像发布/订阅类型定义头文件，用于在ROS2环境下处理图像数据的发布和订阅
#include "carla/ros2/types/ImagePubSubTypes.h"// 引入Carla项目中ROS2相关的相机信息发布/订阅类型定义头文件，用于处理相机参数等信息的发布和订阅
#include "carla/ros2/types/CameraInfoPubSubTypes.h"// 引入Carla项目中ROS2相关的监听器（Listener）头文件，可能用于监听一些事件、消息等
#include "carla/ros2/listeners/CarlaListener.h"
// 引入eProsima Fast DDS库中与域参与者（DomainParticipant）相关的头文件，
// 域参与者是Fast DDS中用于参与数据分发服务的一个核心概念，多个参与者可以在同一个域内进行通信
#include <fastdds/dds/domain/DomainParticipant.hpp> // 引入eProsima Fast DDS库中与发布者（Publisher）相关的头文件，用于创建发布者对象来发布数据
#include <fastdds/dds/publisher/Publisher.hpp>// 引入eProsima Fast DDS库中与主题（Topic）相关的头文件，主题用于定义发布和订阅的数据类型和名称等信息
#include <fastdds/dds/topic/Topic.hpp> // 引入eProsima Fast DDS库中与数据写入器（DataWriter）相关的头文件，用于将数据写入到对应的主题中进行发布
#include <fastdds/dds/publisher/DataWriter.hpp>// 引入eProsima Fast DDS库中与类型支持（TypeSupport）相关的头文件，用于注册和管理数据类型
#include <fastdds/dds/topic/TypeSupport.hpp>
// 引入eProsima Fast DDS库中与服务质量（QoS，Quality of Service）相关的头文件，用于配置各种服务质量参数，如可靠性、实时性等
// 服务质量 (QoS) 
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>

// RTPS规范，全称为实时发布/订阅 协议DDS互操作规范（The Real-Time Publish-Subscribe Protocol (RTPS) DDS Interoperability Wire Protocol Specification）
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/qos/QosPolicies.h>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>

// 定义在Carla项目的ros2命名空间下，方便组织和管理与ROS2相关的代码
namespace carla {
namespace ros2 {

// 为eProsima Fast DDS库的相关命名空间创建别名efd，方便后续代码书写，减少代码冗长度
  namespace efd = eprosima::fastdds::dds;// 为eProsima Fast RTPS库中的返回码类型（ReturnCode_t）创建别名erc，方便后续使用返回码进行错误处理等操作
  using erc = eprosima::fastrtps::types::ReturnCode_t;
// CarlaRGBCameraPublisherImpl结构体，用于存储Carla RGBCamera发布者的具体实现相关的内部数据成员，
    // 可以看作是对CarlaRGBCameraPublisher类内部实现细节的一种封装，将相关的数据和操作放在一起
  struct CarlaRGBCameraPublisherImpl {// 指向域参与者对象的指针，用于参与Fast DDS的数据分发服务，初始化为nullptr
    efd::DomainParticipant* _participant { nullptr };// 指向发布者对象的指针，用于发布数据，初始化为nullptr
    efd::Publisher* _publisher { nullptr };// 指向主题对象的指针，用于定义发布的数据主题相关信息，初始化为nullptr
    efd::Topic* _topic { nullptr }; // 指向数据写入器对象的指针，用于将实际数据写入到主题中进行发布，初始化为nullptr
    efd::DataWriter* _datawriter { nullptr }; // 类型支持对象，用于注册和管理要发布的图像数据类型，这里初始化为sensor_msgs::msg::ImagePubSubType类型的实例
    efd::TypeSupport _type { new sensor_msgs::msg::ImagePubSubType() };// CarlaListener对象，用于监听相关事件，具体功能取决于CarlaListener的实现
    CarlaListener _listener {}; // 用于存储要发布的图像消息数据，初始化为默认构造的空图像消息
    sensor_msgs::msg::Image _image {};
  };
// CarlaCameraInfoPublisherImpl结构体，与CarlaRGBCameraPublisherImpl类似，
    // 不过是用于存储相机信息发布相关的内部数据成员，比如相机的参数信息等
  struct CarlaCameraInfoPublisherImpl {
    efd::DomainParticipant* _participant { nullptr };
    efd::Publisher* _publisher { nullptr };
    efd::Topic* _topic { nullptr };
    efd::DataWriter* _datawriter { nullptr };
    efd::TypeSupport _type { new sensor_msgs::msg::CameraInfoPubSubType() };
    CarlaListener _listener {};// 用于标记相机信息是否已经初始化，初始化为false
    bool _init {false};// 用于存储要发布的相机信息消息数据，初始化为默认构造的空相机信息消息
    sensor_msgs::msg::CameraInfo _info {};
  };
// 判断Carla RGBCamera发布者是否已经初始化的函数，通过查看CarlaCameraInfoPublisherImpl结构体中的_init成员来判断
  bool CarlaRGBCameraPublisher::HasBeenInitialized() const {
    return _impl_info->_init;
  }
// 初始化相机信息数据的函数，根据传入的相机参数（偏移量、高度、宽度、视场角等）构建相机信息消息，
    // 并设置感兴趣区域（ROI）信息，最后将初始化标记置为true，表示相机信息已初始化
  void CarlaRGBCameraPublisher::InitInfoData(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, float fov, bool do_rectify) {
    _impl_info->_info = std::move(sensor_msgs::msg::CameraInfo(height, width, fov));
    SetInfoRegionOfInterest(x_offset, y_offset, height, width, do_rectify);
    _impl_info->_init = true;
  }
// 初始化整个Carla RGBCamera发布者的函数，调用InitImage和InitInfo函数分别对图像发布和相机信息发布相关的部分进行初始化，
    // 只有当这两部分都初始化成功时才返回true，否则返回false
  bool CarlaRGBCameraPublisher::Init() {
    return InitImage() && InitInfo();
  }
// 初始化图像发布相关部分的函数，主要完成以下步骤：
    // 1. 检查类型支持对象是否有效，若无效则输出错误信息并返回false。
    // 2. 设置域参与者的服务质量参数（QoS），包括设置名称等，然后通过域参与者工厂创建域参与者对象，若创建失败则返回false。
    // 3. 使用创建好的域参与者对象注册要发布的图像数据类型。
    // 4. 设置发布者的服务质量参数并创建发布者对象，若创建失败则返回false。
    // 5. 设置主题的服务质量参数，根据给定的规则构建主题名称，然后创建主题对象，若创建失败则返回false。
    // 6. 设置数据写入器的服务质量参数，获取对应的监听器对象，然后创建数据写入器对象，若创建失败则返回false。
    // 7. 设置图像数据的帧ID，最后返回true表示初始化成功。
  bool CarlaRGBCameraPublisher::InitImage() {
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
// 初始化相机信息发布相关部分的函数，与InitImage函数的逻辑类似，不过操作的对象是相机信息相关的数据成员和组件，
    // 同样完成创建域参与者、发布者、主题、数据写入器等一系列操作，若过程中任何一步失败则返回false，全部成功则返回true
  bool CarlaRGBCameraPublisher::InitInfo() {
    if (_impl_info->_type == nullptr) {
        std::cerr << "Invalid TypeSupport" << std::endl;
        return false;
    }

    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    pqos.name(_name);
    auto factory = efd::DomainParticipantFactory::get_instance();
    _impl_info->_participant = factory->create_participant(0, pqos);
    if (_impl_info->_participant == nullptr) {
        std::cerr << "Failed to create DomainParticipant" << std::endl;
        return false;
    }
    _impl_info->_type.register_type(_impl_info->_participant);

    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
    _impl_info->_publisher = _impl_info->_participant->create_publisher(pubqos, nullptr);
    if (_impl_info->_publisher == nullptr) {
      std::cerr << "Failed to create Publisher" << std::endl;
      return false;
    }

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
    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
    efd::DataWriterListener* listener = (efd::DataWriterListener*)_impl_info->_listener._impl.get();
    _impl_info->_datawriter = _impl_info->_publisher->create_datawriter(_impl_info->_topic, wqos, listener);
    if (_impl_info->_datawriter == nullptr) {
        std::cerr << "Failed to create DataWriter" << std::endl;
        return false;
    }

    _frame_id = _name;
    return true;
  }
// 发布数据的函数，调用PublishImage和PublishInfo函数分别发布图像数据和相机信息数据，
    // 只有当这两部分数据都发布成功时才返回true，否则返回false
  bool CarlaRGBCameraPublisher::Publish() {
    return PublishImage() && PublishInfo();
  }
// 发布图像数据的函数，通过数据写入器尝试将存储的图像消息数据写入到对应的主题中进行发布，
    // 根据返回的结果码（rcode）判断是否发布成功，若返回码表示成功（RETCODE_OK）则返回true，
    // 否则根据不同的错误返回码输出相应的错误信息并返回false，若遇到未知的返回码也输出错误信息并返回false
  bool CarlaRGBCameraPublisher::PublishImage() {
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

  bool CarlaRGBCameraPublisher::PublishInfo() {
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
	// 执行信息的发布动作
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

void CarlaRGBCameraPublisher::SetImageData(int32_t seconds, uint32_t nanoseconds, uint32_t height, uint32_t width, const uint8_t* data) {
    std::vector<uint8_t> vector_data;
    const size_t size = height * width * 4;
    vector_data.resize(size);
    std::memcpy(&vector_data[0], &data[0], size);
    SetImageData(seconds, nanoseconds, height, width, std::move(vector_data));
  }

  void CarlaRGBCameraPublisher::SetImageData(int32_t seconds, uint32_t nanoseconds, uint32_t height, uint32_t width, std::vector<uint8_t>&& data) {

    builtin_interfaces::msg::Time time;
    time.sec(seconds);
    time.nanosec(nanoseconds);

    std_msgs::msg::Header header;
    header.stamp(std::move(time));
    header.frame_id(_frame_id);
    _impl->_image.header(header);

    _impl->_image.width(width);
    _impl->_image.height(height);
    _impl->_image.encoding("bgra8");
    _impl->_image.is_bigendian(0);
    _impl->_image.step(_impl->_image.width() * sizeof(uint8_t) * 4);
    _impl->_image.data(std::move(data));
  }

  void CarlaRGBCameraPublisher::SetCameraInfoData(int32_t seconds, uint32_t nanoseconds) {
    builtin_interfaces::msg::Time time;
    time.sec(seconds);
    time.nanosec(nanoseconds);

    std_msgs::msg::Header header;
    header.stamp(std::move(time));
    header.frame_id(_frame_id);
    _impl_info->_info.header(header);
  }

  void CarlaRGBCameraPublisher::SetInfoRegionOfInterest( uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, bool do_rectify) {
    sensor_msgs::msg::RegionOfInterest roi;
    roi.x_offset(x_offset);
    roi.y_offset(y_offset);
    roi.height(height);
    roi.width(width);
    roi.do_rectify(do_rectify);
    _impl_info->_info.roi(roi);
  }

  CarlaRGBCameraPublisher::CarlaRGBCameraPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaRGBCameraPublisherImpl>()),
  _impl_info(std::make_shared<CarlaCameraInfoPublisherImpl>()) {
    _name = ros_name;
    _parent = parent;
  }

  CarlaRGBCameraPublisher::~CarlaRGBCameraPublisher() {
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

  CarlaRGBCameraPublisher::CarlaRGBCameraPublisher(const CarlaRGBCameraPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    _impl_info = other._impl_info;
  }

  CarlaRGBCameraPublisher& CarlaRGBCameraPublisher::operator=(const CarlaRGBCameraPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    _impl_info = other._impl_info;

    return *this;
  }

  CarlaRGBCameraPublisher::CarlaRGBCameraPublisher(CarlaRGBCameraPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    _impl_info = std::move(other._impl_info);
  }

  CarlaRGBCameraPublisher& CarlaRGBCameraPublisher::operator=(CarlaRGBCameraPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    _impl_info = std::move(other._impl_info);

    return *this;
  }
}
}
