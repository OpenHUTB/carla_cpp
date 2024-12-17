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
 * @file Twist.h
 * This header file contains the declaration of the described types in the IDL file.
 *
 * This file was generated by the tool gen.
 */

#ifndef _FAST_DDS_GENERATED_GEOMETRY_MSGS_MSG_TWIST_H_
#define _FAST_DDS_GENERATED_GEOMETRY_MSGS_MSG_TWIST_H_

#include "Vector3.h"// 包含Vector3类的头文件，Vector3类用于表示三维向量

#include <fastrtps/utils/fixed_size_string.hpp>// 包含一些用于序列化的工具类
// 包含一些标准库头文件
#include <stdint.h>
#include <array>
#include <string>
#include <vector>
#include <map>
#include <bitset>
// Windows平台下的DLL导出宏定义
#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#define eProsima_user_DllExport __declspec( dllexport )
#else
#define eProsima_user_DllExport
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define eProsima_user_DllExport
#endif  // _WIN32
// Windows平台下的DLL导入/导出宏定义
#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#if defined(Twist_SOURCE)
#define Twist_DllAPI __declspec( dllexport )
#else
#define Twist_DllAPI __declspec( dllimport )
#endif // Twist_SOURCE
#else
#define Twist_DllAPI
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define Twist_DllAPI
#endif // _WIN32
// 引入fastcdr命名空间，用于序列化
namespace eprosima {
namespace fastcdr {
class Cdr;
} // namespace fastcdr
} // namespace eprosima
// 定义geometry_msgs命名空间，其中包含msg子命名空间
namespace geometry_msgs {
    namespace msg {
        //这个类表示用户在 IDL 文件中定义的 Twist 结构
        class Twist
        {
        public:

            //默认构造函数
            eProsima_user_DllExport Twist();

            //默认构造函数
            eProsima_user_DllExport ~Twist();

            //复制构造函数
            //将要被复制的对象 geometry_msgs::msg::Twist 的引用
            eProsima_user_DllExport Twist(
                    const Twist& x);

            //移动构造函数
            //将要被移动的对象 geometry_msgs::msg::Twist 的引用
            eProsima_user_DllExport Twist(
                    Twist&& x) noexcept;

            //复制赋值操作符
            //将要被复制的对象 geometry_msgs::msg::Twist 的引用
            eProsima_user_DllExport Twist& operator =(
                    const Twist& x);

            //移动赋值操作符
            //将要被移动的对象 geometry_msgs::msg::Twist 的引用
            eProsima_user_DllExport Twist& operator =(
                    Twist&& x) noexcept;

            比较操作符
            //要比较的 geometry_msgs::msg::Twist 对象
            eProsima_user_DllExport bool operator ==(
                    const Twist& x) const;
            //比较运算符
            //要进行比较的 geometry_msgs::msg::Twist 对象
            eProsima_user_DllExport bool operator !=(
                    const Twist& x) const;

            //此函数将值复制到成员 linear 中
            //要复制到成员 linear 中的新值
            eProsima_user_DllExport void linear(
                    const geometry_msgs::msg::Vector3& _linear);

            //此函数将值移动到成员 linear 中
            //要移动到成员 linear 中的新值
            eProsima_user_DllExport void linear(
                    geometry_msgs::msg::Vector3&& _linear);

            //此函数返回对成员 linear 的常量引用
            //对成员 linear 的常量引用
            eProsima_user_DllExport const geometry_msgs::msg::Vector3& linear() const;

            //此函数返回对成员 linear 的引用
            //对成员 linear 的引用
            eProsima_user_DllExport geometry_msgs::msg::Vector3& linear();
            //此函数将值复制到成员 angular 中
            //要复制到成员 angular 中的新值
            eProsima_user_DllExport void angular(
                    const geometry_msgs::msg::Vector3& _angular);

            //此函数将值移动到成员 angular 中
            //要移动到成员 angular 中的新值
            eProsima_user_DllExport void angular(
                    geometry_msgs::msg::Vector3&& _angular);

            //此函数返回对成员 angular 的常量引用
            //对成员 angular 的常量引用
            eProsima_user_DllExport const geometry_msgs::msg::Vector3& angular() const;

            //此函数返回对成员 angular 的引用
            //对成员 angular 的引用
            eProsima_user_DllExport geometry_msgs::msg::Vector3& angular();

            //此函数返回根据缓冲区对齐方式的对象的最大序列化大小
            //current_alignment 缓冲区对齐方式
            //最大序列化大小
            eProsima_user_DllExport static size_t getMaxCdrSerializedSize(
                    size_t current_alignment = 0);

            //此函数返回根据缓冲区对齐方式的数据的序列化大小
            //要计算其序列化大小的数据
            //current_alignment 缓冲区对齐方式
            //序列化大小
            eProsima_user_DllExport static size_t getCdrSerializedSize(
                    const geometry_msgs::msg::Twist& data,
                    size_t current_alignment = 0);

            //此函数使用 CDR 序列化来序列化一个对象
            //cdr CDR 序列化对象
            eProsima_user_DllExport void serialize(
                    eprosima::fastcdr::Cdr& cdr) const;

            //此函数使用 CDR 序列化来反序列化一个对象
            //cdr CDR 序列化对象
            eProsima_user_DllExport void deserialize(
                    eprosima::fastcdr::Cdr& cdr);

            //此函数返回根据缓冲区对齐方式的对象键的最大序列化大小
            //current_alignment 缓冲区对齐方式
            //最大序列化大小
            eProsima_user_DllExport static size_t getKeyMaxCdrSerializedSize(
                    size_t current_alignment = 0);

            //此函数告诉你是否为这种类型定义了键（Key）
            eProsima_user_DllExport static bool isKeyDefined();

            //此函数使用 CDR 序列化来序列化一个对象的键成员
            //cdr CDR 序列化对象
            eProsima_user_DllExport void serializeKey(
                    eprosima::fastcdr::Cdr& cdr) const;

        private:
            geometry_msgs::msg::Vector3 m_linear;
            geometry_msgs::msg::Vector3 m_angular;
        };
    } // namespace msg
} // namespace geometry_msgs

#endif // _FAST_DDS_GENERATED_GEOMETRY_MSGS_MSG_TWIST_H_
