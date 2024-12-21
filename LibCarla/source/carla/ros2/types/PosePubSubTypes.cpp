/*!
 * @file PosePubSubTypes.cpp
 * 此源文件包含了序列化相关函数的实现。
 * 该文件由工具 fastcdrgen 生成。
 */

// 引入 FastCDR 库中用于处理快速缓冲区相关操作的头文件
#include <fastcdr/FastBuffer.h>
// 引入 FastCDR 库中用于处理 Cdr（Common Data Representation，一种数据表示格式）相关操作的头文件
#include <fastcdr/Cdr.h>

// 引入自定义的 PosePubSubTypes 头文件，可能包含了 PosePubSubType 类的声明以及相关依赖类型（如 Pose 等）的定义
#include "PosePubSubTypes.h"

// 为 eprosima::fastrtps::rtps::SerializedPayload_t 类型定义别名 SerializedPayload_t，方便代码书写和阅读，该类型用于表示序列化后的负载数据
using SerializedPayload_t = eprosima::fastrtps::rtps::SerializedPayload_t;
// 为 eprosima::fastrtps::rtps::InstanceHandle_t 类型定义别名 InstanceHandle_t，该类型通常用于表示实例句柄，在数据处理中可能涉及实例标识等操作
using InstanceHandle_t = eprosima::fastrtps::rtps::InstanceHandle_t;

namespace geometry_msgs {
    namespace msg {

        // PosePubSubType 类的默认构造函数，用于初始化该主题数据类型相关的属性和资源
        PosePubSubType::PosePubSubType()
        {
            // 设置该类型的名称，用于标识，这里明确了其所属的命名空间和具体类型名称
            setName("geometry_msgs::msg::dds_::Pose_");
            // 获取 Pose 类型的最大 CDR 序列化大小
            auto type_size = Pose::getMaxCdrSerializedSize();
            // 考虑可能的子消息对齐（按照 4 字节对齐），更新类型大小
            type_size += eprosima::fastcdr::Cdr::alignment(type_size, 4); /* possible submessage alignment */
            // 计算最终的类型大小，加上 4 字节用于封装相关操作（具体封装用途由相关协议或应用场景决定），并转换为 uint32_t 类型赋值给成员变量 m_typeSize
            m_typeSize = static_cast<uint32_t>(type_size) + 4; /*encapsulation*/
            // 获取 Pose 类型是否定义了键（Key），并赋值给成员变量 m_isGetKeyDefined，用于后续判断是否需要处理键相关操作
            m_isGetKeyDefined = Pose::isKeyDefined();
            // 根据 Pose 类型的键最大 CDR 序列化大小来确定键缓冲区的长度，若大于 16 则取其自身大小，否则取 16，用于后续存储键相关数据
            size_t keyLength = Pose::getKeyMaxCdrSerializedSize() > 16?
                    Pose::getKeyMaxCdrSerializedSize() : 16;
            // 分配键缓冲区内存空间，将分配后的指针赋值给成员变量 m_keyBuffer，用于后续键序列化等操作临时存储数据
            m_keyBuffer = reinterpret_cast<unsigned char*>(malloc(keyLength));
            // 将键缓冲区内存初始化为 0，确保数据的初始状态正确
            memset(m_keyBuffer, 0, keyLength);
        }

        // PosePubSubType 类的析构函数，用于释放构造函数中分配的键缓冲区内存资源，防止内存泄漏
        PosePubSubType::~PosePubSubType()
        {
            if (m_keyBuffer!= nullptr)
            {
                free(m_keyBuffer);
            }
        }

        // 序列化函数，将给定的数据（void* 指针指向的实际为 Pose 类型的数据）转换为适合网络传输等的序列化格式，并存入 SerializedPayload_t 结构体中，返回序列化是否成功
        bool PosePubSubType::serialize(
                void* data,
                SerializedPayload_t* payload)
        {
            // 将传入的 void* 数据指针转换为 Pose* 类型指针，方便后续操作对应的 Pose 类型对象
            Pose* p_type = static_cast<Pose*>(data);

            // 创建一个 FastBuffer 对象，用于管理原始缓冲区，将 payload 中的数据指针转换为 char* 类型作为缓冲区起始地址，同时传入最大缓冲区大小，方便后续数据写入操作
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->max_size);
            // 创建一个 Cdr 对象，用于执行序列化操作，传入 FastBuffer 对象以及相关的字节序（默认为默认字节序，使用 DDS_CDR 相关配置）信息，指定了序列化的具体规则和格式
            eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);
            // 根据序列化对象的字节序设置 payload 的封装格式（大端序或小端序标记），以便接收方能够正确解析数据
            payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS? CDR_BE : CDR_LE;
            // 先序列化封装相关的头部信息（可能包含一些协议相关的标识等），这是按照特定的序列化规则进行的前置操作
            ser.serialize_encapsulation();

            try
            {
                // 调用 Pose 类型对象自身的 serialize 函数，将具体的 Pose 数据序列化到之前创建的 Cdr 对象中，按照其内部定义的序列化逻辑进行数据转换
                p_type->serialize(ser);
            }
            catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
            {
                // 如果在序列化过程中出现内存不足异常，则返回 false，表示序列化失败
                return false;
            }

            // 获取序列化后数据的实际长度，并赋值给 payload 的 length 成员，用于标识有效数据的长度
            payload->length = static_cast<uint32_t>(ser.getSerializedDataLength());
            return true;
        }

        // 反序列化函数，用于将给定的序列化数据（SerializedPayload_t 结构体表示）还原为 Pose 类型的数据（通过 void* 指针指向的内存区域），返回反序列化是否成功
        bool PosePubSubType::deserialize(
                SerializedPayload_t* payload,
                void* data)
        {
            try
            {
                // 将传入的 void* 数据指针转换为 Pose* 类型指针，以便后续操作对应的 Pose 类型对象
                Pose* p_type = static_cast<Pose*>(data);

                // 创建一个 FastBuffer 对象，用于管理原始缓冲区，将 payload 中的数据指针转换为 char* 类型作为缓冲区起始地址，同时传入实际接收到的数据长度，用于限定缓冲区范围
                eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->length);

                // 创建一个 Cdr 对象，用于执行反序列化操作，传入 FastBuffer 对象以及相关的字节序（默认为默认字节序，使用 DDS_CDR 相关配置）信息，指定了反序列化的具体规则和格式
                eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);

                // 先反序列化封装相关的头部信息（如之前序列化时添加的协议标识等），按照特定规则进行解析，获取字节序等信息，并设置 payload 的封装格式
                deser.read_encapsulation();
                payload->encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS? CDR_BE : CDR_LE;

                // 调用 Pose 类型对象自身的 deserialize 函数，从 Cdr 对象中还原出具体的 Pose 数据，按照其内部定义的反序列化逻辑进行数据恢复
                p_type->deserialize(deser);
            }
            catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
            {
                // 如果在反序列化过程中出现内存不足异常，则返回 false，表示反序列化失败
                return false;
            }

            return true;
        }

        // 返回一个函数对象，该函数对象用于获取给定数据（void* 指针指向的 Pose 类型数据）的序列化大小，以方便在一些场景下动态获取数据序列化后的长度信息
        std::function<uint32_t()> PosePubSubType::getSerializedSizeProvider(
                void* data)
        {
            return [data]() -> uint32_t
                   {
                       // 调用 Pose 类型的 getCdrSerializedSize 函数获取数据本身的序列化大小，并加上 4 字节（用于封装相关，与序列化函数中的处理对应），返回最终的序列化大小
                       return static_cast<uint32_t>(type::getCdrSerializedSize(*static_cast<Pose*>(data))) +
                              4u /*encapsulation*/;
                   };
        }

        // 创建用于存储 Pose 类型数据的内存空间，返回指向该内存空间的指针，便于后续向其中填充数据等操作
        void* PosePubSubType::createData()
        {
            return reinterpret_cast<void*>(new Pose());
        }

        // 释放之前通过 createData 函数创建的用于存储数据的内存空间，传入指向该内存空间的指针（void* 数据）进行资源释放操作，这里是删除之前创建的 Pose 类型对象所占用的内存
        void PosePubSubType::deleteData(
                void* data)
        {
            delete(reinterpret_cast<Pose*>(data));
        }

        // 获取给定数据（void* 指针指向的 Pose 类型数据）的键（Key）信息，并存入 InstanceHandle_t 结构体中，可用于在 DDS 系统中进行实例标识、查找等操作，返回获取键信息是否成功
        bool PosePubSubType::getKey(
                void* data,
                InstanceHandle_t* handle,
                bool force_md5)
        {
            // 如果该类型未定义键（通过 m_isGetKeyDefined 判断），则直接返回 false，表示无法获取键信息
            if (!m_isGetKeyDefined)
            {
                return false;
            }

            Pose* p_type = static_cast<Pose*>(data);

            // 创建一个 FastBuffer 对象，用于管理键缓冲区，将 m_keyBuffer 作为起始地址，传入 Pose 类型的键最大 CDR 序列化大小作为缓冲区长度，用于后续键数据的序列化操作临时存储
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(m_keyBuffer),
                    Pose::getKeyMaxCdrSerializedSize());

            // 创建一个 Cdr 对象，用于执行键数据的序列化操作，指定为大端序（可能根据相关要求或默认规则），将 Pose 类型对象的键数据序列化到 FastBuffer 管理的缓冲区中
            eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::BIG_ENDIANNESS);
            p_type->serializeKey(ser);
            if (force_md5 || Pose::getKeyMaxCdrSerializedSize() > 16)
            {
                // 如果需要强制使用 MD5（可能根据配置或应用场景决定）或者键序列化大小大于 16 字节，则进行 MD5 相关操作
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
                // 否则，直接将键缓冲区中的数据复制到 InstanceHandle_t 结构体的 value 数组中，用于传递键信息
                for (uint8_t i = 0; i < 16; ++i)
                {
                    handle->value[i] = m_keyBuffer[i];
                }
            }
            return true;
        }
    } //End of namespace msg
} //End of namespace geometry_msgs
