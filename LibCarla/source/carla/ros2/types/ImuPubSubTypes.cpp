// 版权所有 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).  
//   
// 根据 Apache 许可证，版本 2.0（“许可证”）获得许可；  
// 除非遵守许可证，否则您不能使用此文件。  
// 您可以在以下地址获得许可证的副本：  
//   
//     http://www.apache.org/licenses/LICENSE-2.0  
//   
// 除非适用法律要求或书面同意，  
// 软件在“按原样”基础上分发，没有任何明示或暗示的担保或条件。  
// 请参阅许可证以获取特定语言管理的权限和限制。  

/*!  
 * @file ImuPubSubTypes.cpp  
 * 此文件包含 IMU 消息类型的序列化函数实现。  
 * 此文件由工具 fastcdrgen 生成，用于创建 C++ 代码以进行序列化。  
 */ 
#include <fastcdr/FastBuffer.h>
#include <fastcdr/Cdr.h>

#include "ImuPubSubTypes.h"// 包含 IMU 消息类型的头文件 

using SerializedPayload_t = eprosima::fastrtps::rtps::SerializedPayload_t;// 定义序列化载荷的类型别名 
using InstanceHandle_t = eprosima::fastrtps::rtps::InstanceHandle_t;// 定义实例句柄的类型别名

namespace sensor_msgs {
    namespace msg {
// ImuPubSubType 类的构造函数 
        ImuPubSubType::ImuPubSubType()
        {
            setName("sensor_msgs::msg::dds_::Imu_");// 设置与此消息类型关联的名称
            auto type_size = Imu::getMaxCdrSerializedSize();// 获取 IMU 类型的最大序列化大小 
            type_size += eprosima::fastcdr::Cdr::alignment(type_size, 4); /* 可能的子消息对齐 */ 
            m_typeSize = static_cast<uint32_t>(type_size) + 4; /* 加上封装的大小 */ 
            m_isGetKeyDefined = Imu::isKeyDefined();// 检查 IMU 类型是否定义了键  
            size_t keyLength = Imu::getKeyMaxCdrSerializedSize() > 16 ?
                    Imu::getKeyMaxCdrSerializedSize() : 16;// 确定键的长度 
            m_keyBuffer = reinterpret_cast<unsigned char*>(malloc(keyLength));// 为键缓冲区分配内存 
            memset(m_keyBuffer, 0, keyLength);// 将键缓冲区初始化为零
        }
// ImuPubSubType 类的析构函数 
        ImuPubSubType::~ImuPubSubType()
        {
            if (m_keyBuffer != nullptr)// 检查键缓冲区是否不为空  
            {
                free(m_keyBuffer);// 释放分配的键缓冲区内存  
            }
        }
// 序列化函数，将 IMU 数据转换为序列化载荷
        bool ImuPubSubType::serialize(
                void* data,
                SerializedPayload_t* payload)
        {
            Imu* p_type = static_cast<Imu*>(data);// 将输入数据转换为 IMU 类型  

            // 创建用于原始数据的缓冲区
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->max_size);
            // 处理序列化的对象 
            eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);
            payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
            // 序列化封装
            ser.serialize_encapsulation();

            try
            {
                // 序列化IMU对象 
                p_type->serialize(ser);
            }
            catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
            {
                return false;// 如果没有足够的内存序列化，则返回false  
            }

            // 获取序列化数据的长度  
            payload->length = static_cast<uint32_t>(ser.getSerializedDataLength());
            return true;// 序列化成功  
        }
// 反序列化函数，将序列化载荷转换回 IMU 对象  
        bool ImuPubSubType::deserialize(
                SerializedPayload_t* payload,
                void* data)
        {
            try
            {
                // 将输入数据转换为 IMU 类型  
                Imu* p_type = static_cast<Imu*>(data);

                 // 创建用于原始数据的缓冲区 
                eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->length);

                // 处理反序列化的对象  
                eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);

                // 反序列化封装 
                deser.read_encapsulation();
                payload->encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

                // 反序列化对象  
                p_type->deserialize(deser);
            }
            catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
            {
                return false;// 如果没有足够的内存反序列化，则返回false  
            }

            return true;// 反序列化成功 
        }
// 获取序列化大小提供者，计算数据的序列化大小 
        std::function<uint32_t()> ImuPubSubType::getSerializedSizeProvider(
                void* data)
        {
            return [data]() -> uint32_t
                   {
                       return static_cast<uint32_t>(type::getCdrSerializedSize(*static_cast<Imu*>(data))) +
                              4u ; // 加上封装的大小 
                   };
        }
// 创建 IMU 数据的实例 
        void* ImuPubSubType::createData()
        {
            return reinterpret_cast<void*>(new Imu());// 返回一个新的 IMU 对象  
        }
// 删除 IMU 数据的实例
        void ImuPubSubType::deleteData(
                void* data)
        {
            delete(reinterpret_cast<Imu*>(data));// 删除传入的 IMU 对象  
        }
// 获取 IMU 对象的键 
        bool ImuPubSubType::getKey(
                void* data,
                InstanceHandle_t* handle,
                bool force_md5)
        {
            if (!m_isGetKeyDefined)// 如果未定义获取键，则返回false  
            {
                return false;
            }

            Imu* p_type = static_cast<Imu*>(data);// 将数据转换为 IMU 类型  

            // 创建管理原始缓冲区的对象 
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(m_keyBuffer),
                    Imu::getKeyMaxCdrSerializedSize());

            // 处理序列化键的对象
            eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::BIG_ENDIANNESS);
            p_type->serializeKey(ser);// 序列化键  
            if (force_md5 || Imu::getKeyMaxCdrSerializedSize() > 16)// 如果强制使用 MD5 或键的最大大小大于16
            {
                m_md5.init(); // 初始化 MD5
                m_md5.update(m_keyBuffer, static_cast<unsigned int>(ser.getSerializedDataLength()));// 更新 MD5 哈希 
                m_md5.finalize();// 完成 MD5 哈希计算 
                for (uint8_t i = 0; i < 16; ++i) // 将 MD5 摘要复制到句柄中 
                {
                    handle->value[i] = m_md5.digest[i];
                }
            }
            else// 否则直接将键缓冲区的值复制到句柄中 
            {
                for (uint8_t i = 0; i < 16; ++i)
                {
                    handle->value[i] = m_keyBuffer[i];
                }
            }
            return true;// 获取键成功
        }
    } // msg 命名空间结束
} // sensor_msgs 命名空间结束 
