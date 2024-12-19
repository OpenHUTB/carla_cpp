// 版权 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).  
//  
// 根据Apache许可证第2.0版（“许可证”）授权；  
// 除非遵守许可证，否则您不得使用此文件。  
// 您可以在以下地址获取许可证副本：  
//  
//     http://www.apache.org/licenses/LICENSE-2.0  
//  
// 除非适用法律要求或书面同意，  
// 根据许可证分发的软件在“按原样”基础上分发，  
// 不提供任何形式的保证或条件，无论是明示的还是暗示的。  
// 有关许可证下的特定权限和限制，请参见许可证。  

/*!  
 * @file Pose.h  
 * 此头文件包含IDL文件中描述的类型的声明。  
 *  
 * 此文件由工具gen生成。  
 */  

#ifndef _FAST_DDS_GENERATED_GEOMETRY_MSGS_MSG_POSE_H_  
#define _FAST_DDS_GENERATED_GEOMETRY_MSGS_MSG_POSE_H_  

#include "Point.h"       // 包含Point类的头文件  
#include "Quaternion.h"  // 包含Quaternion类的头文件  

#include <fastrtps/utils/fixed_size_string.hpp>  

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

// Pose类的DLL导出宏定义  
#if defined(_WIN32)  
#if defined(EPROSIMA_USER_DLL_EXPORT)  
#if defined(Pose_SOURCE)  
#define Pose_DllAPI __declspec( dllexport )  
#else  
#define Pose_DllAPI __declspec( dllimport )  
#endif // Pose_SOURCE  
#else  
#define Pose_DllAPI  
#endif  // EPROSIMA_USER_DLL_EXPORT  
#else  
#define Pose_DllAPI  
#endif // _WIN32  

namespace eprosima {  
namespace fastcdr {  
class Cdr; // 前向声明Cdr类  
} // namespace fastcdr  
} // namespace eprosima  

namespace geometry_msgs {  
    namespace msg {  
        /*!  
         * @brief 此类表示IDL文件中定义的Pose结构。  
         * @ingroup POSE  
         */  
        class Pose  
        {  
        public:  

            /*!  
             * @brief 默认构造函数。  
             */  
            eProsima_user_DllExport Pose();  

            /*!  
             * @brief 默认析构函数。  
             */  
            eProsima_user_DllExport ~Pose();  

            /*!  
             * @brief 拷贝构造函数。  
             * @param x 要复制的geometry_msgs::msg::Pose对象的引用。  
             */  
            eProsima_user_DllExport Pose(  
                    const Pose& x);  

            /*!  
             * @brief 移动构造函数。  
             * @param x 要复制的geometry_msgs::msg::Pose对象的引用。  
             */  
            eProsima_user_DllExport Pose(  
                    Pose&& x) noexcept;  

            /*!  
             * @brief 拷贝赋值运算符。  
             * @param x 要复制的geometry_msgs::msg::Pose对象的引用。  
             */  
            eProsima_user_DllExport Pose& operator =(  
                    const Pose& x);  

            /*!  
             * @brief 移动赋值运算符。  
             * @param x 要复制的geometry_msgs::msg::Pose对象的引用。  
             */  
            eProsima_user_DllExport Pose& operator =(  
                    Pose&& x) noexcept;  

            /*!  
             * @brief 比较运算符。  
             * @param x 要比较的geometry_msgs::msg::Pose对象。  
             */  
            eProsima_user_DllExport bool operator ==(  
                    const Pose& x) const;  

            /*!  
             * @brief 比较运算符。  
             * @param x 要比较的geometry_msgs::msg::Pose对象。  
             */  
            eProsima_user_DllExport bool operator !=(  
                    const Pose& x) const;  

            /*!  
             * @brief 复制成员position的值。  
             * @param _position 要复制到成员position的新值。  
             */  
            eProsima_user_DllExport void position(  
                    const geometry_msgs::msg::Point& _position);  

            /*!  
             * @brief 移动成员position的值。  
             * @param _position 要移动到成员position的新值。  
             */  
            eProsima_user_DllExport void position(  
                    geometry_msgs::msg::Point&& _position);  

            /*!  
             * @brief 返回成员position的常量引用。  
             * @return 成员position的常量引用。  
             */  
            eProsima_user_DllExport const geometry_msgs::msg::Point& position() const;  

            /*!  
             * @brief 返回成员position的引用。  
             * @return 成员position的引用。  
             */  
            eProsima_user_DllExport geometry_msgs::msg::Point& position();  

            /*!  
             * @brief 复制成员orientation的值。  
             * @param _orientation 要复制到成员orientation的新值。  
             */  
            eProsima_user_DllExport void orientation(  
                    const geometry_msgs::msg::Quaternion& _orientation);  

            /*!  
             * @brief 移动成员orientation的值。  
             * @param _orientation 要移动到成员orientation的新值。  
             */  
            eProsima_user_DllExport void orientation(  
                    geometry_msgs::msg::Quaternion&& _orientation);  

            /*!  
             * @brief 返回成员orientation的常量引用。  
             * @return 成员orientation的常量引用。  
             */  
            eProsima_user_DllExport const geometry_msgs::msg::Quaternion& orientation() const;  


            /*!  
             * @brief 返回成员orientation的引用。  
             * @return 成员orientation的引用。  
             */  
            eProsima_user_DllExport geometry_msgs::msg::Quaternion& orientation();  

            /*!  
             * @brief 返回对象的最大序列化大小，取决于缓冲区对齐。  
             * @param current_alignment 缓冲区对齐。  
             * @return 最大序列化大小。  
             */  
            eProsima_user_DllExport static size_t getMaxCdrSerializedSize(  
                    size_t current_alignment = 0);  

            /*!  
             * @brief 返回数据的序列化大小，取决于缓冲区对齐。  
             * @param data 要计算其序列化大小的数据。  
             * @param current_alignment 缓冲区对齐。  
             * @return 序列化大小。  
             */  
            eProsima_user_DllExport static size_t getCdrSerializedSize(  
                    const geometry_msgs::msg::Pose& data,  
                    size_t current_alignment = 0);  

            /*!  
             * @brief 使用CDR序列化对对象进行序列化。  
             * @param cdr CDR序列化对象。  
             */  
            eProsima_user_DllExport void serialize(  
                    eprosima::fastcdr::Cdr& cdr) const;  

            /*!  
             * @brief 使用CDR序列化对对象进行反序列化。  
             * @param cdr CDR序列化对象。  
             */  
            eProsima_user_DllExport void deserialize(  
                    eprosima::fastcdr::Cdr& cdr);  

            /*!  
             * @brief 返回对象键的最大序列化大小，取决于缓冲区对齐。  
             * @param current_alignment 缓冲区对齐。  
             * @return 最大序列化大小。  
             */  
            eProsima_user_DllExport static size_t getKeyMaxCdrSerializedSize(  
                    size_t current_alignment = 0);  

            /*!  
             * @brief 告诉您是否为此类型定义了键。  
             */  
            eProsima_user_DllExport static bool isKeyDefined();  

            /*!  
             * @brief 使用CDR序列化序列化对象的键成员。  
             * @param cdr CDR序列化对象。  
             */  
            eProsima_user_DllExport void serializeKey(  
                    eprosima::fastcdr::Cdr& cdr) const;  

        private:  
            // 类的私有成员变量：（位置）和（方向）  
            geometry_msgs::msg::Point m_position;    // 位置，类型为Point  
            geometry_msgs::msg::Quaternion m_orientation; // 方位，类型为Quaternion  
        };  
    } // namespace msg  
} // namespace geometry_msgs  

#endif // _FAST_DDS_GENERATED_GEOMETRY_MSGS_MSG_POSE_H_