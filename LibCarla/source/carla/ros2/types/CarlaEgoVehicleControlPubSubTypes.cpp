#include <fastcdr/FastBuffer.h>
#include <fastcdr/Cdr.h>

#include "CarlaEgoVehicleControlPubSubTypes.h"

// 为eprosima::fastrtps::rtps::SerializedPayload_t类型定义别名SerializedPayload_t，方便后续代码使用
// SerializedPayload_t用于表示序列化后的负载数据结构
using SerializedPayload_t = eprosima::fastrtps::rtps::SerializedPayload_t;
// 为eprosima::fastrtps::rtps::InstanceHandle_t类型定义别名InstanceHandle_t，方便后续代码使用
// InstanceHandle_t通常用于标识数据实例在分布式系统中的唯一性等相关操作
using InstanceHandle_t = eprosima::fastrtps::rtps::InstanceHandle_t;

namespace carla_msgs {
    namespace msg {
        // CarlaEgoVehicleControlPubSubType类的构造函数，用于初始化该类型相关的属性和资源
        CarlaEgoVehicleControlPubSubType::CarlaEgoVehicleControlPubSubType()
        {
            // 设置该类型在相关系统（可能是数据发布/订阅系统）中的名称
            setName("carla_msgs::msg::dds_::CarlaEgoVehicleControl_");
            // 获取CarlaEgoVehicleControl类型能达到的最大CDR序列化大小
            auto type_size = CarlaEgoVehicleControl::getMaxCdrSerializedSize();
            // 考虑可能的子消息对齐需求（这里按4字节对齐），对类型大小进行调整
            type_size += eprosima::fastcdr::Cdr::alignment(type_size, 4); /* possible submessage alignment */
            // 计算最终的类型大小，额外加上4字节用于封装相关的数据结构等（具体取决于协议或实现要求）
            m_typeSize = static_cast<uint32_t>(type_size) + 4; /*encapsulation*/
            // 判断CarlaEgoVehicleControl类型是否定义了获取键（用于唯一标识等用途）的操作
            m_isGetKeyDefined = CarlaEgoVehicleControl::isKeyDefined();
            // 根据CarlaEgoVehicleControl类型获取键的最大CDR序列化大小来确定键缓冲区的长度
            // 如果获取键的最大序列化大小大于16字节，则使用该实际大小，否则使用16字节作为长度
            size_t keyLength = CarlaEgoVehicleControl::getKeyMaxCdrSerializedSize() > 16?
                    CarlaEgoVehicleControl::getKeyMaxCdrSerializedSize() : 16;
            // 分配用于存储键数据的缓冲区内存，其大小由前面计算的keyLength决定
            m_keyBuffer = reinterpret_cast<unsigned char*>(malloc(keyLength));
            // 将刚分配的键缓冲区内存初始化为0，确保初始状态下数据的一致性
            memset(m_keyBuffer, 0, keyLength);
        }

        // CarlaEgoVehicleControlPubSubType类的析构函数，用于释放构造函数中申请的相关资源
        CarlaEgoVehicleControlPubSubType::~CarlaEgoVehicleControlPubSubType()
        {
            // 检查键缓冲区是否为空，如果不为空，则释放其占用的内存空间
            if (m_keyBuffer!= nullptr)
            {
                free(m_keyBuffer);
            }
        }

        // 序列化函数，将给定的数据对象序列化为可以传输或存储的格式（这里是符合相关协议的格式），存储在SerializedPayload_t结构中
        bool CarlaEgoVehicleControlPubSubType::serialize(
                void* data,
                SerializedPayload_t* payload)
        {
            // 将传入的void*类型数据转换为CarlaEgoVehicleControl*类型的指针，以便后续操作
            CarlaEgoVehicleControl* p_type = static_cast<CarlaEgoVehicleControl*>(data);

            // 创建一个FastBuffer对象，用于管理原始的字节缓冲区，它关联了payload中的数据指针和最大可用空间
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->max_size);
            // 创建一个Cdr对象，用于进行数据的序列化操作，指定了缓冲区、默认字节序以及相关的DDS CDR模式
            eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);
            // 根据Cdr对象当前的字节序（大端或小端）设置payload的封装字节序标识，以便后续解析能正确识别
            payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS? CDR_BE : CDR_LE;
            // 序列化封装相关的信息（具体内容取决于协议实现，可能是一些头部等标识信息）
            ser.serialize_encapsulation();

            try
            {
                // 调用CarlaEgoVehicleControl对象自身的序列化方法，将实际的数据内容序列化到Cdr对象管理的缓冲区中
                p_type->serialize(ser);
            }
            catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
            {
                // 如果在序列化过程中出现内存不足的异常情况，返回false表示序列化失败
                return false;
            }

            // 获取序列化后数据的实际长度，并赋值给payload结构中的length成员，以便后续使用知道有效数据的范围
            payload->length = static_cast<uint32_t>(ser.getSerializedDataLength());
            // 序列化成功，返回true
            return true;
        }

        // 反序列化函数，将给定的SerializedPayload_t结构中的数据解析还原为对应的对象数据结构
        bool CarlaEgoVehicleControlPubSubType::deserialize(
                SerializedPayload_t* payload,
                void* data)
        {
            try
            {
                // 将传入的void*类型数据转换为CarlaEgoVehicleControl*类型的指针，以便后续操作
                CarlaEgoVehicleControl* p_type = static_cast<CarlaEgoVehicleControl*>(data);

                // 创建一个FastBuffer对象，用于管理原始的字节缓冲区，关联了payload中的数据指针和实际有效数据长度
                eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->length);

                // 创建一个Cdr对象，用于进行数据的反序列化操作，指定了缓冲区、默认字节序以及相关的DDS CDR模式
                eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);

                // 反序列化封装相关的信息，以便后续能正确解析实际的数据内容
                deser.read_encapsulation();
                // 根据Cdr对象当前的字节序（大端或小端）设置payload的封装字节序标识，保持与序列化时一致
                payload->encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS? CDR_BE : CDR_LE;

                // 调用CarlaEgoVehicleControl对象自身的反序列化方法，从Cdr对象管理的缓冲区中解析还原数据到对象中
                p_type->deserialize(deser);
            }
            catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
            {
                // 如果在反序列化过程中出现内存不足的异常情况，返回false表示反序列化失败
                return false;
            }

            // 反序列化成功，返回true
            return true;
        }

        // 返回一个函数对象（lambda表达式），该函数对象用于获取给定数据对象序列化后的大小（包含封装等额外部分）
        std::function<uint32_t()> CarlaEgoVehicleControlPubSubType::getSerializedSizeProvider(
                void* data)
        {
            return [data]() -> uint32_t
                   {
                       // 先获取CarlaEgoVehicleControl对象自身序列化后的大小，再加上4字节的封装部分大小，返回最终的序列化大小
                       return static_cast<uint32_t>(type::getCdrSerializedSize(*static_cast<CarlaEgoVehicleControl*>(data))) +
                              4u /*encapsulation*/;
                   };
        }

        // 创建一个CarlaEgoVehicleControl类型的数据对象，并返回其void*类型的指针，用于后续操作（比如序列化等）
        void* CarlaEgoVehicleControlPubSubType::createData()
        {
            return reinterpret_cast<void*>(new CarlaEgoVehicleControl());
        }

        // 释放给定的void*类型指针所指向的CarlaEgoVehicleControl类型的数据对象内存，进行资源清理
        void CarlaEgoVehicleControlPubSubType::deleteData(
                void* data)
        {
            delete(reinterpret_cast<CarlaEgoVehicleControl*>(data));
        }

        // 获取给定数据对象的键（用于唯一标识等目的），并将其填充到InstanceHandle_t结构中
        bool CarlaEgoVehicleControlPubSubType::getKey(
                void* data,
                InstanceHandle_t* handle,
                bool force_md5)
        {
            // 如果该类型没有定义获取键的操作（在构造函数中判断），则直接返回false
            if (!m_isGetKeyDefined)
            {
                return false;
            }

            CarlaEgoVehicleControl* p_type = static_cast<CarlaEgoVehicleControl*>(data);

            // 创建一个FastBuffer对象，用于管理原始的字节缓冲区，关联了用于存储键数据的m_keyBuffer和键数据的最大序列化大小
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(m_keyBuffer),
                    CarlaEgoVehicleControl::getKeyMaxCdrSerializedSize());

            // 创建一个Cdr对象，用于进行键数据的序列化操作，这里指定为大端字节序（具体要求可能取决于系统设计）
            eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::BIG_ENDIANNESS);
            // 调用CarlaEgoVehicleControl对象自身的序列化键的方法，将键数据序列化到Cdr对象管理的缓冲区中
            p_type->serializeKey(ser);
            if (force_md5 || CarlaEgoVehicleControl::getKeyMaxCdrSerializedSize() > 16)
            {
                // 如果要求强制使用MD5（可能用于更可靠的唯一标识等情况）或者键的最大序列化大小大于16字节
                m_md5.init();
                // 使用MD5对象更新要计算哈希的数据范围（从m_keyBuffer开始，长度为序列化后的键数据长度）
                m_md5.update(m_keyBuffer, static_cast<unsigned int>(ser.getSerializedDataLength()));
                // 完成MD5计算，生成最终的哈希值
                m_md5.finalize();
                // 将生成的MD5哈希值的每个字节依次填充到InstanceHandle_t结构的value数组中（通常用于唯一标识该数据对象）
                for (uint8_t i = 0; i < 16; ++i)
                {
                    handle->value[i] = m_md5.digest[i];
                }
            }
            else
            {
                // 如果不需要MD5或者键的最大序列化大小不超过16字节，直接将缓冲区中的键数据复制到InstanceHandle_t结构的value数组中
                for (uint8_t i = 0; i < 16; ++i)
                {
                    handle->value[i] = m_keyBuffer[i];
                }
            }
            return true;
        }
    } 
} 
