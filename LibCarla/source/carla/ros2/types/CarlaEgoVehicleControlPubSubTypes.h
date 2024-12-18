// 版权所有 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).  
//  
// 根据 Apache 许可证，第 2.0 版（“许可证”）进行许可；  
// 除非遵守许可证，否则您不得使用此文件。  
// 您可以在以下网址获得许可证的副本：  
//  
//     http://www.apache.org/licenses/LICENSE-2.0  
//  
// 除非适用法律要求或书面同意，否则根据许可证分发的软件是以“原样”基础提供的，  
// 不提供任何形式的保证或条件，无论是明示还是暗示。  
// 有关许可证下的特定权限和限制，请参见许可证。  

/*!  
 * @file CarlaEgoCarlaEgoVehicleControlPubSubTypes.h  
 * 此头文件包含序列化函数的声明。  
 *  
 * 此文件由工具 fastcdrgen 生成。  
 */ 

#ifndef _FAST_DDS_GENERATED_CARLA_MSGS_MSG_CARLAEGOCarlaEgoVehicleControl_PUBSUBTYPES_H_
#define _FAST_DDS_GENERATED_CARLA_MSGS_MSG_CARLAEGOCarlaEgoVehicleControl_PUBSUBTYPES_H_

#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastrtps/utils/md5.h>

#include "CarlaEgoVehicleControl.h"

#include "HeaderPubSubTypes.h"

// 检查生成的 API 版本是否与当前安装的 Fast DDS 兼容
#if !defined(GEN_API_VER) || (GEN_API_VER != 1)
#error \
    Generated CarlaEgoCarlaEgoVehicleControl is not compatible with current installed Fast DDS. Please, regenerate it with fastddsgen.
#endif  // GEN_API_VER

namespace carla_msgs
{
    namespace msg
    {
        /*!  
         * @brief 此类表示用户在 IDL 文件中定义的类型 CarlaEgoVehicleControl 的 TopicDataType。  
         * @ingroup CarlaEgoVehicleControl  
         */  
        class CarlaEgoVehicleControlPubSubType : public eprosima::fastdds::dds::TopicDataType
        {
        public:

            // 定义类型别名
            typedef CarlaEgoVehicleControl type;

            // 构造函数
            eProsima_user_DllExport CarlaEgoVehicleControlPubSubType();

            // 析构函数
            eProsima_user_DllExport virtual ~CarlaEgoVehicleControlPubSubType() override;

            // 序列化数据
            eProsima_user_DllExport virtual bool serialize(
                    void* data,
                    eprosima::fastrtps::rtps::SerializedPayload_t* payload) override;

            // 反序列化数据
            eProsima_user_DllExport virtual bool deserialize(
                    eprosima::fastrtps::rtps::SerializedPayload_t* payload,
                    void* data) override;

            // 获取序列化大小提供者
            eProsima_user_DllExport virtual std::function<uint32_t()> getSerializedSizeProvider(
                    void* data) override;

            // 获取数据的键
            eProsima_user_DllExport virtual bool getKey(
                    void* data,
                    eprosima::fastrtps::rtps::InstanceHandle_t* ihandle,
                    bool force_md5 = false) override;

            // 创建数据
            eProsima_user_DllExport virtual void* createData() override;

            // 删除数据
            eProsima_user_DllExport virtual void deleteData(
                    void* data) override;

        #ifdef TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED
            // 检查是否有界
            eProsima_user_DllExport inline bool is_bounded() const override
            {
                // 返回 false，表示没有界限
                return false;
            }

        #endif  // TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED

        #ifdef TOPIC_DATA_TYPE_API_HAS_IS_PLAIN
            // 检查是否为简单类型
            eProsima_user_DllExport inline bool is_plain() const override
            {
                // 返回 false，表示不是简单类型
                return false;
            }

        #endif  // TOPIC_DATA_TYPE_API_HAS_IS_PLAIN

        #ifdef TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE
            // 构造样本
            eProsima_user_DllExport inline bool construct_sample(
                    void* memory) const override
            {
                // 不使用 memory 参数
                (void)memory;
                // 返回 false，表示不支持样本构造
                return false;
            }

        #endif  // TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE
            // MD5 哈希
            MD5 m_md5;
            // 键缓冲区
            unsigned char* m_keyBuffer;
        };
    }
}

#endif // _FAST_DDS_GENERATED_CARLA_MSGS_MSG_CARLAEGOCarlaEgoVehicleControl_PUBSUBTYPES_H_
