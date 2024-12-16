#include <fastcdr/FastBuffer.h>
#include <fastcdr/Cdr.h>

#include "TwistWithCovariancePubSubTypes.h"
// 定义SerializedPayload_t类型别名，它来自 eprosima::fastrtps::rtps 命名空间，用于表示序列化后的负载数据
using SerializedPayload_t = eprosima::fastrtps::rtps::SerializedPayload_t;
// 定义InstanceHandle_t类型别名，同样来自 eprosima::fastrtps::rtps 命名空间，用于表示实例句柄
using InstanceHandle_t = eprosima::fastrtps::rtps::InstanceHandle_t;

namespace geometry_msgs {
    namespace msg {
        // TwistWithCovariancePubSubType类的构造函数
        TwistWithCovariancePubSubType::TwistWithCovariancePubSubType()
        {// 设置类型名称，用于标识该消息类型
            setName("geometry_msgs::msg::dds_::TwistWithCovariance_");
            // 获取TwistWithCovariance类型的最大CDR序列化大小
            auto type_size = TwistWithCovariance::getMaxCdrSerializedSize();
            // 考虑可能的子消息对齐，进行字节对齐操作
            type_size += eprosima::fastcdr::Cdr::alignment(type_size, 4);
            // 计算包含封装头部后的总类型大小（加上4字节封装相关部分），并转换为uint32_t类型存储
            m_typeSize = static_cast<uint32_t>(type_size) + 4; 
            // 判断TwistWithCovariance类型是否定义了获取键（Key）的操作
            m_isGetKeyDefined = TwistWithCovariance::isKeyDefined();
            // 获取TwistWithCovariance类型的键（Key）的最大CDR序列化大小
            size_t keyLength = TwistWithCovariance::getKeyMaxCdrSerializedSize() > 16 ?
                    TwistWithCovariance::getKeyMaxCdrSerializedSize() : 16;
            // 分配用于存储键（Key）数据的缓冲区内存空间
            m_keyBuffer = reinterpret_cast<unsigned char*>(malloc(keyLength));
            // 初始化键（Key）缓冲区内存空间为0
            memset(m_keyBuffer, 0, keyLength);
        }
        // TwistWithCovariancePubSubType类的析构函数，用于释放之前分配的键（Key）缓冲区内存
        TwistWithCovariancePubSubType::~TwistWithCovariancePubSubType()
        {
            if (m_keyBuffer != nullptr)
            {
                free(m_keyBuffer);
            }
        }
        // 序列化函数，将给定的数据对象（TwistWithCovariance类型）序列化为SerializedPayload_t格式
        bool TwistWithCovariancePubSubType::serialize(
                void* data,
                SerializedPayload_t* payload)
        {// 将传入的void*类型数据转换为TwistWithCovariance*类型指针，方便后续操作
            TwistWithCovariance* p_type = static_cast<TwistWithCovariance*>(data);

            // 创建一个FastBuffer对象，用于管理原始的字节缓冲区
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->max_size);
            // 创建一个Cdr对象，用于执行实际的序列化操作，指定了字节序、CDR版本等相关配置
            eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);
            payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
            // 首先序列化封装头部信息（可能包含一些协议相关的标识等）
            ser.serialize_encapsulation();

            try
            {
                // 调用TwistWithCovariance对象的serialize函数，将其具体数据序列化到Cdr对象管理的缓冲区中
                p_type->serialize(ser);
            }
            catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
            {// 如果内存不足抛出异常，则返回false表示序列化失败
                return false;
            }

            // 获取序列化后数据的实际长度，并设置到payload的length成员中
            payload->length = static_cast<uint32_t>(ser.getSerializedDataLength());
            return true;
        }
        // 反序列化函数，将SerializedPayload_t格式的数据反序列化为TwistWithCovariance类型的对象
        bool TwistWithCovariancePubSubType::deserialize(
                SerializedPayload_t* payload,
                void* data)
        {
            try
            {
                // 将传入的void*类型数据转换为TwistWithCovariance*类型指针，方便后续操作
                TwistWithCovariance* p_type = static_cast<TwistWithCovariance*>(data);

                // 创建一个FastBuffer对象，用于管理原始的字节缓冲区
                eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->length);

                // 创建一个Cdr对象，用于执行实际的反序列化操作，指定了字节序、CDR版本等相关配置
                eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);

                // 首先反序列化封装头部信息，获取相关的字节序等配置信息，并设置到payload的encapsulation成员中
                deser.read_encapsulation();
                payload->encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

                // 调用TwistWithCovariance对象的deserialize函数，从Cdr对象管理的缓冲区中反序列化出具体数据
                p_type->deserialize(deser);
            }
            catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
            {// 如果内存不足抛出异常，则返回false表示反序列化失败
                return false;
            }
            // 反序列化成功，返回true
            return true;
        }
        // 返回一个函数对象，该函数对象用于获取给定数据对象序列化后的大小
        std::function<uint32_t()> TwistWithCovariancePubSubType::getSerializedSizeProvider(
                void* data)
        {
            return [data]() -> uint32_t
                   {// 计算并返回TwistWithCovariance类型对象序列化后的大小
                       return static_cast<uint32_t>(type::getCdrSerializedSize(*static_cast<TwistWithCovariance*>(data))) +
                              4u /*encapsulation*/;
                   };
        }
        // 创建一个新的TwistWithCovariance类型对象，并返回其void*指针，用于后续的操作
        void* TwistWithCovariancePubSubType::createData()
        {
            return reinterpret_cast<void*>(new TwistWithCovariance());
        }
        // 删除给定的TwistWithCovariance类型对象，释放其占用的内存空间
        void TwistWithCovariancePubSubType::deleteData(
                void* data)
        {
            delete(reinterpret_cast<TwistWithCovariance*>(data));
        }
        // 获取给定数据对象（TwistWithCovariance类型）的键（Key）信息，并填充到InstanceHandle_t结构体中
        bool TwistWithCovariancePubSubType::getKey(
                void* data,
                InstanceHandle_t* handle,
                bool force_md5)
        {
            if (!m_isGetKeyDefined)
            {// 如果TwistWithCovariance类型没有定义获取键（Key）的操作，则直接返回false
                return false;
            }

            TwistWithCovariance* p_type = static_cast<TwistWithCovariance*>(data);

            // 创建一个FastBuffer对象，用于管理存储键（Key）数据的缓冲区
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(m_keyBuffer),
                    TwistWithCovariance::getKeyMaxCdrSerializedSize());

            // 创建一个Cdr对象，用于将键（Key）数据序列化到缓冲区中，指定为大端序
            eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::BIG_ENDIANNESS);
            p_type->serializeKey(ser);
            if (force_md5 || TwistWithCovariance::getKeyMaxCdrSerializedSize() > 16)
            {// 如果强制使用MD5计算或者键（Key）的最大CDR序列化大小大于16字节
                m_md5.init();
                // 使用MD5算法对象更新要计算的数据（键（Key）缓冲区中的数据，长度为已序列化的键
                m_md5.update(m_keyBuffer, static_cast<unsigned int>(ser.getSerializedDataLength()));
                // 完成MD5计算
                m_md5.finalize();
                for (uint8_t i = 0; i < 16; ++i)
                {// 将MD5计算结果的每个字节填充到InstanceHandle_t结构体的value数组中
                    handle->value[i] = m_md5.digest[i];
                }
            }
            else
            {
                for (uint8_t i = 0; i < 16; ++i)
                {// 如果键（Key）大小不超过16字节，直接将键（Key）缓冲区中的数据复制到InstanceHandle_t结构体的value数组中
                    handle->value[i] = m_keyBuffer[i];
                }
            }
            return true;
        }
    }
}
