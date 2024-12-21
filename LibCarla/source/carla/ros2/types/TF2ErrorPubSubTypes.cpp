// 版权所有 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).  
//  
// 根据 Apache 许可证第 2.0 版（“许可证”）获得许可；  
// 您不得在未遵守许可证的情况下使用此文件。  
// 您可以在以下网址获取许可证副本：  
//  
//     http://www.apache.org/licenses/LICENSE-2.0  
//  
// 除非适用法律要求或书面同意，否则本许可证下分发的软件是  
// 以“原样”方式提供的，不提供任何明示或暗示的担保或条件。  
// 有关许可证下特定语言的权限和限制，请参见许可证的内容。

/*!  
 * @file TF2ErrorPubSubTypes.cpp  
 * 该源文件实现了 TF2Error 消息类型的序列化和反序列化函数。  
 *  
 * 此文件由工具 fastcdrgen 生成。  
 */

#include <fastcdr/FastBuffer.h>// 引入 Fast CDR 用于高效的数据序列化
#include <fastcdr/Cdr.h>// 引入 Fast CDR 的主要功能

#include "TF2ErrorPubSubTypes.h"// 引入 TF2Error 消息类型的定义

using SerializedPayload_t = eprosima::fastrtps::rtps::SerializedPayload_t;// 定义序列化载体类型  
using InstanceHandle_t = eprosima::fastrtps::rtps::InstanceHandle_t;// 定义实例句柄类型

namespace tf2_msgs {
    namespace msg {
        TF2ErrorPubSubType::TF2ErrorPubSubType()
        {
            setName("tf2_msgs::msg::dds_::TF2Error_");// 设置消息类型的名称
            auto type_size = TF2Error::getMaxCdrSerializedSize();// 获取 TF2Error 类型的最大序列化大小
            type_size += eprosima::fastcdr::Cdr::alignment(type_size, 4); // 处理字节对齐
            m_typeSize = static_cast<uint32_t>(type_size) + 4; // 加上封装头的总大小 
            m_isGetKeyDefined = TF2Error::isKeyDefined();// 检查 TF2Error 是否定义键 
            // 根据最大键序列化大小分配键缓冲区的大小
            size_t keyLength = TF2Error::getKeyMaxCdrSerializedSize() > 16 ?
                    TF2Error::getKeyMaxCdrSerializedSize() : 16;
            m_keyBuffer = reinterpret_cast<unsigned char*>(malloc(keyLength));// 分配内存用于键缓冲区
            memset(m_keyBuffer, 0, keyLength);// 初始化键缓冲区
        }
// 析构函数，释放分配的资源 
        TF2ErrorPubSubType::~TF2ErrorPubSubType()
        {
            if (m_keyBuffer != nullptr)
            {
                free(m_keyBuffer);// 释放键缓冲区的内存
            }
        }
// 序列化函数，将 TF2Error 对象序列化为字节流 
        bool TF2ErrorPubSubType::serialize(
                void* data,
                SerializedPayload_t* payload)
        {
            TF2Error* p_type = static_cast<TF2Error*>(data); // 将输入数据转换为 TF2Error 类型指针

            // 创建一个快速缓冲区来管理原始字节缓冲区
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->max_size);
            // 创建序列化对象
            eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);
            // 检测字节序  
            payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
            // 序列化封装头 
            ser.serialize_encapsulation();

            try
            {
                // 序列化对象数据
                p_type->serialize(ser);
            }
            catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
            {
                return false;// 捕获内存不足异常 
            }

            // 获取序列化后的数据长度 
            payload->length = static_cast<uint32_t>(ser.getSerializedDataLength());
            return true;// 返回序列化成功
        }
// 反序列化函数，从字节流中还原 TF2Error 对象 
        bool TF2ErrorPubSubType::deserialize(
                SerializedPayload_t* payload,
                void* data)
        {
            try
            {
                // 将数据转换为 TF2Error 类型指针
                TF2Error* p_type = static_cast<TF2Error*>(data);

                // 创建快速缓冲区
                eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->length);

                 // 创建反序列化对象 
                eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);

                // 读取封装头 
                deser.read_encapsulation();
                payload->encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

                // 从字节流中反序列化对象
                p_type->deserialize(deser);
            }
            catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
            {
                return false;// 捕获内存不足异常
            }

            return true;// 返回反序列化成功
        }
// 返回序列化大小提供者的函数
        std::function<uint32_t()> TF2ErrorPubSubType::getSerializedSizeProvider(
                void* data)
        {
            // 返回一个函数来计算序列化大小 
            return [data]() -> uint32_t
                   {
                       return static_cast<uint32_t>(type::getCdrSerializedSize(*static_cast<TF2Error*>(data))) +
                              4u /*封装头*/;
                   };
        }
// 创建设备数据的函数 
        void* TF2ErrorPubSubType::createData()
        {
            // 创建并返回新的 TF2Error 实例 
            return reinterpret_cast<void*>(new TF2Error());
        }
// 删除设备数据的函数 
        void TF2ErrorPubSubType::deleteData(
                void* data)
        {
            delete(reinterpret_cast<TF2Error*>(data));// 释放 TF2Error 实例的内存 
        }
// 获取对象的键
        bool TF2ErrorPubSubType::getKey(
                void* data,
                InstanceHandle_t* handle,
                bool force_md5)
        {
            if (!m_isGetKeyDefined)
            {
                return false;// 如果没有定义键，返回 false
            }

            TF2Error* p_type = static_cast<TF2Error*>(data);// 转换数据类型为 TF2Error  

             // 创建快速缓冲区来管理键的字节序列  
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(m_keyBuffer),
                    TF2Error::getKeyMaxCdrSerializedSize());

            // 创建序列化对象 
            eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::BIG_ENDIANNESS);
            p_type->serializeKey(ser); // 序列化键 
            if (force_md5 || TF2Error::getKeyMaxCdrSerializedSize() > 16)
            {
                m_md5.init();// 初始化 MD5 哈希  
                m_md5.update(m_keyBuffer, static_cast<unsigned int>(ser.getSerializedDataLength()));
                m_md5.finalize();// 完成哈希计算  
                for (uint8_t i = 0; i < 16; ++i)
                {
                    handle->value[i] = m_md5.digest[i];// 将 MD5 哈希值存储到句柄中  
                }
            }
            else
            {
                for (uint8_t i = 0; i < 16; ++i)
                {
                    handle->value[i] = m_keyBuffer[i];// 直接获取键值  
                }
            }
            return true;// 返回成功 
        }
    } // 结束命名空间 msg 
} // 结束命名空间 tf2_msgs  
