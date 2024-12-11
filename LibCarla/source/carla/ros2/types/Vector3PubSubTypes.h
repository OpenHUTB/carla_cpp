// ��Ȩ������������2016���Proyectos y Sistemas de Mantenimiento SL (eProsima)
// ��ѭApache License 2.0���Э�飬ʹ�øô������������Ȩ�޼���������
// �����������ο���http://www.apache.org/licenses/LICENSE-2.0
//
// ���´�����Ҫ�������л�����������ص�ͷ�ļ����ݣ���fastcdrgen��������
// ע�⣺���GEN_API_VER��δ������߲�����1�����ʾ���ɵ�Vector3�뵱ǰ��װ��Fast DDS�����ݣ�����fastddsgen������������
// ����Fast DDS����������������ص�ͷ�ļ�
#include <fastdds/dds/topic/TopicDataType.hpp>
// ����Fast RTPS��MD5��ع��ߵ�ͷ�ļ���������������У��Ȳ���
#include <fastrtps/utils/md5.h>
// �����Զ����Vector3�������ͷ�ļ����Ʋ����ж�����Vector3�ṹ����������
#include "Vector3.h"

#if!defined(GEN_API_VER) || (GEN_API_VER!= 1)
#error "���ɵ�Vector3�뵱ǰ��װ��Fast DDS�����ݣ���ʹ��fastddsgen�����������ɡ�"
#endif  // GEN_API_VER

// ������geometry_msgs�����ռ��µ�msg�����ռ䣬������֯��ص���Ϣ���͵ȴ����߼�
namespace geometry_msgs {
namespace msg {

#ifndef SWIG
// ������detail�����ռ��ڵ����ݣ���Ҫ���ڸ���������Vector3������ص�һЩ�ײ����ϸ��
namespace detail {

// ����һ��ģ��ṹ��Vector3_rob��ͨ��ģ�����Tag�Լ���Ӧ����M����ȡ�ض���Ա�����Ϣ
// ������Ԫ�����ķ�ʽ��ʹ���ⲿ���Ի�ȡ������ض���Ա��ֵ����������ģ�����ָ���ĳ�Ա��
template<typename Tag, typename Tag::type M>
struct Vector3_rob {
    friend constexpr typename Tag::type get(Tag) {
        return M;
    }
};

// ����ṹ��Vector3_f�����ڱ�ʾVector3�����г�Ա����Ϊdouble��ָ������
// ͨ����Ԫ�����ķ�ʽ���ⲿ���Ի�ȡ�������ʾ��Աָ�������
struct Vector3_f {
    typedef double Vector3::* type;
    friend constexpr type get(Vector3_f);
};

// ��ģ��ṹ��Vector3_rob����ʵ������ָ����ԱΪVector3��m_z��Ա�����ں�����ز�������Ըó�Ա�Ĵ���
template struct Vector3_rob<Vector3_f, &Vector3::m_z>;

// �����������T�У�ͨ��Tagָ���ĳ�Ա��ƫ���������ֽ�Ϊ��λ��
// ������ָ������Լ�����ת���ȼ�����ʵ��ƫ�����ļ��㣬����ֵ��һ���������ʽ��ʽ��size_t���ʹ�С
template <typename T, typename Tag>
inline size_t constexpr Vector3_offset_of() {
    return ((::size_t) &reinterpret_cast<char const volatile&>((((T*)0)->*get(Tag()))));
}
}
#endif

// Vector3PubSubType�࣬�̳���eprosima::fastdds::dds::TopicDataType�����ڴ�����Vector3������ص����ݲ���
// ������Ҫ�漰��Fast DDS����£���Vector3�������ݵ����л��������л�����ȡ���ݴ�С����ȡ��ֵ�ȹ���
class Vector3PubSubType : public eprosima::fastdds::dds::TopicDataType {
public:
    // �������ͱ���������������Ҫ����������������Vector3
    typedef Vector3 type;

    // ���캯�������ڳ�ʼ��Vector3PubSubType���ʵ�������ܽ���һЩ��Դ����ȳ�ʼ������
    eProsima_user_DllExport Vector3PubSubType();

    // ���������������ͷ���ʵ��ռ�õ���Դ�����綯̬������ڴ��
    eProsima_user_DllExport virtual ~Vector3PubSubType() override;

    // ���л���������������Vector3���͵����ݣ�ͨ��dataָ��ָ��ת��Ϊ��������Ȼ��������SerializedPayload_t���͵����ݣ��洢��payload�У�
    // ������л��ɹ�����true�����򷵻�false
    eProsima_user_DllExport virtual bool serialize(
            void* data,
            eprosima::fastrtps::rtps::SerializedPayload_t* payload) override;

    // �����л������������յ���SerializedPayload_t���͵����ݣ�ͨ��payloadָ��ָ��ת��ΪVector3���͵����ݣ��洢��dataָ����ڴ�ռ䣩
    // ��������л��ɹ�����true�����򷵻�false
    eProsima_user_DllExport virtual bool deserialize(
            eprosima::fastrtps::rtps::SerializedPayload_t* payload,
            void* data) override;

    // ��ȡ���л������ݴ�С�ĺ����ṩ�ߣ�����һ���������󣬸ú��������ڵ���ʱ�᷵�ظ������ݣ�ͨ��dataָ��ָ�����л���Ĵ�С�����ֽ�Ϊ��λ��
    eProsima_user_DllExport virtual std::function<uint32_t()> getSerializedSizeProvider(
            void* data) override;

    // ��ȡ���ݵļ�ֵ����������Vector3���͵����ݣ�ͨ��dataָ��ָ��ת��Ϊ����Ψһ��ʶ������ʵ���ļ�ֵ��Ϣ���洢��ihandleָ����ڴ�ռ䣩
    // �����ȡ��ֵ�ɹ�����true����force_md5Ϊtrue���ܲ���MD5��ʽ��ȡ��ֵ�����⴦�����򷵻�false
    eProsima_user_DllExport virtual bool getKey(
            void* data,
            eprosima::fastrtps::rtps::InstanceHandle_t* ihandle,
            bool force_md5 = false) override;

    // �������ڴ洢Vector3�������ݵ��ڴ�ռ䣬����ָ����ڴ�ռ��ָ�룬ͨ�����ڷ��䶯̬�ڴ����洢����ʵ��
    eProsima_user_DllExport virtual void* createData() override;

    // �ͷ�֮ǰͨ��createData������������ڴ洢Vector3�������ݵ��ڴ�ռ䣬�����ڴ�й©
    eProsima_user_DllExport virtual void deleteData(
            void* data) override;

#ifdef TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED
    // �жϸ����������Ƿ����н�ģ����ܱ�ʾ���ݴ�С���й̶���Χ���ƣ���Ĭ�Ϸ���true
    eProsima_user_DllExport inline bool is_bounded() const override {
        return true;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED

#ifdef TOPIC_DATA_TYPE_API_HAS_IS_PLAIN
    // �жϸ����������Ƿ��Ǽ����ͣ����ܱ�ʾ���ݽṹ�Ƚϼ򵥡��޸���Ƕ�׵���������ڲ�����is_plain_impl�������ж�
    eProsima_user_DllExport inline bool is_plain() const override {
        return is_plain_impl();
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_IS_PLAIN

#ifdef TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE
    // �ڸ������ڴ�ռ䣨ͨ��memoryָ��ָ�򣩹���һ��Vector3���͵�����ʵ�������Ƶ��ù��캯���Ĳ���
    eProsima_user_DllExport inline bool construct_sample(
            void* memory) const override {
        new (memory) Vector3();
        return true;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE
    // MD5���󣬿������ڼ������ݵ�MD5ֵ�������ڻ�ȡ��ֵ�Ȳ�����ʹ��
    MD5 m_md5;
    // ���ڴ洢��ֵ�������Ϣ�Ļ�����ָ�룬��̬�����ڴ����洢����
    unsigned char* m_keyBuffer;
private:
    // �����жϸ����������Ƿ��Ǽ����͵ľ���ʵ�ֺ�����ͨ���Ƚϳ�Աƫ���������ݴ�С�ȷ�ʽ���ж�
    static constexpr bool is_plain_impl() {
        return 24ULL == (detail::Vector3_offset_of<Vector3, detail::Vector3_f>() + sizeof(double));
    }
};
}
}

#endif // _FAST_DDS_GENERATED_GEOMETRY_MSGS_MSG_VECTOR3_PUBSUBTYPES_H_
