// 定义 _GLIBCXX_USE_CXX11_ABI 为 0，这通常用于控制 C++ 标准库的 ABI（Application Binary Interface，应用程序二进制接口）版本，
// 将其设置为 0 可能是为了遵循特定的兼容性要求或者与项目所依赖的其他库在 ABI 层面保持一致。
#define _GLIBCXX_USE_CXX11_ABI 0

// 引入自定义的相机发布器头文件，该头文件中应该包含了 CarlaISCameraPublisher 类的声明等相关内容，
// 是当前实现文件能够正确定义和实现该类功能的前置依赖。
#include "CarlaISCameraPublisher.h"

// 引入 C++ 标准库中的字符串库，方便后续代码中对字符串类型进行操作，例如构造主题名称、处理各种路径字符串等。
#include <string>

// 引入图像类型的发布/订阅类型定义，可能用于在 ROS2 与 Carla 结合的环境下，处理图像数据在不同组件间的发布和订阅相关逻辑，
// 提供了与图像消息类型相关的特定配置和操作接口。
#include "carla/ros2/types/ImagePubSubTypes.h"
// 引入相机信息类型的发布/订阅类型定义，类似地，用于支持相机信息消息在系统中的发布和订阅操作，
// 使得能够按照特定的格式和规则来传递相机相关的参数信息等内容。
#include "carla/ros2/types/CameraInfoPubSubTypes.h"
// 引入 Carla 监听器，可能用于监听某些 Carla 相关的事件或者消息，以便在合适的时机进行相应的处理，
// 例如根据监听到的信息来触发相机数据的发布等操作。
#include "carla/ros2/listeners/CarlaListener.h"

// 引入 Fast DDS 的 DomainParticipant 类，DomainParticipant 是 Fast DDS 框架中用于参与 DDS（Data Distribution Service，数据分发服务）域的核心类，
// 它负责管理和协调该参与者在整个分布式系统中的相关资源和交互操作。
#include <fastdds/dds/domain/DomainParticipant.hpp>
// 引入 Fast DDS 的 Publisher 类，Publisher 类用于创建数据发布者对象，通过它可以向特定的主题发布消息，
// 是实现数据向外发送的关键组件之一。
#include <fastdds/dds/publisher/Publisher.hpp>
// 引入 Fast DDS 的 Topic 类，Topic 用于定义消息的主题，它确定了发布和订阅操作中消息的分类和标识，
// 不同的组件可以通过相同的主题来进行对应的消息交互。
#include <fastdds/dds/topic/Topic.hpp>
// 引入 Fast DDS 的 DataWriter 类，DataWriter 是实际负责将数据写入到相应主题的类，
// 它与 Publisher 配合使用，将具体的数据按照 Topic 定义的规则发布出去。
#include <fastdds/dds/publisher/DataWriter.hpp>
// 引入 Fast DDS 的 TypeSupport 类，TypeSupport 类主要用于将自定义的数据类型注册到 DDS 系统中，
// 使得系统能够识别并正确处理相应类型的数据消息。
#include <fastdds/dds/topic/TypeSupport.hpp>

// 引入 DomainParticipant 的 QOS（Quality of Service，服务质量）设置相关的类，
// 通过设置 QOS 参数可以控制 DomainParticipant 在参与 DDS 域时的各种行为特性，例如可靠性、实时性等方面的表现。
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
// 引入 DomainParticipant 工厂类，用于创建 DomainParticipant 实例，
// 提供了一种统一的创建机制，方便在代码中按需生成参与者对象。
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
// 引入 Publisher 的 QOS 设置相关的类，用于配置 Publisher 在发布消息时的服务质量参数，
// 以满足不同应用场景对消息发布的可靠性、带宽等方面的要求。
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
// 引入 Topic 的 QOS 设置相关的类，用于定义 Topic 在消息传递过程中的服务质量特性，
// 比如消息的持久性、传输顺序等相关参数的设置。
#include <fastdds/dds/topic/qos/TopicQos.hpp>

// 引入 Participant 属性相关的类，用于配置 DomainParticipant 的各种属性参数，
// 这些属性会影响参与者在整个 DDS 系统中的行为和表现。
#include <fastrtps/attributes/ParticipantAttributes.h>
// 引入 QOS 策略相关的类，提供了一系列预定义的 QOS 策略选项，方便根据具体需求灵活组合配置 QOS 参数，
// 以实现期望的消息传递效果。
#include <fastrtps/qos/QosPolicies.h>
// 引入 DataWriter 的 QOS 设置相关的类，用于精细调整 DataWriter 在将数据写入主题时的各种服务质量参数，
// 例如设置数据缓存策略、发送超时时间等。
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
// 引入 DataWriter 监听器类，通过实现该监听器可以监听 DataWriter 在发布数据过程中的各种事件，
// 例如数据发送成功、失败等情况，以便进行相应的后续处理。
#include <fastdds/dds/publisher/DataWriterListener.hpp>

// 定义命名空间 carla，在该命名空间下组织与 Carla 项目相关的各类代码，便于代码的模块化管理和避免命名冲突。
namespace carla {
// 在 carla 命名空间内再定义 ros2 命名空间，进一步细分与 ROS2 集成相关的功能代码，使得代码结构更加清晰，功能层次更明确。
namespace ros2 {
    // 使用 Fast DDS 命名空间的别名 efd，这样在后续代码中使用 Fast DDS 相关的类和类型时，可以使用更简洁的 efd:: 前缀来引用，
    // 提高代码的可读性和书写便利性。
    namespace efd = eprosima::fastdds::dds;
    // 使用别名 erc 定义返回码类型，该类型来自 eprosima::fastrtps::types::ReturnCode_t，
    // 用于统一处理 Fast DDS 相关操作返回的各种状态码，方便在代码中进行错误判断和相应的处理逻辑编写。
    using erc = eprosima::fastrtps::types::ReturnCode_t;

    // CarlaISCameraPublisher 的实现结构体，用于封装与 CarlaISCameraPublisher 类相关的具体实现细节和成员变量，
    // 将内部的数据和操作进行聚合，使得类的外部接口更加清晰简洁，同时便于内部数据的管理和维护。
    struct CarlaISCameraPublisherImpl {
        // DomainParticipant 指针，用于指向参与 DDS 域的参与者对象，通过该指针可以操作参与者相关的各种功能，
        // 初始化为 nullptr，表示尚未关联具体的有效对象。
        efd::DomainParticipant* _participant { nullptr };
        // Publisher 指针，指向数据发布者对象，用于将数据发布到指定的主题，初始值为 nullptr，等待后续创建和赋值。
        efd::Publisher* _publisher { nullptr };
        // Topic 指针，用于指向定义好的消息主题对象，同样初始化为 nullptr，后续会根据实际需求创建并关联具体的主题。
        efd::Topic* _topic { nullptr };
        // DataWriter 指针，负责实际的数据写入操作，将数据按照主题和相关配置发布出去，初始为 nullptr。
        efd::DataWriter* _datawriter { nullptr };
        // 图像类型支持对象，用于注册图像消息类型到 DDS 系统中，确保系统能够正确处理图像类型的数据，
        // 这里初始化时创建了一个 sensor_msgs::msg::ImagePubSubType 类型的实例，表明主要用于处理图像相关的数据发布和订阅。
        efd::TypeSupport _type { new sensor_msgs::msg::ImagePubSubType() };
        // Carla 监听器实例，用于监听 Carla 相关的事件或者消息，可能在相机数据发布等过程中依据监听到的内容进行相应处理，
        // 例如根据特定事件触发数据的发布或者进行一些错误处理等操作。
        CarlaListener _listener {};
        // 存储图像数据的对象，用于暂存要发布的图像数据，方便后续通过 DataWriter 将其发布出去，初始化为默认状态。
        sensor_msgs::msg::Image _image {};
    };

    // CarlaCameraInfoPublisher 的实现结构体，与上面的 CarlaISCameraPublisherImpl 类似，
    // 不过主要是针对相机信息相关的发布功能进行数据和操作的封装，用于管理相机信息发布过程中的各种内部细节。
    struct CarlaCameraInfoPublisherImpl {
        // DomainParticipant 指针，作用同前面的结构体中的对应指针，用于参与 DDS 域操作，初始化为 nullptr。
        efd::DomainParticipant* _participant { nullptr };
        // Publisher 指针，用于发布相机信息数据，初始值为 nullptr，等待创建和赋值。
        efd::Publisher* _publisher { nullptr };
        // Topic 指针，指向相机信息相关的主题对象，初始为 nullptr，后续会根据具体的相机信息主题定义进行创建和关联。
        efd::Topic* _topic { nullptr };
        // DataWriter 指针，负责将相机信息数据实际写入到对应的主题中进行发布，初始为 nullptr。
        efd::DataWriter* _datawriter { nullptr };
        // 相机信息类型支持对象，创建了 sensor_msgs::msg::CameraInfoPubSubType 类型的实例，用于注册相机信息消息类型到 DDS 系统，
        // 保证系统能正确处理相机信息相关的数据传递。
        efd::TypeSupport _type { new sensor_msgs::msg::CameraInfoPubSubType() };
        // Carla 监听器实例，同样用于监听相关事件，辅助相机信息发布过程中的各种处理逻辑，例如错误处理、状态更新等。
        CarlaListener _listener {};
        // 初始化标志，用于记录相机信息发布相关的初始化状态，初始化为 false，表示尚未完成初始化操作，
        // 在合适的初始化流程执行完毕后会被设置为 true。
        bool _init {false};
        // 存储相机信息的对象，用于保存要发布的相机信息数据，如相机的内参、外参等相关参数信息，初始化为默认状态。
        sensor_msgs::msg::CameraInfo _info {};
    };

    // 检查相机发布器是否已初始化，通过返回内部实现结构体中记录的初始化标志来判断，
    // 外部代码可以调用该函数来确认相机发布器是否已经完成了必要的初始化步骤，以便决定是否可以进行后续的操作，比如发布数据等。
    bool CarlaISCameraPublisher::HasBeenInitialized() const {
        return _impl_info->_init;
    }

    // 初始化相机发布器的信息数据，包括设置相机信息的一些基本参数，如高度、宽度、视场角等，
    // 并调用相关函数设置感兴趣区域，最后将初始化标志设置为 true，表示信息数据已成功初始化。
    void CarlaISCameraPublisher::InitInfoData(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, float fov, bool do_rectify) {
        // 使用移动构造函数初始化相机信息对象，传递相应的参数来构建基本的相机信息内容。
        _impl_info->_info = std::move(sensor_msgs::msg::CameraInfo(height, width, fov));
        // 调用函数设置相机信息中的感兴趣区域，根据传入的偏移量、尺寸以及是否矫正等参数进行配置。
        SetInfoRegionOfInterest(x_offset, y_offset, height, width, do_rectify);
        // 将初始化标志设置为 true，表明相机信息数据已完成初始化，可以进行后续的发布等相关操作了。
        _impl_info->_init = true;
    }

    // 初始化相机发布器，通过分别调用初始化图像和初始化相机信息的函数来完成整个相机发布器的初始化工作，
    // 只有当这两个部分都成功初始化后，才返回 true，表示整个相机发布器初始化成功，否则返回 false。
    bool CarlaISCameraPublisher::Init() {
        return InitImage() && InitInfo();
    }

    // 初始化图像发布器，该函数主要负责创建和配置与图像发布相关的各个组件，包括 DomainParticipant、Publisher、Topic、DataWriter 等，
    // 如果在创建或配置过程中出现任何错误，将输出相应的错误信息并返回 false，只有所有组件都成功创建和配置后才返回 true。
    bool CarlaISCameraPublisher::InitImage() {
        // 首先检查图像类型支持对象是否有效，如果为空指针，则表示类型支持无效，输出错误信息并返回 false。
        if (_impl->_type == nullptr) {
            std::cerr << "Invalid TypeSupport" << std::endl;
            return false;
        }
        // 获取默认的 DomainParticipant QOS 设置，后续可以根据实际需求在此基础上进行修改或者直接使用默认值来创建参与者对象。
        efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
        // 设置参与者的名称，使用传入的 _name 参数来命名，以便在 DDS 系统中进行区分和识别。
        pqos.name(_name);
        // 获取 DomainParticipant 工厂实例，通过该工厂来创建具体的 DomainParticipant 对象。
        auto factory = efd::DomainParticipantFactory::get_instance();
        // 使用工厂创建 DomainParticipant 对象，传入 DDS 域 ID（这里为 0）和前面配置好的 QOS 参数，
        // 如果创建失败，输出错误信息并返回 false。
        _impl->_participant = factory->create_participant(0, pqos);
        if (_impl->_participant == nullptr) {
            std::cerr << "Failed to create DomainParticipant" << std::endl;
            return false;
        }
        // 将图像消息类型注册到刚创建的 DomainParticipant 对象中，使得系统能够识别和处理图像类型的数据。
        _impl->_type.register_type(_impl->_participant);

        // 获取默认的 Publisher QOS 设置，同样可根据需求进一步调整这些设置，用于创建 Publisher 对象。
        efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
        // 通过 DomainParticipant 对象创建 Publisher，传入 QOS 参数和一个空的监听器指针（可能后续会另行设置具体的监听器），
        // 如果创建失败，输出错误信息并返回 false。
        _impl->_publisher = _impl->_participant->create_publisher(pubqos, nullptr);
        if (_impl->_publisher == nullptr) {
            std::cerr << "Failed to create Publisher" << std::endl;
            return false;
        }

        // 获取默认的 Topic QOS 设置，用于后续创建主题时配置主题的相关属性。
        efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
        // 定义图像主题类型的字符串，用于构建完整的主题名称，表明该主题主要用于发布图像数据。
        const std::string publisher_type {"/image"};
        // 定义基本的主题名称前缀，作为主题名称的一部分，通常用于组织和区分不同来源或功能的主题。
        const std::string base { "rt/carla/" };
        // 初始化主题名称字符串，先赋值为基本主题名称前缀。
        std::string topic_name = base;
        // 如果存在父主题（即 _parent 字符串不为空），则将父主题名称添加到主题名称中，形成更具层次结构的主题名称。
        if (!_parent.empty())
            topic_name += _parent + "/";
        // 添加当前相机发布器的名称到主题名称中，进一步细化主题的标识。
        topic_name += _name;
        // 最后添加图像主题类型字符串，构建出完整的用于发布图像数据的主题名称。
        topic_name += publisher_type;
        // 通过 DomainParticipant 对象创建主题，传入构建好的主题名称、图像类型支持对象中获取的类型名称以及 Topic QOS 参数，
        // 如果创建失败，输出错误信息并返回 false。
        _impl->_topic = _impl->_participant->create_topic(topic_name, _impl->_type->getName(), tqos);
        if (_impl->_topic == nullptr) {
            std::cerr << "Failed to create Topic" << std::endl;
            return false;
        }

        // 获取默认的 DataWriter QOS 设置，后续可根据实际情况对其进行调整，例如设置内存策略等特定参数。
        efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
        // 设置 DataWriter 的内存策略，这里选择预分配并可重新分配内存的模式，以适应可能的数据量变化等情况。
        wqos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
        // 获取监听器对象指针，将其转换为 efd::DataWriterListener* 类型，用于传递给 DataWriter 创建函数，以便监听数据写入相关的事件。
        efd::DataWriterListener* listener = (efd::DataWriterListener*)_impl->_listener._impl.get();
        // 通过 Publisher 对象创建 DataWriter，传入主题对象、配置好的 QOS 参数以及监听器指针，
        // 如果创建失败，输出错误信息并返回 false。
        _impl->_datawriter = _impl->_publisher->create_datawriter(_impl->_topic, wqos, listener);
        if (_impl->_datawriter == nullptr) {
            std::cerr << "Failed to create DataWriter" << std::endl;
            return false;
        }
        // 设置帧 ID，将其赋值为相机发布器的名称，帧 ID 通常用于在图像数据等消息中标识该数据所属的帧，方便后续处理时进行区分和关联等操作。
        _frame_id = _name;
        // 如果所有组件都成功创建和配置，返回 true 表示图像发布器初始化成功。
        return true;
    }

    // 初始化相机信息发布器，与初始化图像发布器类似，负责创建和配置相机信息发布相关的各个关键组件，
    // 包括 DomainParticipant、Publisher、Topic、DataWriter 等，过程中若出现错误会输出相应错误信息并返回 false，全部成功则返回 true。
    bool CarlaISCameraPublisher::InitInfo() {
        // 首先检查相机信息类型支持对象是否有效，若为空指针则表示类型支持无效，输出错误信息并返回 false。
        if (_impl_info->_type == nullptr) {
            std::cerr << "Invalid TypeSupport" << std::endl;
            return false;
        }

        // 获取默认的 DomainParticipant QOS 设置，为后续创建 DomainParticipant 对象做准备。
        efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
        // 设置参与者名称，使用传入的 _name 参数来为 DomainParticipant 命名，便于在 DDS 系统中进行识别和管理。
        pqos.name(_name);
        // 获取 DomainParticipant 工厂实例，通过该工厂来创建 DomainParticipant 对象。
        auto factory = efd::DomainParticipantFactory::get_instance();
        // 使用工厂创建 DomainParticipant 对象，传入 DDS 域 ID（此处为 0）和配置好的 QOS 参数，
        // 若创建失败，输出错误信息并返回 false。
        _impl_info->_participant = factory->create_participant(0, pqos);
        if (_impl_info->_participant == nullptr) {
            std::cerr << "Failed to create DomainParticipant" << std::endl;
            return false;
        }
        // 将相机信息消息类型注册到刚创建的 DomainParticipant 对象中，使系统能够正确处理相机信息相关的数据。
        _impl_info->_type.register_type(_impl_info->_participant);

        // 获取默认的 Publisher QOS 设置，用于后续创建 Publisher 对象时配置其属性。
        efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
        // 通过 DomainParticipant 对象创建 Publisher，传入 QOS 参数和空的监听器指针（可能后续另行设置具体监听器），
        // 若创建失败，输出错误信息并返回 false。
        _impl_info->_publisher = _impl_info->_participant->create_publisher(pubqos, nullptr);
        if (_impl_info->_publisher == nullptr) {
            std::cerr << "Failed to create Publisher" << std::endl;
            return false;
        }

        // 获取默认的 Topic QOS 设置，用于配置即将创建的主题的相关属性。
        efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
        // 定义相机信息主题类型的字符串，表明该主题用于发布相机信息数据。
        const std::string publisher_type {"/camera_info"};
        // 定义基本主题名称前缀，作为主题名称的一部分，用于组织和区分不同功能的主题。
        const std::string base { "rt/carla/" };
        // 初始化主题名称字符串，先赋值为基本主题名称前缀。
        std::string topic_name = base;
        // 如果存在父主题（即 _parent 字符串不为空），则将父主题名称添加到主题名称中，构建更具层次结构的主题名称。
        if (!_parent.empty())
            topic_name += _parent + "/";
        // 添加当前相机发布器的名称到主题名称中，进一步细化主题标识。
        topic_name += _name;
        // 最后添加相机信息主题类型字符串，构建出完整的用于发布相机信息数据的主题名称。
        topic_name += publisher_type;
        // 通过 DomainParticipant 对象创建主题，传入构建好的主题名称、相机信息类型支持对象中获取的类型名称以及 Topic QOS 参数，
        // 若创建失败，输出错误信息并返回 false。
        _impl_info->_topic = _impl_info->_participant->create_topic(topic_name, _impl_info->_type->getName(), tqos);
        if (_impl_info->_topic == nullptr) {
            std::cerr << "Failed to create Topic" << std::endl;
            return false;
        }

        // 获取默认的 DataWriter QOS 设置，后续可按需调整其参数。
        efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
        // 获取监听器对象指针，将其转换为 efd::DataWriterListener* 类型，用于传递给 DataWriter 创建函数，以监听数据写入相关事件。
        efd::DataWriterListener* listener = (efd::DataWriterListener*)_impl_info->_listener._impl.get();
        // 通过 Publisher 对象创建 DataWriter，传入主题对象、配置好的 QOS 参数以及监听器指针，
        // 若创建失败，输出错误信息并返回 false。
        _impl_info->_datawriter = _impl_info->_publisher->create_datawriter(_impl_info->_topic, wqos, listener);
        if (_impl_info->_datawriter == nullptr) {
            std::cerr << "Failed to create DataWriter" << std::endl;
            return false;
        }

        // 设置帧 ID，将其赋值为相机发布器的名称，便于在相机信息数据等消息中进行标识和关联操作。
        _frame_id = _name;
        // 如果所有组件都成功创建和配置，返回 true 表示相机信息发布器初始化成功。
        return true;
    }

    // 发布图像和相机信息，通过分别调用发布图像和发布相机信息的函数来实现，
    // 只有当图像和相机信息都成功发布时，才返回 true，否则返回 false。
    bool CarlaISCameraPublisher::Publish() {
        return PublishImage() && PublishInfo();
    }

    // 发布图像，尝试通过 DataWriter 将存储的图像数据写入到对应的主题中进行发布，
    // 根据写入操作的返回码来判断发布是否成功，若返回码表示成功则返回 true，否则根据不同的错误返回码输出相应错误信息并返回 false。
    bool CarlaISCameraPublisher::PublishImage() {
        // 创建一个实例句柄对象，在 Fast DDS 中用于标识数据实例，通常在写入数据时需要传递该句柄。
        eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
        // 调用 DataWriter 的 write 方法尝试将图像数据写入主题，传入图像数据指针和实例句柄对象，
        // 获取操作的返回码用于后续的错误判断。
        eprosima::fastrtps::types::ReturnCode_t rcode = _impl->_datawriter->write(&_impl->_image, instance_handle);
        // 如果返回码表示操作成功（RETCODE_OK），则返回 true 表示图像发布成功。
        if (rcode == erc::ReturnCodeValue::RETCODE_OK) {
            return true;
        }
        // 以下根据不同的错误返回码分别输出相应的错误信息，并返回 false 表示图像发布失败。

        // 如果返回码表示发生错误（RETCODE_ERROR），输出相应错误信息。
        if (rcode == erc::ReturnCodeValue::RETCODE_ERROR) {
            std::cerr << "RETCODE_ERROR" << std::endl;
            return false;
        }
        // 如果返回码表示不支持的操作（RETCODE_UNSUPPORTED），输出相应错误信息。
        if (rcode == erc::ReturnCodeValue::RETCODE_UNSUPPORTED) {
            std::cerr << "RETCODE_UNSUPPORTED" << std::endl;
            return false;
        }
        // 如果返回码表示参数错误（RETCODE_BAD_PARAMETER），输出相应错误信息。
        if (rcode == erc::ReturnCodeValue::RETCODE_BAD_PARAMETER) {
            std::cerr << "RETCODE_BAD_PARAMETER" << std::endl;
            return false;
        }
        // 如果返回码表示前置条件未满足（RETCODE_PRECONDITION_NOT_MET），输出相应错误信息。
        if (rcode == erc::ReturnCodeValue::RETCODE_PRECONDITION_NOT_MET) {
            std::cerr << "RETCODE_PRECONDITION_NOT_MET" << std::endl;
            return false;
        }
        // 如果返回码表示资源不足（RETCODE_OUT_OF_RESOURCES），输出相应错误信息。
        if (rcode == erc::ReturnCodeValue::RETCODE_OUT_OF_RESOURCES) {
            std::cerr << "RETCODE_OUT_OF_RESOURCES" << std::endl;
            return false;
        }
        // 如果返回码表示未启用相关功能（RETCODE_NOT_ENABLED），输出相应错误信息。
        if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ENABLED) {
            std::cerr << "RETCODE_NOT_ENABLED" << std::endl;
            return false;
        }
        // 如果返回码表示策略不可变（RETCODE_IMMUTABLE_POLICY），输出相应错误信息。
        if (rcode == erc::ReturnCodeValue::RETCODE_IMMUTABLE_POLICY) {
            std::cerr << "RETCODE_IMMUTABLE_POLICY" << std::endl;
            return false;
        }
        // 如果返回码表示策略不一致（RETCODE_INCONSISTENT_POLICY），输出相应错误信息。
        if (rcode == erc::ReturnCodeValue::RETCODE_INCONSISTENT_POLICY) {
            std::cerr << "RETCODE_INCONSISTENT_POLICY" << std::endl;
            return false;
        }
        // 如果返回码表示对象已被删除（RETCODE_ALREADY_DELETED），输出相应错误信息。
        if (rcode == erc::ReturnCodeValue::RETCODE_ALREADY_DELETED) {
            std::cerr << "RETCODE_ALREADY_DELETED" << std::endl;
            return false;
        }
        // 如果返回码表示操作超时（RETCODE_TIMEOUT），输出相应错误信息。
        if (rcode == erc::ReturnCodeValue::RETCODE_TIMEOUT) {
            std::cerr << "RETCODE_TIMEOUT" << std::endl;
            return false;
        }
        // 如果返回码表示无数据可操作（RETCODE_NO_DATA），输出相应错误信息。
        if (rcode == erc::ReturnCodeValue::RETCODE_NO_DATA) {
            std::cerr << "RETCODE_NO_DATA" << std::endl;
            return false;
        }
        // 如果返回码表示非法操作（RETCODE_ILLEGAL_OPERATION），输出相应错误信息。
        if (rcode == erc::ReturnCodeValue::RETCODE_ILLEGAL_OPERATION) {
            std::cerr << "RETCODE_ILLEGAL_OPERATION" << std::endl;
            return false;
        }
        // 如果返回码表示操作不被安全策略允许（RETCODE_NOT_ALLOWED_BY_SECURITY），输出相应错误信息。
        if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ALLOWED_BY_SECURITY) {
            std::cerr << "RETCODE_NOT_ALLOWED_BY_SECURITY" << std::endl;
            return false;
        }
        // 如果返回的是未知的错误返回码，输出相应提示信息并返回 false。
        std::cerr << "UNKNOWN" << std::endl;
        return false;
    }

    // 发布相机信息，与发布图像类似，尝试通过对应的 DataWriter 将存储的相机信息数据写入到相应主题中进行发布，
    // 根据写入操作的返回码判断发布是否成功，若返回码表示成功则返回 true，否则根据不同错误返回码输出相应错误信息并返回 false。
    bool CarlaISCameraPublisher::PublishInfo() {
        // 创建一个实例句柄对象，用于在写入数据时标识数据实例，符合 Fast DDS 的操作要求。
        eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
        // 调用 DataWriter 的 write 方法尝试将相机信息数据写入主题，传入相机信息数据指针和实例句柄对象，
        // 获取操作返回码用于后续的错误判断。
        eprosima::fastrtps::types::ReturnCode_t rcode = _impl_info->_datawriter->write(&_impl_info->_info, instance_handle);
        // 如果返回码表示操作成功（RETCODE_OK），则返回 true 表示相机信息发布成功。
        if (rcode == erc::ReturnCodeValue::RETCODE_OK) {
            return true;
        }
        // 以下根据不同的错误返回码分别输出相应的错误信息，并返回 false 表示相机信息发布失败。

        // 如果返回码表示发生错误（RETCODE_ERROR），输出相应错误信息。
        if (rcode == erc::ReturnCodeValue::RETCODE_ERROR) {
            std::cerr << "RETCODE_ERROR" << std::endl;
            return false;
        }
        // 如果返回码表示不支持的操作（RETCODE_UNSUPPORTED），输出相应错误信息。
        if (rcode == erc::ReturnCodeValue::RETCODE_UNSUPPORTED) {
            std::cerr << "RETCODE_UNSUPPORTED" << std::endl;
            return false;
        }
        // 如果返回码表示参数错误（RETCODE_BAD_PARAMETER），输出相应错误信息。
        if (rcode == erc::ReturnCodeValue::RETCODE_BAD_PARAMETER) {
            std::cerr << "RETCODE_BAD_PARAMETER" << std::endl;
            return false;
        }
        // 如果返回码表示前置条件未满足（RETCODE_PRECONDITION_NOT_MET），输出相应错误信息。
        if (rcode == erc::ReturnCodeValue::RETCODE_PRECONDITION_NOT_MET) {
            std::cerr << "RETCODE_PRECONDITION_NOT_MET" << std::endl;
            return false;
        }
        // 如果返回码表示资源不足（RETCODE_OUT_OF_RESOURCES），输出相应错误信息。
        if (rcode == erc::ReturnCodeValue::RETCODE_OUT_OF_RESOURCES) {
            std::cerr << "RETCODE_OUT_OF_RESOURCES" << std::endl;
            return false;
        }
        // 如果返回码表示未启用相关功能（RETCODE_NOT_ENABLED），输出相应错误信息。
        if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ENABLED) {
            std::cerr << "RETCODE_NOT_ENABLED" << std::endl;
            return false;
        }
        // 如果返回码表示策略不可变（RETCODE_IMMUTABLE_POLICY），输出相应错误信息。
        if (rcode == erc::ReturnCodeValue::RETCODE_IMMUTABLE_POLICY) {
            std::cerr << "RETCODE_IMMUTABLE_POLICY" << std::endl;
            return false;
        }
        // 如果返回码表示策略不一致（RETCODE_INCONSISTENT_POLICY），输出相应错误信息。
        if (rcode == erc::ReturnCodeValue::RETCODE_INCONSISTENT_POLICY) {
            std::cerr << "RETCODE_INCONSISTENT_POLICY" << std::endl;
            return false;
        }
        // 如果返回码表示对象已被删除（RETCODE_ALREADY_DELETED），输出相应错误信息。
        if (rcode == erc::ReturnCodeValue::RETCODE_ALREADY_DELETED) {
            std::cerr << "RETCODE_ALREADY_DELETED" << std::endl;
            return false;
        }
        // 如果返回码表示操作超时（RETCODE_TIMEOUT），输出相应错误信息。
        if (rcode == erc::ReturnCodeValue::RETCODE_TIMEOUT) {
            std::cerr << "RETCODE_TIMEOUT" << std::endl;
            return false;
        }
        // 如果返回码表示无数据可操作（RETCODE_NO_DATA），输出相应错误信息。
        if (rcode == erc::ReturnCodeValue::RETCODE_NO_DATA) {
            std::cerr << "RETCODE_NO_DATA" << std::endl;
            return false;
        }
        // 如果返回码表示非法操作（RETCODE_ILLEGAL_OPERATION），输出相应错误信息。
        if (rcode == erc::ReturnCodeValue::RETCODE_ILLEGAL_OPERATION) {
            std::cerr << "RETCODE_ILLEGAL_OPERATION" << std::endl;
            return false;
        }
        // 如果返回码表示操作不被安全策略允许（RETCODE_NOT_ALLOWED_BY_SECURITY），输出相应错误信息。
        if (rcode == erc::ReturnCodeValue::RETCODE_NOT_ALLOWED_BY_SECURITY) {
            std::cerr << "RETCODE_NOT_ALLOWED_BY_SECURITY" << std::endl;
            return false;
        }
        // 如果返回的是未知的错误返回码，输出相应提示信息并返回 false。
        std::cerr << "UNKNOWN" << std::endl;
        return false;
    }

    // 设置图像数据，先将传入的图像数据复制到一个向量中（假设图像数据格式为 BGRA，按此格式计算数据大小并调整向量大小），
    // 然后调用另一个函数将处理好的数据设置到相应的内部数据结构中，用于后续的图像发布操作。
    void CarlaISCameraPublisher::SetImageData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, const uint8_t* data) {
        // 创建一个用于存储图像数据的向量，后续将把传入的图像数据复制到这个向量中进行管理。
        std::vector<uint8_t> vector_data;
        // 根据图像的高度、宽度以及假设的 BGRA 格式（每个像素占4个字节）来计算图像数据的总大小。
        const size_t size = height * width * 4;
        // 调整向量的大小，使其能够容纳计算得出的图像数据量。
        vector_data.resize(size);
        // 使用 memcpy 函数将传入的图像数据指针所指向的数据复制到创建的向量中，从向量的起始位置开始复制，复制的数据量为前面计算出的大小。
        std::memcpy(&vector_data[0], &data[0], size);
        // 调用 SetData 函数，将处理好的图像数据（包含时间戳等相关信息）设置到内部用于存储图像数据的结构中，以便后续发布操作使用。
        SetData(seconds, nanoseconds, height, width, std::move(vector_data));
    }

    // 设置相机信息的感兴趣区域，创建一个 RegionOfInterest 对象，根据传入的偏移量、高度、宽度以及是否矫正等参数进行相应设置，
    // 然后将该对象设置到相机信息内部的数据结构中，用于后续发布相机信息时包含此感兴趣区域相关的内容。
    void CarlaISCameraPublisher::SetInfoRegionOfInterest(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, bool do_rectify) {
        // 创建一个 RegionOfInterest 类型的对象，用于表示相机信息中的感兴趣区域。
        sensor_msgs::msg::RegionOfInterest roi;
        // 设置感兴趣区域在图像中的 x 轴偏移量，该偏移量表示感兴趣区域在图像水平方向上的起始位置。
        roi.x_offset(x_offset);
        // 设置感兴趣区域在图像中的 y 轴偏移量，用于确定感兴趣区域在图像垂直方向上的起始位置。
        roi.y_offset(y_offset);
        // 设置感兴趣区域的高度，即该区域在垂直方向上覆盖的像素数量。
        roi.height(height);
        // 设置感兴趣区域的宽度，代表该区域在水平方向上覆盖的像素数量。
        roi.width(width);
        // 设置是否对感兴趣区域进行矫正的标志，根据传入的参数确定是否需要在后续处理中对该区域的数据进行矫正操作。
        roi.do_rectify(do_rectify);
        // 将设置好的感兴趣区域对象赋值给相机信息内部存储结构中的相应成员，以便在发布相机信息时包含此区域信息。
        _impl_info->_info.roi(roi);
    }

    // 设置图像相关的数据（包含时间戳、图像尺寸、编码格式等信息）到内部的图像数据结构中，
    // 先构建时间戳对象，再设置消息头信息（包含时间戳和帧 ID），最后将图像的各项属性（宽度、高度、编码、字节序、每行字节数以及实际图像数据等）设置到对应的成员变量中。
    void CarlaISCameraPublisher::SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, std::vector<uint8_t>&& data) {
        // 创建一个用于表示时间的对象，用于记录图像数据对应的时间戳信息。
        builtin_interfaces::msg::Time time;
        // 设置时间对象中的秒数部分，来源于传入的参数，用于精确标识图像数据的时间信息。
        time.sec(seconds);
        // 设置时间对象中的纳秒数部分，同样依据传入的参数进行赋值，进一步细化时间戳的精度。
        time.nanosec(nanoseconds);

        // 创建一个消息头对象，通常用于包含消息的一些通用元信息，如时间戳、帧 ID 等，便于在消息传递过程中进行相关的处理和识别。
        std_msgs::msg::Header header;
        // 将构建好的时间对象通过移动语义赋值给消息头的时间戳成员，确保时间信息正确关联到消息头中。
        header.stamp(std::move(time));
        // 设置消息头中的帧 ID，使用类内部保存的帧 ID 信息，用于在整个系统中标识该图像数据所属的帧，方便后续关联和处理。
        header.frame_id(_frame_id);

        // 将设置好的消息头通过移动语义赋值给内部用于存储图像数据的对象的消息头成员，使得图像数据包含正确的元信息。
        _impl->_image.header(std::move(header));
        // 设置图像数据对象的宽度属性，使用传入的图像宽度参数进行赋值，反映图像在水平方向上的像素数量。
        _impl->_image.width(width);
        // 设置图像数据对象的高度属性，依据传入的图像高度参数进行赋值，代表图像在垂直方向上的像素数量。
        _impl->_image.height(height);
        // 设置图像数据的编码格式为 "bgra8"，这里应该是遵循某种图像数据编码规范，表明图像数据的像素格式及编码方式，
        // 该字符串可能来源于项目中约定的编码格式列表（例如从 include/sensor_msgs/image_encodings.h 中定义的相关字符串选取）。
        _impl->_image.encoding("bgra8");
        // 设置图像数据是否为大端序，这里设置为 0，表示为小端序，用于明确图像数据在内存中的字节顺序，方便在不同平台间进行数据处理和传输时保持一致性。
        _impl->_image.is_bigendian(0);
        // 根据图像宽度以及每个像素所占字节数（这里假设为 4 字节，对应前面提到的 BGRA 格式）计算每行图像数据所占的字节数，
        // 并赋值给图像数据对象的相应成员，用于后续在处理图像数据时进行正确的内存操作和数据解析等。
        _impl->_image.step(_impl->_image.width() * sizeof(uint8_t) * 4);
        // 通过移动语义将传入的图像数据向量赋值给图像数据对象的实际数据成员，完成图像数据的整体设置，使得内部数据结构准备好待发布的图像内容。
        _impl->_image.data(std::move(data));
    }

    // 设置相机信息数据中的时间戳部分，先构建时间对象，根据传入的秒数和纳秒数参数进行赋值，
    // 然后将包含时间戳的消息头设置到相机信息内部的数据结构中，用于更新相机信息的时间相关属性，方便后续发布准确的相机信息。
    void CarlaISCameraPublisher::SetCameraInfoData(int32_t seconds, uint32_t nanoseconds) {
        // 创建一个用于表示时间的对象，用于承载相机信息对应的时间戳信息。
        builtin_interfaces::msg::Time time;
        // 设置时间对象中的秒数部分，依据传入的参数进行赋值，确定时间戳的秒数信息。
        time.sec(seconds);
        // 设置时间对象中的纳秒数部分，按照传入的参数完成赋值，进一步细化时间戳的精度。
        time.nanosec(nanoseconds);

        // 创建一个消息头对象，用于包含相机信息的通用元信息，如时间戳、帧 ID 等，便于在消息传递和处理过程中进行相关操作。
        std_msgs::msg::Header header;
        // 将构建好的时间对象通过移动语义赋值给消息头的时间戳成员，使得消息头包含正确的时间信息。
        header.stamp(std::move(time));
        // 设置消息头中的帧 ID，使用类内部保存的帧 ID 信息，用于在系统中标识该相机信息所属的帧，方便后续关联和识别。
        header.frame_id(_frame_id);
        // 将包含设置好的时间戳和帧 ID 的消息头赋值给相机信息内部存储结构中的相应成员，更新相机信息的时间相关属性，以便后续发布包含准确时间信息的相机信息数据。
        _impl_info->_info.header(header);
    }

    // 构造函数，用于创建 CarlaISCameraPublisher 对象，初始化对象时创建并关联用于图像发布和相机信息发布的内部实现结构体对象，
    // 同时接收并保存相机发布器的名称以及父主题名称等参数，用于后续构建主题名称等相关操作。
    CarlaISCameraPublisher::CarlaISCameraPublisher(const char* ros_name, const char* parent) :
        _impl(std::make_shared<CarlaISCameraPublisherImpl>()),
        _impl_info(std::make_shared<CarlaCameraInfoPublisherImpl>()) {
        // 将传入的相机发布器的名称参数赋值给类内部对应的成员变量，用于后续操作中对该对象的标识等用途。
        _name = ros_name;
        // 将传入的父主题名称参数赋值给类内部的相应成员变量，可能在构建主题名称等操作时会依据该父主题信息来形成更具层次结构的主题标识。
        _parent = parent;
    }

    // 析构函数，用于在对象销毁时释放和清理相关的资源，包括删除 DataWriter、Publisher、Topic 以及 DomainParticipant 等对象，
    // 按照创建的相反顺序依次进行释放操作，确保资源正确回收，避免内存泄漏等问题。
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

    // 拷贝构造函数，用于创建一个新的 CarlaISCameraPublisher 对象，通过拷贝另一个已存在对象的相关成员变量来初始化新对象，
    // 实现对象之间的数据复制，使得新对象具有与原对象相同的属性状态，方便在需要复制对象的场景下使用。
    CarlaISCameraPublisher::CarlaISCameraPublisher(const CarlaISCameraPublisher& other) {
        // 拷贝帧 ID，将另一个对象的帧 ID 赋值给当前新对象的帧 ID 成员变量，保持标识信息的一致性。
        _frame_id = other._frame_id;
        // 拷贝 ROS 名称，把原对象的 ROS 名称复制给新对象的对应成员变量，用于后续操作中对新对象的标识等用途。
        _name = other._name;
        // 拷贝父主题，将原对象的父主题名称赋值给新对象的相应成员变量，可能在构建主题名称等相关操作时会用到该信息。
        _parent = other._parent;
        // 拷贝实现结构体，将原对象中用于图像发布的内部实现结构体对象指针赋值给新对象的对应指针，实现内部数据结构的复制，
        // 使得新对象能够共享类似的内部状态和功能配置（注意这里只是浅拷贝指针，若需要深拷贝内部数据结构可能需要进一步处理）。
        _impl = other._impl;
        // 拷贝相机信息实现结构体，同样将原对象中针对相机信息发布的内部实现结构体对象指针赋值给新对象的对应指针，
        // 完成相机信息相关内部状态的复制，便于新对象具备与原对象类似的相机信息发布功能配置（同样是浅拷贝指针情况）。
        _impl_info = other._impl_info;
    }

    // 拷贝赋值运算符，用于将一个 CarlaISCameraPublisher 对象的状态赋值给另一个已存在的对象，
    // 通过拷贝各个成员变量的值，使得目标对象的属性和状态与源对象一致，实现对象间数据的赋值操作，支持类似对象赋值的语法形式。
    CarlaISCameraPublisher& CarlaISCameraPublisher::operator=(const CarlaISCameraPublisher& other) {
        // 拷贝帧 ID，将源对象的帧 ID 赋值给当前对象的帧 ID 成员变量，保证标识信息的同步更新。
        _frame_id = other._frame_id;
        // 拷贝 ROS 名称，把源对象的 ROS 名称复制给当前对象的对应成员变量，用于后续操作中对该对象的正确标识等用途。
        _name = other._name;
        // 拷贝父主题，将源对象的父主题名称赋值给当前对象的相应成员变量，以便在构建主题名称等操作时依据该信息进行处理。
        _parent = other._parent;
        // 拷贝实现结构体，将源对象中用于图像发布的内部实现结构体对象指针赋值给当前对象的对应指针，实现内部数据结构的赋值，
        // 使得当前对象的图像发布相关内部状态与源对象一致（同样是浅拷贝指针情况，若涉及资源管理可能需进一步考虑深拷贝等情况）。
        _impl = other._impl;
        // 拷贝相机信息实现结构体，把源对象中针对相机信息发布的内部实现结构体对象指针赋值给当前对象的对应指针，
        // 完成相机信息相关内部状态的赋值，确保当前对象具备与源对象相同的相机信息发布功能配置（浅拷贝指针情况）。
        _impl_info = other._impl_info;

        return *this;
    }

    // 移动构造函数，用于通过“移动”另一个对象的资源来构造新的 CarlaISCameraPublisher 对象，
    // 它会将原对象的相关成员变量（如帧 ID、名称、主题相关指针等）所占用的资源转移给新对象，原对象则处于一种可析构的有效但空状态，
    // 这种方式避免了不必要的资源复制开销，常用于高效地传递资源所有权的场景。
    CarlaISCameraPublisher::CarlaISCameraPublisher(CarlaISCameraPublisher&& other) {
        // 通过移动语义将原对象的帧 ID 资源转移给新对象，原对象的帧 ID 成员变量变为未定义状态，新对象获得该帧 ID 的所有权。
        _frame_id = std::move(other._frame_id);
        // 移动 ROS 名称，将原对象的 ROS 名称资源转移给新对象，原对象的该成员变量不再拥有此名称资源，新对象可以使用它进行后续操作。
        _name = std::move(other._name);
        // 移动父主题，把原对象的父主题名称资源转移给新对象，使得新对象获得该名称资源用于后续构建主题名称等相关操作，原对象相应成员变量释放资源。
        _parent = std::move(other._parent);
        // 移动实现结构体，将原对象中用于图像发布的内部实现结构体对象的资源所有权转移给新对象，原对象的该指针变为空指针，新对象可以使用其内部资源，
        // 实现高效的资源转移，避免了数据的复制操作，提高了资源利用效率（注意后续对原对象的操作需谨慎，避免访问已转移的资源）。
        _impl = std::move(other._impl);
        // 移动相机信息实现结构体，同样将原对象中针对相机信息发布的内部实现结构体对象的资源所有权转移给新对象，
        // 原对象对应指针变为空指针，新对象获得其资源用于相机信息发布相关操作，实现资源的有效转移和重新利用。
        _impl_info = std::move(other._impl_info);
    }

    // 移动赋值运算符，用于将一个 CarlaISCameraPublisher 对象的资源“移动”给另一个已存在的对象，
    // 通过移动语义操作各个成员变量，使得目标对象获取源对象的资源所有权，源对象进入一种可析构的有效但空状态，
    // 实现资源的高效转移赋值，常用于需要重新分配资源所有权的场景，避免了传统赋值操作中可能的资源复制开销。
    CarlaISCameraPublisher& CarlaISCameraPublisher::operator=(CarlaISCameraPublisher&& other) {
        // 通过移动语义将源对象的帧 ID 资源转移给当前对象，当前对象获得帧 ID 的所有权，源对象的帧 ID 成员变量变为未定义状态。
        _frame_id = std::move(other._frame_id);
        // 移动 ROS 名称，将源对象的 ROS 名称资源转移给当前对象，当前对象获得该名称资源用于后续操作，源对象相应成员变量释放资源。
        _name = std::move(other._name);
        // 移动父主题，把源对象的父主题名称资源转移给当前对象，使得当前对象可以使用该名称进行后续构建主题名称等相关操作，源对象对应成员变量变为空。
        _parent = std::move(other._parent);
        // 移动实现结构体，将源对象中用于图像发布的内部实现结构体对象的资源所有权转移给当前对象，
        // 当前对象获得其资源用于图像发布相关操作，源对象的该指针变为空指针，实现资源的高效转移（后续需注意对源对象的操作避免访问已转移资源）。
        _impl = std::move(other._impl);
        // 移动相机信息实现结构体，同样将源对象中针对相机信息发布的内部实现结构体对象的资源所有权转移给当前对象，
        // 当前对象获得其资源用于相机信息发布相关操作，源对象对应指针变为空指针，完成资源的有效转移和重新配置。
        _impl_info = std::move(other._impl_info);

        return *this;
    }
}
}

