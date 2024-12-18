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
// 请参阅许可证以获取有关权限和限制的具体语言。  

/*!  
 * @file PointPubSubTypes.h  
 * 此头文件包含对点类型的序列化函数的声明。  
 *  
 * 此文件是由工具 fastcdrgen 生成的。  
 */  

#ifndef _FAST_DDS_GENERATED_GEOMETRY_MSGS_MSG_POINT_PUBSUBTYPES_H_  
#define _FAST_DDS_GENERATED_GEOMETRY_MSGS_MSG_POINT_PUBSUBTYPES_H_  

#include <fastdds/dds/topic/TopicDataType.hpp> // 引入 Fast DDS 主题数据类型头文件  
#include <fastrtps/utils/md5.h> // 引入 MD5 工具以进行校验和计算  

#include "Point.h" // 引入 Point 结构的定义  

// 检查 API 版本兼容性  
#if !defined(GEN_API_VER) || (GEN_API_VER != 1)  
#error \
    生成的 Point 与当前安装的 Fast DDS 不兼容。请使用 fastddsgen 重新生成。  
#endif  // GEN_API_VER  

namespace geometry_msgs  
{  
    namespace msg  
    {  
        #ifndef SWIG  
        namespace detail {  

            // 辅助模板，用于检索 Point 的成员变量  
            template<typename Tag, typename Tag::type M>  
            struct Point_rob  
            {  
                friend constexpr typename Tag::type get(  
                        Tag)  
                {  
                    return M; // 返回成员变量  
                }  
            };  

            // Point 成员变量的标记结构  
            struct Point_f  
            {  
                typedef double Point::* type; // 定义类型为指向 Point 的 double 成员的指针  
                friend constexpr type get(  
                        Point_f); // 友元函数以访问成员  
            };  

            // 显式特化以访问 Point 的 m_z 成员  
            template struct Point_rob<Point_f, &Point::m_z>;  

            // 计算 Point 结构中成员变量的偏移量的函数  
            template <typename T, typename Tag>  
            inline size_t constexpr Point_offset_of() {  
                return ((::size_t) &reinterpret_cast<char const volatile&>((((T*)0)->*get(Tag()))));  
            }  
        }  
        #endif  

        /*!  
         * @brief 此类表示用户在 IDL 文件中定义的 Point 类型的 TopicDataType。  
         * @ingroup POINT  
         */  
        class PointPubSubType : public eprosima::fastdds::dds::TopicDataType  
        {  
        public:  

            typedef Point type; // 将类型定义为 Point  

            eProsima_user_DllExport PointPubSubType(); // 构造函数  

            eProsima_user_DllExport virtual ~PointPubSubType() override; // 析构函数  

            // 将 Point 数据序列化到有效负载中  
            eProsima_user_DllExport virtual bool serialize(  
                    void* data,  
                    eprosima::fastrtps::rtps::SerializedPayload_t* payload) override;  

            // 将有效负载反序列化回 Point 数据  
            eProsima_user_DllExport virtual bool deserialize(  
                    eprosima::fastrtps::rtps::SerializedPayload_t* payload,  
                    void* data) override;  

            // 提供获取序列化数据大小的函数  
            eProsima_user_DllExport virtual std::function<uint32_t()> getSerializedSizeProvider(  
                    void* data) override;  

            // 获取 Point 实例的键  
            eProsima_user_DllExport virtual bool getKey(  
                    void* data,  
                    eprosima::fastrtps::rtps::InstanceHandle_t* ihandle,  
                    bool force_md5 = false) override;  

            // 创建一个新的 Point 实例  
            eProsima_user_DllExport virtual void* createData() override;  

            // 删除 Point 实例  
            eProsima_user_DllExport virtual void deleteData(  
                    void* data) override;  

        #ifdef TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED  
            // 检查 Point 类型是否是有界的  
            eProsima_user_DllExport inline bool is_bounded() const override  
            {  
                return true; // Point 是有界类型  
            }  

        #endif  // TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED  

        #ifdef TOPIC_DATA_TYPE_API_HAS_IS_PLAIN  
            // 检查 Point 类型是否是简单类型（没有嵌套类型）  
            eProsima_user_DllExport inline bool is_plain() const override  
            {  
                return is_plain_impl(); // 调用实现函数  
            }  

        #endif  // TOPIC_DATA_TYPE_API_HAS_IS_PLAIN  

        #ifdef TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE  
            // 在提供的内存中构造 Point 的样本  
            eProsima_user_DllExport inline bool construct_sample(  
                    void* memory) const override  
            {  
                new (memory) Point(); // 在指定内存中使用 placement new 创建 Point  
                return true; // 表示成功  
            }  

        #endif  // TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE  

            MD5 m_md5; // 用于数据完整性的 MD5 校验和  
            unsigned char* m_keyBuffer; // 实例键的缓冲区  

        private:  

            // 确定 Point 是否是简单类型的实现  
            static constexpr bool is_plain_impl()  
            {  
                return 24ULL == (detail::Point_offset_of<Point, detail::Point_f>() + sizeof(double));  
            } // 检查大小是否与预期值匹配  
        };  
    }  
}  

#endif // _FAST_DDS_GENERATED_GEOMETRY_MSGS_MSG_POINT_PUBSUBTYPES_H_  
