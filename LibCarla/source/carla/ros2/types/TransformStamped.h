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
 * @file TransformStamped.h  
 * 此头文件包含在 IDL 文件中描述的类型的声明。  
 *  
 * 此文件是由工具 gen 生成的。  
 */

#ifndef _FAST_DDS_GENERATED_GEOMETRY_MSGS_MSG_TRANSFORMSTAMPED_H_// 防止重复包含
#define _FAST_DDS_GENERATED_GEOMETRY_MSGS_MSG_TRANSFORMSTAMPED_H_

#include "Header.h"// 引入 Header 头文件
#include "Transform.h"// 引入 Transform 头文件

#include <fastrtps/utils/fixed_size_string.hpp>// 引入 Fast RTPS 的固定大小字符串

#include <stdint.h>// 引入标准整数类型
#include <array>// 引入数组支持
#include <string>// 引入字符串支持
#include <vector>// 引入向量支持
#include <map>// 引入映射支持
#include <bitset>// 引入位集合支持

#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#define eProsima_user_DllExport __declspec( dllexport )// Windows 下导出符号
#else
#define eProsima_user_DllExport// 无导出符号
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define eProsima_user_DllExport// 无导出符号
#endif  // _WIN32

#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#if defined(TransformStamped_SOURCE)
#define TransformStamped_DllAPI __declspec( dllexport )// Windows 下导出 TransformStamped
#else
#define TransformStamped_DllAPI __declspec( dllimport )// Windows 下导入 TransformStamped
#endif // TransformStamped_SOURCE
#else
#define TransformStamped_DllAPI// 无导出符号
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define TransformStamped_DllAPI// 无导出符号
#endif // _WIN32

namespace eprosima {
namespace fastcdr {
class Cdr;// 前向声明 Cdr 类
} // namespace fastcdr
} // namespace eprosima

namespace geometry_msgs {
    namespace msg {
        /*!  
         * @brief 此类表示用户在 IDL 文件中定义的 TransformStamped 结构。  
         * @ingroup TRANSFORMSTAMPED  
         */
        class TransformStamped
        {
        public:

            /*!  
             * @brief 默认构造函数。  
             */
            eProsima_user_DllExport TransformStamped();

            /*!  
             * @brief 默认析构函数。  
             */ 
            eProsima_user_DllExport ~TransformStamped();

            /*!  
             * @brief 拷贝构造函数。  
             * @param x 将被复制的 geometry_msgs::msg::TransformStamped 对象的引用。  
             */ 
            eProsima_user_DllExport TransformStamped(
                    const TransformStamped& x);

            /*!  
             * @brief 移动构造函数。  
             * @param x 将被移动的 geometry_msgs::msg::TransformStamped 对象的引用。  
             */
            eProsima_user_DllExport TransformStamped(
                    TransformStamped&& x) noexcept;

            /*!  
             * @brief 拷贝赋值运算符。  
             * @param x 将被复制的 geometry_msgs::msg::TransformStamped 对象的引用。  
             */ 
            eProsima_user_DllExport TransformStamped& operator =(
                    const TransformStamped& x);

            /*!  
             * @brief 移动赋值运算符。  
             * @param x 将被移动的 geometry_msgs::msg::TransformStamped 对象的引用。  
             */
            eProsima_user_DllExport TransformStamped& operator =(
                    TransformStamped&& x) noexcept;

            /*!  
             * @brief 比较运算符。  
             * @param x 要比较的 geometry_msgs::msg::TransformStamped 对象。  
             */ 
            eProsima_user_DllExport bool operator ==(
                    const TransformStamped& x) const;

            /*!  
             * @brief 比较运算符。  
             * @param x 要比较的 geometry_msgs::msg::TransformStamped对象。
             */
            eProsima_user_DllExport bool operator !=(
                    const TransformStamped& x) const;

            /*!  
             * @brief 此函数复制成员 header 的值。  
             * @param _header 要复制到成员 header 的新值。  
             */
            eProsima_user_DllExport void header(
                    const std_msgs::msg::Header& _header);

            /*!  
             * @brief 此函数移动成员 header 的值。  
             * @param _header 要移动到成员 header 的新值。  
             */ 
            eProsima_user_DllExport void header(
                    std_msgs::msg::Header&& _header);

            /*!  
             * @brief 此函数返回成员 header 的常量引用。  
             * @return 常量引用到成员 header。  
             */
            eProsima_user_DllExport const std_msgs::msg::Header& header() const;

            /*!  
             * @brief 此函数返回成员 header 的引用。  
             * @return 引用到成员 header。  
             */
            eProsima_user_DllExport std_msgs::msg::Header& header();
            /*!  
             * @brief 此函数复制成员 child_frame_id 的值。  
             * @param _child_frame_id 要复制到成员 child_frame_id 的新值。  
             */
            eProsima_user_DllExport void child_frame_id(
                    const std::string& _child_frame_id);

            /*!  
            * @brief 此函数移动成员 child_frame_id 的值。  
             * @param _child_frame_id 要移动到成员 child_frame_id 的新值。  
             */
            eProsima_user_DllExport void child_frame_id(
                    std::string&& _child_frame_id);

            /*!  
             * @brief 此函数返回成员 child_frame_id 的常量引用。  
             * @return 常量引用到成员 child_frame_id。  
             */
            eProsima_user_DllExport const std::string& child_frame_id() const;

            /*!  
             * @brief 此函数返回成员 child_frame_id 的引用。  
             * @return 引用到成员 child_frame_id。  
             */
            eProsima_user_DllExport std::string& child_frame_id();
            /*!  
             * @brief 此函数复制成员 transform 的值。  
             * @param _transform 要复制到成员 transform 的新值。  
             */
            eProsima_user_DllExport void transform(
                    const geometry_msgs::msg::Transform& _transform);

            /*!  
             * @brief 此函数移动成员 transform 的值。  
             * @param _transform 要移动到成员 transform 的新值。  
             */ 
            eProsima_user_DllExport void transform(
                    geometry_msgs::msg::Transform&& _transform);

            /*!  
             * @brief 此函数返回成员 transform 的常量引用。  
             * @return 常量引用到成员 transform。  
             */
            eProsima_user_DllExport const geometry_msgs::msg::Transform& transform() const;

            /*!  
             * @brief 此函数返回成员 transform 的引用。  
             * @return 引用到成员 transform。  
             */ 
            eProsima_user_DllExport geometry_msgs::msg::Transform& transform();

            /*!  
             * @brief 此函数返回对象的最大序列化尺寸，取决于缓冲区对齐。  
             * @param current_alignment 缓冲区对齐。  
             * @return 最大序列化尺寸。  
             */
            eProsima_user_DllExport static size_t getMaxCdrSerializedSize(
                    size_t current_alignment = 0);

           /*!  
            * @brief 此函数返回数据的序列化尺寸，取决于缓冲区对齐。  
            * @param data 要计算序列化尺寸的数据。  
            * @param current_alignment 缓冲区对齐。  
            * @return 序列化尺寸。  
            */ 
            eProsima_user_DllExport static size_t getCdrSerializedSize(
                    const geometry_msgs::msg::TransformStamped& data,
                    size_t current_alignment = 0);

            /*!  
             * @brief 此函数使用 CDR 序列化对象序列化对象。  
             * @param cdr CDR 序列化对象。  
             */
            eProsima_user_DllExport void serialize(
                    eprosima::fastcdr::Cdr& cdr) const;

            /*!  
             * @brief 此函数使用 CDR 序列化对象反序列化对象。  
             * @param cdr CDR 序列化对象。  
             */
            eProsima_user_DllExport void deserialize(
                    eprosima::fastcdr::Cdr& cdr);

            /*!  
             * @brief 此函数返回对象键的最大序列化尺寸，取决于缓冲区对齐。  
             * @param current_alignment 缓冲区对齐。  
             * @return 最大序列化尺寸。  
             */ 
            eProsima_user_DllExport static size_t getKeyMaxCdrSerializedSize(
                    size_t current_alignment = 0);

            /*!  
             * @brief 此函数告知您是否已为此类型定义键。  
             */ 
            eProsima_user_DllExport static bool isKeyDefined();

            /*!  
             * @brief 此函数使用 CDR 序列化对象序列化对象的键成员。  
             * @param cdr CDR 序列化对象。  
             */
            eProsima_user_DllExport void serializeKey(
                    eprosima::fastcdr::Cdr& cdr) const;

        private:
            std_msgs::msg::Header m_header;// 消息头
            std::string m_child_frame_id;// 子框架 ID
            geometry_msgs::msg::Transform m_transform;// 转换数据

        };
    } // namespace msg
} // namespace geometry_msgs

#endif // _FAST_DDS_GENERATED_GEOMETRY_MSGS_MSG_TRANSFORMSTAMPED_H_
