#define _GLIBCXX_USE_CXX11_ABI 0

#include "CarlaOpticalFlowCameraPublisher.h"// 引入Carla光流相机发布者的头文件

#include <string>// 引入字符串处理的标准库
#include <cmath>// 引入数学计算的标准库（可能用于图像处理或数据转换）
// 引入Carla ROS 2类型定义的头文件，用于序列化/反序列化图像和相机信息数据
#include "carla/ros2/types/ImagePubSubTypes.h"// 引入图像消息类型的定义
#include "carla/ros2/types/CameraInfoPubSubTypes.h"// 引入相机信息消息类型的定义
#include "carla/ros2/listeners/CarlaListener.h"// 引入Carla监听器的头文件
// 引入FastDDS相关的头文件，用于实现DDS（Data Distribution Service）通信
#include <fastdds/dds/domain/DomainParticipant.hpp> // 引入域参与者的类定义
#include <fastdds/dds/publisher/Publisher.hpp>// 引入发布者的类定义
#include <fastdds/dds/topic/Topic.hpp>// 引入主题的类定义
#include <fastdds/dds/publisher/DataWriter.hpp> // 引入数据写入器的类定义
#include <fastdds/dds/topic/TypeSupport.hpp>// 引入类型支持的类定义（用于序列化/反序列化）
// 引入FastDDS的QoS（Quality of Service）相关的头文件
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>// 引入域参与者QoS配置的类定义
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>// 引入域参与者工厂的类定义
#include <fastdds/dds/publisher/qos/PublisherQos.hpp> // 引入发布者QoS配置的类定义
#include <fastdds/dds/topic/qos/TopicQos.hpp>// 引入主题QoS配置的类定义
// 引入FastRTPS（FastDDS的底层实现）相关的头文件，用于配置参与者属性和QoS策略
#include <fastrtps/attributes/ParticipantAttributes.h>// 引入参与者属性的类定义
#include <fastrtps/qos/QosPolicies.h> // 引入QoS策略的类定义
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>// 引入数据写入器QoS配置的类定义
#include <fastdds/dds/publisher/DataWriterListener.hpp>// 引入数据写入器监听器的类定义（用于处理写入事件）
/**
 * @brief 通用CLAMP函数，用于将值限制在指定范围内。
 *
 * @param value 要限制的值。
 * @param low 范围的下限。
 * @param high 范围的上限。
 * @return 限制后的值。
 * @template T 值的类型，需要支持比较操作。
 */
template <typename T> T CLAMP(const T& value, const T& low, const T& high)
{
  return value < low ? low : (value > high ? high : value);
}
/**
 * @namespace carla
 * @brief Carla项目的命名空间。
 */
 /**
  * @namespace carla::ros2
  * @brief Carla项目中与ROS 2相关的功能的命名空间。
  */
namespace carla {
namespace ros2 {
    /**
 * @brief FastDDS DDS命名空间的别名，用于简化代码。
 */
  namespace efd = eprosima::fastdds::dds;
  /**
 * @brief FastRTPS返回码类型的别名，用于简化代码。
 */
  using erc = eprosima::fastrtps::types::ReturnCode_t;
  /**
 * @brief Carla光流相机发布者内部实现的结构体。
 *
 * 包含了发布图像数据所需的FastDDS组件和辅助数据。
 */
  struct CarlaOpticalFlowCameraPublisherImpl {
    efd::DomainParticipant* _participant { nullptr };///< 域参与者，用于创建其他DDS实体。
    efd::Publisher* _publisher { nullptr };///< 发布者，用于发送数据。
    efd::Topic* _topic { nullptr };///< 主题，定义了发布的数据类型。
    efd::DataWriter* _datawriter { nullptr };///< 数据写入器，用于将数据写入主题。
    efd::TypeSupport _type { new sensor_msgs::msg::ImagePubSubType() };///< 类型支持，用于序列化和反序列化图像数据。
    CarlaListener _listener {}; ///< Carla监听器，可能用于接收相关事件或数据。
    sensor_msgs::msg::Image _image {};///< 存储待发布的图像数据。
  };
  /**
 * @brief Carla相机信息发布者内部实现的结构体。
 *
 * 包含了发布相机信息数据所需的FastDDS组件和辅助数据。
 */
  struct CarlaCameraInfoPublisherImpl {
    efd::DomainParticipant* _participant { nullptr };///< 域参与者。
    efd::Publisher* _publisher { nullptr };///< 发布者。
    efd::Topic* _topic { nullptr }; ///< 主题。
    efd::DataWriter* _datawriter { nullptr };///< 数据写入器。
    efd::TypeSupport _type { new sensor_msgs::msg::CameraInfoPubSubType() };///< 类型支持，用于序列化和反序列化相机信息数据。
    CarlaListener _listener {};///< Carla监听器。
    bool _init { false};///< 初始化标志。
    sensor_msgs::msg::CameraInfo _info {};///< 存储待发布的相机信息数据。
  };
  /**
 * @brief 检查类是否已经被初始化。
 *
 * @return 如果类已经被初始化，则返回true；否则返回false。
 */
  bool CarlaOpticalFlowCameraPublisher::HasBeenInitialized() const {
    return _impl_info->_init;
  }
  /**
 * @brief 初始化摄像头信息数据。
 *
 * @param x_offset X轴偏移量
 * @param y_offset Y轴偏移量
 * @param height 图像高度
 * @param width 图像宽度
 * @param fov 视场角
 * @param do_rectify 是否进行校正
 */
  void CarlaOpticalFlowCameraPublisher::InitInfoData(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, float fov, bool do_rectify) {
    _impl_info->_info = std::move(sensor_msgs::msg::CameraInfo(height, width, fov));
    SetInfoRegionOfInterest(x_offset, y_offset, height, width, do_rectify);
    _impl_info->_init = true;
  }
  /**
 * @brief 初始化类。
 *
 * 调用InitImage()和InitInfo()进行初始化，并返回两者的逻辑与结果。
 *
 * @return 如果初始化成功，则返回true；否则返回false。
 */
  bool CarlaOpticalFlowCameraPublisher::Init() {
    return InitImage() && InitInfo();
  }
  /**
 * @brief 初始化图像相关的资源。
 *
 * 创建一个DomainParticipant、Publisher、Topic和DataWriter用于发布图像数据。
 *
 * @return 如果初始化成功，则返回true；否则返回false。
 */
  bool CarlaOpticalFlowCameraPublisher::InitImage() {
    if (_impl->_type == nullptr) {
        std::cerr << "Invalid TypeSupport" << std::endl;
        return false;
    }
    /// 设置DomainParticipant的QoS策略为默认值，并设置名称。
    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    pqos.name(_name);
    /// 获取DomainParticipantFactory的实例。
    auto factory = efd::DomainParticipantFactory::get_instance();
    /// 创建DomainParticipant。
    _impl->_participant = factory->create_participant(0, pqos);
    if (_impl->_participant == nullptr) {
        /// 如果创建DomainParticipant失败，输出错误信息并返回false。
        std::cerr << "Failed to create DomainParticipant" << std::endl;
        return false;
    }
    /// 在DomainParticipant中注册数据类型。
    _impl->_type.register_type(_impl->_participant);
    /// 设置Publisher的QoS策略为默认值。
    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
    /// 创建Publisher。
    _impl->_publisher = _impl->_participant->create_publisher(pubqos, nullptr);
    if (_impl->_publisher == nullptr) {
        /// 如果创建Publisher失败，输出错误信息并返回false。
      std::cerr << "Failed to create Publisher" << std::endl;
      return false;
    }
    /// 设置Topic的QoS策略为默认值。
    efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
    /// 构建Topic的名称。
    const std::string publisher_type {"/image"};
    const std::string base { "rt/carla/" };
    std::string topic_name = base;
    if (!_parent.empty())
      topic_name += _parent + "/";
    topic_name += _name;
    topic_name += publisher_type;
    /// 创建Topic。
    _impl->_topic = _impl->_participant->create_topic(topic_name, _impl->_type->getName(), tqos);
    if (_impl->_topic == nullptr) {
        /// 如果创建Topic失败，输出错误信息并返回false。
        std::cerr << "Failed to create Topic" << std::endl;
        return false;
    }
    /// 设置DataWriter的QoS策略为默认值，并修改历史内存策略。
    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
    wqos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    /// 获取DataWriter的监听器。
    efd::DataWriterListener* listener = (efd::DataWriterListener*)_impl->_listener._impl.get();
    /// 创建DataWriter。
    _impl->_datawriter = _impl->_publisher->create_datawriter(_impl->_topic, wqos, listener);
    if (_impl->_datawriter == nullptr) {
        /// 如果创建DataWriter失败，输出错误信息并返回false。
        std::cerr << "Failed to create DataWriter" << std::endl;
        return false;
    }
    /// 设置帧ID为名称。
    _frame_id = _name;
    /// 所有对象都成功创建，返回true。
    return true;
  }
  /**
 * @brief 初始化信息并设置CarlaOpticalFlowCameraPublisher的相关参数。
 *
 * 此函数负责初始化域参与者（DomainParticipant）、发布者（Publisher）、主题（Topic）和数据写入器（DataWriter），
 * 并确保它们被正确创建。如果任何一步失败，函数将输出错误信息并返回false。
 *
 * @return 如果初始化成功，返回true；否则返回false。
 */
  bool CarlaOpticalFlowCameraPublisher::InitInfo() {
      /**
     * 检查类型支持是否有效。如果_impl_info->_type为空，则输出错误信息并返回false。
     */
    if (_impl_info->_type == nullptr) {
        std::cerr << "Invalid TypeSupport" << std::endl;
        return false;
    }
    /**
     * 设置域参与者的QoS策略，并将其命名为_name。
     */
    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    pqos.name(_name);
    auto factory = efd::DomainParticipantFactory::get_instance();
    _impl_info->_participant = factory->create_participant(0, pqos);
    /**
     * 如果域参与者创建失败，输出错误信息并返回false。
     */
    if (_impl_info->_participant == nullptr) {
        std::cerr << "Failed to create DomainParticipant" << std::endl;
        return false;
    }
    /**
     * 在域参与者中注册类型。
     */
    _impl_info->_type.register_type(_impl_info->_participant);
    /**
    * 设置发布者的QoS策略，并创建一个发布者。
    */
    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
    _impl_info->_publisher = _impl_info->_participant->create_publisher(pubqos, nullptr);
    /**
     * 如果发布者创建失败，输出错误信息并返回false。
     */
    if (_impl_info->_publisher == nullptr) {
      std::cerr << "Failed to create Publisher" << std::endl;
      return false;
    }
    /**
    * 设置主题的QoS策略，并创建一个主题。主题名称由基础名称、父级名称（如果存在）、自身名称和类型名称组成。
    */
    efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
    const std::string publisher_type {"/camera_info"};
    const std::string base { "rt/carla/" };
    std::string topic_name = base;
    if (!_parent.empty())
      topic_name += _parent + "/";
    topic_name += _name;
    topic_name += publisher_type;
    _impl_info->_topic = _impl_info->_participant->create_topic(topic_name, _impl_info->_type->getName(), tqos);
    /**
   * 如果主题创建失败，输出错误信息并返回false。
   */
    if (_impl_info->_topic == nullptr) {
        std::cerr << "Failed to create Topic" << std::endl;
        return false;
    }
    /**
     * 设置数据写入器的QoS策略，并创建一个数据写入器。
     */
    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
    efd::DataWriterListener* listener = (efd::DataWriterListener*)_impl_info->_listener._impl.get();
    _impl_info->_datawriter = _impl_info->_publisher->create_datawriter(_impl_info->_topic, wqos, listener);
    /**
     * 如果数据写入器创建失败，输出错误信息并返回false。
     */
    if (_impl_info->_datawriter == nullptr) {
        std::cerr << "Failed to create DataWriter" << std::endl;
        return false;
    }
    /// 设置帧ID为名称。
    _frame_id = _name;
    /// 所有对象都成功创建，返回true。
    return true;
  }
  /**
 * @brief 发布图像和相关信息。
 *
 * 此函数负责调用PublishImage()和PublishInfo()函数来发布图像和相关信息。
 * 如果两者都成功发布，则返回true；否则返回false。
 *
 * @return 如果图像和相关信息都成功发布，返回true；否则返回false。
 */
  bool CarlaOpticalFlowCameraPublisher::Publish() {
    return PublishImage() && PublishInfo();
  }
  /**
 * @brief 发布图像数据。
 *
 * 此函数尝试使用DataWriter发布图像数据。根据返回的ReturnCode值，函数会输出相应的错误信息（如果有的话）
 * 并返回发布是否成功的布尔值。
 *
 * @return 如果图像数据成功发布，返回true；否则返回false。
 */
  bool CarlaOpticalFlowCameraPublisher::PublishImage() {
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    /**
    * 尝试写入图像数据到DataWriter。
    */
    eprosima::fastrtps::types::ReturnCode_t rcode = _impl->_datawriter->write(&_impl->_image, instance_handle);
    /**
    * 根据返回的ReturnCode值处理发布结果。
    */
    if (rcode == erc::ReturnCodeValue::RETCODE_OK) {
        /// @retval true 表示发布信息成功。
        return true;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ERROR) {
        /// @retval false 表示通用错误。
        std::cerr << "RETCODE_ERROR" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_UNSUPPORTED) {
        /// @retval false 表示操作不支持。
        std::cerr << "RETCODE_UNSUPPORTED" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_BAD_PARAMETER) {
        /// @retval false 表示参数错误。
        std::cerr << "RETCODE_BAD_PARAMETER" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_PRECONDITION_NOT_MET) {
        /// @retval false 表示前提条件未满足。
        std::cerr << "RETCODE_PRECONDITION_NOT_MET" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_OUT_OF_RESOURCES) {
        /// @retval false 表示资源不足。
        std::cerr << "RETCODE_OUT_OF_RESOURCES" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ENABLED) {
        /// @retval false 表示功能未启用。
        std::cerr << "RETCODE_NOT_ENABLED" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_IMMUTABLE_POLICY) {
        /// @retval false 表示策略不可变。
        std::cerr << "RETCODE_IMMUTABLE_POLICY" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_INCONSISTENT_POLICY) {
        /// @retval false 表示策略不一致。
        std::cerr << "RETCODE_INCONSISTENT_POLICY" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ALREADY_DELETED) {
        /// @retval false 表示实体已被删除。
        std::cerr << "RETCODE_ALREADY_DELETED" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_TIMEOUT) {
        /// @retval false 表示操作超时。
        std::cerr << "RETCODE_TIMEOUT" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NO_DATA) {
        /// @retval false 表示无数据。
        std::cerr << "RETCODE_NO_DATA" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ILLEGAL_OPERATION) {
        /// @retval false 表示非法操作。
        std::cerr << "RETCODE_ILLEGAL_OPERATION" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ALLOWED_BY_SECURITY) {
        /// @retval false 表示安全策略不允许。
        std::cerr << "RETCODE_NOT_ALLOWED_BY_SECURITY" << std::endl;
        return false;
    }
    /// @retval false 表示未知错误，发布失败。
    std::cerr << "UNKNOWN" << std::endl;
    return false;
  }
  /**
 * @brief 发布信息函数
 *
 * 此函数尝试通过数据写入器发布信息。根据返回码（ReturnCode_t）的不同，函数会返回不同的结果，
 * 并在控制台输出相应的错误信息。
 *
 * @return 如果发布信息成功，则返回true；否则返回false。
 */
  bool CarlaOpticalFlowCameraPublisher::PublishInfo() {
      /// @var instance_handle
   /// 用于存储数据写入操作后的实例句柄。
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    /// @var rcode
    /// 存储数据写入操作的返回码。
    eprosima::fastrtps::types::ReturnCode_t rcode = _impl_info->_datawriter->write(&_impl_info->_info, instance_handle);
    // 检查返回码，并根据不同的返回码执行相应的操作
    if (rcode == erc::ReturnCodeValue::RETCODE_OK) {
        /// @retval true 表示发布信息成功。
        return true;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ERROR) {
        /// @retval false 表示发生错误。
        std::cerr << "RETCODE_ERROR" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_UNSUPPORTED) {
        /// @retval false 表示操作不支持。
        std::cerr << "RETCODE_UNSUPPORTED" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_BAD_PARAMETER) {
        /// @retval false 表示参数错误。
        std::cerr << "RETCODE_BAD_PARAMETER" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_PRECONDITION_NOT_MET) {
        /// @retval false 表示前提条件未满足。
        std::cerr << "RETCODE_PRECONDITION_NOT_MET" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_OUT_OF_RESOURCES) {
        /// @retval false 表示资源不足。
        std::cerr << "RETCODE_OUT_OF_RESOURCES" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ENABLED) {
        /// @retval false 表示未启用。
        std::cerr << "RETCODE_NOT_ENABLED" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_IMMUTABLE_POLICY) {
        /// @retval false 表示策略不可变。
        std::cerr << "RETCODE_IMMUTABLE_POLICY" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_INCONSISTENT_POLICY) {
        /// @retval false 表示策略不一致。
        std::cerr << "RETCODE_INCONSISTENT_POLICY" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ALREADY_DELETED) {
        /// @retval false 表示已删除。
        std::cerr << "RETCODE_ALREADY_DELETED" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_TIMEOUT) {
        /// @retval false 表示超时。
        std::cerr << "RETCODE_TIMEOUT" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NO_DATA) {
        /// @retval false 表示无数据。
        std::cerr << "RETCODE_NO_DATA" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_ILLEGAL_OPERATION) {
        /// @retval false 表示非法操作。
        std::cerr << "RETCODE_ILLEGAL_OPERATION" << std::endl;
        return false;
    }
    if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ALLOWED_BY_SECURITY) {
        /// @retval false 表示安全策略不允许。
        std::cerr << "RETCODE_NOT_ALLOWED_BY_SECURITY" << std::endl;
        return false;
    }/// @retval false 表示未知错误。
    std::cerr << "UNKNOWN" << std::endl;
    return false;
  }

  void CarlaOpticalFlowCameraPublisher::SetImageData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, const float* data) {
    constexpr float pi = 3.1415f;
    constexpr float rad2ang = 360.0f/(2.0f*pi);
    const size_t max_index = width * height * 2;
    std::vector<uint8_t> vector_data;
    vector_data.resize(height * width * 4);
    size_t data_index = 0;
    for (size_t index = 0; index < max_index; index += 2) {
        const float vx = data[index];
        const float vy = data[index + 1];
        float angle = 180.0f + std::atan2(vy, vx) * rad2ang;
        if (angle < 0)
        {
            angle = 360.0f + angle;
        }
        angle = std::fmod(angle, 360.0f);

        const float norm = std::sqrt(vx * vx + vy * vy);
        const float shift = 0.999f;
        const float a = 1.0f / std::log(0.1f + shift);
        const float intensity = CLAMP<float>(a * std::log(norm + shift), 0.0f, 1.0f);

        const float& H = angle;
        const float S = 1.0f;
        const float V = intensity;
        const float H_60 = H * (1.0f / 60.0f);

        const float C = V * S;
        const float X = C * (1.0f - std::abs(std::fmod(H_60, 2.0f) - 1.0f));
        const float m = V - C;

        float r = 0;
        float g = 0;
        float b = 0;
        const unsigned int angle_case = static_cast<const unsigned int>(H_60);
        switch (angle_case) {
        case 0:
            r = C;
            g = X;
            b = 0;
            break;
        case 1:
            r = X;
            g = C;
            b = 0;
            break;
        case 2:
            r = 0;
            g = C;
            b = X;
            break;
        case 3:
            r = 0;
            g = X;
            b = C;
            break;
        case 4:
            r = X;
            g = 0;
            b = C;
            break;
        case 5:
            r = C;
            g = 0;
            b = X;
            break;
        default:
            r = 1;
            g = 1;
            b = 1;
            break;
        }

        const uint8_t R = static_cast<uint8_t>((r + m) * 255.0f);
        const uint8_t G = static_cast<uint8_t>((g + m) * 255.0f);
        const uint8_t B = static_cast<uint8_t>((b + m) * 255.0f);

        vector_data[data_index++] = B;
        vector_data[data_index++] = G;
        vector_data[data_index++] = R;
        vector_data[data_index++] = 0;
    }
    SetData(seconds, nanoseconds, height, width, std::move(vector_data));
  }

  void CarlaOpticalFlowCameraPublisher::SetInfoRegionOfInterest( uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, bool do_rectify) {
    sensor_msgs::msg::RegionOfInterest roi;
    roi.x_offset(x_offset);
    roi.y_offset(y_offset);
    roi.height(height);
    roi.width(width);
    roi.do_rectify(do_rectify);
    _impl_info->_info.roi(roi);
  }

  void CarlaOpticalFlowCameraPublisher::SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, std::vector<uint8_t>&& data) {
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

  void CarlaOpticalFlowCameraPublisher::SetCameraInfoData(int32_t seconds, uint32_t nanoseconds) {
    builtin_interfaces::msg::Time time;
    time.sec(seconds);
    time.nanosec(nanoseconds);

    std_msgs::msg::Header header;
    header.stamp(std::move(time));
    header.frame_id(_frame_id);
    _impl_info->_info.header(header);
  }

  CarlaOpticalFlowCameraPublisher::CarlaOpticalFlowCameraPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaOpticalFlowCameraPublisherImpl>()),
  _impl_info(std::make_shared<CarlaCameraInfoPublisherImpl>()) {
    _name = ros_name;
    _parent = parent;
  }

  CarlaOpticalFlowCameraPublisher::~CarlaOpticalFlowCameraPublisher() {
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

  CarlaOpticalFlowCameraPublisher::CarlaOpticalFlowCameraPublisher(const CarlaOpticalFlowCameraPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    _impl_info = other._impl_info;
  }

  CarlaOpticalFlowCameraPublisher& CarlaOpticalFlowCameraPublisher::operator=(const CarlaOpticalFlowCameraPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
    _impl_info = other._impl_info;

    return *this;
  }

  CarlaOpticalFlowCameraPublisher::CarlaOpticalFlowCameraPublisher(CarlaOpticalFlowCameraPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    _impl_info = std::move(other._impl_info);

  }

  CarlaOpticalFlowCameraPublisher& CarlaOpticalFlowCameraPublisher::operator=(CarlaOpticalFlowCameraPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
    _impl_info = std::move(other._impl_info);

    return *this;
  }
}
}
