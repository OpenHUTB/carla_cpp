#define _GLIBCXX_USE_CXX11_ABI 0
// 定义了一个宏，用于设置C++标准库的ABI（应用程序二进制接口）版本为0，这可能与代码所依赖的库的编译设置相关。

#include "CarlaMapSensorPublisher.h"
#include <string>
#include "carla/ros2/types/StringPubSubTypes.h"
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
// 引入了一系列必要的头文件，包括自定义的Carla相关头文件、标准库的<string>头文件以及Fast DDS相关的头文件，用于实现与Carla模拟器以及Fast DDS消息中间件的交互功能。

namespace carla {
namespace ros2 {
    // 定义了carla::ros2命名空间，以下的所有类型和函数都在这个命名空间内。

    efd = eprosima::fastdds::dds;
    using erc = eprosima::fastrtps::types::ReturnCode_t;
    // 为了方便使用，给eprosima::fastdds::dds和eprosima::fastrtps::types::ReturnCode_t分别定义了别名efd和erc。

    struct CarlaMapSensorPublisherImpl {
        efd::DomainParticipant* _participant { nullptr };
        efd::Publisher* _publisher { nullptr };
        efd::Topic* _topic { nullptr };
        efd::DataWriter* _datawriter { nullptr };
        efd::TypeSupport _type { new std_msgs::msg::StringPubSubType() };
        CarlaListener _listener {};
        std_msgs::msg::String _string {};
    };
    // 定义了一个名为CarlaMapSensorPublisherImpl的结构体，用于存储与Carla地图传感器发布者相关的内部实现细节。
    // 包含了Fast DDS的域参与者、发布者、主题、数据写入器等相关对象的指针，以及消息类型支持对象、监听器对象和一个用于存储要发布的字符串消息的对象。

    bool CarlaMapSensorPublisher::Init() {
        if (_impl->_type == nullptr) {
            std::cerr << "Invalid TypeSupport" << std::endl;
            return false;
        }
        // 首先检查消息类型支持对象是否为空，如果为空则输出错误信息并返回false，表示初始化失败。

        efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
        pqos.name(_name);
        auto factory = efd::DomainParticipantFactory::get_instance();
        _impl->_participant = factory->create_participant(0, pqos);
        if (_impl->_participant == nullptr) {
            std::cerr << "Failed to create DomainParticipant" << std::endl;
            return false;
        }
        _impl->_type.register_type(_impl->_participant);
        // 设置域参与者的默认质量服务属性（QoS），并设置其名称为传入的_name。然后通过域参与者工厂创建一个域参与者对象，
        // 如果创建失败则输出错误信息并返回false。成功创建后，将消息类型注册到该域参与者上。

        efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
        _impl->_publisher = _impl->_participant->create_publisher(pubqos, nullptr);
        if (_impl->_publisher == nullptr) {
            std::cerr << "Failed to create Publisher" << std::endl;
            return false;
        }
        // 设置发布者的默认QoS属性，然后通过已创建的域参与者对象创建一个发布者对象，
        // 如果创建失败则输出错误信息并返回false。

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
        // 设置主题的默认QoS属性，根据传入的_parent和_name构建主题名称，然后通过域参与者对象创建一个主题对象，
        // 如果创建失败则输出错误信息并返回false。

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
        // 设置数据写入器的默认QoS属性，指定其历史内存策略。获取监听器对象的指针，然后通过发布者对象创建一个数据写入器对象，
        // 如果创建失败则输出错误信息并返回false。最后设置_frame_id为_name，并返回true表示初始化成功。

    }

    bool CarlaMapSensorPublisher::Publish() {
        eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
        eprosima::fastrtps::types::ReturnCode_t rcode = _impl->_datawriter->write(&_impl->_string, instance_handle);
        if (rcode == erc::ReturnCodeValue::RETCODE_OK) {
            return true;
        }
        // 创建一个实例句柄对象，然后调用数据写入器的write方法尝试发布存储在_impl->_string中的消息，获取返回码rcode。
        // 如果返回码为RETCODE_OK，表示发布成功，返回true。

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
        // 根据不同的返回码值，输出相应的错误信息并返回false，表示发布失败。如果返回码不属于上述已处理的情况，则输出"UNKNOWN"并返回false。

    }

    void CarlaMapSensorPublisher::SetData(const char* data) {
        _impl->_string.data(data);
    }
    // 将传入的字符指针指向的数据设置为要发布的字符串消息的数据。

    CarlaMapSensorPublisher::CarlaMapSensorPublisher(const char* ros_name, const char* parent) :
    _impl(std::make_shared<CarlaMapSensorPublisherImpl>()) {
        _name = ros_name;
        _parent = parent;
    }
    // 构造函数，接受一个表示ROS名称的字符指针和一个表示父名称的字符指针作为参数。
    // 在构造函数内部，创建一个CarlaMapSensorPublisherImpl结构体的共享指针，并初始化_name和_parent成员变量。

    CarlaMapSensorPublisher::~CarlaMapSensorPublisher() {
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
    // 析构函数，用于清理在构造函数和其他操作中创建的Fast DDS相关对象。
    // 如果_impl指针为空则直接返回，否则依次删除数据写入器、发布者、主题和域参与者对象，以释放相关资源。

    CarlaMapSensorPublisher::CarlaMapSensorPublisher(const CarlaMapSensorPublisher& other) {
        _frame_id = other._frame_id;
        _name = other._name;
        _parent = other._parent;
        _impl = other._impl;
    }
    // 拷贝构造函数，用于创建一个与传入的CarlaMapSensorPublisher对象相同的新对象，
    // 拷贝其_frame_id、_name、_parent和_impl成员变量的值。

    CarlaMapSensorPublisher& CarlaMapSensorPublisher::operator=(const CarlaMapSensorPublisher& other) {
        _frame_id = other._frame_id;
        _name = other._name;
        _parent = other._parent;
        _impl = other._impl;

        return *this;
    }
    // 赋值运算符重载函数，用于将一个CarlaMapSensorPublisher对象的值赋给另一个对象，
    // 赋值其_frame_id、_name、_parent和_impl成员变量的值，并返回被赋值的对象本身。

    CarlaMapSensorPublisher::CarlaMapSensorPublisher(CarlaMapSensorPublisher&& other) {
        _frame_id = std::move(other._frame_id);
        _name = std::move(other._name);
        _parent = std::move(other._parent);
        _impl = std::move(other._impl);
    }
    // 移动构造函数，用于通过移动语义创建一个新的CarlaMapSensorPublisher对象，
    // 将传入对象的_frame_id、_name、_parent和_impl成员变量的值移动到新对象中，原对象的这些成员变量将变为未定义状态。

    CarlaMapSensorPublisher& CarlaMapSensorPublisher::operator=(CarlaMapSensorPublisher&& other) {
        _frame_id = std::move(other._frame_id);
        _name = std::move(other._name);
        _parent = std::move(other._parent);
        _impl = std::move(other._impl);

        return *this;
    }
    // 移动赋值运算符重载函数，用于通过移动语义将一个CarlaMapSensorPublisher对象的值赋给另一个对象，
    // 移动其_frame_id、_name、_parent和_impl成员变量的值，并返回被赋值的对象本身。

}
}