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
 * @file Vector3.h  
 * 此头文件包含在 IDL 文件中描述的类型的声明。  
 *  
 * 此文件是由工具 gen 生成的。  
 */ 

#ifndef _FAST_DDS_GENERATED_GEOMETRY_MSGS_MSG_VECTOR3_H_
#define _FAST_DDS_GENERATED_GEOMETRY_MSGS_MSG_VECTOR3_H_

#include <fastrtps/utils/fixed_size_string.hpp>// 包含固定大小字符串的定义

#include <stdint.h>// 包含整数类型定义
#include <array>// 包含数组的定义
#include <string>// 包含字符串的定义
#include <vector>// 包含向量的定义
#include <map>// 包含映射的定义
#include <bitset>// 包含位集的定义
// Windows 平台下的 DLL 导出宏定义
#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#define eProsima_user_DllExport __declspec( dllexport )// 导出符号
#else
#define eProsima_user_DllExport// 不导出符号
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define eProsima_user_DllExport// 不导出符号
#endif  // _WIN32
// Windows 平台下的 Vector3 DLL 导出宏定义
#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#if defined(Vector3_SOURCE)
#define Vector3_DllAPI __declspec( dllexport )// 导出符号
#else
#define Vector3_DllAPI __declspec( dllimport )// 导入符号
#endif // Vector3_SOURCE
#else
#define Vector3_DllAPI// 不导出符号
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define Vector3_DllAPI// 不导出符号
#endif // _WIN32

namespace eprosima {
namespace fastcdr {
class Cdr;// 前向声明 Cdr 类
} // namespace fastcdr
} // namespace eprosima


namespace geometry_msgs {
    namespace msg {
        /*!  
         * @brief 此类表示用户在 IDL 文件中定义的 Vector3 结构。  
         * @ingroup VECTOR3  
         */ 
        class Vector3
        {
        public:

            /*!  
             * @brief 默认构造函数。  
             */ 

            /*!  
             * @brief 默认析构函数。  
             */ 
            eProsima_user_DllExport ~Vector3();

            /*!  
             * @brief 复制构造函数。  
             * @param x 要复制的 geometry_msgs::msg::Vector3 对象的引用。  
             */
            eProsima_user_DllExport Vector3(
                    const Vector3& x);

            /*!  
             * @brief 移动构造函数。  
             * @param x 要移动的 geometry_msgs::msg::Vector3 对象的引用。  
             */ 
            eProsima_user_DllExport Vector3(
                    Vector3&& x) noexcept;

            /*!  
             * @brief 复制赋值运算符。  
             * @param x 要复制的 geometry_msgs::msg::Vector3 对象的引用。  
             */ 
            eProsima_user_DllExport Vector3& operator =(
                    const Vector3& x);

            /*!  
             * @brief 移动赋值运算符。  
             * @param x 要移动的 geometry_msgs::msg::Vector3 对象的引用。  
             */
            eProsima_user_DllExport Vector3& operator =(
                    Vector3&& x) noexcept;

            /*!  
             * @brief 比较运算符。  
             * @param x 要比较的 geometry_msgs::msg::Vector3 对象。  
             */ 
            eProsima_user_DllExport bool operator ==(
                    const Vector3& x) const;

            /*!  
             * @brief 比较运算符。  
             * @param x 要比较的 geometry_msgs::msg::Vector3 对象。  
             */ 
            eProsima_user_DllExport bool operator !=(
                    const Vector3& x) const;

            /*!  
             * @brief 设置成员 x 的值  
             * @param _x 成员 x 的新值  
             */
            eProsima_user_DllExport void x(
                    double _x);

            /*!  
             * @brief 返回成员 x 的值  
             * @return 成员 x 的值  
             */ 
            eProsima_user_DllExport double x() const;

            /*!  
             * @brief 返回对成员 x 的引用  
             * @return 对成员 x 的引用  
             */  
            eProsima_user_DllExport double& x();

            /*!  
             * @brief 设置成员 y 的值  
             * @param _y 成员 y 的新值  
             */ 
            eProsima_user_DllExport void y(
                    double _y);

            /*!  
             * @brief 返回成员 y 的值  
             * @return 成员 y 的值  
             */ 
            eProsima_user_DllExport double y() const;

            /*!  
             * @brief 返回对成员 y 的引用  
             * @return 对成员 y 的引用  
             */ 
            eProsima_user_DllExport double& y();

            /*!  
             * @brief 设置成员 z 的值  
             * @param _z 成员 z 的新值  
             */ 
            eProsima_user_DllExport void z(
                    double _z);

            /*!  
             * @brief 返回成员 z 的值  
             * @return 成员 z 的值  
             */ 
            eProsima_user_DllExport double z() const;

            /*!  
             * @brief 返回对成员 z 的引用  
             * @return 对成员 z 的引用  
             */
            eProsima_user_DllExport double& z();

            /*!  
            * @brief 此函数返回对象的最大序列化大小  
            * 取决于缓冲区对齐。  
            * @param current_alignment 缓冲区对齐。  
            * @return 最大序列化大小。  
            */ 
            eProsima_user_DllExport static size_t getMaxCdrSerializedSize(
                    size_t current_alignment = 0);

            /*!  
             * @brief 此函数返回数据的序列化大小  
             * 取决于缓冲区对齐。  
             * @param data 计算其序列化大小的数据。  
             * @param current_alignment 缓冲区对齐。  
             * @return 序列化大小。  
             */ 
            eProsima_user_DllExport static size_t getCdrSerializedSize(
                    const geometry_msgs::msg::Vector3& data,
                    size_t current_alignment = 0);

            /*!  
             * @brief 此函数使用 CDR 序列化序列化对象。  
             * @param cdr CDR 序列化对象。  
             */ 
            eProsima_user_DllExport void serialize(
                    eprosima::fastcdr::Cdr& cdr) const;

            /*!  
             * @brief 此函数使用 CDR 序列化反序列化对象。  
             * @param cdr CDR 序列化对象。  
             */ 
            eProsima_user_DllExport void deserialize(
                    eprosima::fastcdr::Cdr& cdr);

            /*!  
             * @brief 此函数返回对象键的最大序列化大小  
             * 取决于缓冲区对齐。  
             * @param current_alignment 缓冲区对齐。  
             * @return 最大序列化大小。  
             */
            eProsima_user_DllExport static size_t getKeyMaxCdrSerializedSize(
                    size_t current_alignment = 0);

            /*!  
             * @brief 此函数告诉您此类型是否定义了键  
             */
            eProsima_user_DllExport static bool isKeyDefined();

            /*!  
             * @brief 此函数使用 CDR 序列化序列化对象的键成员。  
             * @param cdr CDR 序列化对象。  
             */ 
            eProsima_user_DllExport void serializeKey(
                    eprosima::fastcdr::Cdr& cdr) const;

        private:
            double m_x;// 成员变量 x
            double m_y;// 成员变量 y
            double m_z;// 成员变量 z
        };
    } // namespace msg
} // namespace geometry_msgs

#endif // _FAST_DDS_GENERATED_GEOMETRY_MSGS_MSG_VECTOR3_H_
