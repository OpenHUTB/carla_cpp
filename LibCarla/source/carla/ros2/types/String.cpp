// 版权所有 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).  
//  
// 根据 Apache 许可证第 2.0 版（“许可证”）进行许可；  
// 除非遵守许可证，否则不得使用此文件。  
// 您可以在以下网址获取许可证副本：  
//  
//     http://www.apache.org/licenses/LICENSE-2.0  
//  
// 除非适用法律要求或书面同意，否则根据许可证分发的软件是以“原样”基础提供的，  
// 不提供任何形式的保证或条件，无论是明示或暗示的。  
// 请参阅许可证以了解特定语言所规定的权限和限制。  

/*!  
 * @file String.cpp  
 * 此源文件包含 IDL 文件中描述的类型的定义。  
 *  
 * 此文件是由工具 gen 生成的。  
 */  

#ifdef _WIN32  
// 在 Visual Studio 中移除链接器警告 LNK4221  
namespace {  
char dummy;  
}  // namespace  
#endif  // _WIN32  

#include "String.h"  
#include <fastcdr/Cdr.h>  

#include <fastcdr/exceptions/BadParamException.h>  
using namespace eprosima::fastcdr::exception;  

#include <utility>  

// 定义最大 CDR 序列化大小  
#define std_msgs_msg_String_max_cdr_typesize 260ULL;  
#define std_msgs_msg_String_max_key_cdr_typesize 0ULL;  

// 构造函数  
std_msgs::msg::String::String()  
{  
    // 字符串 m_data  
    m_data ="";  
}  

// 析构函数  
std_msgs::msg::String::~String()  
{  
}  

// 拷贝构造函数  
std_msgs::msg::String::String(  
        const String& x)  
{  
    m_data = x.m_data;  
}  

// 移动构造函数  
std_msgs::msg::String::String(  
        String&& x) noexcept  
{  
    m_data = std::move(x.m_data);  
}  

// 拷贝赋值运算符  
std_msgs::msg::String& std_msgs::msg::String::operator =(  
        const String& x)  
{  
    m_data = x.m_data;  

    return *this;  
}  

// 移动赋值运算符  
std_msgs::msg::String& std_msgs::msg::String::operator =(  
        String&& x) noexcept  
{  
    m_data = std::move(x.m_data);  

    return *this;  
}  

// 相等运算符  
bool std_msgs::msg::String::operator ==(  
        const String& x) const  
{  
    return (m_data == x.m_data);  
}  

// 不相等运算符  
bool std_msgs::msg::String::operator !=(  
        const String& x) const  
{  
    return !(*this == x);  
}  

// 获取最大 CDR 序列化大小  
size_t std_msgs::msg::String::getMaxCdrSerializedSize(  
        size_t current_alignment)  
{  
    static_cast<void>(current_alignment);  
    return std_msgs_msg_String_max_cdr_typesize;  
}  

// 获取给定数据的 CDR 序列化大小  
size_t std_msgs::msg::String::getCdrSerializedSize(  
        const std_msgs::msg::String& data,  
        size_t current_alignment)  
{  
    size_t initial_alignment = current_alignment;  
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4) + data.data().size() + 1;  

    return current_alignment - initial_alignment;  
}  

// 序列化  
void std_msgs::msg::String::serialize(  
        eprosima::fastcdr::Cdr& scdr) const  
{  
    scdr << m_data.c_str();  
}  

// 反序列化  
void std_msgs::msg::String::deserialize(  
        eprosima::fastcdr::Cdr& dcdr)  
{  
    dcdr >> m_data;  
}  

/*!  
 * @brief 此函数将新值复制到成员数据中  
 * @param _data 要复制到成员数据的新值  
 */  
void std_msgs::msg::String::data(  
        const std::string& _data)  
{  
    m_data = _data;  
}  

/*!  
 * @brief 此函数将新值移动到成员数据中  
 * @param _data 要移动到成员数据的新值  
 */  
void std_msgs::msg::String::data(  
        std::string&& _data)  
{  
    m_data = std::move(_data);  
}  

/*!  
 * @brief 此函数返回对成员数据的常量引用  
 * @return 对成员数据的常量引用  
 */  
const std::string& std_msgs::msg::String::data() const  
{  
    return m_data;  
}  

/*!  
 * @brief 此函数返回对成员数据的引用  
 * @return 对成员数据的引用  
 */  
std::string& std_msgs::msg::String::data()  
{  
    return m_data;  
}  

// 获取键的最大 CDR 序列化大小  
size_t std_msgs::msg::String::getKeyMaxCdrSerializedSize(  
        size_t current_alignment)  
{  
    static_cast<void>(current_alignment);  
    return std_msgs_msg_String_max_key_cdr_typesize;  
}  

// 检查键是否已定义  
bool std_msgs::msg::String::isKeyDefined()  
{  
    return false;  
}  

// 序列化键  
void std_msgs::msg::String::serializeKey(  
        eprosima::fastcdr::Cdr& scdr) const  
{  
    (void) scdr;  
}