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
 * @file TF2Error.h
 * 此头文件包含了在IDL文件（接口描述语言文件）中所描述类型的声明。
 * 该文件由名为“gen”的工具生成。
 */

#ifndef _FAST_DDS_GENERATED_TF2_MSGS_MSG_TF2ERROR_H_
#define _FAST_DDS_GENERATED_TF2_MSGS_MSG_TF2ERROR_H_

// 引入FastRTPS库中用于处理固定大小字符串的头文件，
// 可能在类的某些操作或者成员变量处理中会用到其提供的功能
#include <fastrtps/utils/fixed_size_string.hpp>

// 引入C++标准库中的一些常用头文件，用于支持后续代码中涉及的不同数据类型和容器操作
#include <stdint.h>
#include <array>
#include <string>
#include <vector>
#include <map>
#include <bitset>

// 以下是针对Windows平台下动态链接库相关的宏定义部分
// 如果在Windows平台且定义了EPROSIMA_USER_DLL_EXPORT宏，
// 则将eProsima_user_DllExport定义为__declspec(dllexport)，用于导出函数或类，以便在动态链接库外部使用
#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#define eProsima_user_DllExport __declspec( dllexport )
#else
#define eProsima_user_DllExport
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define eProsima_user_DllExport
#endif  // _WIN32

// 同样是针对Windows平台下动态链接库相关的宏定义，根据不同条件定义TF2Error_DllAPI
// 如果在Windows平台、定义了EPROSIMA_USER_DLL_EXPORT且定义了TF2Error_SOURCE，
// 则将TF2Error_DllAP定义为__declspec(dllexport)，用于导出；
// 否则定义为__declspec(dllimport)，用于导入，方便在不同模块间正确使用类或函数
#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#if defined(TF2Error_SOURCE)
#define TF2Error_DllAPI __declspec( dllexport )
#else
#define TF2Error_DllAPI __declspec( dllimport )
#endif // TF2Error_SOURCE
#else
#define TF2Error_DllAPI
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define TF2Error_DllAPI
#endif // _WIN32

// 在eprosima命名空间下的fastcdr子命名空间中前置声明Cdr类，
// Cdr类在后续的序列化和反序列化操作中会起到关键作用，
// 这里只是声明，其具体定义应该在其他包含的相关头文件中提供
namespace eprosima {
namespace fastcdr {
class Cdr;
} // namespace fastcdr
} // namespace eprosima

namespace tf2_msgs {
    namespace msg {

        // 定义一些表示不同错误类型的常量，这些常量可能用于标识TF2相关操作中出现的具体错误情况，方便代码中进行错误判断和处理
        const uint8_t TF2Error__NO_ERROR = 0;
        const uint8_t TF2Error__LOOKUP_ERROR = 1;
        const uint8_t TF2Error__CONNECTIVITY_ERROR = 2;
        const uint8_t TF2Error__EXTRAPOLATION_ERROR = 3;
        const uint8_t TF2Error__INVALID_ARGUMENT_ERROR = 4;
        const uint8_t TF2Error__TIMEOUT_ERROR = 5;
        const uint8_t TF2Error__TRANSFORM_ERROR = 6;

        // TF2Error类的定义，它代表了在IDL文件中由用户定义的TF2Error结构，
        // 通过 @ingroup TF2ERROR注释标记，可能用于特定的消息分组管理（例如文档生成或者代码模块化组织等方面）
        class TF2Error
        {
        public:
            // 默认构造函数声明，用于创建TF2Error类的对象实例，
            // 使用eProsima_user_DllExport宏标记，意味着在符合相应条件（如Windows平台动态链接库相关设置）时，
            // 该函数可以正确地被导出供外部使用，函数体的具体初始化逻辑未在此处体现，但可能涉及对成员变量的默认设置等操作
            eProsima_user_DllExport TF2Error();

            // 默认析构函数声明，用于释放TF2Error类对象占用的资源，
            // 虽然当前函数体为空，但在完整的类实现中可能会负责清理如动态分配内存等相关资源，
            // 确保对象生命周期结束时内存管理的正确性，同样使用相应宏标记以适配动态链接库相关情况
            eProsima_user_DllExport ~TF2Error();

            // 拷贝构造函数声明，根据传入的另一个TF2Error对象创建一个新的相同副本，
            // 参数x是一个const引用，保证不会修改传入的原对象，
            // 通过eProsima_user_DllExport宏标记用于动态链接库相关的函数导出，
            // 其内部实现应该会对原对象的各个成员变量进行深拷贝，以保证新对象与原对象的数据独立性
            eProsima_user_DllExport TF2Error(
                    const TF2Error& x);

            // 移动构造函数声明，通过移动语义将传入的TF2Error对象的资源所有权转移到新创建的对象中，
            // 这样可以提高性能，特别是对于包含大量数据或者复杂资源管理的对象，
            // 使用noexcept关键字表示该函数不会抛出异常（通常在移动操作能保证不抛异常的情况下使用），
            // 并用eProsima_user_DllExport宏标记以便在动态链接库相关场景下正确导出使用
            eProsima_user_DllExport TF2Error(
                    TF2Error&& x) noexcept;

            // 拷贝赋值运算符重载声明，用于将一个TF2Error对象的值赋给当前对象，
            // 遵循拷贝语义进行数据复制，使得当前对象获取传入对象的副本，
            // 通过eProsima_user_DllExport宏标记以符合动态链接库函数导出要求，
            // 实现中会逐个对成员变量进行赋值操作
            eProsima_user_DllExport TF2Error& operator =(
                    const TF2Error& x);

            // 移动赋值运算符重载声明，通过移动语义将一个TF2Error对象的资源所有权转移给当前对象，
            // 避免不必要的拷贝开销，提升赋值操作的效率，
            // 使用noexcept表示不会抛出异常，且通过eProsima_user_DllExport宏标记用于动态链接库相关使用，
            // 其操作会将传入对象的资源有效转移到当前对象中
            eProsima_user_DllExport TF2Error& operator =(
                    TF2Error&& x) noexcept;

            // 比较运算符==的重载声明，用于比较当前对象与传入的TF2Error对象是否相等，
            // 通过比较对象内部的各个成员变量（此处主要涉及m_error和m_error_string等成员）来确定整体对象是否相等，
            // 使用eProsima_user_DllExport宏标记用于动态链接库相关的函数导出，方便在代码中判断两个TF2Error对象的内容一致性
            eProsima_user_DllExport bool operator ==(
                    const TF2Error& x) const;

            // 比较运算符!=的重载声明，通过调用==运算符并取反来判断当前对象与传入的TF2Error对象是否不相等，
            // 基于==运算符实现相反的比较逻辑，方便在不同场景下使用，同样使用相应宏标记用于动态链接库函数导出
            eProsima_user_DllExport bool operator!=(
                    const TF2Error& x) const;

            // 此函数用于设置成员变量error（即m_error）的值，
            // 传入一个新的uint8_t类型的值作为参数，用于更新成员变量error的值，方便外部代码根据实际情况设置错误代码，
            // 通过eProsima_user_DllExport宏标记以适配动态链接库相关的函数导出
            eProsima_user_DllExport void error(
                    uint8_t _error);

            // 此函数用于返回成员变量error（即m_error）的值，
            // 提供了一种只读的访问方式，外部代码可以获取当前对象中存储的错误代码，用于错误判断等操作，
            // 使用eProsima_user_DllExport宏标记用于动态链接库相关函数导出
            eProsima_user_DllExport uint8_t error() const;

            // 此函数用于返回成员变量error（即m_error）的引用，
            // 外部代码可以通过该引用获取和修改成员变量error的值，提供了一种可读写的访问方式，方便外部根据需要更新错误代码，
            // 通过eProsima_user_DllExport宏标记用于动态链接库相关函数导出
            eProsima_user_DllExport uint8_t& error();

            // 此函数用于拷贝成员变量error_string（即m_error_string）的值，
            // 传入一个新的std::string类型的对象，将其内容复制给成员变量error_string，遵循拷贝语义进行赋值，
            // 通过eProsima_user_DllExport宏标记以适配动态链接库相关的函数导出，方便外部更新该成员变量的错误描述信息
            eProsima_user_DllExport void error_string(
                    const std::string& _error_string);

            // 此函数用于移动成员变量error_string（即m_error_string）的值，
            // 传入一个新的std::string类型的对象，通过移动语义将其资源所有权转移给成员变量error_string，提升性能（适用于临时对象等情况），
            // 使用eProsima_user_DllExport宏标记用于动态链接库相关函数导出，方便外部以移动方式更新该成员变量的错误描述信息
            eProsima_user_DllExport void error_string(
                    std::string&& _error_string);

            // 此函数返回成员变量error_string（即m_error_string）的常量引用，
            // 外部可以通过该引用获取成员变量的值，但不能修改它，提供了一种只读的访问方式，保障数据的安全性，
            // 使用eProsima_user_DllExport宏标记用于动态链接库相关函数导出，便于外部代码获取错误描述信息但不改变其内容
            eProsima_user_DllExport const std::string& error_string() const;

            // 此函数返回成员变量error_string（即m_error_string）的普通引用，
            // 外部可以通过该引用获取和修改成员变量的值，提供了一种可读写的访问方式，方便外部根据需要更新成员变量内容，
            // 通过eProsima_user_DllExport宏标记用于动态链接库相关函数导出，使得外部能灵活操作错误描述信息
            eProsima_user_DllExport std::string& error_string();

            // 此静态函数用于获取对象根据缓冲区对齐情况的最大CDR序列化大小，
            // 参数current_alignment表示缓冲区对齐要求，默认值为0，
            // 返回的最大序列化大小可用于在一些场景下（如分配缓冲区空间等）提前知晓该类型对象序列化后的最大可能长度，
            // 使用eProsima_user_DllExport宏标记用于动态链接库相关函数导出，方便在不同模块中调用获取该信息
            eProsima_user_DllExport static size_t getMaxCdrSerializedSize(
                    size_t current_alignment = 0);

            // 此静态函数用于获取给定数据（一个TF2Error类型对象）根据缓冲区对齐情况的CDR序列化大小，
            // 需要传入具体的TF2Error类型数据对象以及缓冲区对齐参数（默认为0），
            // 返回的序列化大小可用于准确分配内存空间等操作，例如在进行网络传输或存储数据时确保有足够的缓冲区，
            // 通过eProsima_user_DllExport宏标记用于动态链接库相关函数导出，便于在相关代码中准确计算序列化长度
            eProsima_user_DllExport static size_t getCdrSerializedSize(
                    const tf2_msgs::msg::TF2Error& data,
                    size_t current_alignment = 0);

            // 此函数用于使用CDR序列化方式对对象进行序列化操作，
            // 传入一个eprosima::fastcdr::Cdr类型的对象（用于处理序列化的具体细节，如数据格式转换、写入缓冲区等操作），
            // 将当前TF2Error对象转换为适合网络传输或存储等的序列化格式，
            // 使用eProsima_user_DllExport宏标记用于动态链接库相关函数导出，确保在分布式通信场景下对象能正确序列化
            eProsima_user_DllExport void serialize(
                    eprosima::fastcdr::Cdr& cdr) const;

            // 此函数用于使用CDR序列化方式对对象进行反序列化操作，
            // 传入一个eprosima::fastcdr::Cdr类型的对象（用于处理反序列化的具体细节，如从缓冲区读取数据、数据格式还原等操作），
            // 将接收到的序列化数据还原为TF2Error类型的对象，
            // 使用eProsima_user_DllExport宏标记用于动态链接库相关函数导出，保障在分布式通信中能从序列化数据恢复出原始对象
            eProsima_user_DllExport void deserialize(
                    eprosima::fastcdr::Cdr& cdr);

            // 此静态函数用于获取对象的键（Key）根据缓冲区对齐情况的最大CDR序列化大小，
            // 参数current_alignment表示缓冲区对齐情况，默认值为0，
            // 返回的大小可用于了解键相关的序列化特性（虽然目前可能只是预留接口，具体实现和用途依赖后续代码），
            // 通过eProsima_user_DllExport宏标记用于动态链接库相关函数导出，为后续键相关功能扩展做准备
            eProsima_user_DllExport static size_t getKeyMaxCdrSerializedSize(
                    size_t current_alignment = 0);

            // 此静态函数用于判断该类型是否已经定义了键（Key），返回一个bool值，
            // 用于在代码中知晓当前TF2Error类型是否具备键相关功能（同样可能是为后续扩展或者特定使用场景预留的接口），
            // 使用eProsima_user_DllExport宏标记用于动态链接库相关函数导出，方便在不同模块中根据此情况进行后续操作
            eProsima_user_DllExport static bool isKeyDefined();

            // 此函数用于使用CDR序列化方式对对象的键（Key）成员进行序列化操作，
            // 传入一个eprosima::fastcdr::Cdr类型的对象，将键相关的数据转换为序列化格式（目前可能只是预留接口，具体实现和作用依赖后续实际代码逻辑），
            // 使用eProsima_user_DllExport宏标记用于动态链接库相关函数导出，为可能的基于键的消息路由、过滤等功能扩展提供支持
            eProsima_user_DllExport void serializeKey(
                    eprosima::fastcdr::Cdr& cdr) const;

        private:
            // 定义一个私有成员变量m_error，类型为uint8_t，用于存储错误代码，代表具体的错误类型，
            // 外部代码不能直接访问它，需要通过类提供的公有接口来操作该变量，增强了类的封装性和数据安全性
            uint8_t m_error;

            // 定义一个私有成员变量m_error_string，类型为std::string，用于存储与错误相关的详细描述信息，
            // 同样外部代码不能直接访问，需借助公有接口进行读写等操作，保证了数据的封装性
            std::string m_error_string;
        };
    } // namespace msg
} // namespace tf2_msgs

#endif // _FAST_DDS_GENERATED_TF2_MSGS_MSG_TF2ERROR_H_
