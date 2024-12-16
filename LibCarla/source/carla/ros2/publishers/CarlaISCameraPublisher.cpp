#define _GLIBCXX_USE_CXX11_ABI 0// 定义了使用 C++11 ABI 的相关设置为 0，可能与链接的 C++ 库的 ABI 兼容性有关

#include "CarlaISCameraPublisher.h"

#include <string>

#include "carla/ros2/types/ImagePubSubTypes.h"
#include "carla/ros2/types/CameraInfoPubSubTypes.h"
#include "carla/ros2/listeners/CarlaListener.h"

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>

#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/qos/QosPolicies.h>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>

namespace carla {
namespace ros2 {

  namespace efd = eprosima::fastdds::dds;
  using erc = eprosima::fastrtps::types::ReturnCode_t;

  struct CarlaISCameraPublisherImpl {
    efd::DomainParticipant* _participant { nullptr };
    efd::Publisher* _publisher { nullptr };
    efd::Topic* _topic { nullptr };
    efd::DataWriter* _datawriter { nullptr };
    efd::TypeSupport _type { new sensor_msgs::msg::ImagePubSubType() };
    CarlaListener _listener {};
    sensor_msgs::msg::Image _image {};
  };

  struct CarlaCameraInfoPublisherImpl {
// 指向领域参与者对象的指针，用于参与 DDS 通信域，初始化为空指针
    efd::DomainParticipant* _participant { nullptr };
// 指向发布者对象的指针，用于发布相机信息消息，初始化为空指针
    efd::Publisher* _publisher { nullptr };
    efd::Topic* _topic { nullptr };
    efd::DataWriter* _datawriter { nullptr };
    efd::TypeSupport _type { new sensor_msgs::msg::CameraInfoPubSubType() };
    CarlaListener _listener {};
// 用于标记相机信息是否已经初始化的布尔变量，初始化为 false
    bool _init {false};
    sensor_msgs::msg::CameraInfo _info {};
  };// 初始化相机信息数据，设置相机信息的一些基本参数，如高度、宽度、视野等，并标记为已初始化

  bool CarlaISCameraPublisher::HasBeenInitialized() const {
    return _impl_info->_init;
    // 初始化相机信息数据，设置相机信息的一些基本参数，如高度、宽度、视野等，并标记为已初始化
  }

  void CarlaISCameraPublisher::InitInfoData(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, float fov, bool do_rectify) {
    _impl_info->_info = std::move(sensor_msgs::msg::CameraInfo(height, width, fov));
    SetInfoRegionOfInterest(x_offset, y_offset, height, width, do_rectify);
    _impl_info->_init = true;
  }
// 初始化整个 CarlaISCameraPublisher，包括图像和相机信息相关的初始化，只有两者都初始化成功才返回 true
  bool CarlaISCameraPublisher::Init() {
    return InitImage() && InitInfo();
  }

  bool CarlaISCameraPublisher::InitImage() {
    // 检查消息类型支持是否有效，如果为空指针则输出错误信息并返回 false
    if (_impl->_type == nullptr) {
        std::cerr << "Invalid TypeSupport" << std::endl;
        return false;
    }
    // 获取默认的领域参与者 QoS 设置，并设置其名称为传入的 _name
    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    pqos.name(_name);
    // 获取领域参与者工厂的单例实例
    auto factory = efd::DomainParticipantFactory::get_instance();
    _impl->_participant = factory->create_participant(0, pqos);
    if (_impl->_participant == nullptr) {
        std::cerr << "Failed to create DomainParticipant" << std::endl;
        return false;
    }
    _impl->_type.register_type(_impl->_participant);

    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
    // 在领域参与者上创建发布者对象，如果创建失败则输出错误信息并返回 false
    _impl->_publisher = _impl->_participant->create_publisher(pubqos, nullptr);
    if (_impl->_publisher == nullptr) {
      std::cerr << "Failed to create Publisher" << std::endl;
      return false;
    }

    efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
     // 定义发布者类型对应的字符串，这里是 "/image"，表示图像类型的消息发布
    const std::string publisher_type {"/image"};
    const std::string base { "rt/carla/" };
    std::string topic_name = base;
    // 如果父名称不为空，则将父名称添加到主题名称中
    if (!_parent.empty())
      topic_name += _parent + "/";
    topic_name += _name;
    topic_name += publisher_type;
    _impl->_topic = _impl->_participant->create_topic(topic_name, _impl->_type->getName(), tqos);
    if (_impl->_topic == nullptr) {
        std::cerr << "Failed to create Topic" << std::endl;
        return false;
    }
 // 定义发布者类型对应的字符串，这里是 "/image"，表示图像类型的消息发布
    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
    wqos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
     // 定义发布者类型对应的字符串，这里是 "/image"，表示图像类型的消息发布
    efd::DataWriterListener* listener = (efd::DataWriterListener*)_impl->_listener._impl.get();
    _impl->_datawriter = _impl->_publisher->create_datawriter(_impl->_topic, wqos, listener);
    if (_impl->_datawriter == nullptr) {
        std::cerr << "Failed to create DataWriter" << std::endl;
        return false;
    } // 定义发布者类型对应的字符串，这里是 "/image"，表示图像类型的消息发布
    _frame_id = _name;
    return true;
  }

  bool CarlaISCameraPublisher::InitInfo() {
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
    }//注册消息类型到领域参与者，以便 DDS 系统能够识别和处理该类型的消息
    _impl_info->_type.register_type(_impl_info->_participant);

    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
     // 在领域参与者上创建发布者对象，如果创建失败则输出错误信息并返回 false
    _impl_info->_publisher = _impl_info->_participant->create_publisher(pubqos, nullptr);
    if (_impl_info->_publisher == nullptr) {
      std::cerr << "Failed to create Publisher" << std::endl;
      return false;
    }
// 获取默认的主题 QoS 设置
    efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
    const std::string publisher_type {"/camera_info"};
    const std::string base { "rt/carla/" };
    std::string topic_name = base;
    // 如果父名称不为空，则将父名称添加到主题名称中
    if (!_parent.empty())
      topic_name += _parent + "/";
    // 将自身名称和发布者类型添加到主题名称中，形成完整的主题名称
    topic_name += _name;
    topic_name += publisher_type;
    _impl_info->_topic = _impl_info->_participant->create_topic(topic_name, _impl_info->_type->getName(), tqos);
    if (_impl_info->_topic == nullptr) {
        std::cerr << "Failed to create Topic" << std::endl;
        return false;
    }
    // 获取默认的数据写入器 QoS 设置，并获取监听器对象指针，用于后续创建数据写入器时传入，以便监听写入操作相关事件
    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
    efd::DataWriterListener* listener = (efd::DataWriterListener*)_impl_info->_listener._impl.get();
    _impl_info->_datawriter = _impl_info->_publisher->create_datawriter(_impl_info->_topic, wqos, listener);
    if (_impl_info->_datawriter == nullptr) {
        std::cerr << "Failed to create DataWriter" << std::endl;
        return false;
    }
// 设置相机信息消息的帧 ID 为自身名称
    _frame_id = _name;
    return true;
  }
// 调用数据写入器的写入方法，尝试将图像消息写入 DDS 网络，并获取返回码
  bool CarlaISCameraPublisher::Publish() {
    return PublishImage() && PublishInfo();
  }
// 发布图像消息，根据数据写入器的写入操作返回码判断是否发布成功，如果失败则输出相应错误信息并返回 false
  bool CarlaISCameraPublisher::PublishImage() {
     // 定义实例句柄，可能用于标识要写入的消息实例，具体作用与 DDS 内部机制相关
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    // 调用数据写入器的写入方法，尝试将图像消息写入 DDS 网络，并获取返回码
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

  bool CarlaISCameraPublisher::PublishInfo() {
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

  void CarlaISCameraPublisher::SetImageData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, const uint8_t* data) {
    std::vector<uint8_t> vector_data;
    const size_t size = height * width * 4;
    vector_data.resize(size);
    std::memcpy(&vector_data[0], &data[0], size);
    SetData(seconds, nanoseconds, height, width, std::move(vector_data));
  }

  void CarlaISCameraPublisher::SetInfoRegionOfInterest( uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, bool do_rectify) {
    sensor_msgs::msg::RegionOfInterest roi;
    roi.x_offset(x_offset);
    roi.y_offset(y_offset);
    roi.height(height);
    roi.width(width);
    roi.do_rectify(do_rectify);
    _impl_info->_info.roi(roi);
  }

  void CarlaISCameraPublisher::SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, std::vector<uint8_t>&& data) {
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

  void CarlaISCameraPublisher::SetCameraInfoData(int32_t seconds, uint32_t nanoseconds) {
    builtin_interfaces::msg::Time time;
    time.sec(seconds);
    time.nanosec(nanoseconds);

    std_msgs::msg::Header header;
    header.stamp(std::move(time));
    header.frame_id(_frame_id);
    _impl_info->_info.header(header);
  }

  CarlaISCameraPublisher::CarlaISCameraPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaISCameraPublisherImpl>()),
  _impl_info(std::make_shared<CarlaCameraInfoPublisherImpl>()) {
    _name = ros_name;
    _parent = parent;
  }

  CarlaISCameraPublisher::~CarlaISCameraPublisher() {
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

  CarlaISCameraPublisher::CarlaISCameraPublisher(const CarlaISCameraPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    _impl_info = other._impl_info;
  }

  CarlaISCameraPublisher& CarlaISCameraPublisher::operator=(const CarlaISCameraPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    _impl_info = other._impl_info;

    return *this;
  }

  CarlaISCameraPublisher::CarlaISCameraPublisher(CarlaISCameraPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    _impl_info = std::move(other._impl_info);
  }

  CarlaISCameraPublisher& CarlaISCameraPublisher::operator=(CarlaISCameraPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    _impl_info = std::move(other._impl_info);

    return *this;
  }
}
}
