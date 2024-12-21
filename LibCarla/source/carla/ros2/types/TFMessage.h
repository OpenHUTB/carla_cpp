// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not使用此文件，除非符合许可证要求。
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
 * @file TFMessage.h
 * 此头文件包含了在IDL文件（接口描述语言文件）中所描述类型的声明。
 * 该文件由名为“gen”的工具生成。
 */

#ifndef _FAST_DDS_GENERATED_TF2_MSGS_MSG_TFMESSAGE_H_
#define _FAST_DDS_GENERATED_TF2_MSGS_MSG_TFMESSAGE_H_

// 引入 TransformStamped.h 头文件，可能其中定义了 TransformStamped 类型，
// 该类型与 TFMessage 类中的成员变量相关，用于存储具体的变换信息
#include "TransformStamped.h"

// 引入 FastRTPS 库中用于处理固定大小字符串的头文件，
// 或许在类的实现或相关操作中会用到其提供的字符串处理功能
#include <fastrtps/utils/fixed_size_string.hpp>

// 引入 C++ 标准库中的一些常用头文件，用于支持后续代码中涉及的不同数据类型和容器操作
#include <stdint.h>
#include <array>
#include <string>
#include <vector>
#include <map>
#include <bitset>

// 以下是针对 Windows 平台下动态链接库相关的宏定义部分
// 如果在 Windows 平台且定义了 EPROSIMA_USER_DLL_EXPORT 宏，
// 则将 eProsima_user_DllExport 定义为 __declspec(dllexport)，用于导出函数或类，以便在动态链接库外部使用
#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#define eProsima_user_DllExport __declspec( dllexport )
#else
#define eProsima_user_DllExport
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define eProsima_user_DllExport
#endif  // _WIN32

// 同样是针对 Windows 平台下动态链接库相关的宏定义，根据不同条件定义 TFMessage_DllAPI
// 如果在 Windows 平台、定义了 EPROSIMA_USER_DLL_EXPORT 且定义了 TFMessage_SOURCE，
// 则将 TFMessage_DllAPI 定义为 __declspec(dllexport)，用于导出；
// 否则定义为 __declspec(dllimport)，用于导入，方便在不同模块间正确使用类或函数
#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#if defined(TFMessage_SOURCE)
#define TFMessage_DllAPI __declspec( dllexport )
#else
#define TFMessage_DllAPI __declspec( dllimport )
#endif // TFMessage_SOURCE
#else
#define TFMessage_DllAPI
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define TFMessage_DllAPI
#endif // _WIN32

// 在 eprosima 命名空间下的 fastcdr 子命名空间中前置声明 Cdr 类，
// Cdr 类在后续的序列化和反序列化操作中会起到关键作用，
// 这里只是声明，其具体定义应该在其他包含的相关头文件中提供
namespace eprosima {
namespace fastcdr {
class Cdr;
} // namespace fastcdr
} // namespace eprosima

namespace tf2_msgs {
    namespace msg {
        // TFMessage 类的定义，它代表了在 IDL 文件中由用户定义的 TFMessage 结构，
        // 通过 @ingroup TFMESSAGE 注释标记，可能用于特定的消息分组管理（例如文档生成或者代码模块化组织等方面）
        class TFMessage
        {
        public:
            // 默认构造函数声明，用于创建 TFMessage 类的对象实例，
            // 使用 eProsima_user_DllExport 宏标记，意味着在符合相应条件（如 Windows 平台动态链接库相关设置）时，
            // 该函数可以正确地被导出供外部使用，函数体的具体初始化逻辑未在此处体现，但可能涉及对成员变量的默认设置等操作
            eProsima_user_DllExport TFMessage();

            // 默认析构函数声明，用于释放 TFMessage 类对象占用的资源，
            // 虽然当前函数体为空，但在完整的类实现中可能会负责清理如动态分配内存等相关资源，
            // 确保对象生命周期结束时内存管理的正确性，同样使用相应宏标记以适配动态链接库相关情况
            eProsima_user_DllExport ~TFMessage();

            // 拷贝构造函数声明，根据传入的另一个 TFMessage 对象创建一个新的相同副本，
            // 参数 x 是一个 const 引用，保证不会修改传入的原对象，
            // 通过 eProsima_user_DllExport 宏标记用于动态链接库相关的函数导出，
            // 其内部实现应该会对原对象的各个成员变量进行深拷贝，以保证新对象与原对象的数据独立性
            eProsima_user_DllExport TFMessage(
                    const TFMessage& x);

            // 移动构造函数声明，通过移动语义将传入的 TFMessage 对象的资源所有权转移到新创建的对象中，
            // 这样可以提高性能，特别是对于包含大量数据或者复杂资源管理的对象，
            // 使用 noexcept 关键字表示该函数不会抛出异常（通常在移动操作能保证不抛异常的情况下使用），
            // 并用 eProsima_user_DllExport 宏标记以便在动态链接库相关场景下正确导出使用
            eProsima_user_DllExport TFMessage(
                    TFMessage&& x) noexcept;

            // 拷贝赋值运算符重载声明，用于将一个 TFMessage 对象的值赋给当前对象，
            // 遵循拷贝语义进行数据复制，使得当前对象获取传入对象的副本，
            // 通过 eProsima_user_DllExport 宏标记以符合动态链接库函数导出要求，
            // 实现中会逐个对成员变量进行赋值操作
            eProsima_user_DllExport TFMessage& operator =(
                    const TFMessage& x);

            // 移动赋值运算符重载声明，通过移动语义将一个 TFMessage 对象的资源所有权转移给当前对象，
            // 避免不必要的拷贝开销，提升赋值操作的效率，
            // 使用 noexcept 表示不会抛出异常，且通过 eProsima_user_DllExport 宏标记用于动态链接库相关使用，
            // 其操作会将传入对象的资源有效转移到当前对象中
            eProsima_user_DllExport TFMessage& operator =(
                    TFMessage&& x) noexcept;

            // 比较运算符 == 的重载声明，用于比较当前对象与传入的 TFMessage 对象是否相等，
            // 通过比较对象内部的各个成员变量（此处主要涉及 m_transforms 向量中的元素等）来确定整体对象是否相等，
            // 使用 eProsima_user_DllExport 宏标记用于动态链接库相关的函数导出，方便在代码中判断两个 TFMessage 对象的内容一致性
            eProsima_user_DllExport bool operator ==(
                    const TFMessage& x) const;

            // 比较运算符!= 的重载声明，通过调用 == 运算符并取反来判断当前对象与传入的 TFMessage 对象是否不相等，
            // 基于 == 运算符实现相反的比较逻辑，方便在不同场景下使用，同样使用相应宏标记用于动态链接库函数导出
            eProsima_user_DllExport bool operator!=(
                    const TFMessage& x) const;

            // 此函数用于拷贝成员变量 transforms 的值，
            // 传入一个新的 std::vector<geometry_msgs::msg::TransformStamped> 类型对象，
            // 将其内容复制给成员变量 m_transforms，遵循拷贝语义进行赋值，
            // 通过 eProsima_user_DllExport 宏标记以适配动态链接库相关的函数导出，方便外部更新该成员变量的值
            eProsima_user_DllExport void transforms(
                    const std::vector<geometry_msgs::msg::TransformStamped>& _transforms);

            // 此函数用于移动成员变量 transforms 的值，
            // 传入一个新的 std::vector<geometry_msgs::msg::TransformStamped> 类型对象，
            // 通过移动语义将其资源所有权转移给成员变量 m_transforms，提升性能（适用于临时对象等情况），
            // 使用 eProsima_user_DllExport 宏标记用于动态链接库相关函数导出，方便外部以移动方式更新该成员变量
            eProsima_user_DllExport void transforms(
                    std::vector<geometry_msgs::msg::TransformStamped>&& _transforms);

            // 此函数返回成员变量 transforms（即 m_transforms）的常量引用，
            // 外部可以通过该引用获取成员变量的值，但不能修改它，提供了一种只读的访问方式，保障数据的安全性，
            // 使用 eProsima_user_DllExport 宏标记用于动态链接库相关函数导出，便于外部代码获取变换信息但不改变其内容
            eProsima_user_DllExport const std::vector<geometry_msgs::msg::TransformStamped>& transforms() const;

            // 此函数返回成员变量 transforms（即 m_transforms）的普通引用，
            // 外部可以通过该引用获取和修改成员变量的值，提供了一种可读写的访问方式，方便外部根据需要更新成员变量内容，
            // 通过 eProsima_user_DllExport 宏标记用于动态链接库相关函数导出，使得外部能灵活操作变换信息
            eProsima_user_DllExport std::vector<geometry_msgs::msg::TransformStamped>& transforms();

            // 此静态函数用于获取对象根据缓冲区对齐情况的最大 CDR 序列化大小，
            // 参数 current_alignment 表示缓冲区对齐要求，默认值为 0，
            // 返回的最大序列化大小可用于在一些场景下（如分配缓冲区空间等）提前知晓该类型对象序列化后的最大可能长度，
            // 使用 eProsima_user_DllExport 宏标记用于动态链接库相关函数导出，方便在不同模块中调用获取该信息
            eProsima_user_DllExport static size_t getMaxCdrSerializedSize(
                    size_t current_alignment = 0);

            // 此静态函数用于获取给定数据（一个 TFMessage 类型对象）根据缓冲区对齐情况的 CDR 序列化大小，
            // 需要传入具体的 TFMessage 类型数据对象以及缓冲区对齐参数（默认为 0），
            // 返回的序列化大小可用于准确分配内存空间等操作，例如在进行网络传输或存储数据时确保有足够的缓冲区，
            // 通过 eProsima_user_DllExport 宏标记用于动态链接库相关函数导出，便于在相关代码中准确计算序列化长度
            eProsima_user_DllExport static size_t getCdrSerializedSize(
                    const tf2_msgs::msg::TFMessage& data,
                    size_t current_alignment = 0);

            // 此函数用于使用 CDR 序列化方式对对象进行序列化操作，
            // 传入一个 eprosima::fastcdr::Cdr 类型的对象（用于处理序列化的具体细节，如数据格式转换、写入缓冲区等操作），
            // 将当前 TFMessage 对象转换为适合网络传输或存储等的序列化格式，
            // 使用 eProsima_user_DllExport 宏标记用于动态链接库相关函数导出，确保在分布式通信场景下对象能正确序列化
            eProsima_user_DllExport void serialize(
                    eprosima::fastcdr::Cdr& cdr) const;

            // 此函数用于使用 CDR 序列化方式对对象进行反序列化操作，
            // 传入一个 eprosima::fastcdr::Cdr 类型的对象（用于处理反序列化的具体细节，如从缓冲区读取数据、数据格式还原等操作），
            // 将接收到的序列化数据还原为 TFMessage 类型的对象，
            // 使用 eProsima_user_DllExport 宏标记用于动态链接库相关函数导出，保障在分布式通信中能从序列化数据恢复出原始对象
            eProsima_user_DllExport void deserialize(
                    eprosima::fastcdr::Cdr& cdr);

            // 此静态函数用于获取对象的键（Key）根据缓冲区对齐情况的最大 CDR 序列化大小，
            // 参数 current_alignment 表示缓冲区对齐情况，默认值为 0，
            // 返回的大小可用于了解键相关的序列化特性（虽然目前可能只是预留接口，具体实现和用途依赖后续代码），
            // 通过 eProsima_user_DllExport 宏标记用于动态链接库相关函数导出，为后续键相关功能扩展做准备
            eProsima_user_DllExport static size_t getKeyMaxCdrSerializedSize(
                    size_t current_alignment = 0);

            // 此静态函数用于判断该类型是否已经定义了键（Key），返回一个 bool 值，
            // 用于在代码中知晓当前 TFMessage 类型是否具备键相关功能（同样可能是为后续扩展或者特定使用场景预留的接口），
            // 使用 eProsima_user_DllExport 宏标记用于动态链接库相关函数导出，方便在不同模块中根据此情况进行后续操作
            eProsima_user_DllExport static bool isKeyDefined();

            // 此函数用于使用 CDR 序列化方式对对象的键（Key）成员进行序列化操作，
            // 传入一个 eprosima::fastcdr::Cdr 类型的对象，将键相关的数据转换为序列化格式（目前可能只是预留接口，具体实现和作用依赖后续实际代码逻辑），
            // 使用 eProsima_user_DllExport 宏标记用于动态链接库相关函数导出，为可能的基于键的消息路由、过滤等功能扩展提供支持
            eProsima_user_DllExport void serializeKey(
                    eprosima::fastcdr::Cdr& cdr) const;

        private:
            // 定义一个私有成员变量 m_transforms，类型为 std::vector<geometry_msgs::msg::TransformStamped>，
            // 用于存储一系列的变换信息（可能是空间坐标变换等相关数据，具体依赖 TransformStamped 类型的定义），
            // 由于是私有成员，外部不能直接访问它，只能通过类提供的公有接口来间接操作该变量，增强了类的封装性和数据安全性
            std::vector<geometry_msgs::msg::TransformStamped> m_transforms;

        };
    } // namespace msg
} // namespace tf2_msgs

#endif // _FAST_DDS_GENERATED_TF2_MSGS_MSG_TFMESSAGE_H_
