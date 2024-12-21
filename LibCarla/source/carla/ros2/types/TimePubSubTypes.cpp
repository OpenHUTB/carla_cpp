#include <fastcdr/FastBuffer.h>
#include <fastcdr/Cdr.h>

#include "TimePubSubTypes.h"

using SerializedPayload_t = eprosima::fastrtps::rtps::SerializedPayload_t;
using InstanceHandle_t = eprosima::fastrtps::rtps::InstanceHandle_t;

// 定义在 builtin_interfaces::msg 命名空间下的 TimePubSubType 类
namespace builtin_interfaces {
    namespace msg {
        // TimePubSubType 类的构造函数
        TimePubSubType::TimePubSubType()
        {
            // 设置类型名称，用于标识该数据类型在 DDS（Data Distribution Service）中的名称
            setName("builtin_interfaces::msg::dds_::Time_");
            // 获取 Time 类型的最大 CDR（Common Data Representation）序列化大小
            auto type_size = Time::getMaxCdrSerializedSize();
            // 根据可能的子消息对齐要求，对类型大小进行对齐调整（按 4 字节对齐）
            type_size += eprosima::fastcdr::Cdr::alignment(type_size, 4); /* possible submessage alignment */
            // 计算最终的类型大小，加上 4 字节用于封装（encapsulation）相关信息
            m_typeSize = static_cast<uint32_t>(type_size) + 4; /*encapsulation*/
            // 判断 Time 类型是否定义了获取键（Key）的操作
            m_isGetKeyDefined = Time::isKeyDefined();
            // 根据 Time 类型获取键的最大 CDR 序列化大小来确定键缓冲区的长度
            // 如果大于 16 字节则使用实际的最大键序列化大小，否则使用 16 字节
            size_t keyLength = Time::getKeyMaxCdrSerializedSize() > 16?
                    Time::getKeyMaxCdrSerializedSize() : 16;
            // 分配键缓冲区的内存空间
            m_keyBuffer = reinterpret_cast<unsigned char*>(malloc(keyLength));
            // 将键缓冲区的内存初始化为 0
            memset(m_keyBuffer, 0, keyLength);
        }

        // TimePubSubType 类的析构函数，用于释放键缓冲区的内存
        TimePubSubType::~TimePubSubType()
        {
            if (m_keyBuffer!= nullptr)
            {
                free(m_keyBuffer);
            }
        }

        // 序列化函数，将给定的数据对象（Time 类型）序列化为 SerializedPayload_t 格式
        bool TimePubSubType::serialize(
                void* data,
                SerializedPayload_t* payload)
        {
            // 将传入的 void* 类型数据转换为 Time* 类型指针，方便后续操作
            Time* p_type = static_cast<Time*>(data);

            // 创建一个 FastBuffer 对象，用于管理原始缓冲区（指向 payload->data），其大小为 payload->max_size
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->max_size);
            // 创建一个 Cdr 对象，用于执行序列化操作，设置了字节序（默认字节序）和 DDS CDR 相关模式
            eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);
            // 根据序列化对象的字节序设置 payload 的封装字节序标识（CDR_BE 表示大端序，CDR_LE 表示小端序）
            payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS? CDR_BE : CDR_LE;
            // 序列化封装相关的头部信息（比如可能的长度等元数据）
            ser.serialize_encapsulation();
            // 调用 Time 对象的 serialize 方法，将具体的 Time 数据序列化到 Cdr 对象中
            p_type->serialize(ser);
            // 获取序列化后的数据长度，并设置到 payload 的 length 成员中
            payload->length = static_cast<uint32_t>(ser.getSerializedDataLength());
            return true;
        }

        // 反序列化函数，将 SerializedPayload_t 格式的数据反序列化为 Time 类型对象
        bool TimePubSubType::deserialize(
                SerializedPayload_t* payload,
                void* data)
        {
            // 将传入的 void* 类型数据转换为 Time* 类型指针，方便后续操作
            Time* p_type = static_cast<Time*>(data);

            // 创建一个 FastBuffer 对象，用于管理原始缓冲区（指向 payload->data），其大小为 payload->length（实际数据长度）
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->length);

            // 创建一个 Cdr 对象，用于执行反序列化操作，设置了字节序（默认字节序）和 DDS CDR 相关模式
            eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);

            // 反序列化封装相关的头部信息（比如读取长度等元数据）
            deser.read_encapsulation();
            // 根据反序列化对象的字节序设置 payload 的封装字节序标识（CDR_BE 表示大端序，CDR_LE 表示小端序）
            payload->encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS? CDR_BE : CDR_LE;

            // 调用 Time 对象的 deserialize 方法，从 Cdr 对象中反序列化出具体的 Time 数据
            p_type->deserialize(deser);

            return true;
        }

        // 返回一个函数对象，该函数对象用于获取给定 Time 数据对象序列化后的大小（包含封装部分）
        std::function<uint32_t()> TimePubSubType::getSerializedSizeProvider(
                void* data)
        {
            return [data]() -> uint32_t
                   {
                       // 计算并返回 Time 数据对象的 CDR 序列化大小（包含封装部分，额外加 4 字节）
                       return static_cast<uint32_t>(type::getCdrSerializedSize(*static_cast<Time*>(data))) +
                              4u /*encapsulation*/;
                   };
        }

        // 创建一个 Time 类型的数据对象，并返回其 void* 指针表示（用于后续操作，比如序列化等）
        void* TimePubSubType::createData()
        {
            return reinterpret_cast<void*>(new Time());
        }

        // 删除之前通过 createData 创建的 Time 类型数据对象，释放内存
        void TimePubSubType::deleteData(
                void* data)
        {
            delete(reinterpret_cast<Time*>(data));
        }

        // 获取给定 Time 数据对象的键（Key）信息，用于在 DDS 等场景中标识该对象实例
        bool TimePubSubType::getKey(
                void* data,
                InstanceHandle_t* handle,
                bool force_md5)
        {
            if (!m_isGetKeyDefined)
            {
                return false;
            }

            Time* p_type = static_cast<Time*>(data);

            // 创建一个 FastBuffer 对象，用于管理键缓冲区（指向 m_keyBuffer），其大小为 Time 类型获取键的最大 CDR 序列化大小
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(m_keyBuffer),
                    Time::getKeyMaxCdrSerializedSize());

            // 创建一个 Cdr 对象，用于将 Time 数据对象的键信息序列化到键缓冲区中，设置为大端序
            eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::BIG_ENDIANNESS);
            p_type->serializeKey(ser);
            if (force_md5 || Time::getKeyMaxCdrSerializedSize() > 16)
            {
                // 如果需要强制使用 MD5 或者键序列化大小大于 16 字节，则进行 MD5 计算相关操作
                m_md5.init();
                m_md5.update(m_keyBuffer, static_cast<unsigned int>(ser.getSerializedDataLength()));
                m_md5.finalize();
                for (uint8_t i = 0; i < 16; ++i)
                {
                    handle->value[i] = m_md5.digest[i];
                }
            }
            else
            {
                // 如果不需要 MD5 计算，直接将键缓冲区的前 16 字节内容复制到 InstanceHandle_t 的 value 数组中
                for (uint8_t i = 0; i < 16; ++i)
                {
                    handle->value[i] = m_keyBuffer[i];
                }
            }
            return true;
        }
    }
}
