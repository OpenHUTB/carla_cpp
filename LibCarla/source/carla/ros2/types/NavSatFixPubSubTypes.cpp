#include <fastcdr/FastBuffer.h>
#include <fastcdr/Cdr.h>

#include "NavSatFixPubSubTypes.h"

// 定义SerializedPayload_t类型别名，等同于eprosima::fastrtps::rtps::SerializedPayload_t类型，
// 用于表示序列化后的负载数据结构，方便后续代码中使用
using SerializedPayload_t = eprosima::fastrtps::rtps::SerializedPayload_t;
// 定义InstanceHandle_t类型别名，等同于eprosima::fastrtps::rtps::InstanceHandle_t类型，
// 通常用于标识实例相关的句柄，在一些发布/订阅等操作中会用到
using InstanceHandle_t = eprosima::fastrtps::rtps::InstanceHandle_t;

namespace sensor_msgs {
    namespace msg {
        // NavSatFixPubSubType类的构造函数定义
        NavSatFixPubSubType::NavSatFixPubSubType()
        {
            // 设置该类型的名称，用于在相关的发布/订阅系统等环境中标识这个类型，此处具体名称为 "sensor_msgs::msg::dds_::NavSatFix_"
            setName("sensor_msgs::msg::dds_::NavSatFix_");
            // 获取NavSatFix类型的最大CDR序列化大小，CDR（Common Data Representation）是一种数据序列化格式
            auto type_size = NavSatFix::getMaxCdrSerializedSize();
            // 考虑可能的子消息对齐情况，对类型大小进行调整，使其按4字节对齐（可能是为了满足某些内存对齐要求等）
            type_size += eprosima::fastcdr::Cdr::alignment(type_size, 4); /* possible submessage alignment */
            // 计算最终的类型大小，加上4字节用于封装（可能是添加了一些额外的头部等封装信息用于传输等用途），并转换为uint32_t类型后赋值给m_typeSize成员变量
            m_typeSize = static_cast<uint32_t>(type_size) + 4; /*encapsulation*/
            // 判断NavSatFix类型是否定义了获取键（Key）的操作，将结果赋值给m_isGetKeyDefined成员变量，用于后续相关操作的判断依据
            m_isGetKeyDefined = NavSatFix::isKeyDefined();
            // 根据NavSatFix类型获取最大CDR序列化键大小与16比较，取较大值作为键缓冲区（m_keyBuffer）的长度，
            // 这可能是为了保证足够空间存储键相关的数据，或者满足某些固定大小的要求（比如后续和16字节相关的操作）
            size_t keyLength = NavSatFix::getKeyMaxCdrSerializedSize() > 16?
                    NavSatFix::getKeyMaxCdrSerializedSize() : 16;
            // 分配内存空间给m_keyBuffer，用于存储键相关的数据，将返回的无符号字符指针进行类型转换后赋值给m_keyBuffer成员变量
            m_keyBuffer = reinterpret_cast<unsigned char*>(malloc(keyLength));
            // 将刚分配的键缓冲区内存初始化为0，确保其中的数据初始状态是已知的、干净的
            memset(m_keyBuffer, 0, keyLength);
        }

        // NavSatFixPubSubType类的析构函数定义，用于释放资源
        NavSatFixPubSubType::~NavSatFixPubSubType()
        {
            // 判断键缓冲区指针是否为空（即是否成功分配过内存），如果不为空则释放对应的内存空间
            if (m_keyBuffer!= nullptr)
            {
                free(m_keyBuffer);
            }
        }

        // 用于将给定的数据对象进行序列化，并填充到SerializedPayload_t结构中
        bool NavSatFixPubSubType::serialize(
                void* data,
                SerializedPayload_t* payload)
        {
            // 将传入的void*类型数据转换为NavSatFix*类型指针，以便后续操作对应的NavSatFix对象
            NavSatFix* p_type = static_cast<NavSatFix*>(data);

            // 创建一个FastBuffer对象，用于管理原始的缓冲区，它包装了SerializedPayload_t结构中的数据指针和最大可用空间，
            // 后续的数据序列化操作将基于这个缓冲区进行
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->max_size);
            // 创建一个Cdr对象，用于实际的数据序列化操作，指定了缓冲区、默认字节序（DEFAULT_ENDIAN）以及采用的CDR模式（DDS_CDR），
            // 字节序相关设置用于确保数据在不同平台间序列化和反序列化的一致性
            eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);
            // 根据Cdr对象当前的字节序设置，确定封装的字节序标识（CDR_BE表示大端序，CDR_LE表示小端序），并赋值给SerializedPayload_t结构中的encapsulation成员
            payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS? CDR_BE : CDR_LE;
            // 对数据进行封装序列化，可能是添加一些头部等封装信息，具体由Cdr类的实现决定
            ser.serialize_encapsulation();

            try
            {
                // 调用NavSatFix对象的serialize方法，将对象的数据通过Cdr对象序列化到之前创建的缓冲区中，
                // 如果序列化过程中出现内存不足等异常情况，会抛出相应的异常
                p_type->serialize(ser);
            }
            catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
            {
                // 如果捕获到内存不足异常，返回false表示序列化失败
                return false;
            }

            // 获取序列化后的数据实际长度，并赋值给SerializedPayload_t结构中的length成员，然后返回true表示序列化成功
            payload->length = static_cast<uint32_t>(ser.getSerializedDataLength());
            return true;
        }

        // 用于将SerializedPayload_t结构中的序列化数据反序列化到对应的对象中
        bool NavSatFixPubSubType::deserialize(
                SerializedPayload_t* payload,
                void* data)
        {
            try
            {
                // 将传入的void*类型数据转换为NavSatFix*类型指针，以便后续操作对应的NavSatFix对象
                NavSatFix* p_type = static_cast<NavSatFix*>(data);

                // 创建一个FastBuffer对象，用于管理原始的缓冲区，它包装了SerializedPayload_t结构中的数据指针和实际数据长度，
                // 后续的数据反序列化操作将基于这个缓冲区进行
                eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->length);
                // 创建一个Cdr对象，用于实际的数据反序列化操作，指定了缓冲区、默认字节序（DEFAULT_ENDIAN）以及采用的CDR模式（DDS_CDR）
                eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);
                // 先对封装信息进行反序列化读取，例如可能会读取之前序列化时添加的头部等封装相关信息
                deser.read_encapsulation();
                // 根据Cdr对象当前的字节序设置，确定封装的字节序标识（CDR_BE表示大端序，CDR_LE表示小端序），并赋值给SerializedPayload_t结构中的encapsulation成员
                payload->encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS? CDR_BE : CDR_LE;

                // 调用NavSatFix对象的deserialize方法，将缓冲区中的数据通过Cdr对象反序列化到对应的对象中，
                // 如果反序列化过程中出现内存不足等异常情况，会抛出相应的异常
                p_type->deserialize(deser);
            }
            catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
            {
                // 如果捕获到内存不足异常，返回false表示反序列化失败
                return false;
            }

            // 如果反序列化过程没有出现异常，返回true表示反序列化成功
            return true;
        }

        // 返回一个函数对象，该函数对象用于获取给定数据对象序列化后的大小（包含封装等额外部分）
        std::function<uint32_t()> NavSatFixPubSubType::getSerializedSizeProvider(
                void* data)
        {
            return [data]() -> uint32_t
                   {
                       // 先获取NavSatFix对象本身的CDR序列化大小，然后加上4字节的封装部分，返回最终的序列化大小
                       return static_cast<uint32_t>(type::getCdrSerializedSize(*static_cast<NavSatFix*>(data))) +
                              4u /*encapsulation*/;
                   };
        }

        // 创建一个新的NavSatFix对象，并返回其void*类型的指针，用于在需要创建对应数据对象的场景中（比如发布消息前创建新消息对象等）
        void* NavSatFixPubSubType::createData()
        {
            return reinterpret_cast<void*>(new NavSatFix());
        }

        // 用于删除之前通过createData等方式创建的NavSatFix对象，释放其占用的内存资源
        void NavSatFixPubSubType::deleteData(
                void* data)
        {
            delete(reinterpret_cast<NavSatFix*>(data));
        }

        // 用于获取给定数据对象对应的键（Key），并填充到InstanceHandle_t结构中，可用于标识、查找等用途
        bool NavSatFixPubSubType::getKey(
                void* data,
                InstanceHandle_t* handle,
                bool force_md5)
        {
            // 如果之前判断NavSatFix类型没有定义获取键的操作，直接返回false，表示无法获取键
            if (!m_isGetKeyDefined)
            {
                return false;
            }

            NavSatFix* p_type = static_cast<NavSatFix*>(data);

            // 创建一个FastBuffer对象，用于管理键相关的原始缓冲区，它包装了之前分配的m_keyBuffer和NavSatFix类型最大键序列化大小，
            // 后续的键序列化操作将基于这个缓冲区进行
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(m_keyBuffer),
                    NavSatFix::getKeyMaxCdrSerializedSize());

            // 创建一个Cdr对象，用于实际的键序列化操作，指定了缓冲区和大端序（BIG_ENDIANNESS），可能键序列化有特定的字节序要求
            eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::BIG_ENDIANNESS);
            // 调用NavSatFix对象的serializeKey方法，将对象的键相关数据通过Cdr对象序列化到键缓冲区中
            p_type->serializeKey(ser);
            if (force_md5 || NavSatFix::getKeyMaxCdrSerializedSize() > 16)
            {
                // 如果强制使用MD5或者键序列化大小大于16字节，进行以下MD5相关操作
                m_md5.init();
                // 使用MD5对象更新数据，传入键缓冲区指针和序列化后的数据长度，计算MD5哈希值
                m_md5.update(m_keyBuffer, static_cast<unsigned int>(ser.getSerializedDataLength()));
                m_md5.finalize();
                // 将计算得到的MD5哈希值的前16个字节复制到InstanceHandle_t结构的value数组中，用于后续的标识等用途
                for (uint8_t i = 0; i < 16; ++i)
                {
                    handle->value[i] = m_md5.digest[i];
                }
            }
            else
            {
                // 如果不满足上述条件，直接将键缓冲区的前16个字节复制到InstanceHandle_t结构的value数组中
                for (uint8_t i = 0; i < 16; ++i)
                {
                    handle->value[i] = m_keyBuffer[i];
                }
            }
            return true;
        }
    } 
} 
