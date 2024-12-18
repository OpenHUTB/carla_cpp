#include <fastcdr/FastBuffer.h>
#include <fastcdr/Cdr.h>

#include "TransformStampedPubSubTypes.h"

// 使用类型别名，将eprosima::fastrtps::rtps::SerializedPayload_t简化为SerializedPayload_t，方便后续代码书写和阅读
using SerializedPayload_t = eprosima::fastrtps::rtps::SerializedPayload_t;
// 使用类型别名，将eprosima::fastrtps::rtps::InstanceHandle_t简化为InstanceHandle_t
using InstanceHandle_t = eprosima::fastrtps::rtps::InstanceHandle_t;

namespace geometry_msgs {
    namespace msg {
        // TransformStampedPubSubType类的构造函数定义
        TransformStampedPubSubType::TransformStampedPubSubType()
        {
            // 设置该类型的名称，此处设置为"geometry_msgs::msg::dds_::TransformStamped_"，可能用于标识该类型在系统中的唯一性等用途
            setName("geometry_msgs::msg::dds_::TransformStamped_");

            // 获取TransformStamped类型能够达到的最大CDR序列化大小，CDR可能是一种特定的数据序列化格式
            auto type_size = TransformStamped::getMaxCdrSerializedSize();
            // 对获取到的类型大小进行字节对齐操作，以满足可能的子消息对齐要求（按4字节对齐），确保数据存储符合某种规范，便于后续处理
            type_size += eprosima::fastcdr::Cdr::alignment(type_size, 4); /* possible submessage alignment */
            // 计算最终的类型大小，额外加上4字节，可能是用于封装（encapsulation）相关的数据开销，比如添加一些头部等信息来包裹实际序列化的数据
            m_typeSize = static_cast<uint32_t>(type_size) + 4; /*encapsulation*/

            // 判断TransformStamped类型是否定义了获取键（key）的相关操作，将结果赋值给成员变量m_isGetKeyDefined，后续在获取键的操作中可能会依据这个变量来判断是否能执行相应逻辑
            m_isGetKeyDefined = TransformStamped::isKeyDefined();

            // 根据TransformStamped类型获取最大键（key）的CDR序列化大小来确定用于存储键数据的缓冲区大小
            // 如果获取到的最大键序列化大小大于16字节，就使用实际的最大键序列化大小作为缓冲区大小，否则使用16字节作为缓冲区大小
            size_t keyLength = TransformStamped::getKeyMaxCdrSerializedSize() > 16?
                    TransformStamped::getKeyMaxCdrSerializedSize() : 16;
            // 通过malloc动态分配内存来创建用于存储键数据的缓冲区，将分配的内存地址转换为无符号字符指针（unsigned char*）后赋值给m_keyBuffer成员变量
            m_keyBuffer = reinterpret_cast<unsigned char*>(malloc(keyLength));
            // 使用memset将刚分配的缓冲区内存初始化为0，确保缓冲区中的数据初始状态是已知且干净的
            memset(m_keyBuffer, 0, keyLength);
        }

        // TransformStampedPubSubType类的析构函数定义，用于释放构造函数中动态分配的内存资源
        TransformStampedPubSubType::~TransformStampedPubSubType()
        {
            // 判断m_keyBuffer指针是否为空，如果不为空，说明之前有分配内存，需要调用free函数来释放该内存，避免内存泄漏
            if (m_keyBuffer!= nullptr)
            {
                free(m_keyBuffer);
            }
        }

        // 用于将给定的数据对象序列化到SerializedPayload_t结构中的函数
        bool TransformStampedPubSubType::serialize(
                void* data,
                SerializedPayload_t* payload)
        {
            // 将传入的void*类型数据指针转换为TransformStamped*类型指针，方便后续操作，假设传入的数据实际就是TransformStamped类型的对象
            TransformStamped* p_type = static_cast<TransformStamped*>(data);

            // 创建一个FastBuffer对象，用于管理原始缓冲区，它包装了payload->data所指向的内存区域，并且指定了该缓冲区的最大可用大小为payload->max_size
            // 这个FastBuffer对象可能提供了一些方便的缓冲区操作接口，供后续序列化操作使用
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->max_size);
            // 创建一个Cdr对象，用于执行实际的数据序列化操作
            // 传入刚刚创建的FastBuffer对象作为底层数据存储，指定默认字节序（DEFAULT_ENDIAN）以及DDS_CDR模式（可能是与DDS相关的特定CDR序列化模式）
            eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);
            // 根据Cdr对象当前的字节序设置SerializedPayload_t结构中的封装（encapsulation）字段，表示数据是大端序（BIG_ENDIANNESS）还是小端序（LITTLE_ENDIANNESS）
            payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS? CDR_BE : CDR_LE;
            // 调用Cdr对象的serialize_encapsulation函数，执行封装相关的数据序列化操作，可能是写入一些头部等用于标识序列化格式、字节序等信息
            ser.serialize_encapsulation();

            try
            {
                // 调用TransformStamped对象的serialize函数，通过传入的Cdr对象ser，将该对象自身的数据序列化到之前创建的FastBuffer所管理的缓冲区中
                p_type->serialize(ser);
            }
            catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
            {
                // 如果在序列化过程中出现内存不足的异常情况，直接返回false，表示序列化失败
                return false;
            }

            // 获取已经序列化的数据的实际长度，通过Cdr对象的getSerializedDataLength函数获取，并将其赋值给SerializedPayload_t结构中的length字段，用于记录实际序列化后的数据长度
            payload->length = static_cast<uint32_t>(ser.getSerializedDataLength());
            // 如果序列化过程没有出现异常且顺利完成，返回true表示序列化成功
            return true;
        }

        // 用于将SerializedPayload_t结构中的数据反序列化到指定的数据对象中的函数
        bool TransformStampedPubSubType::deserialize(
                SerializedPayload_t* payload,
                void* data)
        {
            try
            {
                // 将传入的void*类型数据指针转换为TransformStamped*类型指针，以便后续按照TransformStamped类型来进行反序列化操作，假设传入的数据接收对象就是该类型
                TransformStamped* p_type = static_cast<TransformStamped*>(data);

                // 创建一个FastBuffer对象，用于管理原始缓冲区，它包装了payload->data所指向的内存区域，并且指定了该缓冲区的可用大小为payload->length（即实际接收到的序列化数据的长度）
                eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->length);
                // 创建一个Cdr对象，用于执行实际的数据反序列化操作
                // 传入刚刚创建的FastBuffer对象作为底层数据存储，指定默认字节序（DEFAULT_ENDIAN）以及DDS_CDR模式（与序列化时相对应的特定模式）
                eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);

                // 调用Cdr对象的read_encapsulation函数，执行反序列化中与封装相关的操作，比如读取并解析之前序列化时写入的头部等标识信息，以获取字节序等相关信息
                deser.read_encapsulation();
                // 根据Cdr对象当前解析出的字节序设置SerializedPayload_t结构中的封装（encapsulation）字段，与序列化时的处理相对应
                payload->encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS? CDR_BE : CDR_LE;

                // 调用TransformStamped对象的deserialize函数，通过传入的Cdr对象deser，将FastBuffer中存储的序列化数据反序列化为该对象自身的数据
                p_type->deserialize(deser);
            }
            catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
            {
                // 如果在反序列化过程中出现内存不足的异常情况，直接返回false，表示反序列化失败
                return false;
            }

            // 如果反序列化过程没有出现异常且顺利完成，返回true表示反序列化成功
            return true;
        }

        // 返回一个函数对象，该函数对象用于获取给定数据对象序列化后的大小（包含封装等开销）
        std::function<uint32_t()> TransformStampedPubSubType::getSerializedSizeProvider(
                void* data)
        {
            return [data]() -> uint32_t
                   {
                       // 先获取TransformStamped类型对象的CDR序列化大小（不包含封装开销），再加上4字节的封装开销，返回最终的序列化后大小
                       return static_cast<uint32_t>(type::getCdrSerializedSize(*static_cast<TransformStamped*>(data))) +
                              4u /*encapsulation*/;
                   };
        }

        // 创建一个新的TransformStamped类型的数据对象，并返回其void*类型的指针，用于后续操作，比如作为要序列化的数据对象等
        void* TransformStampedPubSubType::createData()
        {
            return reinterpret_cast<void*>(new TransformStamped());
        }

        // 用于删除通过createData函数创建的TransformStamped类型的数据对象，释放其占用的内存资源，避免内存泄漏
        void TransformStampedPubSubType::deleteData(
                void* data)
        {
            delete(reinterpret_cast<TransformStamped*>(data));
        }

        // 用于获取给定数据对象对应的键（key）信息，并填充到InstanceHandle_t结构中的函数
        bool TransformStampedPubSubType::getKey(
                void* data,
                InstanceHandle_t* handle,
                bool force_md5)
        {
            // 如果之前判断TransformStamped类型没有定义获取键的操作（m_isGetKeyDefined为false），则直接返回false，表示无法获取键信息
            if (!m_isGetKeyDefined)
            {
                return false;
            }

            // 将传入的void*类型数据指针转换为TransformStamped*类型指针，方便后续按照该类型来获取键相关的操作
            TransformStamped* p_type = static_cast<TransformStamped*>(data);

            // 创建一个FastBuffer对象，用于管理原始缓冲区，它包装了m_keyBuffer所指向的内存区域，并且指定了该缓冲区的大小为TransformStamped类型获取最大键的CDR序列化大小
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(m_keyBuffer),
                    TransformStamped::getKeyMaxCdrSerializedSize());

            // 创建一个Cdr对象，用于执行将键数据序列化到FastBuffer所管理的缓冲区中的操作，这里指定了大端序（BIG_ENDIANNESS）
            eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::BIG_ENDIANNESS);
            // 调用TransformStamped对象的serializeKey函数，通过传入的Cdr对象ser，将该对象的键数据序列化到FastBuffer所管理的缓冲区中
            p_type->serializeKey(ser);

            // 如果force_md5为true（可能是强制使用MD5算法处理键数据的情况）或者TransformStamped类型获取的最大键的CDR序列化大小大于16字节
            if (force_md5 || TransformStamped::getKeyMaxCdrSerializedSize() > 16)
            {
                // 初始化MD5相关的操作对象（可能是用于计算MD5哈希值的类实例）
                m_md5.init();
                // 使用MD5操作对象更新要计算哈希值的数据范围，传入m_keyBuffer指针以及已经序列化的键数据的实际长度
                m_md5.update(m_keyBuffer, static_cast<unsigned int>(ser.getSerializedDataLength()));
                // 完成MD5哈希值的计算
                m_md5.finalize();
                // 将计算得到的MD5哈希值的每一个字节依次赋值给InstanceHandle_t结构中的value数组，用于后续通过键来标识或查找等操作
                for (uint8_t i = 0; i < 16; ++i)
                {
                    handle->value[i] = m_md5.digest[i];
                }
            }
            else
            {
                // 如果不需要使用MD5算法处理键数据且键数据长度小于等于16字节，直接将缓冲区中的键数据逐字节赋值给InstanceHandle_t结构中的value数组
                for (uint8_t i = 0; i < 16; ++i)
                {
                    handle->value[i] = m_keyBuffer[i];
                }
            }
            // 如果键获取及相关处理操作成功完成，返回true表示获取键信息成功
            return true;
        }
    } 
} 
