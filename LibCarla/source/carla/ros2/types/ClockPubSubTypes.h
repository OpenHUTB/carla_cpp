// 版权所有 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).  
//  
// 根据 Apache 许可证，第 2.0 版（“许可证”）进行许可；  
// 除非遵守许可证，否则不可使用此文件。  
// 您可以在以下地址获得许可证副本：  
//  
//     http://www.apache.org/licenses/LICENSE-2.0  
//  
// 除非适用法律要求或书面同意，  
// 按照许可证分发的软件将以“原样”基础提供，  
// 不提供任何形式的明示或暗示的担保或条件。  
// 请参阅许可证了解特定语言所涵盖的权限和  
// 限制条款。  

/*!  
 * @file ClockPubSubTypes.h  
 * 该头文件包含序列化函数的声明。  
 *  
 * 此文件由工具 fastcdrgen 生成。  
 */ 

#ifndef _FAST_DDS_GENERATED_ROSGRAPH_MSG_CLOCK_PUBSUBTYPES_H_// 如果未定义宏 _FAST_DDS_GENERATED_ROSGRAPH_MSG_CLOCK_PUBSUBTYPES_H_ 
#define _FAST_DDS_GENERATED_ROSGRAPH_MSG_CLOCK_PUBSUBTYPES_H_

#include <fastdds/dds/topic/TopicDataType.hpp>// 引入 Fast DDS 主题数据类型的头文件  
#include <fastrtps/utils/md5.h>// 引入 MD5 工具头文件  

#include "Clock.h"// 引入 Clock 类型的定义

#if !defined(GEN_API_VER) || (GEN_API_VER != 1)// 检查生成的 API 版本是否与当前安装的 Fast DDS 兼容  
#error \
    Generated Clock is not compatible with current installed Fast DDS. Please, regenerate it with fastddsgen.
#endif  // GEN_API_VER

namespace rosgraph// 命名空间 rosgraph 
{
    namespace msg// 命名空间 msg  
    {
       /*!  
         * @brief 此类表示用户在 IDL 文件中定义的类型 Clock 的 TopicDataType。  
         * @ingroup Clock  
         */ 
        class ClockPubSubType : public eprosima::fastdds::dds::TopicDataType// Clock 的 PubSub 类型类，继承自 TopicDataType
        {
        public:

            typedef Clock type;// typedef 用于定义 Clock 类型 

            eProsima_user_DllExport ClockPubSubType();// 构造函数 

            eProsima_user_DllExport virtual ~ClockPubSubType() override;// 析构函数

            eProsima_user_DllExport virtual bool serialize(// 序列化函数 
                    void* data,
                    eprosima::fastrtps::rtps::SerializedPayload_t* payload) override;

            eProsima_user_DllExport virtual bool deserialize(// 反序列化函数 
                    eprosima::fastrtps::rtps::SerializedPayload_t* payload,
                    void* data) override;

            eProsima_user_DllExport virtual std::function<uint32_t()> getSerializedSizeProvider(// 获取序列化大小的函数 
                    void* data) override;

            eProsima_user_DllExport virtual bool getKey(// 获取键值的函数 
                    void* data,
                    eprosima::fastrtps::rtps::InstanceHandle_t* ihandle,
                    bool force_md5 = false) override;

            eProsima_user_DllExport virtual void* createData() override;// 创建数据的函数

            eProsima_user_DllExport virtual void deleteData( // 删除数据的函数
                    void* data) override;

        #ifdef TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED
            eProsima_user_DllExport inline bool is_bounded() const override // 检查是否有界的函数 
            {
                return false;// 返回 false，表示没有界限
            }

        #endif  // TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED

        #ifdef TOPIC_DATA_TYPE_API_HAS_IS_PLAIN
            eProsima_user_DllExport inline bool is_plain() const override// 检查是否为简单类型的函数
            {
                return false;// 返回 false，表示不是简单类型
            }

        #endif  // TOPIC_DATA_TYPE_API_HAS_IS_PLAIN

        #ifdef TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE
            eProsima_user_DllExport inline bool construct_sample(// 构造样本的函数  
                    void* memory) const override
            {
                (void)memory;
                return false;// 返回 false，表示不构造样本
            }

        #endif  // TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE
            MD5 m_md5;// MD5 对象，用于处理数据的 MD5 校验和 
            unsigned char* m_keyBuffer;// 存储键值的缓冲区  
        };
    }
}

#endif // _FAST_DDS_GENERATED_ROSGRAPH_MSG_CLOCK_PUBSUBTYPES_H_ // 结束宏定义  
