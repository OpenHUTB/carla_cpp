// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).  
//  
// 根据 Apache License, Version 2.0（“许可证”）授权；  
// 除非遵守该许可证，否则您不得使用此文件。  
// 您可以在以下网址获取许可证的副本：  
//  
//     http://www.apache.org/licenses/LICENSE-2.0  
//  
// 除非适用法律要求或书面同意，否则根据许可证分发的软件是以“原样”方式分发的，  
// 不附带任何明示或暗示的担保或条件。  
// 请参阅许可证以获取关于权限和限制的具体内容。  

/*!  
 * @file CameraInfoPubSubTypes.h  
 * 该头文件包含序列化函数的声明。  
 *  
 * 此文件是由工具 fastcdrgen 生成的。  
 */  

#ifndef _FAST_DDS_GENERATED_SENSOR_MSGS_MSG_CAMERAINFO_PUBSUBTYPES_H_  
#define _FAST_DDS_GENERATED_SENSOR_MSGS_MSG_CAMERAINFO_PUBSUBTYPES_H_  

#include <fastdds/dds/topic/TopicDataType.hpp> // 引入 Fast DDS 主题数据类型  
#include <fastrtps/utils/md5.h> // 引入 MD5 相关功能  

#include "CameraInfo.h" // 引入 CameraInfo 类型定义  

#include "RegionOfInterestPubSubTypes.h" // 引入 RegionOfInterest 的 PubSub 类型定义  
#include "HeaderPubSubTypes.h" // 引入 Header 的 PubSub 类型定义  

#if !defined(GEN_API_VER) || (GEN_API_VER != 1)  
#error \
    生成的 CameraInfo 与当前安装的 Fast DDS 不兼容。请使用 fastddsgen 重新生成它。  
#endif  // GEN_API_VER  

namespace sensor_msgs  
{  
    namespace msg  
    {  
        /*!  
         * @brief 此类表示用户在 IDL 文件中定义的 CameraInfo 类型的 TopicDataType。  
         * @ingroup CameraInfo  
         */  
        class CameraInfoPubSubType : public eprosima::fastdds::dds::TopicDataType  
        {  
        public:  

            typedef CameraInfo type; // 定义类型别名  

            eProsima_user_DllExport CameraInfoPubSubType(); // 构造函数  

            eProsima_user_DllExport virtual ~CameraInfoPubSubType() override; // 析构函数  

            eProsima_user_DllExport virtual bool serialize(  
                    void* data,  
                    eprosima::fastrtps::rtps::SerializedPayload_t* payload) override; // 序列化函数  

            eProsima_user_DllExport virtual bool deserialize(  
                    eprosima::fastrtps::rtps::SerializedPayload_t* payload,  
                    void* data) override; // 反序列化函数  

            eProsima_user_DllExport virtual std::function<uint32_t()> getSerializedSizeProvider(  
                    void* data) override; // 获取序列化大小的函数提供者  

            eProsima_user_DllExport virtual bool getKey(  
                    void* data,  
                    eprosima::fastrtps::rtps::InstanceHandle_t* ihandle,  
                    bool force_md5 = false) override; // 获取键的函数  

            eProsima_user_DllExport virtual void* createData() override; // 创建数据的函数  

            eProsima_user_DllExport virtual void deleteData(  
                    void* data) override; // 删除数据的函数  

        #ifdef TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED  
            eProsima_user_DllExport inline bool is_bounded() const override  
            {  
                return false; // 表示此类型不受限制  
            }  

        #endif  // TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED  

        #ifdef TOPIC_DATA_TYPE_API_HAS_IS_PLAIN  
            eProsima_user_DllExport inline bool is_plain() const override  
            {  
                return false; // 表示此类型不是简单类型  
            }  

        #endif  // TOPIC_DATA_TYPE_API_HAS_IS_PLAIN  

        #ifdef TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE  
            eProsima_user_DllExport inline bool construct_sample(  
                    void* memory) const override  
            {  
                (void)memory; // 不使用 memory 参数  
                return false; // 表示不支持构造样本  
            }  

        #endif  // TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE  
            MD5 m_md5; // MD5 哈希对象  
            unsigned char* m_keyBuffer; // 存储键的缓冲区  
        };  
    }  
}  

#endif // _FAST_DDS_GENERATED_SENSOR_MSGS_MSG_CAMERAINFO_PUBSUBTYPES_H_
