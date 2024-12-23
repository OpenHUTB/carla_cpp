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
 * @file RegionOfInterestPubSubTypes.h  
 * 此头文件包含序列化函数的声明。  
 *  
 * 此文件是由工具 fastcdrgen 生成的。  
 */

#ifndef _FAST_DDS_GENERATED_SENSOR_MSGS_MSG_REGIONOFINTEREST_PUBSUBTYPES_H_// 防止重复包含
#define _FAST_DDS_GENERATED_SENSOR_MSGS_MSG_REGIONOFINTEREST_PUBSUBTYPES_H_

#include <fastdds/dds/topic/TopicDataType.hpp>// 引入 Fast DDS 主题数据类型
#include <fastrtps/utils/md5.h>// 引入 MD5 哈希支持

#include "RegionOfInterest.h"// 引入 RegionOfInterest 结构的定义

#if !defined(GEN_API_VER) || (GEN_API_VER != 1)// 确保与 Fast DDS 版本兼容
#error \
    Generated RegionOfInterest is not compatible with current installed Fast DDS. Please, regenerate it with fastddsgen.
#endif  // GEN_API_VER

namespace sensor_msgs
{
    namespace msg
    {

        #ifndef SWIG
        namespace detail {

            template<typename Tag, typename Tag::type M>
            struct RegionOfInterest_rob
            {
                friend constexpr typename Tag::type get(
                        Tag)
                {
                    return M;
                }
            };

            struct RegionOfInterest_f
            {
                typedef bool RegionOfInterest::* type;// 定义 RegionOfInterest 中 bool 类型的成员
                friend constexpr type get(
                        RegionOfInterest_f);
            };

            template struct RegionOfInterest_rob<RegionOfInterest_f, &RegionOfInterest::m_do_rectify>;

            template <typename T, typename Tag>
            inline size_t constexpr RegionOfInterest_offset_of() {
                return ((::size_t) &reinterpret_cast<char const volatile&>((((T*)0)->*get(Tag()))));
            }
        }
        #endif

        /*!  
         * @brief 此类表示用户在 IDL 文件中定义的 RegionOfInterest 类型的 TopicDataType。  
         * @ingroup RegionOfInterest  
         */
        class RegionOfInterestPubSubType : public eprosima::fastdds::dds::TopicDataType
        {
        public:

            typedef RegionOfInterest type;// 定义类型别名

            eProsima_user_DllExport RegionOfInterestPubSubType();// 构造函数

            eProsima_user_DllExport virtual ~RegionOfInterestPubSubType() override;// 析构函数

            eProsima_user_DllExport virtual bool serialize(
                    void* data,
                    eprosima::fastrtps::rtps::SerializedPayload_t* payload) override;// 序列化函数

            eProsima_user_DllExport virtual bool deserialize(
                    eprosima::fastrtps::rtps::SerializedPayload_t* payload,
                    void* data) override;// 反序列化函数

            eProsima_user_DllExport virtual std::function<uint32_t()> getSerializedSizeProvider(
                    void* data) override;// 获取序列化大小的提供者

            eProsima_user_DllExport virtual bool getKey(
                    void* data,
                    eprosima::fastrtps::rtps::InstanceHandle_t* ihandle,
                    bool force_md5 = false) override;// 获取键值

            eProsima_user_DllExport virtual void* createData() override;// 创建数据

            eProsima_user_DllExport virtual void deleteData(
                    void* data) override;// 删除数据

        #ifdef TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED
            eProsima_user_DllExport inline bool is_bounded() const override
            {
                return true;// 返回是否是有界的
            }

        #endif  // TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED

        #ifdef TOPIC_DATA_TYPE_API_HAS_IS_PLAIN
            eProsima_user_DllExport inline bool is_plain() const override
            {
                return is_plain_impl();// 返回是否是简单类型
            }

        #endif  // TOPIC_DATA_TYPE_API_HAS_IS_PLAIN

        #ifdef TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE
            eProsima_user_DllExport inline bool construct_sample(
                    void* memory) const override
            {
                new (memory) RegionOfInterest();// 在给定内存中构造样本
                return true;
            }

        #endif  // TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE

            MD5 m_md5;// MD5 哈希
            unsigned char* m_keyBuffer;// 键缓冲区

        private:

            static constexpr bool is_plain_impl()
            {
                return 17ULL == (detail::RegionOfInterest_offset_of<RegionOfInterest, detail::RegionOfInterest_f>() + sizeof(bool));

            }};
    }
}

#endif // _FAST_DDS_GENERATED_SENSOR_MSGS_MSG_REGIONOFINTEREST_PUBSUBTYPES_H_
