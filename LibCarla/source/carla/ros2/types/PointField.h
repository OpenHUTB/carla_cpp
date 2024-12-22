// 版权所有 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).  
//  
// 根据 Apache 许可证，版本 2.0（“许可证”）授权；  
// 除非遵守许可证，否则您不得使用此文件。  
// 您可以在以下网址获得许可证：  
//  
//     http://www.apache.org/licenses/LICENSE-2.0  
//  
// 除非适用法律要求或书面同意，  
// 否则根据许可证分发的软件是按“原样”提供的，  
// 不附有任何明示或暗示的担保或条件。  
// 有关许可证下的特定权限和限制，请参阅许可证。  

/*!  
 * @file PointField.h  
 * 此头文件包含在 IDL 文件中描述的类型的声明。  
 *  
 * 此文件由工具 gen 生成。  
 */  

#ifndef _FAST_DDS_GENERATED_SENSOR_MSGS_MSG_POINTFIELD_H_  
#define _FAST_DDS_GENERATED_SENSOR_MSGS_MSG_POINTFIELD_H_  

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
#if defined(PointField_SOURCE)  
#define PointField_DllAPI __declspec( dllexport )  
#else  
#define PointField_DllAPI __declspec( dllimport )  
#endif // PointField_SOURCE  
#else  
#define PointField_DllAPI  
#endif  // EPROSIMA_USER_DLL_EXPORT  
#else  
#define PointField_DllAPI  
#endif // _WIN32  

namespace eprosima {  
namespace fastcdr {  
class Cdr;  
} // namespace fastcdr  
} // namespace eprosima  

namespace sensor_msgs {  
    namespace msg {  
        const uint8_t PointField__INT8 = 1;  
        const uint8_t PointField__UINT8 = 2;  
        const uint8_t PointField__INT16 = 3;  
        const uint8_t PointField__UINT16 = 4;  
        const uint8_t PointField__INT32 = 5;  
        const uint8_t PointField__UINT32 = 6;  
        const uint8_t PointField__FLOAT32 = 7;  
        const uint8_t PointField__FLOAT64 = 8;  

        /*!  
         * @brief 此类表示用户在 IDL 文件中定义的结构 PointField。  
         * @ingroup POINTFIELD  
         */  
        class PointField  
        {  
        public:  

            /*!  
             * @brief 默认构造函数。  
             */  
            eProsima_user_DllExport PointField();  

            /*!  
             * @brief 默认析构函数。  
             */  
            eProsima_user_DllExport ~PointField();  

            /*!  
             * @brief 复制构造函数。  
             * @param x 要复制的 sensor_msgs::msg::PointField 对象的引用。  
             */  
            eProsima_user_DllExport PointField(  
                    const PointField& x);  

            /*!  
             * @brief 移动构造函数。  
             * @param x 要复制的 sensor_msgs::msg::PointField 对象的引用。  
             */  
            eProsima_user_DllExport PointField(  
                    PointField&& x) noexcept;  

            /*!  
             * @brief 复制赋值运算符。  
             * @param x 要复制的 sensor_msgs::msg::PointField 对象的引用。  
             */  
            eProsima_user_DllExport PointField& operator =(  
                    const PointField& x);  

            /*!  
             * @brief 移动赋值运算符。  
             * @param x 要复制的 sensor_msgs::msg::PointField 对象的引用。  
             */  
            eProsima_user_DllExport PointField& operator =(  
                    PointField&& x) noexcept;  

            /*!  
             * @brief 比较运算符。  
             * @param x 要比较的 sensor_msgs::msg::PointField 对象。  
             */  
            eProsima_user_DllExport bool operator ==(  
                    const PointField& x) const;  

            /*!  
             * @brief 比较运算符。  
             * @param x 要比较的 sensor_msgs::msg::PointField 对象。  
             */  
            eProsima_user_DllExport bool operator !=(  
                    const PointField& x) const;  

            /*!  
             * @brief 此函数复制成员 name 的值  
             * @param _name 要复制到成员 name 的新值  
             */  
            eProsima_user_DllExport void name(  
                    const std::string& _name);  

            /*!  
             * @brief 此函数移动成员 name 的值  
             * @param _name 要移动到成员 name 的新值  
             */  
            eProsima_user_DllExport void name(  
                    std::string&& _name);  

            /*!  
             * @brief 此函数返回成员 name 的常量引用  
             * @return 成员 name 的常量引用  
             */  
            eProsima_user_DllExport const std::string& name() const;  

            /*!  
             * @brief 此函数返回成员 name 的引用  
             * @return 成员 name 的引用  
             */  
            eProsima_user_DllExport std::string& name();  

            /*!  
             * @brief 此函数设置成员 offset 的值  
             * @param _offset 成员 offset 的新值  
             */  
            eProsima_user_DllExport void offset(  
                    uint32_t _offset);  

            /*!  
             * @brief 此函数返回成员 offset 的值  
             * @return 成员 offset 的值  
             */  
            eProsima_user_DllExport uint32_t offset() const;  

            /*!  
             * @brief 此函数返回成员 offset 的引用  
             * @return 成员 offset 的引用  
             */  
            eProsima_user_DllExport uint32_t& offset();  

            /*!  
             * @brief 此函数设置成员 datatype 的值  
             * @param _datatype 成员 datatype 的新值  
             */  
            eProsima_user_DllExport void datatype(  
                    uint8_t _datatype);  

            /*!  
             * @brief 此函数返回成员 datatype 的值  
             * @return 成员 datatype 的值  
             */  
            eProsima_user_DllExport uint8_t datatype() const;  

            /*!  
             * @brief 此函数返回成员 datatype 的引用  
             * @return 成员 datatype 的引用  
             */  
            eProsima_user_DllExport uint8_t& datatype();  

            /*!  
             * @brief 此函数设置成员 count 的值  
             * @param _count 成员 count 的新值  
             */  
            eProsima_user_DllExport void count(  
        uint32_t _count);  

            /*!  
             * @brief 此函数返回成员 count 的值  
             * @return 成员 count 的值  
             */  
            eProsima_user_DllExport uint32_t count() const;  

            /*!  
             * @brief 此函数返回成员 count 的引用  
             * @return 成员 count 的引用  
             */  
            eProsima_user_DllExport uint32_t& count();  

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
             * @param data 要计算其序列化大小的数据。  
             * @param current_alignment 缓冲区对齐。  
             * @return 序列化大小。  
             */  
            eProsima_user_DllExport static size_t getCdrSerializedSize(  
                    const sensor_msgs::msg::PointField& data,  
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
             * @brief 此函数告诉您此类型的键是否已定义  
             */  
            eProsima_user_DllExport static bool isKeyDefined();  

            /*!  
             * @brief 此函数使用 CDR 序列化序列化对象的键成员。  
             * @param cdr CDR 序列化对象。  
             */  
            eProsima_user_DllExport void serializeKey(  
                    eprosima::fastcdr::Cdr& cdr) const;  

            private:  
                std::string m_name; // 名称  
                uint32_t m_offset;  // 偏移量  
                uint8_t m_datatype; // 数据类型  
                uint32_t m_count;   // 计数

        };
    } // namespace msg
} // namespace sensor_msgs

#endif // _FAST_DDS_GENERATED_SENSOR_MSGS_MSG_POINTFIELD_H_
