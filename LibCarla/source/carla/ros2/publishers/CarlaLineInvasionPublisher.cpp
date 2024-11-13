#define _GLIBCXX_USE_CXX11_ABI 0

#include "CarlaLineInvasionPublisher.h"

#include <string>
// ����Carla��ص���Ϣ���Ͷ���
#include "carla/ros2/types/CarlaLineInvasionPubSubTypes.h"
// ����Carla���������ͷ�ļ�
#include "carla/ros2/listeners/CarlaListener.h"
// ����Fast DDS��ص�ͷ�ļ�������DDSͨ�ŵĸ��ֹ���ʵ��
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
// ����Fast DDS��ص�QoS����������������ͷ�ļ�
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
// ����Fast RTPS��ص����Ժ�QoS����ͷ�ļ�
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/qos/QosPolicies.h>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>


namespace carla {
namespace ros2 {
	// Ϊ�˷���ʹ�ã���eprosima::fastdds::dds�����ռ������efd
  namespace efd = eprosima::fastdds::dds;
  // ��eprosima::fastrtps::types::ReturnCode_t�����erc�����ڱ�ʾ����������
  using erc = eprosima::fastrtps::types::ReturnCode_t;
  // ����CarlaLineInvasionPublisherImpl�ṹ�壬���ڴ洢��������صĸ��ֶ���ָ�������
  struct CarlaLineInvasionPublisherImpl {
  	// �������ָ�룬���ڲ���DDSͨ���򣬳�ʼ��Ϊnullptr
    efd::DomainParticipant* _participant { nullptr };
    // ������ָ�룬���ڷ������ݣ���ʼ��Ϊnullptr
    efd::Publisher* _publisher { nullptr };
    // ����ָ�룬���ڱ�ʶ�������ݵ����⣬��ʼ��Ϊnullptr
    efd::Topic* _topic { nullptr };
    // ����д����ָ�룬���ڽ�����д�뵽���⣬��ʼ��Ϊnullptr
    efd::DataWriter* _datawriter { nullptr };
    // ����֧�ֶ�������ע����Ϣ���ͣ�������Carla�ĳ��������¼���Ϣ����
    efd::TypeSupport _type { new carla_msgs::msg::LaneInvasionEventPubSubType() };
    // Carla�������������ڼ�������¼�
    CarlaListener _listener {};
    // �洢���������¼������ݽṹ����ʼ��ΪĬ��ֵ
    carla_msgs::msg::LaneInvasionEvent _event {};
  };
// CarlaLineInvasionPublisher��ĳ�ʼ�����������ڳ�ʼ����������صĸ��ֶ���
  bool CarlaLineInvasionPublisher::Init() {
  	// �������֧�ֶ����Ƿ�Ϊ�գ����Ϊ�������������Ϣ������false
    if (_impl->_type == nullptr) {
        std::cerr << "Invalid TypeSupport" << std::endl;
        return false;
    }
// ��ȡĬ�ϵ��������QoS���ã�������������Ϊ�����_name
    efd::DomainParticipantQos pqos = efd::PARTICIPANT_QOS_DEFAULT;
    pqos.name(_name);
    // ��ȡ������߹����ĵ���ʵ��
    auto factory = efd::DomainParticipantFactory::get_instance();
    // ʹ�ù�������������߶��󣬴�����ID��QoS���ã��������ʧ�������������Ϣ������false
    _impl->_participant = factory->create_participant(0, pqos);
    if (_impl->_participant == nullptr) {
        std::cerr << "Failed to create DomainParticipant" << std::endl;
        return false;
    }
    // �ڴ��������������ע����Ϣ����
    _impl->_type.register_type(_impl->_participant);
// ��ȡĬ�ϵķ�����QoS����
    efd::PublisherQos pubqos = efd::PUBLISHER_QOS_DEFAULT;
    // ʹ��������ߴ��������߶��󣬴���QoS���úͿյļ�����ָ�룬�������ʧ�������������Ϣ������false
    _impl->_publisher = _impl->_participant->create_publisher(pubqos, nullptr);
    if (_impl->_publisher == nullptr) {
      std::cerr << "Failed to create Publisher" << std::endl;
      return false;
    }
// ��ȡĬ�ϵ�����QoS����
    efd::TopicQos tqos = efd::TOPIC_QOS_DEFAULT;
    const std::string base { "rt/carla/" };
    std::string topic_name = base;
    // ��������Ʋ�Ϊ�գ��򽫸�������ӵ�����������
    if (!_parent.empty())
      topic_name += _parent + "/";
      // ������������Ƶ�����������
    topic_name += _name;
    // ʹ��������ߴ���������󣬴����������ơ���Ϣ�������ƺ�QoS���ã��������ʧ�������������Ϣ������false
    _impl->_topic = _impl->_participant->create_topic(topic_name, _impl->_type->getName(), tqos);
    if (_impl->_topic == nullptr) {
        std::cerr << "Failed to create Topic" << std::endl;
        return false;
    }
// ��ȡĬ�ϵ�����д����QoS���ã����������ڴ����ΪԤ���䲢�����·���ģʽ
    efd::DataWriterQos wqos = efd::DATAWRITER_QOS_DEFAULT;
    wqos.endpoint().history_memory_policy = eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
     // ��ȡ�����������ָ�룬��������д�����ļ���������
    efd::DataWriterListener* listener = (efd::DataWriterListener*)_impl->_listener._impl.get();
    // ʹ�÷����ߴ�������д�������󣬴������⡢QoS���úͼ�����ָ�룬�������ʧ�������������Ϣ������false
    _impl->_datawriter = _impl->_publisher->create_datawriter(_impl->_topic, wqos, listener);
    if (_impl->_datawriter == nullptr) {
        std::cerr << "Failed to create DataWriter" << std::endl;
        return false;
    }
    // ����֡IDΪ��������
    _frame_id = _name;
    return true;
  }
// CarlaLineInvasionPublisher��ķ������������ڽ����ݷ���������
  bool CarlaLineInvasionPublisher::Publish() {
  	// ����ʵ����������ڱ�ʶ����ʵ��
    eprosima::fastrtps::rtps::InstanceHandle_t instance_handle;
    // ��������д������write�������¼�����д�룬��ȡ������
    eprosima::fastrtps::types::ReturnCode_t rcode = _impl->_datawriter->write(&_impl->_event, instance_handle);
    // ���������ΪRETCODE_OK����ʾд��ɹ�������true
    if (rcode == erc::ReturnCodeValue::RETCODE_OK) {
        return true;
    }
     // ���ݲ�ͬ�ķ�����ֵ�������Ӧ�Ĵ�����Ϣ������false
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
// ����Ҫ�����ĳ��������¼����ݵĺ���
  void CarlaLineInvasionPublisher::SetData(int32_t seconds, uint32_t nanoseconds, const int32_t* data) {
  	// ����һ��ʱ��ṹ�壬��������������ֵ
    builtin_interfaces::msg::Time time;
    time.sec(seconds);
    time.nanosec(nanoseconds);
// ����һ����Ϣͷ�ṹ�壬����ʱ�����֡ID
    std_msgs::msg::Header header;
    header.stamp(std::move(time));
    header.frame_id(_frame_id);
// ���ó��������¼�����Ϣͷ
    _impl->_event.header(std::move(header));
    // ���ó��������¼��п�Խ������ǵ�����
    _impl->_event.crossed_lane_markings({data[0], data[1], data[2]});
  }
// CarlaLineInvasionPublisher��Ĺ��캯�������ڳ�ʼ������ĳ�Ա����
  CarlaLineInvasionPublisher::CarlaLineInvasionPublisher(const char* ros_name, const char* parent) :
  _impl(std::make_shared<CarlaLineInvasionPublisherImpl>()) {
    _name = ros_name;
    _parent = parent;
  }
// CarlaLineInvasionPublisher������������������ͷ������Դ
  CarlaLineInvasionPublisher::~CarlaLineInvasionPublisher() {
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
// �������캯�������ڸ�����һ��CarlaLineInvasionPublisher����ĳ�Ա����
  CarlaLineInvasionPublisher::CarlaLineInvasionPublisher(const CarlaLineInvasionPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;
  }
// ������ֵ��������غ��������ڽ���һ��CarlaLineInvasionPublisher����ĳ�Ա������ֵ����ǰ����
  CarlaLineInvasionPublisher& CarlaLineInvasionPublisher::operator=(const CarlaLineInvasionPublisher& other) {
    _frame_id = other._frame_id;
    _name = other._name;
    _parent = other._parent;
    _impl = other._impl;

    return *this;
  }
// �ƶ����캯���������ƶ���һ��CarlaLineInvasionPublisher�������Դ����ǰ����
  CarlaLineInvasionPublisher::CarlaLineInvasionPublisher(CarlaLineInvasionPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);
  }
// �ƶ���ֵ��������غ��������ڽ���һ��CarlaLineInvasionPublisher�������Դ�ƶ���ֵ����ǰ����
  CarlaLineInvasionPublisher& CarlaLineInvasionPublisher::operator=(CarlaLineInvasionPublisher&& other) {
    _frame_id = std::move(other._frame_id);
    _name = std::move(other._name);
    _parent = std::move(other._parent);
    _impl = std::move(other._impl);

    return *this;
  }
}
}
