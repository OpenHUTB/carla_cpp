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
 * @file TwistPubSubTypes.h  
 * 此头文件包含序列化函数的声明。  
 *  
 * 此文件是由工具 fastcdrgen 生成的。  
 */ 

#ifndef _FAST_DDS_GENERATED_GEOMETRY_MSGS_MSG_TWIST_PUBSUBTYPES_H_
#define _FAST_DDS_GENERATED_GEOMETRY_MSGS_MSG_TWIST_PUBSUBTYPES_H_

#include <fastdds/dds/topic/TopicDataType.hpp>// 引入 Fast DDS 主题数据类型的头文件
#include <fastrtps/utils/md5.h>// 引入 MD5 哈希的头文件

#include "Twist.h"// 引入 Twist 类型的定义

#include "Vector3PubSubTypes.h"// 引入 Vector3 类型的定义
// 检查生成的 API 版本是否与当前安装的 Fast DDS 兼容
#if !defined(GEN_API_VER) || (GEN_API_VER != 1)
#error \
    Generated Twist is not compatible with current installed Fast DDS. Please, regenerate it with fastddsgen.
#endif  // GEN_API_VER

namespace geometry_msgs
{
    namespace msg
    {

        #ifndef SWIG
        namespace detail {
// 用于获取 Twist 类中成员的偏移量
            template<typename Tag, typename Tag::type M>
            struct Twist_rob
            {
                friend constexpr typename Tag::type get(
                        Tag)
                {
                    return M;// 返回成员 M
                }
            };
// 定义 Twist_f 结构体，用于获取 Twist 类中的 Vector3 类型成员
            struct Twist_f
            {
                typedef geometry_msgs::msg::Vector3 Twist::* type;// 定义类型为 Twist 中的 Vector3 成员
                friend constexpr type get(
                        Twist_f);
            };
// 实例化 Twist_rob 以获取 m_angular 成员的偏移量
            template struct Twist_rob<Twist_f, &Twist::m_angular>;
// 获取类型 T 中 Tag 成员的偏移量
            template <typename T, typename Tag>
            inline size_t constexpr Twist_offset_of() {
                return ((::size_t) &reinterpret_cast<char const volatile&>((((T*)0)->*get(Tag()))));
            }
        }
        #endif

        /*!  
         * @brief 此类表示用户在 IDL 文件中定义的类型 Twist 的 TopicDataType。  
         * @ingroup TWIST  
         */
        class TwistPubSubType : public eprosima::fastdds::dds::TopicDataType
        {
        public:

            typedef Twist type;// 定义类型别名为 Twist

            eProsima_user_DllExport TwistPubSubType();// 构造函数

            eProsima_user_DllExport virtual ~TwistPubSubType() override;// 析构函数
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
// 创建数据实例
            eProsima_user_DllExport virtual void* createData() override;
// 删除数据实例
            eProsima_user_DllExport virtual void deleteData(
                    void* data) override;

        #ifdef TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED
// 检查数据类型是否有界
            eProsima_user_DllExport inline bool is_bounded() const override
            {
                return true;// 表示此数据类型是有界的
            }

        #endif  // TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED

        #ifdef TOPIC_DATA_TYPE_API_HAS_IS_PLAIN
// 检查数据类型是否是简单类型
            eProsima_user_DllExport inline bool is_plain() const override
            {
                return is_plain_impl();// 调用实现函数检查
            }

        #endif  // TOPIC_DATA_TYPE_API_HAS_IS_PLAIN
// 在给定内存中构造 Twist 对象
        #ifdef TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE
            eProsima_user_DllExport inline bool construct_sample(
                    void* memory) const override
            {
                new (memory) Twist();// 在给定内存中构造 Twist 对象
                return true;// 返回成功
            }

        #endif  // TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE
            MD5 m_md5;// MD5 哈希
            unsigned char* m_keyBuffer;// 键缓冲区
        private:
// 检查此数据类型是否是简单类型的实现
            static constexpr bool is_plain_impl()
            {
                return 48ULL == (detail::Twist_offset_of<Twist, detail::Twist_f>() + sizeof(geometry_msgs::msg::Vector3));

            }};
    }
}

#endif // _FAST_DDS_GENERATED_GEOMETRY_MSGS_MSG_TWIST_PUBSUBTYPES_H_
