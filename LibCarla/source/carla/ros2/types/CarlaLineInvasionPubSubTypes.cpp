#include <fastcdr/FastBuffer.h>
#include <fastcdr/Cdr.h>

#include "CarlaLineInvasionPubSubTypes.h"

// 为eprosima::fastrtps::rtps::SerializedPayload_t类型定义别名SerializedPayload_t，方便后续代码使用，
// SerializedPayload_t通常用于表示序列化后的负载数据结构，包含了实际要传输的数据以及相关的元信息等
using SerializedPayload_t = eprosima::fastrtps::rtps::SerializedPayload_t;
// 为eprosima::fastrtps::rtps::InstanceHandle_t类型定义别名InstanceHandle_t，
// InstanceHandle_t一般用于在发布/订阅系统中标识一个实例，例如区分不同的主题实例等
using InstanceHandle_t = eprosima::fastrtps::rtps::InstanceHandle_t;

namespace carla_msgs {
    namespace msg {
        // LaneInvasionEventPubSubType类的构造函数，用于初始化与LaneInvasionEvent消息类型相关的发布/订阅类型的各种属性
        LaneInvasionEventPubSubType::LaneInvasionEventPubSubType()
        {
            // 设置此消息类型的名称，该名称遵循特定的命名规范，方便在整个系统中唯一标识这个消息类型
            setName("carla_msgs::msg::dds_::LaneInvasionEvent_");

            // 获取LaneInvasionEvent消息类型所能达到的最大CDR（Common Data Representation，一种数据序列化格式）序列化大小
            auto type_size = LaneInvasionEvent::getMaxCdrSerializedSize();
            // 考虑可能的子消息对齐情况，按照4字节对齐方式调整类型大小。这样做可以确保数据在内存中的布局符合某些底层传输或存储的要求，
            // 例如在一些网络传输或者内存对齐优化场景下，对齐的数据访问效率更高
            type_size += eprosima::fastcdr::Cdr::alignment(type_size, 4); /* possible submessage alignment */
            // 计算最终的类型大小，额外加上4字节用于封装相关信息（具体的封装逻辑可能在序列化等相关操作中体现），
            // 并将其转换为uint32_t类型后赋值给m_typeSize成员变量，m_typeSize用于记录这个消息类型序列化后的整体大小估计值
            m_typeSize = static_cast<uint32_t>(type_size) + 4; /*encapsulation*/

            // 检查LaneInvasionEvent消息类型是否定义了获取键（Key）的相关操作，将结果赋值给m_isGetKeyDefined成员变量，
            // 后续在获取键相关操作时会依据这个变量判断是否可以执行相应逻辑
            m_isGetKeyDefined = LaneInvasionEvent::isKeyDefined();

            // 根据LaneInvasionEvent消息类型获取键（Key）的最大CDR序列化大小来确定用于存储键数据的缓冲区大小。
            // 如果获取键的最大序列化大小大于16字节，则使用实际的最大序列化大小作为缓冲区大小；否则使用16字节作为缓冲区大小，
            // 这里的16字节可能是基于某种默认约定或者系统兼容性考虑的固定长度
            size_t keyLength = LaneInvasionEvent::getKeyMaxCdrSerializedSize() > 16?
                    LaneInvasionEvent::getKeyMaxCdrSerializedSize() : 16;

            // 分配一块内存缓冲区用于存储键数据，通过malloc函数申请内存，并将返回的指针转换为unsigned char*类型后赋值给m_keyBuffer成员变量，
            // m_keyBuffer后续用于在获取键等相关操作中临时存储键的序列化数据等
            m_keyBuffer = reinterpret_cast<unsigned char*>(malloc(keyLength));
            // 将刚分配的内存缓冲区初始化为0，确保缓冲区中的数据初始状态是已知且一致的，避免出现未初始化的随机数据导致后续操作出现问题
            memset(m_keyBuffer, 0, keyLength);
        }

        // LaneInvasionEventPubSubType类的析构函数，用于释放构造函数中分配的资源（主要是用于存储键数据的内存缓冲区）
        LaneInvasionEventPubSubType::~LaneInvasionEventPubSubType()
        {
            // 检查m_keyBuffer指针是否为空，如果不为空，表示之前成功分配了内存缓冲区，需要通过free函数释放对应的内存，避免内存泄漏
            if (m_keyBuffer!= nullptr)
            {
                free(m_keyBuffer);
            }
        }

        // 序列化函数，用于将给定的LaneInvasionEvent类型的数据对象序列化为可传输或存储的格式，并填充到SerializedPayload_t结构中
        bool LaneInvasionEventPubSubType::serialize(
                void* data,
                SerializedPayload_t* payload)
        {
            // 将传入的void*类型数据指针转换为LaneInvasionEvent*类型指针，方便后续操作具体的LaneInvasionEvent对象
            LaneInvasionEvent* p_type = static_cast<LaneInvasionEvent*>(data);

            // 创建一个FastBuffer对象，用于管理原始的缓冲区数据。它将使用SerializedPayload_t结构中的data成员作为实际的缓冲区，
            // 并以payload->max_size作为缓冲区的最大可用大小，这个FastBuffer对象后续会在序列化过程中协助管理数据的读写操作
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->max_size);
            // 创建一个Cdr对象，用于执行具体的序列化操作。它关联了前面创建的FastBuffer对象，指定了默认的字节序（DEFAULT_ENDIAN）以及使用DDS_CDR格式，
            // Cdr对象提供了一系列序列化相关的方法，用于将各种数据类型按照指定格式序列化为字节流
            eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);
            // 根据Cdr对象当前的字节序设置SerializedPayload_t结构中的encapsulation成员，用于标识字节序情况，
            // 如果是大端序（BIG_ENDIANNESS）则设置为CDR_BE，小端序（LITTLE_ENDIANNESS）则设置为CDR_LE，方便后续反序列化或者其他处理时知晓字节序信息
            payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS? CDR_BE : CDR_LE;
            // 调用Cdr对象的serialize_encapsulation方法，对封装相关信息进行序列化，具体的封装信息内容可能取决于协议或者应用的要求
            ser.serialize_encapsulation();

            try
            {
                // 调用LaneInvasionEvent对象的serialize方法，通过传入的Cdr对象将自身的各个数据成员按照指定格式序列化到缓冲区中，
                // 如果序列化过程中出现内存不足等异常情况，会抛出NotEnoughMemoryException异常
                p_type->serialize(ser);
            }
            catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
            {
                // 如果捕获到内存不足异常，返回false表示序列化失败
                return false;
            }

            // 获取序列化后的数据实际长度，通过Cdr对象的getSerializedDataLength方法获取，并将其转换为uint32_t类型后赋值给SerializedPayload_t结构的length成员，
            // 用于记录实际序列化后的数据长度信息
            payload->length = static_cast<uint32_t>(ser.getSerializedDataLength());
            // 如果序列化过程顺利完成，返回true表示序列化成功
            return true;
        }

        // 反序列化函数，用于将SerializedPayload_t结构中存储的序列化数据还原为LaneInvasionEvent类型的对象
        bool LaneInvasionEventPubSubType::deserialize(
                SerializedPayload_t* payload,
                void* data)
        {
            try
            {
                // 将传入的void*类型数据指针转换为LaneInvasionEvent*类型指针，方便后续操作具体的LaneInvasionEvent对象
                LaneInvasionEvent* p_type = static_cast<LaneInvasionEvent*>(data);

                // 创建一个FastBuffer对象，用于管理原始的缓冲区数据。它将使用SerializedPayload_t结构中的data成员作为实际的缓冲区，
                // 并以payload->length作为缓冲区的实际可用大小（因为这里只处理已经序列化好的有效数据长度部分），
                // 这个FastBuffer对象后续会在反序列化过程中协助管理数据的读写操作
                eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->length);

                // 创建一个Cdr对象，用于执行具体的反序列化操作。它关联了前面创建的FastBuffer对象，指定了默认的字节序（DEFAULT_ENDIAN）以及使用DDS_CDR格式，
                // Cdr对象提供了一系列反序列化相关的方法，用于将字节流按照指定格式还原为各种数据类型的对象
                eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);

                // 调用Cdr对象的read_encapsulation方法，读取并处理封装相关信息，例如根据封装信息中的字节序标识等设置相应的内部状态，
                // 同时也会对SerializedPayload_t结构中的encapsulation成员进行相应设置，确保后续数据反序列化操作能正确进行
                deser.read_encapsulation();
                payload->encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS? CDR_BE : CDR_LE;

                // 调用LaneInvasionEvent对象的deserialize方法，通过传入的Cdr对象将缓冲区中的字节流按照指定格式反序列化为自身的各个数据成员，
                // 如果反序列化过程中出现内存不足等异常情况，会抛出NotEnoughMemoryException异常
                p_type->deserialize(deser);
            }
            catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
            {
                // 如果捕获到内存不足异常，返回false表示反序列化失败
                return false;
            }

            // 如果反序列化过程顺利完成，返回true表示反序列化成功
            return true;
        }

        // 获取序列化大小提供函数，返回一个可调用对象（lambda表达式），该对象用于获取给定LaneInvasionEvent类型数据对象的序列化大小（包含封装部分）
        std::function<uint32_t()> LaneInvasionEventPubSubType::getSerializedSizeProvider(
                void* data)
        {
            return [data]() -> uint32_t
                   {
                       // 调用type::getCdrSerializedSize函数获取LaneInvasionEvent对象的CDR序列化大小（不包含封装部分），
                       // 然后加上4字节的封装大小，返回最终的序列化大小估计值，以uint32_t类型返回
                       return static_cast<uint32_t>(type::getCdrSerializedSize(*static_cast<LaneInvasionEvent*>(data))) +
                              4u /*encapsulation*/;
                   };
        }

        // 创建数据函数，用于创建一个新的LaneInvasionEvent类型的对象，并返回其void*类型的指针，
        // 这个函数通常在需要动态分配一个新的消息对象实例时使用，例如在发布消息前创建要发布的消息数据对象
        void* LaneInvasionEventPubSubType::createData()
        {
            return reinterpret_cast<void*>(new LaneInvasionEvent());
        }

        // 删除数据函数，用于释放之前通过createData函数创建的LaneInvasionEvent类型对象所占用的内存空间，
        // 避免内存泄漏，确保动态分配的对象在不再使用时能正确被销毁
        void LaneInvasionEventPubSubType::deleteData(
                void* data)
        {
            delete(reinterpret_cast<LaneInvasionEvent*>(data));
        }

        // 获取键函数，用于获取给定LaneInvasionEvent类型数据对象的键信息，并填充到InstanceHandle_t结构中，
        // 键信息通常用于在发布/订阅系统中唯一标识一个消息实例或者进行一些查找、匹配等操作
        bool LaneInvasionEventPubSubType::getKey(
                void* data,
                InstanceHandle_t* handle,
                bool force_md5)
        {
            // 首先检查是否定义了获取键的操作，如果m_isGetKeyDefined为false，表示没有定义获取键的相关逻辑，直接返回false
            if (!m_isGetKeyDefined)
            {
                return false;
            }

            // 将传入的void*类型数据指针转换为LaneInvasionEvent*类型指针，方便后续操作具体的LaneInvasionEvent对象
            LaneInvasionEvent* p_type = static_cast<LaneInvasionEvent*>(data);

            // 创建一个FastBuffer对象，用于管理临时的缓冲区数据。它将使用之前在构造函数中分配的m_keyBuffer作为实际的缓冲区，
            // 并以LaneInvasionEvent消息类型获取键的最大CDR序列化大小作为缓冲区大小，这个FastBuffer对象后续会在获取键的序列化等操作中协助管理数据
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(m_keyBuffer),
                    LaneInvasionEvent::getKeyMaxCdrSerializedSize());

            // 创建一个Cdr对象，用于执行将键数据序列化的操作。这里指定了字节序为大端序（BIG_ENDIANNESS），
            // 并通过调用LaneInvasionEvent对象的serializeKey方法将键相关的数据按照指定格式序列化到FastBuffer管理的缓冲区中
            eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::BIG_ENDIANNESS);
            p_type->serializeKey(ser);

            // 根据条件判断是否使用MD5哈希算法来处理键数据。如果force_md5为true或者获取键的最大CDR序列化大小大于16字节，
            // 则进行MD5相关操作，通常是为了生成一个固定长度（这里是16字节）的哈希值作为键的最终表示，方便在一些场景下进行比较、查找等操作
            if (force_md5 || LaneInvasionEvent::getKeyMaxCdrSerializedSize() > 16)
            {
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
                // 如果不满足上述使用MD5的条件，则直接将缓冲区中的数据（前16字节，因为前面确定缓冲区大小至少是16字节）复制到InstanceHandle_t结构的value成员中，
                // 作为键的表示，这里假设键数据本身长度不超过16字节或者不需要进行MD5哈希处理的简单情况
                for (uint8_t i = 0; i < 16; ++i)
                {
                    handle->value[i] = m_keyBuffer[i];
                }
            }
            // 如果获取键的操作顺利完成，返回true表示成功获取键信息
            return true;
        }
    } 
} 
