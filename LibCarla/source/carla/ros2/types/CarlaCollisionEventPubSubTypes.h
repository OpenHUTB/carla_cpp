// 版权所有 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// 根据 Apache 许可证，第 2.0 版（“许可证”）进行许可；
// 除非遵守许可证，否则不得使用此文件。
// 您可以在以下网址获取许可证的副本：
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// 除非适用法律要求或书面同意，
// 根据许可证分发的软件是以“原样”基础分发的，
// 不附有任何形式的保证或条件，无论是明示还是暗示的。
// 有关许可证下特定权限和限制的信息，请参见许可证。

/*!
 * @file CarlaCollisionEventPubSubTypes.h
 * 此头文件包含序列化函数的声明。
 *
 * 此文件是由工具 fastcdrgen 生成的。
 */
#ifndef _FAST_DDS_GENERATED_CARLA_MSGS_MSG_CARLACOLLISIONEVENT_PUBSUBTYPES_H_
#define _FAST_DDS_GENERATED_CARLA_MSGS_MSG_CARLACOLLISIONEVENT_PUBSUBTYPES_H_

#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastrtps/utils/md5.h>

#include "CarlaCollisionEvent.h"// 包含 CarlaCollisionEvent 类型的定义
#include "Vector3PubSubTypes.h"// 包含 Vector3 类型的序列化定义
#include "HeaderPubSubTypes.h"// 包含 Header 类型的序列化定义

// 检查生成的 API 版本是否与当前安装的 Fast DDS 兼容
#if !defined(GEN_API_VER) || (GEN_API_VER != 1)
#error \
    Generated CarlaCollisionEvent is not compatible with current installed Fast DDS. Please, regenerate it with fastddsgen.
#endif  // GEN_API_VER

namespace carla_msgs
{
    namespace msg
    {

        /*!  
         * @brief 此类表示用户在 IDL 文件中定义的类型 CarlaCollisionEvent 的 TopicDataType。  
         * @ingroup CARLACOLLISIONEVENT  
         */
        class CarlaCollisionEventPubSubType : public eprosima::fastdds::dds::TopicDataType
        {
        public:

            typedef CarlaCollisionEvent type;// 定义类型别名

            eProsima_user_DllExport CarlaCollisionEventPubSubType();// 构造函数

            eProsima_user_DllExport virtual ~CarlaCollisionEventPubSubType() override;// 析构函数

            // 序列化函数，将数据序列化到 payload 中
            eProsima_user_DllExport virtual bool serialize(
                    void* data,
                    eprosima::fastrtps::rtps::SerializedPayload_t* payload) override;

            // 反序列化函数，从 payload 中反序列化数据
            eProsima_user_DllExport virtual bool deserialize(
                    eprosima::fastrtps::rtps::SerializedPayload_t* payload,
                    void* data) override;

            // 获取序列化大小的提供者
            eProsima_user_DllExport virtual std::function<uint32_t()> getSerializedSizeProvider(
                    void* data) override;

            // 获取数据的键
            eProsima_user_DllExport virtual bool getKey(
                    void* data,
                    eprosima::fastrtps::rtps::InstanceHandle_t* ihandle,
                    bool force_md5 = false) override;

            // 创建数据实例
            eProsima_user_DllExport virtual void* createData() override;

            // 删除数据实例
            eProsima_user_DllExport virtual void deleteData(
                    void* data) override;

        #ifdef TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED
            // 检查类型是否有界
            eProsima_user_DllExport inline bool is_bounded() const override
            {
                return false;// 此类型没有界限
            }

        #endif  // TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED

        #ifdef TOPIC_DATA_TYPE_API_HAS_IS_PLAIN
            // 检查类型是否为简单类型
            eProsima_user_DllExport inline bool is_plain() const override
            {
                return false;// 此类型不是简单类型
            }

        #endif  // TOPIC_DATA_TYPE_API_HAS_IS_PLAIN

        #ifdef TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE
            // 构造样本
            eProsima_user_DllExport inline bool construct_sample(
                    void* memory) const override
            {
                (void)memory;// 忽略内存参数
                return false;// 不支持样本构造
            }

        #endif  // TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE

            MD5 m_md5;// MD5 哈希值
            unsigned char* m_keyBuffer;// 键缓冲区

        };
    }
}

#endif // _FAST_DDS_GENERATED_CARLA_MSGS_MSG_CARLACOLLISIONEVENT_PUBSUBTYPES_H_
