// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE 2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*!
 * @file Float32PubSubTypes.h
 * 此头文件包含序列化函数的声明。
 * 该文件由 fastcdrgen 工具生成。
 */

// 防止重复包含此头文件，若未定义_FAST_DDS_GENERATED_STD_MSGS_MSG_FLOAT32_PUBSUBTYPES_H_则定义，若已定义则跳过此部分代码的重复编译
#ifndef _FAST_DDS_GENERATED_STD_MSGS_MSG_FLOAT32_PUBSUBTYPES_H_
#define _FAST_DDS_GENERATED_STD_MSGS_MSG_FLOAT32_PUBSUBTYPES_H_

// 包含 Fast DDS 中主题数据类型相关的头文件
#include <fastdds/dds/topic/TopicDataType.hpp>
// 包含用于计算 MD5 哈希值的相关头文件
#include <fastrtps/utils/md5.h>
// 包含 Float32 类型的定义所在头文件（推测是自定义的消息类型相关的头文件）
#include "Float32.h"

// 检查生成的代码版本与当前安装的 Fast DDS 是否兼容，如果不兼容则报错提示需要重新生成
#if!defined(GEN_API_VER) || (GEN_API_VER!= 1)
#error \
    Generated Float32 is not compatible with current installed Fast DDS. Please, regenerate it with fastddsgen.
#endif  // GEN_API_VER

// 定义在 std_msgs 命名空间下，用于组织相关消息类型相关的代码逻辑
namespace std_msgs {
    namespace msg {
        // 以下部分代码在非 SWIG（一种用于连接 C/C++ 与其他编程语言的工具相关的编译条件下）的情况进行定义
        #ifndef SWIG
        namespace detail {
            // 定义一个模板结构体 Float32_rob，用于通过标签获取特定类型的值
            template<typename Tag, typename Tag::type M>
            struct Float32_rob {
                // 友元函数，用于获取指定标签对应的类型的值
                friend constexpr typename Tag::type get(
                        Tag) {
                    return M;
                }
            };

            // 定义结构体 Float32_f，其内部定义了一个类型别名 type，用于指向 Float32 类中成员的指针类型（这里指向 float 类型成员）
            struct Float32_f {
                typedef float Float32::* type;
                // 友元函数声明，用于获取 Float32_f 对应的类型（即成员指针类型）
                friend constexpr type get(
                        Float32_f);
            };

            // 特化 Float32_rob 模板结构体，将其与 Float32 类中的 m_data 成员关联起来，用于后续通过标签获取该成员相关操作
            template struct Float32_rob<Float32_f, &Float32::m_data>;

            // 定义一个模板函数，用于计算给定类型中特定成员相对于该类型起始地址的偏移量
            template <typename T, typename Tag>
            inline size_t constexpr Float32_offset_of() {
                // 通过巧妙的指针运算获取成员的偏移量，先将空指针 0 转换为 T* 类型指针，再通过获取成员指针并转换为 char 类型的引用，最后取其地址得到偏移量
                return ((::size_t) &reinterpret_cast<char const volatile&>((((T*)0)->*get(Tag()))));
            }
        }
        #endif

        /*!
         * @brief 此类表示由用户在 IDL 文件中定义的 Float32 类型的主题数据类型（TopicDataType）。
         * @ingroup FLOAT32
         * 它继承自 eprosima::fastdds::dds::TopicDataType，意味着需要实现其中定义的一些虚函数来满足主题数据类型的相关操作要求，比如序列化、反序列化等。
         */
        class Float32PubSubType : public eprosima::fastdds::dds::TopicDataType {
        public:
            // 定义类型别名，表明此主题数据类型对应的实际数据类型为 Float32
            typedef Float32 type;

            // 构造函数，通常用于初始化一些内部状态等，导出供外部使用（可能涉及动态库相关的标记）
            eProsima_user_DllExport Float32PubSubType();

            // 析构函数，用于释放资源等清理操作，标记为虚函数以支持多态性，导出供外部使用（可能涉及动态库相关的标记）
            eProsima_user_DllExport virtual ~Float32PubSubType() override;

            // 序列化函数，将给定的数据（这里是 Float32 类型的数据）序列化为适合网络传输等的格式（存储在 SerializedPayload_t 结构体中），若成功序列化返回 true，否则返回 false，需实现此虚函数
            eProsima_user_DllExport virtual bool serialize(
                    void* data,
                    eprosima::fastrtps::rtps::SerializedPayload_t* payload) override;

            // 反序列化函数，将接收到的序列化数据（SerializedPayload_t 结构体中的内容）转换为实际的 Float32 类型的数据存储在给定的内存地址（data 指针指向的内存），若成功反序列化返回 true，否则返回 false，需实现此虚函数
            eProsima_user_DllExport virtual bool deserialize(
                    eprosima::fastrtps::rtps::SerializedPayload_t* payload,
                    void* data) override;

            // 获取序列化后数据大小的函数提供者，返回一个函数对象，调用该函数对象可以获取给定数据序列化后的大小（以字节为单位），需实现此虚函数
            eProsima_user_DllExport virtual std::function<uint32_t()> getSerializedSizeProvider(
                    void* data) override;

            // 获取数据键值的函数，根据给定的数据（这里是 Float32 类型的数据）生成一个实例句柄（InstanceHandle_t），用于标识数据实例等用途，可选择是否强制使用 MD5 算法来生成，需实现此虚函数
            eProsima_user_DllExport virtual bool getKey(
                    void* data,
                    eprosima::fastrtps::rtps::InstanceHandle_t* ihandle,
                    bool force_md5 = false) override;

            // 创建对应类型数据的内存空间，返回指向该内存空间的指针，需实现此虚函数
            eProsima_user_DllExport virtual void* createData() override;

            // 释放之前通过 createData 创建的数据内存空间，需实现此虚函数
            eProsima_user_DllExport virtual void deleteData(
                    void* data) override;

        #ifdef TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED
            // 判断此主题数据类型是否是有界的（例如数据长度是否固定等含义），如果宏定义了 TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED，则实现此内联函数，这里返回 true 表示是有界的
            eProsima_user_DllExport inline bool is_bounded() const override {
                return true;
            }

        #endif  // TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED

        #ifdef TOPIC_DATA_TYPE_API_HAS_IS_PLAIN
            // 判断此主题数据类型是否是简单的（具体含义由相关实现定义），如果宏定义了 TOPIC_DATA_TYPE_API_HAS_IS_PLAIN，则调用 is_plain_impl 函数来判断并返回结果
            eProsima_user_DllExport inline bool is_plain() const override {
                return is_plain_impl();
            }

        #endif  // TOPIC_DATA_TYPE_API_HAS_IS_PLAIN

        #ifdef TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE
            // 在给定的内存地址（memory 指针指向的内存）构造一个样本数据（这里就是创建一个 Float32 类型的对象），如果宏定义了 TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE，则实现此内联函数
            eProsima_user_DllExport inline bool construct_sample(
                    void* memory) const override {
                new (memory) Float32();
                return true;
            }

        #endif  // TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE

            // 用于存储 MD5 哈希值相关的对象，可能用于数据标识、校验等用途
            MD5 m_md5;
            // 用于存储键值缓冲区相关的指针，可能在生成键值等操作中使用
            unsigned char* m_keyBuffer;

        private:
            // 静态常量函数，用于判断此主题数据类型是否是简单的具体实现逻辑，通过比较成员偏移量和 float 类型大小等方式来判断
            static constexpr bool is_plain_impl() {
                return 4ULL == (detail::Float32_offset_of<Float32, detail::Float32_f>() + sizeof(float));
            }
        };
    }
}

// 结束头文件的条件编译定义，与开头的#ifndef 对应
#endif // _FAST_DDS_GENERATED_STD_MSGS_MSG_FLOAT32_PUBSUBTYPES_H_
