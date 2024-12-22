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
 * @file Transform.h
 * 此头文件包含了在IDL文件中所描述类型的声明。
 * 该文件是由工具gen生成的。
 */
// 防止重复包含此头文件的宏定义，根据条件判断是否已经定义了_FAST_DDS_GENERATED_GEOMETRY_MSGS_MSG_TRANSFORM_H_，如果未定义则继续往下执行，定义之后的重复包含会被忽略
#ifndef _FAST_DDS_GENERATED_GEOMETRY_MSGS_MSG_TRANSFORM_H_
#define _FAST_DDS_GENERATED_GEOMETRY_MSGS_MSG_TRANSFORM_H_

// 包含Vector3头文件，可能是用于表示三维向量相关的数据结构或操作
#include "Vector3.h"
// 包含Quaternion头文件，通常用于表示四元数相关内容，常用于旋转等几何操作的表示
#include "Quaternion.h"

// 引入固定大小字符串相关的工具类，可能用于特定的字符串处理场景，比如有确定长度要求的字符串存储等
#include "fastrtps/utils/fixed_size_string.hpp"

// 引入C++标准库中的一些头文件，用于支持多种数据类型和容器操作
#include <stdint.h>
#include <array>
#include <string>
#include <vector>
#include <map>
#include <bitset>

// 在Windows平台下，根据是否定义了EPROSIMA_USER_DLL_EXPORT宏来定义eProsima_user_DllExport
#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#define eProsima_user_DllExport __declspec( dllexport )
#else
#define eProsima_user_DllExport
#endif  // EPROSIMA_USER_DLL_EXPORT
// 如果不是Windows平台，直接定义eProsima_user_DllExport为空（可能表示默认的链接属性之类的情况）
#else
#define eProsima_user_DllExport
#endif  // _WIN32

// 在Windows平台下，根据是否定义了EPROSIMA_USER_DLL_EXPORT以及Transform_SOURCE等条件来定义Transform_DllAPI，用于控制动态链接库相关的导入导出属性
#if defined(_WIN32)
#if defined(EPROSIMA_USER_DLL_EXPORT)
#if defined(Transform_SOURCE)
#define Transform_DllAPI __declspec( dllexport )
#else
#define Transform_DllAPI __declspec( dllimport )
#endif // Transform_SOURCE
#else
#define Transform_DllAPI
#endif  // EPROSIMA_USER_DLL_EXPORT
// 如果不是Windows平台，直接定义Transform_DllAPI为空（同样可能与链接相关设置有关）
#else
#define Transform_DllAPI
#endif // _WIN32

// eprosima命名空间，可能是整个代码库所在的顶层命名空间，里面可能包含多个子模块相关的内容
namespace eprosima {
// fastcdr命名空间，从名字看可能和某种数据的序列化、反序列化机制（CDR相关）有关，Cdr类应该是用于处理具体序列化操作的类
namespace fastcdr {
class Cdr;
} // namespace fastcdr
} // namespace eprosima

// geometry_msgs命名空间，通常用于存放和几何消息相关的类型定义等内容，msg可能是表示消息类型相关的子命名空间
namespace geometry_msgs {
    namespace msg {
        /*!
         * @brief 此类表示用户在IDL文件中定义的Transform结构。
         * @ingroup TRANSFORM
         */
        class Transform
        {
        public:
            /*!
             * @brief 默认构造函数，用于创建一个默认初始化状态的Transform对象。
             */
            eProsima_user_DllExport Transform();

            /*!
             * @brief 默认析构函数，用于释放对象占用的资源等清理操作。
             */
            eProsima_user_DllExport ~Transform();

            /*!
             * @brief 拷贝构造函数，用于从另一个Transform对象拷贝创建一个新的对象。
             * @param x 将要被拷贝的geometry_msgs::msg::Transform对象的引用。
             */
            eProsima_user_DllExport Transform(
                    const Transform& x);

            /*!
             * @brief 移动构造函数，用于通过“移动”语义从另一个Transform对象创建新对象（通常涉及资源所有权的转移，相比拷贝更高效）。
             * @param x 将要被移动的geometry_msgs::msg::Transform对象的引用。
             */
            eProsima_user_DllExport Transform(
                    Transform&& x) noexcept;

            /*!
             * @brief 拷贝赋值运算符重载，用于将一个Transform对象的值拷贝赋值给另一个对象。
             * @param x 将要被拷贝的geometry_msgs::msg::Transform对象的引用。
             */
            eProsima_user_DllExport Transform& operator =(
                    const Transform& x);

            /*!
             * @brief 移动赋值运算符重载，用于通过“移动”语义将一个Transform对象的值赋值给另一个对象（涉及资源所有权转移）。
             * @param x 将要被移动的geometry_msgs::msg::Transform对象的引用。
             */
            eProsima_user_DllExport Transform& operator =(
                    Transform&& x) noexcept;

            /*!
             * @brief 相等比较运算符重载，用于比较两个Transform对象是否相等。
             * @param x 要进行比较的geometry_msgs::msg::Transform对象。
             */
            eProsima_user_DllExport bool operator ==(
                    const Transform& x) const;

            /*!
             * @brief 不等比较运算符重载，用于比较两个Transform对象是否不相等。
             * @param x 要进行比较的geometry_msgs::msg::Transform对象。
             */
            eProsima_user_DllExport bool operator!=(
                    const Transform& x) const;

            /*!
             * @brief 此函数用于拷贝成员变量translation的值，将传入的新值拷贝到成员translation中。
             * @param _translation 要拷贝到成员translation的新值，类型为geometry_msgs::msg::Vector3。
             */
            eProsima_user_DllExport void translation(
                    const geometry_msgs::msg::Vector3& _translation);

            /*!
             * @brief 此函数用于移动成员变量translation的值，将传入的新值“移动”到成员translation中（资源所有权转移）。
             * @param _translation 要移动到成员translation的新值，类型为geometry_msgs::msg::Vector3。
             */
            eProsima_user_DllExport void translation(
                    geometry_msgs::msg::Vector3&& _translation);

            /*!
             * @brief 此函数返回成员变量translation的常量引用，外部可通过该引用访问成员变量translation的值，但不能修改它。
             * @return 成员变量translation的常量引用，类型为geometry_msgs::msg::Vector3。
             */
            eProsima_user_DllExport const geometry_msgs::msg::Vector3& translation() const;

            /*!
             * @brief 此函数返回成员变量translation的引用，外部可通过该引用访问并修改成员变量translation的值。
             * @return 成员变量translation的引用，类型为geometry_msgs::msg::Vector3。
             */
            eProsima_user_DllExport geometry_msgs::msg::Vector3& translation();

            /*!
             * @brief 此函数用于拷贝成员变量rotation的值，将传入的新值拷贝到成员rotation中。
             * @param _rotation 要拷贝到成员rotation的新值，类型为geometry_msgs::msg::Quaternion。
             */
            eProsima_user_DllExport void rotation(
                    const geometry_msgs::msg::Quaternion& _rotation);

            /*!
             * @brief 此函数用于移动成员变量rotation的值，将传入的新值“移动”到成员rotation中（资源所有权转移）。
             * @param _rotation 要移动到成员rotation的新值，类型为geometry_msgs::msg::Quaternion。
             */
            eProsima_user_DllExport void rotation(
                    geometry_msgs::msg::Quaternion&& _rotation);

            /*!
             * @brief 此函数返回成员变量rotation的常量引用，外部可通过该引用访问成员变量rotation的值，但不能修改它。
             * @return 成员变量rotation的常量引用，类型为geometry_msgs::msg::Quaternion。
             */
            eProsima_user_DllExport const geometry_msgs::msg::Quaternion& rotation() const;

            /*!
             * @brief 此函数返回成员变量rotation的引用，外部可通过该引用访问并修改成员变量rotation的值。
             * @return 成员变量rotation的引用，类型为geometry_msgs::msg::Quaternion。
             */
            eProsima_user_DllExport geometry_msgs::msg::Quaternion& rotation();

            /*!
            * @brief 此函数根据缓冲区对齐方式，返回一个对象的最大序列化大小。
            * @param current_alignment 缓冲区对齐方式，默认为0。
            * @return 最大序列化大小，类型为size_t。
            */
            eProsima_user_DllExport static size_t getMaxCdrSerializedSize(
                    size_t current_alignment = 0);

            /*!
             * @brief 此函数根据缓冲区对齐方式，返回给定数据的序列化大小。
             * @param data 要计算其序列化大小的数据，类型为geometry_msgs::msg::Transform。
             * @param current_alignment 缓冲区对齐方式，默认为0。
             * @return 序列化大小，类型为size_t。
             */
            eProsima_user_DllExport static size_t getCdrSerializedSize(
                    const geometry_msgs::msg::Transform& data,
                    size_t current_alignment = 0);

            /*!
             * @brief 此函数使用CDR序列化机制对对象进行序列化操作，将对象转换为适合传输或存储的格式（通常是二进制格式），参数cdr是用于执行序列化的对象。
             * @param cdr CDR序列化对象，类型为eprosima::fastcdr::Cdr。
             */
            eProsima_user_DllExport void serialize(
                    eprosima::fastcdr::Cdr& cdr) const;

            /*!
             * @brief 此函数使用CDR序列化机制对对象进行反序列化操作，将接收到的序列化数据还原为对象，参数cdr是包含序列化数据的对象。
             * @param cdr CDR序列化对象，类型为eprosima::fastcdr::Cdr。
             */
            eProsima_user_DllExport void deserialize(
                    eprosima::fastcdr::Cdr& cdr);

            /*!
             * @brief 此函数根据缓冲区对齐方式，返回对象的键（Key）的最大序列化大小。键通常用于在某些数据存储或查找场景中唯一标识对象。
             * @param current_alignment 缓冲区对齐方式，默认为0。
             * @return 最大序列化大小，类型为size_t。
             */
            eProsima_user_DllExport static size_t getKeyMaxCdrSerializedSize(
                    size_t current_alignment = 0);

            /*!
             * @brief 此函数用于判断对于该类型是否已经定义了键（Key）。
             */
            eProsima_user_DllExport static bool isKeyDefined();

            /*!
             * @brief 此函数使用CDR序列化机制对对象的键成员进行序列化操作，将键相关的数据转换为适合传输或存储的格式。
             * @param cdr CDR序列化对象，类型为eprosima::fastcdr::Cdr。
             */
            eProsima_user_DllExport void serializeKey(
                    eprosima::fastcdr::Cdr& cdr) const;

        private:
            // 成员变量，用于存储平移相关的信息，类型为geometry_msgs::msg::Vector3
            geometry_msgs::msg::Vector3 m_translation;
            // 成员变量，用于存储旋转相关的信息，类型为geometry_msgs::msg::Quaternion
            geometry_msgs::msg::Quaternion m_rotation;
        };
    } // namespace msg
} // namespace geometry_msgs

// 结束头文件的宏定义，防止重复包含
#endif // _FAST_DDS_GENERATED_GEOMETRY_MSGS_MSG_TRANSFORM_H_
