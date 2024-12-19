#define _GLIBCXX_USE_CXX11_ABI 0

#include "CarlaISCameraPublisher.h"// 引入自定义的相机发布器头文件

#include <string>// 引入字符串库

#include "carla/ros2/types/ImagePubSubTypes.h"// 引入图像类型的发布/订阅类型
#include "carla/ros2/types/CameraInfoPubSubTypes.h"// 引入相机信息类型的发布/订阅类型
#include "carla/ros2/listeners/CarlaListener.h"// 引入Carla监听器

#include <fastdds/dds/domain/DomainParticipant.hpp>// 引入Fast DDS的DomainParticipant类
#include <fastdds/dds/publisher/Publisher.hpp>// 引入Fast DDS的Publisher类
#include <fastdds/dds/topic/Topic.hpp>// 引入Fast DDS的Topic类
#include <fastdds/dds/publisher/DataWriter.hpp> // 引入Fast DDS的DataWriter类
#include <fastdds/dds/topic/TypeSupport.hpp> // 引入Fast DDS的TypeSupport类

#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>// 引入DomainParticipant的QOS设置
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>// 引入DomainParticipant工厂类
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>// 引入Publisher的QOS设置
#include <fastdds/dds/topic/qos/TopicQos.hpp> // 引入Topic的QOS设置

#include <fastrtps/attributes/ParticipantAttributes.h>// 引入Participant属性
#include <fastrtps/qos/QosPolicies.h>// 引入QOS策略
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>// 引入DataWriter的QOS设置
#include <fastdds/dds/publisher/DataWriterListener.hpp>// 引入DataWriter监听器

namespace carla {
namespace ros2 {
// 使用Fast DDS命名空间
  namespace efd = eprosima::fastdds::dds;
  using erc = eprosima::fastrtps::types::ReturnCode_t;// 定义返回码类型
// CarlaISCameraPublisher的实现结构体
  struct CarlaISCameraPublisherImpl {
    efd::DomainParticipant* _participant { nullptr };// DomainParticipant指针
    efd::Publisher* _publisher { nullptr };// Publisher指针 
    efd::Topic* _topic { nullptr };// Topic指针
    efd::DataWriter* _datawriter { nullptr };// DataWriter指针
    efd::TypeSupport _type { new sensor_msgs::msg::ImagePubSubType() }; // 图像类型支持
    CarlaListener _listener {};// Carla监听器实例
    sensor_msgs::msg::Image _image {};// 存储图像数据
  };
// CarlaCameraInfoPublisher的实现结构体
  struct CarlaCameraInfoPublisherImpl {
    efd::DomainParticipant* _participant { nullptr };// DomainParticipant指针
    efd::Publisher* _publisher { nullptr };// Publisher指针
    efd::Topic* _topic { nullptr }; // Topic指针
    efd::DataWriter* _datawriter { nullptr };// DataWriter指针
    efd::TypeSupport _type { new sensor_msgs::msg::CameraInfoPubSubType() }; // 图像类型支持
    CarlaListener _listener {};// Carla监听器实例
    bool _init {false};// 初始化标志
    sensor_msgs::msg::CameraInfo _info {};// 存储相机信息  
  };
// 检查相机发布器是否已初始化
  bool CarlaISCameraPublisher::HasBeenInitialized() const {
    return _impl_info->_init;// 返回初始化状态
  }

  void CarlaISCameraPublisher::InitInfoData(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, float fov, bool do_rectify) {
    _impl_info->_info = std::move(sensor_msgs::msg::CameraInfo(height, width, fov));// 移动构造相机信息
    SetInfoRegionOfInterest(x_offset, y_offset, height, width, do_rectify);// 设置感兴趣区域
    _impl_info->_init = true;// 标记为已初始化 
  }
// 初始化相机发布器
  bool CarlaISCameraPublisher::Init() {
    return InitImage() && InitInfo();// 初始化图像和相机信息
  }
// 初始化图像发布器 
  bool CarlaISCameraPublisher::InitImage() {
    if (_impl->_type == nullptr) {
        std::cerr << "Invalid TypeSupport" << std::endl; // 检查类型支持
        return false;// 返回失败
    }
    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT; // 默认QOS设置
    pqos.name(_name);// 设置参与者名称 
    auto factory = efd::DomainParticipantFactory::get_instance(); // 获取参与者工厂实例
    _impl->_participant = factory->create_participant(0, pqos);// 创建DomainParticipant 
    if (_impl->_participant == nullptr) {
        std::cerr << "Failed to create DomainParticipant" << std::endl;// 创建失败 
        return false;// 返回失败 
    }
    _impl->_type.register_type(_impl->_participant);// 注册类型 

    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;// 默认Publisher QOS设置
    _impl->_publisher = _impl->_participant->create_publisher(pubqos, nullptr);// 创建Publisher
    if (_impl->_publisher == nullptr) {
      std::cerr << "Failed to create Publisher" << std::endl;// 创建失败
      return false;// 返回失败
    }

    efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;// 默认Topic QOS设置 
    const std::string publisher_type {"/image"};// 图像主题类型
    const std::string base { "rt/carla/" };// 基本主题名称
    std::string topic_name = base;// 主题名称
    if (!_parent.empty())
      topic_name += _parent + "/";// 添加父主题
    topic_name += _name;// 添加当前名称 
    topic_name += publisher_type; // 添加图像主题类型
    _impl->_topic = _impl->_participant->create_topic(topic_name, _impl->_type->getName(), tqos);
    if (_impl->_topic == nullptr) {
        std::cerr << "Failed to create Topic" << std::endl;//创建失败  
        return false;//返回失败
    }

    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;// 默认DataWriter QOS设置
    wqos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE; // 设置内存策略 
    efd::DataWriterListener* listener = (efd::DataWriterListener*)_impl->_listener._impl.get();// 获取监听器
    _impl->_datawriter = _impl->_publisher->create_datawriter(_impl->_topic, wqos, listener);// 创建DataWriter 
    if (_impl->_datawriter == nullptr) {
        std::cerr << "Failed to create DataWriter" << std::endl;// 创建失败
        return false;// 返回失败
    }
    _frame_id = _name;// 设置帧ID 
    return true;// 返回成功
  }
// 初始化相机信息发布器 
  bool CarlaISCameraPublisher::InitInfo() {
    if (_impl_info->_type == nullptr) {
        std::cerr << "Invalid TypeSupport" << std::endl; // 检查类型支持
        return false;// 返回失败
    }

    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;// 默认QOS设置
    pqos.name(_name);// 设置参与者名称
    auto factory = efd::DomainParticipantFactory::get_instance();// 获取参与者工厂实例
    _impl_info->_participant = factory->create_participant(0, pqos);// 创建DomainParticipant 
    if (_impl_info->_participant == nullptr) {
        std::cerr << "Failed to create DomainParticipant" << std::endl;// 创建失败
        return false; // 返回失败
    }
    _impl_info->_type.register_type(_impl_info->_participant);// 注册类型 

    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;// 默认Publisher QOS设置
    _impl_info->_publisher = _impl_info->_participant->create_publisher(pubqos, nullptr);// 创建Publisher
    if (_impl_info->_publisher == nullptr) {
      std::cerr << "Failed to create Publisher" << std::endl;// 创建失败
      return false;// 返回失败
    }

    efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT; // 默认Topic QOS设置
    const std::string publisher_type {"/camera_info"};// 相机信息主题类型
    const std::string base { "rt/carla/" };// 基本主题名称 
    std::string topic_name = base;// 主题名称
    if (!_parent.empty())
      topic_name += _parent + "/";// 添加父主题 
    topic_name += _name;// 添加当前名称
    topic_name += publisher_type; // 添加相机信息主题类型 
    _impl_info->_topic = _impl_info->_participant->create_topic(topic_name, _impl_info->_type->getName(), tqos);
    if (_impl_info->_topic == nullptr) {
        std::cerr << "Failed to create Topic" << std::endl;//创建失败
        return false;//返回失败
    }
    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;// 默认DataWriter QOS设置
    efd::DataWriterListener* listener = (efd::DataWriterListener*)_impl_info->_listener._impl.get();// 获取监听器
    _impl_info->_datawriter = _impl_info->_publisher->create_datawriter(_impl_info->_topic, wqos, listener);// 创建DataWriter 
    if (_impl_info->_datawriter == nullptr) {
        std::cerr << "Failed to create DataWriter" << std::endl; // 创建失败
        return false;// 返回失败
    }

    _frame_id = _name;// 设置帧ID 
    return true; // 返回成功
  }
// 发布图像和相机信息
  bool CarlaISCameraPublisher::Publish() {
    return PublishImage() && PublishInfo();// 发布图像和信息
  }
// 发布图像
  bool CarlaISCameraPublisher::PublishImage() {
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    eprosima::fastrtps::types::ReturnCode_t rcode = _impl->_datawriter->write(&_impl->_image, instance_handle);
    if (rcode == erc::ReturnCodeValue::RETCODE_OK) {
        return true;
    }
    // 处理各种返回码并输出错误信息
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
// 发布相机信息
  bool CarlaISCameraPublisher::PublishInfo() {
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    eprosima::fastrtps::types::ReturnCode_t rcode = _impl_info->_datawriter->write(&_impl_info->_info, instance_handle);
    if (rcode == erc::ReturnCodeValue::RETCODE_OK) {
        return true;
    }
    // 处理各种返回码并输出错误信息
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
// 设置图像数据
  void CarlaISCameraPublisher::SetImageData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, const uint8_t* data) {
    std::vector<uint8_t> vector_data;// 创建数据向量
    const size_t size = height * width * 4;// 计算数据大小（假设为BGRA格式）
    vector_data.resize(size);// 调整向量大小
    std::memcpy(&vector_data[0], &data[0], size); // 复制数据
    SetData(seconds, nanoseconds, height, width, std::move(vector_data));// 设置数据 
  }
// 设置相机信息的感兴趣区域
  void CarlaISCameraPublisher::SetInfoRegionOfInterest( uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, bool do_rectify) {
    sensor_msgs::msg::RegionOfInterest roi;// 创建感兴趣区域对象
    roi.x_offset(x_offset);// 设置x偏移
    roi.y_offset(y_offset);// 设置y偏移
    roi.height(height);// 设置高度  
    roi.width(width);// 设置宽度
    roi.do_rectify(do_rectify);// 设置是否进行矫正 
    _impl_info->_info.roi(roi);// 设置相机信息中的感兴趣区域
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
// 拷贝构造函数 
  CarlaISCameraPublisher::CarlaISCameraPublisher(const CarlaISCameraPublisher& other) {
    _frame_id = other._frame_id;// 拷贝帧ID  
    _name = other._name; // 拷贝ROS名称  
    _parent = other._parent;// 拷贝父主题  
    _impl = other._impl; // 拷贝实现结构体  
    _impl_info = other._impl_info;// 拷贝相机信息实现结构体
  }
// 拷贝赋值运算符 
  CarlaISCameraPublisher& CarlaISCameraPublisher::operator=(const CarlaISCameraPublisher& other) {
    _frame_id = other._frame_id;// 拷贝帧ID 
    _name = other._name;// 拷贝ROS名称 
    _parent = other._parent;// 拷贝父主题 
    _impl = other._impl;// 拷贝实现结构体
    _impl_info = other._impl_info;// 拷贝相机信息实现结构体 

    return *this;// 返回自身  
  }
// 移动构造函数  
  CarlaISCameraPublisher::CarlaISCameraPublisher(CarlaISCameraPublisher&& other) {
    _frame_id = std::move(other._frame_id);// 移动帧ID
    _name = std::move(other._name); // 移动ROS名称
    _parent = std::move(other._parent); // 移动父主题 
    _impl = std::move(other._impl);// 移动实现结构体 
    _impl_info = std::move(other._impl_info); // 移动相机信息实现结构体
  }
// 移动赋值运算符 
  CarlaISCameraPublisher& CarlaISCameraPublisher::operator=(CarlaISCameraPublisher&& other) {
    _frame_id = std::move(other._frame_id);// 移动帧ID  
    _name = std::move(other._name);// 移动ROS名称  
    _parent = std::move(other._parent);// 移动父主题  
    _impl = std::move(other._impl);// 移动实现结构体
    _impl_info = std::move(other._impl_info);// 移动相机信息实现结构体 

    return *this;// 返回自身
  }
}
}
