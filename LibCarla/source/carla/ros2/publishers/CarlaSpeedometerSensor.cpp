#define _GLIBCXX_USE_CXX11_ABI 0
/**
 * @file
 * 包含必要的头文件，实现CARLA车速传感器的ROS2和FastDDS通信。
 */

 /**
  * @brief 包含CARLA车速传感器的头文件。
  */
#include "CarlaSpeedometerSensor.h"
  /**
   * @brief 包含标准字符串库。
   */
#include <string>
   /**
    * @brief 包含CARLA ROS2类型的Float32发布和订阅类型定义。
    */
#include "carla/ros2/types/Float32PubSubTypes.h"
    /**
     * @brief 包含CARLA ROS2监听器相关定义。
     */
#include "carla/ros2/listeners/CarlaListener.h"
     /**
      * @brief 包含FastDDS DomainParticipant类的声明。
      */
#include <fastdds/dds/domain/DomainParticipant.hpp>
      /**
       * @brief 包含FastDDS Publisher类的声明。
       */
#include <fastdds/dds/publisher/Publisher.hpp>
       /**
        * @brief 包含FastDDS Topic类的声明。
        */
#include <fastdds/dds/topic/Topic.hpp>
        /**
         * @brief 包含FastDDS DataWriter类的声明。
         */
#include <fastdds/dds/publisher/DataWriter.hpp>
         /**
          * @brief 包含FastDDS TypeSupport类的声明，用于支持特定类型的数据传输。
          */
#include <fastdds/dds/topic/TypeSupport.hpp>
          /**
           * @brief 包含FastDDS DomainParticipantQos类的声明，用于配置DomainParticipant的QoS策略。
           */
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
           /**
            * @brief 包含FastDDS DomainParticipantFactory类的声明，用于创建DomainParticipant实例。
            */
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
            /**
             * @brief 包含FastDDS PublisherQos类的声明，用于配置Publisher的QoS策略。
             */
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
             /**
              * @brief 包含FastDDS TopicQos类的声明，用于配置Topic的QoS策略。
              */
#include <fastdds/dds/topic/qos/TopicQos.hpp>
              /**
               * @brief 包含FastRTPS的ParticipantAttributes类，用于配置RTPS参与者属性。
               */
#include <fastrtps/attributes/ParticipantAttributes.h>
               /**
                * @brief 包含FastRTPS的QosPolicies类，用于配置RTPS的QoS策略。
                */
#include <fastrtps/qos/QosPolicies.h>
                /**
                 * @brief 包含FastDDS DataWriterQos类的声明，用于配置DataWriter的QoS策略。
                 */
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
                 /**
                  * @brief 包含FastDDS DataWriterListener类的声明，用于监听DataWriter的事件。
                  */
#include <fastdds/dds/publisher/DataWriterListener.hpp>

/**
  * @namespace carla::ros2
  * @brief 包含与ROS2集成相关的Carla模块的实现。
  */
namespace carla {
/**
 * @namespace carla::ros2::efd
 * @brief eprosima::fastdds::dds命名空间的别名，用于简化代码中对FastDDS库的引用。
 */
namespace ros2 {
    /**
 * @typedef erc
 * @brief eprosima::fastrtps::types::ReturnCode_t类型的别名，用于表示FastRTPS操作的结果代码。
 */
  namespace efd = eprosima::fastdds::dds;
  using erc = eprosima::fastrtps::types::ReturnCode_t;
  /**
 * @struct CarlaSpeedometerSensorImpl
 * @brief Carla车速传感器实现的内部结构体。
 *
 * 该结构体封装了与FastDDS相关的资源，包括DomainParticipant、Publisher、Topic和DataWriter，以及用于发布车速数据的TypeSupport和监听器。
 */
  struct CarlaSpeedometerSensorImpl {
      /**
     * @brief FastDDS的DomainParticipant指针，用于创建和管理FastDDS的通信实体。
     */
    efd::DomainParticipant* _participant { nullptr };
    /**
     * @brief FastDDS的Publisher指针，用于发布数据。
     */
    efd::Publisher* _publisher { nullptr };
    /**
    * @brief FastDDS的Topic指针，用于定义数据的主题。
    */
    efd::Topic* _topic { nullptr };
    /**
     * @brief FastDDS的DataWriter指针，用于写入数据到指定的Topic。
     */
    efd::DataWriter* _datawriter { nullptr };
    /**
     * @brief TypeSupport对象，用于注册和识别发布的消息类型。
     */
    efd::TypeSupport _type { new std_msgs::msg::Float32PubSubType() };
    /**
     * @brief CarlaListener对象，用于处理相关的回调事件。
     */
    CarlaListener _listener {};
    /**
     * @brief 用于发布的车速数据。
     */
    std_msgs::msg::Float32 _float {};
  };
  /**
 * @brief 初始化Carla车速传感器。
 *
 * 该函数负责初始化Carla车速传感器的FastDDS相关资源，包括创建DomainParticipant、Publisher、Topic和DataWriter。
 *
 * @return 初始化成功返回true，否则返回false。
 */
  bool CarlaSpeedometerSensor::Init() {
      /**
     * @brief 检查TypeSupport是否有效。
     */
    if (_impl->_type == nullptr) {
        std::cerr << "Invalid TypeSupport" << std::endl;
        return false;
    }
    /**
     * @brief 设置DomainParticipant的QoS策略，并创建DomainParticipant。
     */
    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    pqos.name(_name);
    auto factory = efd::DomainParticipantFactory::get_instance();
    _impl->_participant = factory->create_participant(0, pqos);
    if (_impl->_participant == nullptr) {
        std::cerr << "Failed to create DomainParticipant" << std::endl;
        return false;
    }
    /**
     * @brief 注册TypeSupport。
     */
    _impl->_type.register_type(_impl->_participant);
    /**
     * @brief 设置Publisher的QoS策略，并创建Publisher。
     */
    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
    _impl->_publisher = _impl->_participant->create_publisher(pubqos, nullptr);
    if (_impl->_publisher == nullptr) {
      std::cerr << "Failed to create Publisher" << std::endl;
      return false;
    }
    /**
     * @brief 设置Topic的QoS策略，并创建Topic。
     */
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
    /**
     * @brief 设置DataWriter的QoS策略，并创建DataWriter。
     */
    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
    wqos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    efd::DataWriterListener* listener = (efd::DataWriterListener*)_impl->_listener._impl.get();
    _impl->_datawriter = _impl->_publisher->create_datawriter(_impl->_topic, wqos, listener);
    if (_impl->_datawriter == nullptr) {
        std::cerr << "Failed to create DataWriter" << std::endl;
        return false;
    }
    /**
     * @brief 设置帧ID为传感器名称。
     */
    _frame_id = _name;
    return true;
  }

  bool CarlaSpeedometerSensor::Publish() {
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    eprosima::fastrtps::types::ReturnCode_t rcode = _impl->_datawriter->write(&_impl->_float, instance_handle);
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

  void CarlaSpeedometerSensor::SetData(float data) {
    _impl->_float.data(data);
  }

  CarlaSpeedometerSensor::CarlaSpeedometerSensor(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaSpeedometerSensorImpl>()) {
    _name = ros_name;
    _parent = parent;
  }

  CarlaSpeedometerSensor::~CarlaSpeedometerSensor() {
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
  }

  CarlaSpeedometerSensor::CarlaSpeedometerSensor(const CarlaSpeedometerSensor& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
  }

  CarlaSpeedometerSensor& CarlaSpeedometerSensor::operator=(const CarlaSpeedometerSensor& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;

    return *this;
  }

  CarlaSpeedometerSensor::CarlaSpeedometerSensor(CarlaSpeedometerSensor&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
  }

  CarlaSpeedometerSensor& CarlaSpeedometerSensor::operator=(CarlaSpeedometerSensor&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);

    return *this;
  }
}
}
