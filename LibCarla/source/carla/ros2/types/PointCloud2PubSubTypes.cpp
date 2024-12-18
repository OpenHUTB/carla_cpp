#include <fastcdr/FastBuffer.h>
#include <fastcdr/Cdr.h>

#include "PointCloud2PubSubTypes.h"

using SerializedPayload_t = eprosima::fastrtps::rtps::SerializedPayload_t;
using InstanceHandle_t = eprosima::fastrtps::rtps::InstanceHandle_t;

// 定义在sensor_msgs命名空间下的msg命名空间
namespace sensor_msgs {
    namespace msg {
        // PointCloud2PubSubType类的构造函数
        PointCloud2PubSubType::PointCloud2PubSubType()
        {
            // 设置类型名称，用于标识该类型
            setName("sensor_msgs::msg::dds_::PointCloud2_");
            // 获取PointCloud2类型的最大CDR序列化大小
            auto type_size = PointCloud2::getMaxCdrSerializedSize();
            // 根据对齐要求对类型大小进行调整（可能是为了满足子消息对齐需求）
            type_size += eprosima::fastcdr::Cdr::alignment(type_size, 4); /* possible submessage alignment */
            // 计算最终的类型大小，加上4可能是用于封装相关的处理
            m_typeSize = static_cast<uint32_t>(type_size) + 4; /*encapsulation*/
            // 判断PointCloud2类型是否定义了键（用于唯一标识等用途）
            m_isGetKeyDefined = PointCloud2::isKeyDefined();
            // 根据PointCloud2的键最大CDR序列化大小来确定用于存储键的缓冲区长度
            size_t keyLength = PointCloud2::getKeyMaxCdrSerializedSize() > 16?
                    PointCloud2::getKeyMaxCdrSerializedSize() : 16;
            // 分配用于存储键的缓冲区内存
            m_keyBuffer = reinterpret_cast<unsigned char*>(malloc(keyLength));
            // 初始化键缓冲区内存为0
            memset(m_keyBuffer, 0, keyLength);
        }

        // PointCloud2PubSubType类的析构函数，用于释放之前分配的键缓冲区内存
        PointCloud2PubSubType::~PointCloud2PubSubType()
        {
            if (m_keyBuffer!= nullptr)
            {
                free(m_keyBuffer);
            }
        }

        // 序列化函数，将PointCloud2类型的数据对象序列化为字节流并存入SerializedPayload_t结构中
        bool PointCloud2PubSubType::serialize(
                void* data,
                SerializedPayload_t* payload)
        {
            // 将传入的void*指针转换为PointCloud2*类型的指针，指向实际要序列化的数据对象
            PointCloud2* p_type = static_cast<PointCloud2*>(data);

            // 创建FastBuffer对象，用于管理存放序列化数据的原始缓冲区，关联到SerializedPayload_t中的数据缓冲区
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->max_size);
            // 创建Cdr对象，用于执行序列化操作，指定了缓冲区、字节序和CDR相关模式
            eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);
            // 根据序列化时的字节序设置SerializedPayload_t中的封装字节序标识
            payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS? CDR_BE : CDR_LE;
            // 序列化封装相关的信息（可能是头部等固定格式的部分）
            ser.serialize_encapsulation();

            try
            {
                // 调用PointCloud2对象自身的序列化函数，将其实际数据序列化到Cdr对象管理的缓冲区中
                p_type->serialize(ser);
            }
            catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
            {
                // 如果内存不足导致序列化失败，返回false
                return false;
            }

            // 获取实际序列化后的数据长度，并存入SerializedPayload_t结构中
            payload->length = static_cast<uint32_t>(ser.getSerializedDataLength());
            return true;
        }

        // 反序列化函数，从SerializedPayload_t结构中的字节流恢复出PointCloud2类型的数据对象
        bool PointCloud2PubSubType::deserialize(
                SerializedPayload_t* payload,
                void* data)
        {
            try
            {
                // 将传入的void*指针转换为PointCloud2*类型的指针，指向要填充反序列化数据的对象
                PointCloud2* p_type = static_cast<PointCloud2*>(data);

                // 创建FastBuffer对象，用于管理存放反序列化数据的原始缓冲区，关联到SerializedPayload_t中的数据缓冲区，长度为实际收到的有效数据长度
                eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->length);

                // 创建Cdr对象，用于执行反序列化操作，指定了缓冲区、字节序和CDR相关模式
                eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);

                // 反序列化封装相关的信息（与序列化时的封装对应）
                deser.read_encapsulation();
                // 根据反序列化时的字节序设置SerializedPayload_t中的封装字节序标识
                payload->encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS? CDR_BE : CDR_LE;

                // 调用PointCloud2对象自身的反序列化函数，从Cdr对象管理的缓冲区中恢复数据到对象中
                p_type->deserialize(deser);
            }
            catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
            {
                // 如果内存不足导致反序列化失败，返回false
                return false;
            }

            return true;
        }

        // 返回一个函数对象，该函数对象用于获取给定PointCloud2数据对象序列化后的大小（包含封装等额外部分）
        std::function<uint32_t()> PointCloud2PubSubType::getSerializedSizeProvider(
                void* data)
        {
            return [data]() -> uint32_t
                   {
                       // 计算并返回序列化后的大小，包含封装部分（额外的4字节）
                       return static_cast<uint32_t>(type::getCdrSerializedSize(*static_cast<PointCloud2*>(data))) +
                              4u /*encapsulation*/;
                   };
        }

        // 创建一个新的PointCloud2类型的数据对象，并返回其void*指针，用于后续操作（如填充数据后序列化等）
        void* PointCloud2PubSubType::createData()
        {
            return reinterpret_cast<void*>(new PointCloud2());
        }

        // 删除之前通过createData创建的PointCloud2类型的数据对象，释放内存
        void PointCloud2PubSubType::deleteData(
                void* data)
        {
            delete(reinterpret_cast<PointCloud2*>(data));
        }

        // 获取给定PointCloud2数据对象的键，用于唯一标识等用途，将键信息填充到InstanceHandle_t结构中
        bool PointCloud2PubSubType::getKey(
                void* data,
                InstanceHandle_t* handle,
                bool force_md5)
        {
            if (!m_isGetKeyDefined)
            {
                return false;
            }

            PointCloud2* p_type = static_cast<PointCloud2*>(data);

            // 创建FastBuffer对象，用于管理存放序列化键数据的缓冲区，关联到之前分配的m_keyBuffer
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(m_keyBuffer),
                    PointCloud2::getKeyMaxCdrSerializedSize());

            // 创建Cdr对象，用于执行键数据的序列化操作，指定了字节序（这里是大端序）
            eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::BIG_ENDIANNESS);
            // 调用PointCloud2对象自身的键序列化函数，将键数据序列化到Cdr对象管理的缓冲区中
            p_type->serializeKey(ser);
            if (force_md5 || PointCloud2::getKeyMaxCdrSerializedSize() > 16)
            {
                // 如果需要使用MD5或者键序列化大小超过16字节，则进行MD5计算相关操作
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
                // 如果不需要MD5且键序列化大小不超过16字节，则直接将缓冲区中的数据复制到InstanceHandle_t结构中作为键
                for (uint8_t i = 0; i < 16; ++i)
                {
                    handle->value[i] = m_keyBuffer[i];
                }
            }
            return true;
        }
    }
}
