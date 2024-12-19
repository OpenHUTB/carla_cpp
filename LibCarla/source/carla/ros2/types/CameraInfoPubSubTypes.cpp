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
 * @file CameraInfoPubSubTypes.cpp
 * 这个头文件包含序列化函数的实现。
 *
 * 该文本由工具fastcdrgen生成
 */

#include <fastcdr/FastBuffer.h>
#include <fastcdr/Cdr.h>

#include "CameraInfoPubSubTypes.h"

using SerializedPayload_t = eprosima::fastrtps::rtps::SerializedPayload_t;
using InstanceHandle_t = eprosima::fastrtps::rtps::InstanceHandle_t;

namespace sensor_msgs {
    namespace msg {
        CameraInfoPubSubType::CameraInfoPubSubType()
        {
            setName("sensor_msgs::msg::dds_::CameraInfo_");
            auto type_size = CameraInfo::getMaxCdrSerializedSize();
            type_size += eprosima::fastcdr::Cdr::alignment(type_size, 4); /* 可能的子消息对齐 */
            m_typeSize = static_cast<uint32_t>(type_size) + 4; /*封装*/
            m_isGetKeyDefined = CameraInfo::isKeyDefined();
            size_t keyLength = CameraInfo::getKeyMaxCdrSerializedSize() > 16 ?
                    CameraInfo::getKeyMaxCdrSerializedSize() : 16;
            m_keyBuffer = reinterpret_cast<unsigned char*>(malloc(keyLength));
            memset(m_keyBuffer, 0, keyLength);
        }

        CameraInfoPubSubType::~CameraInfoPubSubType()
        {
            if (m_keyBuffer != nullptr)
            {
                free(m_keyBuffer);
            }
        }

        bool CameraInfoPubSubType::serialize(
                void* data,
                SerializedPayload_t* payload)
        {
            CameraInfo* p_type = static_cast<CameraInfo*>(data);

            // 管理原始缓冲区的对象。
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->max_size);
            // 序列化（serializes）数据的对象。
            eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);
            payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

            try
            {
                // 序列化封装
                ser.serialize_encapsulation();
                // 序列化对象。
                p_type->serialize(ser);
            }
            catch (eprosima::fastcdr::exception::Exception& /*exception*/)
            {
                return false;
            }

            // 获取序列化长度
            payload->length = static_cast<uint32_t>(ser.getSerializedDataLength());
            return true;
        }

        bool CameraInfoPubSubType::deserialize(
                SerializedPayload_t* payload,
                void* data)
        {
            try
            {
                // 将DATA转换为您类型的指针
                CameraInfo* p_type = static_cast<CameraInfo*>(data);

                // 管理原始缓冲区的对象。
                eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->length);

                // 对数据进行反序列化（deserializes）的对象。
                eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);

                // 反序列化封装。
                deser.read_encapsulation();
                payload->encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;

                // 反序列化对象。
                p_type->deserialize(deser);
            }
            catch (eprosima::fastcdr::exception::Exception& /*exception*/)
            {
                return false;
            }

            return true;
        }

        std::function<uint32_t()> CameraInfoPubSubType::getSerializedSizeProvider(
                void* data)
        {
            return [data]() -> uint32_t
                   {
                       return static_cast<uint32_t>(type::getCdrSerializedSize(*static_cast<CameraInfo*>(data))) +
                              4u /*封装*/;
                   };
        }

        void* CameraInfoPubSubType::createData()
        {
            return reinterpret_cast<void*>(new CameraInfo());
        }

        void CameraInfoPubSubType::deleteData(
                void* data)
        {
            delete(reinterpret_cast<CameraInfo*>(data));
        }

        bool CameraInfoPubSubType::getKey(
                void* data,
                InstanceHandle_t* handle,
                bool force_md5)
        {
            if (!m_isGetKeyDefined)
            {
                return false;
            }

            CameraInfo* p_type = static_cast<CameraInfo*>(data);

            // 管理原始缓冲区的对象。
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(m_keyBuffer),
                    CameraInfo::getKeyMaxCdrSerializedSize());

            // 序列化数据的对象。
            eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::BIG_ENDIANNESS);
            p_type->serializeKey(ser);
            if (force_md5 || CameraInfo::getKeyMaxCdrSerializedSize() > 16)
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
                for (uint8_t i = 0; i < 16; ++i)
                {
                    handle->value[i] = m_keyBuffer[i];
                }
            }
            return true;
        }
    } //End of namespace msg
} //End of namespace sensor_msgs
