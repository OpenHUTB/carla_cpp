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
 * 该文本由工具fastcdrgen生成，主要用于处理与传感器消息中相机信息（CameraInfo）相关的发布/订阅类型的序列化、反序列化以及获取关键信息等操作，以便在数据分发服务（DDS）框架下正确传输和处理相机信息数据。
 */

#include <fastcdr/FastBuffer.h>
#include <fastcdr/Cdr.h>

#include "CameraInfoPubSubTypes.h"

// 定义SerializedPayload_t类型别名，用于表示序列化后的有效载荷，在Fast DDS的RTPS（实时发布/订阅协议）中使用，包含了数据和相关的长度、封装等信息。
using SerializedPayload_t = eprosima::fastrtps::rtps::SerializedPayload_t;
// 定义InstanceHandle_t类型别名，用于表示实例句柄，在Fast DDS中可用于标识不同的实例，例如区分不同的发布或订阅对象等。
using InstanceHandle_t = eprosima::fastrtps::rtps::InstanceHandle_t;

namespace sensor_msgs {
    namespace msg {

        // CameraInfoPubSubType类的构造函数，用于初始化与相机信息发布/订阅类型相关的属性。
        CameraInfoPubSubType::CameraInfoPubSubType()
        {
            // 设置该类型的名称，这里设置为 "sensor_msgs::msg::dds_::CameraInfo_"，用于在DDS框架中标识该类型。
            setName("sensor_msgs::msg::dds_::CameraInfo_");
            // 获取相机信息（CameraInfo）类型的最大CDR（Common Data Representation，一种数据序列化格式）序列化大小，这决定了后续缓冲区等的大小分配。
            auto type_size = CameraInfo::getMaxCdrSerializedSize();
            // 进行可能的子消息对齐操作，确保数据按照4字节对齐，这在一些内存访问和数据传输优化场景中很重要。
            type_size += eprosima::fastcdr::Cdr::alignment(type_size, 4); /* 可能的子消息对齐 */
            // 计算总的类型大小，加上4字节的封装大小（可能用于存储额外的头部等信息），并转换为无符号32位整数类型存储。
            m_typeSize = static_cast<uint32_t>(type_size) + 4; /*封装*/
            // 判断相机信息类型是否定义了获取关键信息（Key）的方法，后续在获取Key时会用到这个标记。
            m_isGetKeyDefined = CameraInfo::isKeyDefined();
            // 根据相机信息类型获取Key的最大CDR序列化大小来确定键缓冲区（Key Buffer）的长度，如果大于16字节则取实际大小，否则取16字节。
            size_t keyLength = CameraInfo::getKeyMaxCdrSerializedSize() > 16?
                    CameraInfo::getKeyMaxCdrSerializedSize() : 16;
            // 分配键缓冲区内存空间，用于存储序列化后的关键信息。
            m_keyBuffer = reinterpret_cast<unsigned char*>(malloc(keyLength));
            // 初始化键缓冲区内存内容为0，确保初始状态下数据的正确性。
            memset(m_keyBuffer, 0, keyLength);
        }

        // CameraInfoPubSubType类的析构函数，用于释放构造函数中分配的键缓冲区内存空间，避免内存泄漏。
        CameraInfoPubSubType::~CameraInfoPubSubType()
        {
            if (m_keyBuffer!= nullptr)
            {
                free(m_keyBuffer);
            }
        }

        // 序列化函数，将相机信息数据转换为可在网络中传输的序列化格式，存储在SerializedPayload_t结构体中。
        bool CameraInfoPubSubType::serialize(
                void* data,
                SerializedPayload_t* payload)
        {
            // 将传入的void*类型数据转换为相机信息（CameraInfo）类型的指针，以便后续进行序列化操作。
            CameraInfo* p_type = static_cast<CameraInfo*>(data);

            // 创建一个FastBuffer对象，用于管理原始缓冲区，将payload中的数据指针转换为char*类型，并指定最大可用大小为payload的max_size，它是实际进行数据读写的基础。
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->max_size);
            // 创建一个Cdr对象，用于序列化数据，指定缓冲区、默认字节序以及使用DDS CDR格式，该对象提供了各种序列化方法来处理数据的转换。
            eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);
            // 根据Cdr对象的字节序设置序列化后的封装格式（大端序或小端序），并存储在payload的encapsulation成员中。
            payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS? CDR_BE : CDR_LE;

            try
            {
                // 首先序列化封装信息，可能包含一些头部或者格式相关的元数据，这是按照CDR格式进行序列化的第一步操作。
                ser.serialize_encapsulation();
                // 使用相机信息对象的serialize方法，通过Cdr对象将相机信息具体内容序列化到缓冲区中，按照其定义的结构和字段顺序进行转换。
                p_type->serialize(ser);
            }
            catch (eprosima::fastcdr::exception::Exception& /*exception*/)
            {
                // 如果在序列化过程中出现异常（比如数据格式不正确、缓冲区溢出等情况），则返回false表示序列化失败。
                return false;
            }

            // 获取序列化后数据的实际长度，将其存储在payload的length成员中，用于表示有效数据的大小。
            payload->length = static_cast<uint32_t>(ser.getSerializedDataLength());
            return true;
        }

        // 反序列化函数，将接收到的序列化数据转换回相机信息（CameraInfo）类型的对象，从SerializedPayload_t结构体中提取数据并解析。
        bool CameraInfoPubSubType::deserialize(
                SerializedPayload_t* payload,
                void* data)
        {
            try
            {
                // 将传入的void*类型数据转换为相机信息（CameraInfo）类型的指针，以便后续进行反序列化操作，填充对应的数据结构。
                CameraInfo* p_type = static_cast<CameraInfo*>(data);

                // 创建一个FastBuffer对象，用于管理原始缓冲区，将payload中的数据指针转换为char*类型，并指定可用长度为payload的length，这是实际进行数据读取的基础。
                eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->length);

                // 创建一个Cdr对象，用于反序列化数据，指定缓冲区、默认字节序以及使用DDS CDR格式，该对象提供了各种反序列化方法来解析接收到的数据。
                eprosima::fastcdr::Cdr deser(fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);

                // 首先反序列化封装信息，获取并设置相应的封装格式（大端序或小端序），与序列化时的封装格式对应，确保数据解析的正确性。
                deser.read_encapsulation();
                payload->encapsulation = deser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS? CDR_BE : CDR_LE;

                // 使用相机信息对象的deserialize方法，通过Cdr对象将缓冲区中的数据反序列化到相机信息对象中，按照其定义的结构和字段顺序进行解析填充。
                p_type->deserialize(deser);
            }
            catch (eprosima::fastcdr::exception::Exception& /*exception*/)
            {
                // 如果在反序列化过程中出现异常（比如数据格式不正确、数据不完整等情况），则返回false表示反序列化失败。
                return false;
            }

            return true;
        }

        // 返回一个函数对象，该函数对象用于获取给定相机信息数据的序列化大小（包含封装等额外部分），可用于在一些内存分配或者传输前预估数据大小的场景。
        std::function<uint32_t()> CameraInfoPubSubType::getSerializedSizeProvider(
                void* data)
        {
            return [data]() -> uint32_t
                   {
                       // 获取相机信息对象的CDR序列化大小，并加上4字节的封装大小，返回总的序列化大小估计值。
                       return static_cast<uint32_t>(type::getCdrSerializedSize(*static_cast<CameraInfo*>(data))) +
                              4u /*封装*/;
                   };
        }

        // 创建一个新的相机信息（CameraInfo）类型的数据对象，返回的是void*类型指针，调用者需要根据实际情况进行类型转换后使用，用于在需要创建新的数据实例时调用。
        void* CameraInfoPubSubType::createData()
        {
            return reinterpret_cast<void*>(new CameraInfo());
        }

        // 删除给定的相机信息（CameraInfo）类型的数据对象，释放其占用的内存空间，防止内存泄漏，需要传入通过createData创建的对应指针。
        void CameraInfoPubSubType::deleteData(
                void* data)
        {
            delete(reinterpret_cast<CameraInfo*>(data));
        }

        // 获取相机信息对象的关键信息（Key），用于在一些需要唯一标识数据实例或者进行数据匹配等场景，例如在DDS的键值匹配查找中使用。
        bool CameraInfoPubSubType::getKey(
                void* data,
                InstanceHandle_t* handle,
                bool force_md5)
        {
            if (!m_isGetKeyDefined)
            {
                // 如果该类型没有定义获取Key的方法，则直接返回false，表示无法获取关键信息。
                return false;
            }

            CameraInfo* p_type = static_cast<CameraInfo*>(data);

            // 创建一个FastBuffer对象，用于管理存储关键信息的缓冲区，将键缓冲区（m_keyBuffer）指针转换为char*类型，并指定最大可用大小为相机信息类型的Key最大CDR序列化大小，用于后续序列化关键信息到该缓冲区。
            eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(m_keyBuffer),
                    CameraInfo::getKeyMaxCdrSerializedSize());

            // 创建一个Cdr对象，指定字节序为大端序（通常在获取关键信息等场景下使用固定字节序），用于序列化关键信息到缓冲区中。
            eprosima::fastcdr::Cdr ser(fastbuffer, eprosima::fastcdr::Cdr::BIG_ENDIANNESS);
            p_type->serializeKey(ser);
            if (force_md5 || CameraInfo::getKeyMaxCdrSerializedSize() > 16)
            {
                // 如果强制使用MD5或者关键信息序列化后大小大于16字节，则使用MD5算法对关键信息进行处理，初始化MD5对象，更新数据并最终生成摘要。
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
                // 如果关键信息序列化后大小不大于16字节，则直接将键缓冲区中的内容复制到实例句柄（InstanceHandle_t）的value数组中，作为关键信息。
                for (uint8_t i = 0; i < 16; ++i)
                {
                    handle->value[i] = m_keyBuffer[i];
                }
            }
            return true;
        }
    } //End of namespace msg
} //End of namespace sensor_msgs
