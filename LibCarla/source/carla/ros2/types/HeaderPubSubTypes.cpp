#include <fastcdr/FastBuffer.h>
#include <fastcdr/Cdr.h>

#include "HeaderPubSubTypes.h"
// 定义SerializedPayload_t类型别名，用于表示序列化后的负载数据类型
using SerializedPayload_t = eprosima::fastrtps::rtps::SerializedPayload_t;
// 定义InstanceHandle_t类型别名，用于表示实例句柄类型
using InstanceHandle_t = eprosima::fastrtps::rtps::InstanceHandle_t;

namespace std_msgs {
    namespace msg {
// HeaderPubSubType类的构造函数，用于初始化与消息类型相关的属性
        HeaderPubSubType::HeaderPubSubType()
        {// 设置消息类型的名称
            setName("std_msgs::msg::dds_::Header_");
            // 获取Header类型的最大CDR序列化大小
            auto type_size = Header::getMaxCdrSerializedSize();
            // 根据可能的子消息对齐要求，对类型大小进行对齐调整
            type_size += eprosima::fastcdr::Cdr::alignment(type_size, 4);
            // 计算最终的类型大小，加上4字节用于封装
            m_typeSize = static_cast<uint32_t>(type_size) + 4;
            // 判断Header类型是否定义了键（用于标识等用途）
            m_isGetKeyDefined = Header::isKeyDefined();
            // 根据Header类型的最大键序列化大小来确定键缓冲区的长度，取较大值
            size_t keyLength = Header::getKeyMaxCdrSerializedSize() > 16 ?
                    Header::getKeyMaxCdrSerializedSize() : 16;
            // 分配键缓冲区内存
            m_keyBuffer = reinterpret_cast<unsigned char*>(malloc(keyLength));
            // 初始化键缓冲区内存为0
            memset(m_keyBuffer, 0, keyLength);
        }
// HeaderPubSubType类的析构函数，用于释放键缓冲区内存
        HeaderPubSubType::~HeaderPubSubType()
        {
            if (m_keyBuffer != nullptr)
            {
                free(m_keyBuffer);
            }
        }
// 序列化函数，将给定的数据对象（这里是Header类型）序列化为SerializedPayload_t格式
        bool HeaderPubSubType::serialize(
                void* data,
                SerializedPayload_t* payload)
        {// 将传入的void*类型数据转换为Header*类型指针，以便后续操作
            Header* p_type = static_cast<Header*>(data);

            // 创建一个FastBuffer对象，用于管理原始缓冲区，它包装了payload的data指针和最大可用大小
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->max_size);
             // 创建一个Cdr对象，用于实际的数据序列化操作，指定了缓冲区、字节序（默认字节序）以及采用DDS_CDR模式
            eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);
            // 根据序列化对象的字节序设置负载的封装字节序标识（大端序或小端序）
            payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
            // 先序列化封装相关的信息（具体内容由Cdr实现决定）
            ser.serialize_encapsulation();
            // 调用Header对象的序列化函数，将实际的数据内容序列化到Cdr对象中
            p_type->serialize(ser);
            // 获取序列化后的数据长度，并设置到负载结构体的length成员中
            payload->length = static_cast<uint32_t>(ser.getSerializedDataLength());
            return true;
        }
// 反序列化函数，将SerializedPayload_t格式的数据反序列化为对应的数据对象（Header类型）
        bool HeaderPubSubType::deserialize(
                SerializedPayload_t* payload,
                void* data)
        {
            // 将传入的void*类型数据转换为Header*类型指针，以便后续操作
            Header* p_type = static_cast<Header*>(data);

            // 创建一个FastBuffer对象，用于管理原始缓冲区，根据传入负载的实际长度来设置缓冲区大小
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->length);

            // 创建一个Cdr对象，用于实际的数据反序列化操作，指定了缓冲区、字节序（默认字节序）以及采用DDS_CDR模式
            eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);

            // 先反序列化封装相关的信息（例如字节序等相关的封装标识）
            deser.read_encapsulation();
            // 根据反序列化对象的字节序设置负载的封装字节序标识（大端序或小端序）
            payload->encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

            // 调用Header对象的反序列化函数，从Cdr对象中还原出实际的数据内容
            p_type->deserialize(deser);
            return true;
        }
// 返回一个函数对象，该函数对象用于获取给定数据（Header类型）序列化后的大小
        std::function<uint32_t()> HeaderPubSubType::getSerializedSizeProvider(
                void* data)
        {
            return [data]() -> uint32_t
                   {// 计算并返回包含封装部分的序列化大小，先获取Header类型的CDR序列化大小，再加上4字节封装大小
                       return static_cast<uint32_t>(type::getCdrSerializedSize(*static_cast<Header*>(data))) +
                              4u;
                   };
        }
// 创建一个新的Header类型的数据对象，并返回其void*指针表示
        void* HeaderPubSubType::createData()
        {
            return reinterpret_cast<void*>(new Header());
        }
// 删除给定的Header类型的数据对象（释放内存）
        void HeaderPubSubType::deleteData(
                void* data)
        {
            delete(reinterpret_cast<Header*>(data));
        }
// 获取给定数据（Header类型）对应的键值，用于实例标识等用途，填充到InstanceHandle_t结构体中
        bool HeaderPubSubType::getKey(
                void* data,
                InstanceHandle_t* handle,
                bool force_md5)
        {
            if (!m_isGetKeyDefined)
            {
                return false;
            }

            Header* p_type = static_cast<Header*>(data);

            // 创建一个FastBuffer对象，用于管理键缓冲区，大小根据Header类型的最大键序列化大小确定
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(m_keyBuffer),
                    Header::getKeyMaxCdrSerializedSize());

            // 创建一个Cdr对象，用于序列化键数据，指定为大端序
            eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::BIG_ENDIANNESS);
            // 调用Header对象的键序列化函数，将键相关的数据序列化到Cdr对象对应的缓冲区中
            p_type->serializeKey(ser);
            if (force_md5 || Header::getKeyMaxCdrSerializedSize() > 16)
            {// 如果需要强制使用MD5（可能用于更安全的标识等情况）或者键序列化大小大于16字节
                m_md5.init();
                m_md5.update(m_keyBuffer, static_cast<unsigned int>(ser.getSerializedDataLength()));
                m_md5.finalize();
                for (uint8_t i = 0; i < 16; ++i)
                {
                    handle->value[i] = m_md5.digest[i];
                }
            }
            else
            {// 如果不需要MD5且键序列化大小小于等于16字节，直接将键缓冲区的数据复制到实例句柄的value数组中
                for (uint8_t i = 0; i < 16; ++i)
                {
                    handle->value[i] = m_keyBuffer[i];
                }
            }
            return true;
        }
    }
}
