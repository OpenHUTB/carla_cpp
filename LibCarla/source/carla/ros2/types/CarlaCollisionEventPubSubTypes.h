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
 * @file CarlaCollisionEventPubSubTypes.h
 * T此头文件包含了序列化相关函数的声明。
 *
 * 该文件由 fastcdrgen 工具生成。
 */


#ifndef _FAST_DDS_GENERATED_CARLA_MSGS_MSG_CARLACOLLISIONEVENT_PUBSUBTYPES_H_
#define _FAST_DDS_GENERATED_CARLA_MSGS_MSG_CARLACOLLISIONEVENT_PUBSUBTYPES_H_
// 引入 Fast DDS 中用于定义主题数据类型的头文件，为下面自定义的主题数据类型类继承做准备
#include <fastdds/dds/topic/TopicDataType.hpp>// 引入 Fast RTPS 中用于处理 MD5 相关操作的头文件，可能用于数据校验等功能
#include <fastrtps/utils/md5.h>
// 引入相关的自定义消息类型头文件，这些消息类型是构成 CarlaCollisionEvent 消息的组成部分或者与之相关联
#include "CarlaCollisionEvent.h"
#include "Vector3PubSubTypes.h"
#include "HeaderPubSubTypes.h"
// 如果未定义 GEN_API_VER 或者其值不等于 1，则报错，提示生成的 CarlaCollisionEvent 与当前安装的 Fast DDS 不兼容，需重新生成
#if !defined(GEN_API_VER) || (GEN_API_VER != 1)
#error \
    Generated CarlaCollisionEvent is not compatible with current installed Fast DDS. Please, regenerate it with fastddsgen.
#endif  // GEN_API_VER

namespace carla_msgs
{
    namespace msg
    {

        /*!
         * @brief 此类代表了用户在 IDL 文件中定义的 CarlaCollisionEvent 类型的主题数据类型（TopicDataType）。
         * @ingroup CARLACOLLISIONEVENT
         */
        class CarlaCollisionEventPubSubType : public eprosima::fastdds::dds::TopicDataType
        {
        public:
// 定义类型别名，方便代码中明确表示该主题数据类型对应的实际消息类型就是 CarlaCollisionEvent
            typedef CarlaCollisionEvent type;
 // 默认构造函数，用于创建该主题数据类型对象时进行初始化操作
            eProsima_user_DllExport CarlaCollisionEventPubSubType();
// 析构函数，用于在对象生命周期结束时释放相关资源，这里重写了基类的析构函数
            eProsima_user_DllExport virtual ~CarlaCollisionEventPubSubType() override;
 // 序列化函数，用于将给定的数据（void* 指针指向的实际为 CarlaCollisionEvent 类型的数据）转换为适合网络传输等的序列化格式，并存入 SerializedPayload_t 结构体中，返回序列化是否成功
            eProsima_user_DllExport virtual bool serialize(
                    void* data,
                    eprosima::fastrtps::rtps::SerializedPayload_t* payload) override;
 // 反序列化函数，用于将给定的序列化数据（SerializedPayload_t 结构体表示）还原为 CarlaCollisionEvent 类型的数据（通过 void* 指针指向的内存区域），返回反序列化是否成功
            eProsima_user_DllExport virtual bool deserialize(
                    eprosima::fastrtps::rtps::SerializedPayload_t* payload,
                    void* data) override;
  // 返回一个函数对象，该函数对象用于获取给定数据（void* 指针指向的 CarlaCollisionEvent 类型数据）的序列化大小，以方便在一些场景下动态获取数据序列化后的长度信息
            eProsima_user_DllExport virtual std::function<uint32_t()> getSerializedSizeProvider(
                    void* data) override;
 // 获取给定数据（void* 指针指向的 CarlaCollisionEvent 类型数据）的键（Key）信息，并存入 InstanceHandle_t 结构体中，可用于在 DDS 系统中进行实例标识、查找等操作，返回获取键信息是否成功
            eProsima_user_DllExport virtual bool getKey(
                    void* data,
                    eprosima::fastrtps::rtps::InstanceHandle_t* ihandle,
                    bool force_md5 = false) override;
// 创建用于存储 CarlaCollisionEvent 类型数据的内存空间，返回指向该内存空间的指针，便于后续向其中填充数据等操作
            eProsima_user_DllExport virtual void* createData() override;
 // 释放之前通过 createData 函数创建的用于存储数据的内存空间，传入指向该内存空间的指针（void* 数据）进行资源释放操作
            eProsima_user_DllExport virtual void deleteData(
                    void* data) override;

        #ifdef TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED  // 如果定义了 TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED 宏，重写 is_bounded 函数，返回该主题数据类型是否有界（通常用于表示数据量是否固定等情况），这里返回 false
            eProsima_user_DllExport inline bool is_bounded() const override
            {
                return false;
            }

        #endif  // TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED

        #ifdef TOPIC_DATA_TYPE_API_HAS_IS_PLAIN  // 如果定义了 TOPIC_DATA_TYPE_API_HAS_IS_PLAIN 宏，重写 is_plain 函数，返回该主题数据类型是否是简单类型（可能涉及数据结构复杂度等判断），这里返回 false
            eProsima_user_DllExport inline bool is_plain() const override
            {
                return false;
            }

        #endif  // TOPIC_DATA_TYPE_API_HAS_IS_PLAIN

        #ifdef TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE// 如果定义了 TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE 宏，重写 construct_sample 函数，用于构建一个示例数据（可能用于初始化等场景），这里暂时返回 false，表示未实现具体构建逻辑
            eProsima_user_DllExport inline bool construct_sample(
            eProsima_user_DllExport inline bool construct_sample(
                    void* memory) const override
            {
                (void)memory;
                return false;
            }

        #endif  // TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE   // MD5 对象，可能用于对数据进行 MD5 校验等相关操作，保证数据的完整性和一致性

            MD5 m_md5;// 指向用于存储键（Key）相关数据的缓冲区指针，用于在获取键信息等操作中临时存储数据
            unsigned char* m_keyBuffer;

        };
    }
}

#endif // _FAST_DDS_GENERATED_CARLA_MSGS_MSG_CARLACOLLISIONEVENT_PUBSUBTYPES_H_
