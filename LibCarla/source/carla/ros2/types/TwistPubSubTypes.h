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
//定义了一个名为geometry_msgs的命名空间
namespace geometry_msgs
{
//在geometry_msgs命名空间内又定义了一个名为msg的嵌套命名空间，进一步细化代码的组织结构
    namespace msg
    {
//这是一个预处理器指令 如果SWIG未被定义，那么从这一行开始到对应的# 如果SWIG已经被定义，这部分代码将被跳过。这通常用于防止头文件的多重包含
        #ifndef SWIG
//定义了一个名为detail的嵌套命名空间，这是在msg命名空间内部的更深层次的命名空间，用于将特定功能相关的代码分组
        namespace detail {
// 用于获取 Twist 类中成员的偏移量
//定义一个模板，有两个模板参数 用于根据不同的类型来获取特定的类型信息
            template<typename Tag, typename Tag::type M>
           //开始定义一个名为Twist_rob的结构体模板
            struct Twist_rob
            {
             //在Twist_rob结构体模板内定义了一个友元函数get，它是一个常量表达式函数（constexpr） 这里通过Tag模板类型来确定返回类型
                friend constexpr typename Tag::type get(
                        Tag)     //接受一个类型为Tag的参数
                {
                    //这里的M是在模板参数中定义的类型，它应该是与Twist类成员相关的某种类型
                    return M;// 返回成员 M
                }
            };
// 定义 Twist_f 结构体，用于获取 Twist 类中的 Vector3 类型成员
            struct Twist_f
            {
                //定义了一个类型别名type，它表示Twist类中的geometry_msgs::msg::Vector3类型成员的指针类型
                typedef geometry_msgs::msg::Vector3 Twist::* type;// 定义类型为 Twist 中的 Vector3 成员
                //定义了一个友元常量表达式函数get，返回类型是前面定义的type
                friend constexpr type get(
                        Twist_f);
            };
// 实例化 Twist_rob 以获取 m_angular 成员的偏移量
            template struct Twist_rob<Twist_f, &Twist::m_angular>;
// 获取类型 T 中 Tag 成员的偏移量
//定义了一个名为Twist_offset_of的函数模板，有两个模板参数T和Tag
            template <typename T, typename Tag>
           //开始定义Twist_offset_of函数模板，它是一个内联（inline）的常量表达式函数（constexpr），返回类型为size_t
            inline size_t constexpr Twist_offset_of() {
                //计算T类型中Tag成员的偏移量 最后获取这个结果的地址并转换为size_t类型返回
                return ((::size_t) &reinterpret_cast<char const volatile&>((((T*)0)->*get(Tag()))));
            }
        }
        #endif

        /*!  
         * @brief 此类表示用户在 IDL 文件中定义的类型 Twist 的 TopicDataType。  
         * @ingroup TWIST  
         */
//定义了一个名为TwistPubSubType的类，它继承自eprosima::fastdds::dds::TopicDataType类
        class TwistPubSubType : public eprosima::fastdds::dds::TopicDataType
        {
        public:

//构造函数用于创建类的对象时进行初始化操作
            typedef Twist type;// 定义类型别名为 Twist

            eProsima_user_DllExport TwistPubSubType();// 构造函数

//定义了析构函数
//析构函数用于在对象生命周期结束时进行清理操作，比如释放构造函数中分配的资源等，同样这里没有给出具体的实现细节
            eProsima_user_DllExport virtual ~TwistPubSubType() override;// 析构函数
// 序列化数据
//将data转换为一种适合传输或者存储的格式，存储到payload中
// 它返回一个bool类型，表示序列化是否成功。如果序列化过程中出现错误，比如数据格式不正确、内存不足等，可能会返回false
            eProsima_user_DllExport virtual bool serialize(
                    void* data,
                    eprosima::fastrtps::rtps::SerializedPayload_t* payload) override;
// 反序列化数据
// 这个函数与序列化函数相反，它将payload（已经序列化的数据）转换为原始的数据格式，并存储到data中
//同样返回一个bool类型
            eProsima_user_DllExport virtual bool deserialize(
                    eprosima::fastrtps::rtps::SerializedPayload_t* payload,
                    void* data) override;
// 获取序列化大小提供者
//这个函数的目的是获取数据序列化后的大小 它接受一个数据指针data，根据数据内容返回序列化后的大小
//返回值是一个std::function类型，可以用来灵活地表示一个可调用对象，计算序列化大小
            eProsima_user_DllExport virtual std::function<uint32_t()> getSerializedSizeProvider(
                    void* data) override;
// 获取数据的键
            eProsima_user_DllExport virtual bool getKey(
                    void* data,
                    eprosima::fastrtps::rtps::InstanceHandle_t* ihandle,
//有一个默认参数force_md5，如果设置为true，可能会强制使用MD5算法之类的方式来获取键值 
//函数返回一个bool值，表示是否成功获取键值
                    bool force_md5 = false) override;
// 创建数据实例 
//返回一个指向创建的数据实例的指针。返回的指针类型为void*，可以在后续进行类型转换来使用具体的数据类型
            eProsima_user_DllExport virtual void* createData() override;
// 删除数据实例
//它接受一个指向数据实例的指针data，释放该数据实例占用的资源
            eProsima_user_DllExport virtual void deleteData(
                    void* data) override;

        #ifdef TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED
// 检查数据类型是否有界
// eProsima_user_DllExport用于在动态链接库（DLL）场景下，使这个函数可以被外部模块使用  inline表示这个函数是内联函数
            eProsima_user_DllExport inline bool is_bounded() const override
            {
                return true;// 表示此数据类型是有界的
            }
//预处理器指令的结束部分
        #endif  // TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED

//一个预处理器指令
        #ifdef TOPIC_DATA_TYPE_API_HAS_IS_PLAIN
// 检查数据类型是否是简单类型
            eProsima_user_DllExport inline bool is_plain() const override
            {
                return is_plain_impl();// 调用实现函数检查
            }
//与前面的#ifdef对应，结束条件编译部分
        #endif  // TOPIC_DATA_TYPE_API_HAS_IS_PLAIN
// 在给定内存中构造 Twist 对象
        #ifdef TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE
            eProsima_user_DllExport inline bool construct_sample(
// 函数接受一个void*类型的参数memory，这个指针可能指向一块已经分配好的内存区域 表示函数不修改成员变量且重写基类虚函数
                    void* memory) const override
            {
                new (memory) Twist();// 在给定内存中构造 Twist 对象
                return true;// 返回成功
            }

        #endif  // TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE
//定义了一个名为m_md5的MD5类型的成员变量  用于处理MD5哈希相关的操作，这个成员变量可能用于存储与数据相关的MD5哈希值
            MD5 m_md5;// MD5 哈希
// 定义了一个名为m_keyBuffer的无符号字符指针类型的成员变量
            unsigned char* m_keyBuffer;// 键缓冲区
        private:
// 检查此数据类型是否是简单类型的实现
            static constexpr bool is_plain_impl()
            {
                //detail::Twist_f>() 调用Twist_offset_of函数模传递Twist和detail::Twist_f作为模板参数用于计算Twist类型中与detail::Twist_f相关的偏移量  sizeof(geometry获取geometry相关类型的大小
                //进行一个比较操作(48ULL是一个无符号长整型常量)
                return 48ULL == (detail::Twist_offset_of<Twist, detail::Twist_f>() + sizeof(geometry_msgs::msg::Vector3));

            }};
    }
}

//一个预处理器指令的结束部分，与前面的#ifndef对应
#endif // _FAST_DDS_GENERATED_GEOMETRY_MSGS_MSG_TWIST_PUBSUBTYPES_H_
