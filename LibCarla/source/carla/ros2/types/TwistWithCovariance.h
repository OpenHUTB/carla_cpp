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
 * @file TwistWithCovariance.h  
 * 此头文件包含在 IDL 文件中描述的类型的声明。  
 *  
 * 此文件是由工具 gen 生成的。  
 */  

#ifndef _FAST_DDS_GENERATED_GEOMETRY_MSGS_MSG_TWISTWITHCOVARIANCE_H_
#define _FAST_DDS_GENERATED_GEOMETRY_MSGS_MSG_TWISTWITHCOVARIANCE_H_

#include "Twist.h"// 包含 Twist.h 头文件，定义了 Twist 类型

#include <fastrtps/utils/fixed_size_string.hpp>// Fast DDS 的固定大小字符串工具

#include <stdint.h>// 包含整数类型的定义
#include <array>// 包含数组的定义
#include <string>
#include <vector>
#include <map>
#include <bitset>
// Windows 平台的 DLL 导出定义
#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#define eProsima_user_DllExport __declspec( dllexport )
#else
#define eProsima_user_DllExport
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define eProsima_user_DllExport
#endif  // _WIN32
// Windows 平台的 DLL API 定义
#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#if defined(TwistWithCovariance_SOURCE)
#define TwistWithCovariance_DllAPI __declspec( dllexport )
#else
#define TwistWithCovariance_DllAPI __declspec( dllimport )
#endif // TwistWithCovariance_SOURCE
#else
#define TwistWithCovariance_DllAPI
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define TwistWithCovariance_DllAPI
#endif // _WIN32

namespace eprosima {
namespace fastcdr {
class Cdr;// 前向声明 Cdr 类，用于序列化和反序列化
} // namespace fastcdr
} // namespace eprosima


namespace geometry_msgs {
    namespace msg {
        typedef std::array<double, 36> geometry_msgs__TwistWithCovariance__double_array_36;
        /*!  
         * @brief 此类表示用户在 IDL 文件中定义的 TwistWithCovariance 结构。  
         * @ingroup TWISTWITHCOVARIANCE  
         */ 
        class TwistWithCovariance
        {
        public:

            /*!  
             * @brief 默认构造函数。  
             */  
            eProsima_user_DllExport TwistWithCovariance();

            /*!  
             * @brief 默认析构函数。  
             */ 
            eProsima_user_DllExport ~TwistWithCovariance();

            /*!  
             * @brief 复制构造函数。  
             * @param x 要复制的 geometry_msgs::msg::TwistWithCovariance 对象的引用。  
             */ 
            eProsima_user_DllExport TwistWithCovariance(
                    const TwistWithCovariance& x);

            /*!  
             * @brief 移动构造函数。  
             * @param x 要复制的 geometry_msgs::msg::TwistWithCovariance 对象的引用。  
             */ 
            eProsima_user_DllExport TwistWithCovariance(
                    TwistWithCovariance&& x) noexcept;

            /*!  
             * @brief 复制赋值运算符。  
             * @param x 要复制的 geometry_msgs::msg::TwistWithCovariance 对象的引用。  
             */
            eProsima_user_DllExport TwistWithCovariance& operator =(
                    const TwistWithCovariance& x);

            /*!  
             * @brief 移动赋值运算符。  
             * @param x 要复制的 geometry_msgs::msg::TwistWithCovariance 对象的引用。  
             */ 
            eProsima_user_DllExport TwistWithCovariance& operator =(
                    TwistWithCovariance&& x) noexcept;

            /*!  
             * @brief 比较运算符。  
             * @param x 要比较的 geometry_msgs::msg::TwistWithCovariance 对象。  
             * @return 如果两个对象相等，则返回 true，否则返回 false。  
             */ 
            eProsima_user_DllExport bool operator ==(
                    const TwistWithCovariance& x) const;

            /*!  
             * @brief 不等于运算符。  
             * @param x 要比较的 geometry_msgs::msg::TwistWithCovariance 对象。  
             * @return 如果两个对象不等，则返回 true，否则返回 false。  
             */ 
            eProsima_user_DllExport bool operator !=(
                    const TwistWithCovariance& x) const;

            /*!  
             * @brief 复制成员变量 twist 的值。  
             * @param _twist 要复制的新值。  
             */
            eProsima_user_DllExport void twist(
                    const geometry_msgs::msg::Twist& _twist);

            /*!  
             * @brief 移动成员变量 twist 的值。  
             * @param _twist 要移动的新值。  
             */
            eProsima_user_DllExport void twist(
                    geometry_msgs::msg::Twist&& _twist);

            /*!  
             * @brief 返回对成员变量 twist 的常量引用。  
             * @return 对成员变量 twist 的常量引用。  
             */
            eProsima_user_DllExport const geometry_msgs::msg::Twist& twist() const;

            /*!  
             * @brief 返回对成员变量 twist 的引用。  
             * @return 对成员变量 twist 的引用。  
             */ 
            eProsima_user_DllExport geometry_msgs::msg::Twist& twist();
            /*!  
             * @brief 复制成员变量 covariance 的值。  
             * @param _covariance 要复制的新值。  
             */
            eProsima_user_DllExport void covariance(
                    const geometry_msgs::msg::geometry_msgs__TwistWithCovariance__double_array_36& _covariance);

            /*!  
             * @brief 移动成员变量 covariance 的值。  
             * @param _covariance 要移动的新值。  
             */ 
            eProsima_user_DllExport void covariance(
                    geometry_msgs::msg::geometry_msgs__TwistWithCovariance__double_array_36&& _covariance);

            /*!  
             * @brief 返回对成员变量 covariance 的常量引用。  
             * @return 对成员变量 covariance 的常量引用。  
             */ 
            eProsima_user_DllExport const geometry_msgs::msg::geometry_msgs__TwistWithCovariance__double_array_36& covariance() const;

            /*!  
             * @brief 返回对成员变量 covariance 的引用。  
             * @return 对成员变量 covariance 的引用。  
             */
            eProsima_user_DllExport geometry_msgs::msg::geometry_msgs__TwistWithCovariance__double_array_36& covariance();

            /*!  
             * @brief 返回对象的最大序列化大小（取决于缓冲区对齐）。  
             * @param current_alignment 当前对齐。  
             * @return 最大序列化大小。  
             */ 
            eProsima_user_DllExport static size_t getMaxCdrSerializedSize(
                    size_t current_alignment = 0);

            /*!  
             * @brief 返回数据的序列化大小（取决于缓冲区对齐）。  
             * @param data 要计算其序列化大小的数据。  
             * @param current_alignment 当前对齐。  
             * @return 序列化大小。  
             */ 
            eProsima_user_DllExport static size_t getCdrSerializedSize(
                    const geometry_msgs::msg::TwistWithCovariance& data,
                    size_t current_alignment = 0);

            /*!  
             * @brief 使用 CDR 序列化序列化对象。  
             * @param cdr CDR 序列化对象。  
             */
            eProsima_user_DllExport void serialize(
                    eprosima::fastcdr::Cdr& cdr) const;

            /*!  
             * @brief 使用 CDR 反序列化对象。  
             * @param cdr CDR 反序列化对象。  
             */
            eProsima_user_DllExport void deserialize(
                    eprosima::fastcdr::Cdr& cdr);

            /*!  
             * @brief 返回对象键的最大序列化大小（取决于缓冲区对齐）。  
             * @param current_alignment 当前对齐。  
             * @return 最大序列化大小。  
             */ 
            eProsima_user_DllExport static size_t getKeyMaxCdrSerializedSize(
                    size_t current_alignment = 0);

            /*!  
             * @brief 告诉您此类型的键是否已定义。  
             * @return 如果已定义键，则返回 true，否则返回 false。  
             */ 
            eProsima_user_DllExport static bool isKeyDefined();

            /*!  
             * @brief 使用 CDR 序列化对象的键成员。  
             * @param cdr CDR 序列化对象。  
             */  
            eProsima_user_DllExport void serializeKey(
                    eprosima::fastcdr::Cdr& cdr) const;

        private:
            geometry_msgs::msg::Twist m_twist;// 速度信息，包含线速度和角速度
            geometry_msgs::msg::geometry_msgs__TwistWithCovariance__double_array_36 m_covariance;
        };
    } // namespace msg
} // namespace geometry_msgs

#endif // _FAST_DDS_GENERATED_GEOMETRY_MSGS_MSG_TWISTWITHCOVARIANCE_H_
