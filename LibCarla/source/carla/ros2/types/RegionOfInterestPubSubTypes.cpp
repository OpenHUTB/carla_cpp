// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*!
 * @file RegionOfInterestPubSubTypes.cpp
 * This header file contains the implementation of the serialization functions.
 *
 * This file was generated by the tool fastcdrgen.
 */

#include <fastcdr/FastBuffer.h>
#include <fastcdr/Cdr.h>

#include "RegionOfInterestPubSubTypes.h"

using SerializedPayload_t = eprosima::fastrtps::rtps::SerializedPayload_t;
using InstanceHandle_t = eprosima::fastrtps::rtps::InstanceHandle_t;

namespace sensor_msgs {
    namespace msg {
        // RegionOfInterestPubSubType类的构造函数
        // 主要用于初始化一些与类型相关的属性，比如类型名称、类型大小等
        RegionOfInterestPubSubType::RegionOfInterestPubSubType()
        {
            // 设置类型的名称
            setName("sensor_msgs::msg::dds_::RegionOfInterest_");
            // 获取RegionOfInterest类型的最大CDR序列化大小
            auto type_size = RegionOfInterest::getMaxCdrSerializedSize();
            // 考虑可能的子消息对齐情况（按4字节对齐）
            type_size += eprosima::fastcdr::Cdr::alignment(type_size, 4); 
             // 计算最终的类型大小，包含封装部分（额外加4字节）
            m_typeSize = static_cast<uint32_t>(type_size) + 4;
             // 判断RegionOfInterest类型是否定义了键（用于后续一些操作判断）
            m_isGetKeyDefined = RegionOfInterest::isKeyDefined();
            // 根据RegionOfInterest类型的键的最大CDR序列化大小来确定键缓冲区的长度
            // 如果键的最大序列化大小大于16字节，则取实际大小，否则取16字节
            size_t keyLength = RegionOfInterest::getKeyMaxCdrSerializedSize() > 16 ?
                    RegionOfInterest::getKeyMaxCdrSerializedSize() : 16;
            // 分配键缓冲区内存
            m_keyBuffer = reinterpret_cast<unsigned char*>(malloc(keyLength));
             // 初始化键缓冲区内容为0
            memset(m_keyBuffer, 0, keyLength);
        }
        // RegionOfInterestPubSubType类的析构函数
        // 用于释放构造函数中分配的键缓冲区内存
        RegionOfInterestPubSubType::~RegionOfInterestPubSubType()
        {
            if (m_keyBuffer != nullptr)
            {
                free(m_keyBuffer);
            }
        }
        // 序列化函数，将给定的数据（RegionOfInterest类型）序列化为SerializedPayload_t格式
        bool RegionOfInterestPubSubType::serialize(
                void* data,
                SerializedPayload_t* payload)
        { 
            // 将传入的void*类型数据转换为RegionOfInterest*类型指针，方便后续操作
            RegionOfInterest* p_type = static_cast<RegionOfInterest*>(data);
            // 创建一个FastBuffer对象，用于管理原始缓冲区，关联到payload的数据部分及最大大小
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->max_size);
            // 创建一个Cdr对象，用于执行序列化操作，设置了字节序等相关参数
            eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);
            // 根据Cdr对象的字节序设置payload的封装字节序标识（大端序或小端序）
            payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

            try
            { 
                // 先序列化封装部分（可能包含一些头部等信息）
                ser.serialize_encapsulation();
                // 再序列化具体的RegionOfInterest对象
                p_type->serialize(ser);
            }
            catch (eprosima::fastcdr::exception::Exception& /*exception*/)
            { 
                // 如果序列化过程出现异常，返回false表示失败
                return false;
            }
            // 获取序列化后的数据长度，设置到payload的length成员中
            payload->length = static_cast<uint32_t>(ser.getSerializedDataLength());
             // 序列化成功，返回true
            return true;
        }
        // 反序列化函数，将SerializedPayload_t格式的数据反序列化为RegionOfInterest类型
        bool RegionOfInterestPubSubType::deserialize(
                SerializedPayload_t* payload,
                void* data)
        {
            try
            {
                // 将传入的void*类型数据转换为RegionOfInterest*类型指针，方便后续操作
                RegionOfInterest* p_type = static_cast<RegionOfInterest*>(data);
                // 创建一个FastBuffer对象，用于管理原始缓冲区，关联到payload的数据部分及实际长度
                eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->length);
                // 创建一个Cdr对象，用于执行反序列化操作，设置了字节序等相关参数
                eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);
                // 先反序列化封装部分（读取可能的头部等信息）
                deser.read_encapsulation();
                // 根据Cdr对象的字节序设置payload的封装字节序标识（大端序或小端序）
                payload->encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
                // 再反序列化具体的RegionOfInterest对象
                p_type->deserialize(deser);
            }
            catch (eprosima::fastcdr::exception::Exception& /*exception*/)
            {
                // 如果反序列化过程出现异常，返回false表示失败
                return false;
            }
            // 反序列化成功，返回true
            return true;
        }
        // 返回一个函数对象，该函数对象用于获取给定数据序列化后的大小（包含封装部分）
        std::function<uint32_t()> RegionOfInterestPubSubType::getSerializedSizeProvider(
                void* data)
        {
            return [data]() -> uint32_t
                   { 
                       // 获取RegionOfInterest对象的CDR序列化大小并加上封装部分的4字节，返回总大小
                       return static_cast<uint32_t>(type::getCdrSerializedSize(*static_cast<RegionOfInterest*>(data))) +
                              4u /*encapsulation*/;
                   };
        }
        // 创建一个RegionOfInterest类型的数据对象（在堆上分配内存），并返回其void*指针
        void* RegionOfInterestPubSubType::createData()
        {
            return reinterpret_cast<void*>(new RegionOfInterest());
        }
        // 删除传入的void*指针指向的RegionOfInterest类型的数据对象（释放内存）
        void RegionOfInterestPubSubType::deleteData(
                void* data)
        {
            delete(reinterpret_cast<RegionOfInterest*>(data));
        }
        // 获取给定数据（RegionOfInterest类型）对应的键信息，用于一些实例标识等操作
        bool RegionOfInterestPubSubType::getKey(
                void* data,
                InstanceHandle_t* handle,
                bool force_md5)
        {
            // 如果没有定义键，直接返回false
            if (!m_isGetKeyDefined)
            {
                return false;
            }
            RegionOfInterest* p_type = static_cast<RegionOfInterest*>(data);
            // 创建一个FastBuffer对象，用于管理键缓冲区，关联到m_keyBuffer及键的最大CDR序列化大小
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(m_keyBuffer),
                    RegionOfInterest::getKeyMaxCdrSerializedSize());
            // 创建一个Cdr对象，用于序列化键信息，设置为大端序
            eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::BIG_ENDIANNESS);
             // 序列化键信息
            p_type->serializeKey(ser);
            if (force_md5 || RegionOfInterest::getKeyMaxCdrSerializedSize() > 16)
            {
                // 如果需要强制使用MD5或者键的最大序列化大小大于16字节
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
                // 如果键的最大序列化大小小于等于16字节，直接复制键缓冲区内容到实例句柄的value数组中
                for (uint8_t i = 0; i < 16; ++i)
                {
                    handle->value[i] = m_keyBuffer[i];
                }
            }
            return true;
        }
    } //End of namespace msg
} //End of namespace sensor_msgs
