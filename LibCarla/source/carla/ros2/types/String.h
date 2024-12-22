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
 * @file String.h
 * 这个头文件包含了IDL文件中描述的类型的声明。
 *
 * 这个文件是由工具gen生成的。
 */

#ifndef _FAST_DDS_GENERATED_STD_MSGS_MSG_STRING_H_
#define _FAST_DDS_GENERATED_STD_MSGS_MSG_STRING_H_

#include <fastrtps/utils/fixed_size_string.hpp>

#include <stdint.h>
#include <array>
#include <string>
#include <vector>
#include <map>
#include <bitset>

#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#define eProsima_user_DllExport __declspec( dllexport )
#else
#define eProsima_user_DllExport
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define eProsima_user_DllExport
#endif  // _WIN32

#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#if defined(String_SOURCE)
#define String_DllAPI __declspec( dllexport )
#else
#define String_DllAPI __declspec( dllimport )
#endif // String_SOURCE
#else
#define String_DllAPI
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define String_DllAPI
#endif // _WIN32

namespace eprosima {
namespace fastcdr {
class Cdr;
} // namespace fastcdr
} // namespace eprosima

namespace std_msgs {
    namespace msg {
        /*!
         * @brief 这个类表示用户在IDL文件中定义的结构体String。
         * @ingroup STRING
         */
        class String
        {
        public:

            /*!
             * @brief 默认构造函数。
             */
            eProsima_user_DllExport String();

            /*!
             * @brief 默认析构函数。
             */
            eProsima_user_DllExport ~String();

            /*!
             * @brief 拷贝构造函数。
             * @param x 将被拷贝的std_msgs::msg::String对象的引用。
             */
            eProsima_user_DllExport String(
                    const String& x);

            /*!
             * @brief 移动构造函数。
             * @param x 将被移动的std_msgs::msg::String对象的引用。
             */
            eProsima_user_DllExport String(
                    String&& x) noexcept;

            /*!
             * @brief 拷贝赋值运算符。
             * @param x 将被拷贝的std_msgs::msg::String对象的引用。
             */
            eProsima_user_DllExport String& operator =(
                    const String& x);

            /*!
             * @brief 移动赋值运算符。
             * @param x 将被移动的std_msgs::msg::String对象的引用。
             */
            eProsima_user_DllExport String& operator =(
                    String&& x) noexcept;

            /*!
             * @brief 比较运算符。
             * @param x 要比较的std_msgs::msg::String对象。
             */
            eProsima_user_DllExport bool operator ==(
                    const String& x) const;

            /*!
             * @brief 比较运算符。
             * @param x 要比较的std_msgs::msg::String对象。
             */
            eProsima_user_DllExport bool operator !=(
                    const String& x) const;

            /*!
             * @brief 这个函数复制成员数据的值
             * @param _data 成员数据的新值，将被复制。
             */
            eProsima_user_DllExport void data(
                    const std::string& _data);

            /*!
             * @brief 这个函数移动成员数据的值
             * @param _data 成员数据的新值，将被移动。
             */
            eProsima_user_DllExport void data(
                    std::string&& _data);

            /*!
             * @brief 这个函数返回成员数据的常量引用
             * @return 成员数据的常量引用
             */
            eProsima_user_DllExport const std::string& data() const;

            /*!
             * @brief 这个函数返回成员数据的引用
             * @return 成员数据的引用
             */
            eProsima_user_DllExport std::string& data();

            /*!
            * @brief 这个函数返回对象的最大序列化大小
            * 取决于缓冲区对齐。
            * @param current_alignment 缓冲区对齐。
            * @return 最大序列化大小。
            */
            eProsima_user_DllExport static size_t getMaxCdrSerializedSize(
                    size_t current_alignment = 0);

            /*!
             * @brief 这个函数返回数据的序列化大小
             * 取决于缓冲区对齐。
             * @param data 计算其序列化大小的数据。
             * @param current_alignment 缓冲区对齐。
             * @return 序列化大小。
             */
            eProsima_user_DllExport static size_t getCdrSerializedSize(
                    const std_msgs::msg::String& data,
                    size_t current_alignment = 0);

            /*!
             * @brief 这个函数使用CDR序列化序列化对象。
             * @param cdr CDR序列化对象。
             */
            eProsima_user_DllExport void serialize(
                    eprosima::fastcdr::Cdr& cdr) const;

            /*!
             * @brief 这个函数使用CDR序列化反序列化对象。
             * @param cdr CDR序列化对象。
             */
            eProsima_user_DllExport void deserialize(
                    eprosima::fastcdr::Cdr& cdr);

            /*!
             * @brief 这个函数返回对象键的最大序列化大小
             * 取决于缓冲区对齐。
             * @param current_alignment 缓冲区对齐。
             * @return 最大序列化大小。
             */
            eProsima_user_DllExport static size_t getKeyMaxCdrSerializedSize(
                    size_t current_alignment = 0);

            /*!
             * @brief 这个函数告诉你这个类型是否定义了键
             */
            eProsima_user_DllExport static bool isKeyDefined();

            /*!
             * @brief 这个函数使用CDR序列化序列化对象的键成员。
             * @param cdr CDR序列化对象。
             */
            eProsima_user_DllExport void serializeKey(
                    eprosima::fastcdr::Cdr& cdr) const;

        private:
            std::string m_data;
        };
    } // namespace msg
} // namespace std_msgs

#endif // _FAST_DDS_GENERATED_STD_MSGS_MSG_STRING_H_
