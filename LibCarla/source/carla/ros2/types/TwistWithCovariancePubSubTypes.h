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
 * @file TwistWithCovariancePubSubTypes.h  
 * 此头文件包含序列化函数的声明。  
 *  
 * 此文件是由工具 fastcdrgen 生成的。  
 */ 

#ifndef _FAST_DDS_GENERATED_GEOMETRY_MSGS_MSG_TWISTWITHCOVARIANCE_PUBSUBTYPES_H_
#define _FAST_DDS_GENERATED_GEOMETRY_MSGS_MSG_TWISTWITHCOVARIANCE_PUBSUBTYPES_H_

#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastrtps/utils/md5.h>

#include "TwistWithCovariance.h"
#include "TwistPubSubTypes.h"
// 检查生成的 API 版本是否与当前安装的 Fast DDS 兼容
#if !defined(GEN_API_VER) || (GEN_API_VER != 1)
#error \
    生成的 TwistWithCovariance 与当前安装的 Fast DDS 不兼容。请使用 fastddsgen 重新生成它。
#endif  // GEN_API_VER

namespace geometry_msgs
{
    namespace msg
    {
        typedef std::array<double, 36> geometry_msgs__TwistWithCovariance__double_array_36;// 定义一个包含 36 个 double 的数组类型

        #ifndef SWIG
        namespace detail {
// 模板结构，用于获取 m_covariance 的偏移量
            template<typename Tag, typename Tag::type M>
            struct TwistWithCovariance_rob
            {
                friend constexpr typename Tag::type get(
                        Tag)
                {
                    return M;
                }
            };

            struct TwistWithCovariance_f
            {
                typedef geometry_msgs::msg::geometry_msgs__TwistWithCovariance__double_array_36 TwistWithCovariance::* type;
                friend constexpr type get(
                        TwistWithCovariance_f);
            };
// 实例化模板以获取 m_covariance 的偏移量
            template struct TwistWithCovariance_rob<TwistWithCovariance_f, &TwistWithCovariance::m_covariance>;
// 获取偏移量的辅助函数
            template <typename T, typename Tag>
            inline size_t constexpr TwistWithCovariance_offset_of() {
                return ((::size_t) &reinterpret_cast<char const volatile&>((((T*)0)->*get(Tag()))));
            }
        }
        #endif

        /*!  
         * @brief 此类表示用户在 IDL 文件中定义的 TwistWithCovariance 的 TopicDataType。  
         * @ingroup TWISTWITHCOVARIANCE  
         */
        class TwistWithCovariancePubSubType : public eprosima::fastdds::dds::TopicDataType
        {
        public:

            typedef TwistWithCovariance type;// 定义类型别名

            eProsima_user_DllExport TwistWithCovariancePubSubType();// 构造函数

            eProsima_user_DllExport virtual ~TwistWithCovariancePubSubType() override;// 析构函数

            eProsima_user_DllExport virtual bool serialize(
                    void* data,
                    eprosima::fastrtps::rtps::SerializedPayload_t* payload) override;// 序列化函数

            eProsima_user_DllExport virtual bool deserialize(
                    eprosima::fastrtps::rtps::SerializedPayload_t* payload,
                    void* data) override;// 反序列化函数

            eProsima_user_DllExport virtual std::function<uint32_t()> getSerializedSizeProvider(
                    void* data) override;// 获取序列化大小的函数

            eProsima_user_DllExport virtual bool getKey(
                    void* data,
                    eprosima::fastrtps::rtps::InstanceHandle_t* ihandle,
                    bool force_md5 = false) override;// 获取键的函数

            eProsima_user_DllExport virtual void* createData() override;// 创建数据的函数

            eProsima_user_DllExport virtual void deleteData(
                    void* data) override;// 删除数据的函数 

        #ifdef TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED
            eProsima_user_DllExport inline bool is_bounded() const override
            {
                return true;// 判断是否有界
            }

        #endif  // TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED

        #ifdef TOPIC_DATA_TYPE_API_HAS_IS_PLAIN
            eProsima_user_DllExport inline bool is_plain() const override
            {
                return is_plain_impl();// 判断是否为平面类型
            }

        #endif  // TOPIC_DATA_TYPE_API_HAS_IS_PLAIN

        #ifdef TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE
            eProsima_user_DllExport inline bool construct_sample(
                    void* memory) const override
            {
                new (memory) TwistWithCovariance();// 在内存中构造样本
                return true;
            }

        #endif  // TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE
            MD5 m_md5;// MD5 哈希值
            unsigned char* m_keyBuffer;// 键缓冲区
        private:// 判断是否为平面类型的实现
            static constexpr bool is_plain_impl()
            {
                return 336ULL == (detail::TwistWithCovariance_offset_of<TwistWithCovariance, detail::TwistWithCovariance_f>() + sizeof(geometry_msgs::msg::geometry_msgs__TwistWithCovariance__double_array_36));

            }};
    }
}

#endif // _FAST_DDS_GENERATED_GEOMETRY_MSGS_MSG_TWISTWITHCOVARIANCE_PUBSUBTYPES_H_
