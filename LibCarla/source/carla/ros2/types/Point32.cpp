// 版权所有 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).  
//  
// 根据Apache许可证第2.0版（“许可证”）进行许可；除非遵守该许可，否则您不得使用此文件。  
// 您可以在以下网址获取许可证副本：  
//  
//     http://www.apache.org/licenses/LICENSE-2.0  
//  
// 除非适用法律要求或书面同意，依据许可证分发的软件均按“原样”提供，  
// 不附有任何种类的明示或暗示的担保或条件。  
// 请参阅许可证以获取有关特定权限和限制的详细信息。  

/*!  
 * @file Point32.cpp  
 * 此源文件包含IDL文件中所描述类型的定义。  
 *  
 * 此文件是由工具gen生成的。  
 */

#ifdef _WIN32
// 防止在 Windows 平台上产生链接器警告 
namespace {
char dummy;// 声明一个 dummy 变量
}  // namespace
#endif  // _WIN32

#include "Point32.h"// 包含 Point32 类的头文件
#include <fastcdr/Cdr.h>// 包含 CDR 序列化功能的库
// CDR 序列化的最大大小定义
#include <fastcdr/exceptions/BadParamException.h>
using namespace eprosima::fastcdr::exception;

#include <utility>
// 定义 Point32 的最大 CDR 序列化大小
#define geometry_msgs_msg_Point32_max_cdr_typesize 12ULL;
#define geometry_msgs_msg_Point32_max_key_cdr_typesize 0ULL;
// Point32 类的构造、拷贝和移动构造方法，初始化 x, y, z 坐标
geometry_msgs::msg::Point32::Point32()
{
    // 初始化坐标为 0 
    m_x = 0.0;
    // 初始化坐标为 0 
    m_y = 0.0;
   // 初始化坐标为 0 
    m_z = 0.0;
}
// Point32 类的析构函数 
geometry_msgs::msg::Point32::~Point32()
{// 默认析构，无资源管理需要 
}
// 拷贝和移动构造函数
geometry_msgs::msg::Point32::Point32(
        const Point32& x)
{
    m_x = x.m_x;
    m_y = x.m_y;
    m_z = x.m_z;
}
// 移动构造函数
geometry_msgs::msg::Point32::Point32(
        Point32&& x) noexcept
{
    m_x = x.m_x;
    m_y = x.m_y;
    m_z = x.m_z;
}
// 拷贝赋值运算符 
geometry_msgs::msg::Point32& geometry_msgs::msg::Point32::operator =(
        const Point32& x)
{
    m_x = x.m_x;
    m_y = x.m_y;
    m_z = x.m_z;

    return *this;// 返回当前对象的引用 
}
// 移动赋值运算符 
geometry_msgs::msg::Point32& geometry_msgs::msg::Point32::operator =(
        Point32&& x) noexcept
{
    m_x = x.m_x; // 移动 x 的 m_x
    m_y = x.m_y;// 移动 x 的 m_y 
    m_z = x.m_z;// 移动 x 的 m_z

    return *this; // 返回当前对象的引用
}
// 相等运算符
bool geometry_msgs::msg::Point32::operator ==(
        const Point32& x) const
{// 比较 m_x, m_y 和 m_z 是否相等
    return (m_x == x.m_x && m_y == x.m_y && m_z == x.m_z);
}
// 不相等运算符
bool geometry_msgs::msg::Point32::operator !=(
        const Point32& x) const
{
    return !(*this == x);// 如果相等运算符返回 false，则不相等 
}
// 获取最大 CDR 序列化大小 
size_t geometry_msgs::msg::Point32::getMaxCdrSerializedSize(
        size_t current_alignment) // 忽略当前对齐值
{
    static_cast<void>(current_alignment);
    return geometry_msgs_msg_Point32_max_cdr_typesize;// 返回最大 CDR 序列化大小
}
// 获取给定数据的 CDR 序列化大小 
size_t geometry_msgs::msg::Point32::getCdrSerializedSize(
        const geometry_msgs::msg::Point32& data,
        size_t current_alignment)
{
    (void)data; // 忽略输入数据
    size_t initial_alignment = current_alignment;// 保存初始对齐值  
    // 计算 m_x 的序列化大小
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
     // 计算 m_y 的序列化大小
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    // 计算 m_z 的序列化大小 
    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

    return current_alignment - initial_alignment;// 返回总序列化大小
}
// 序列化函数
void geometry_msgs::msg::Point32::serialize(
        eprosima::fastcdr::Cdr& scdr) const
{
    scdr << m_x;// 序列化 m_x 
    scdr << m_y;// 序列化 m_y 
    scdr << m_z;// 序列化 m_z
}
// 反序列化函数
void geometry_msgs::msg::Point32::deserialize(
        eprosima::fastcdr::Cdr& dcdr)
{
    dcdr >> m_x;// 反序列化 m_x 
    dcdr >> m_y;// 反序列化 m_y 
    dcdr >> m_z;// 反序列化 m_z 
}

/*!  
 * @brief 设置成员 x 的值  
 * @param _x 成员 x 的新值  
 */ 
void geometry_msgs::msg::Point32::x(
        float _x)
{
    m_x = _x;// 将新值赋给 m_x  
}

/*!  
 * @brief 返回成员 x 的值  
 * @return 成员 x 的值  
 */ 
float geometry_msgs::msg::Point32::x() const
{
    return m_x;// 返回 m_x 的值
}

/*!  
 * @brief 返回成员 x 的引用  
 * @return 成员 x 的引用  
 */ 
float& geometry_msgs::msg::Point32::x()
{
    return m_x;// 返回 m_x 的引用  
}

/*!  
 * @brief 设置成员 y 的值  
 * @param _y 成员 y 的新值  
 */ 
void geometry_msgs::msg::Point32::y(
        float _y)
{
    m_y = _y;// 将新值赋给 m_y
}

/*!  
 * @brief 返回成员 y 的值  
 * @return 成员 y 的值  
 */  
float geometry_msgs::msg::Point32::y() const
{
    return m_y;// 返回 m_y 的值 
}

/*!  
 * @brief 返回成员 y 的引用  
 * @return 成员 y 的引用  
 */ 
float& geometry_msgs::msg::Point32::y()
{
    return m_y; // 返回 m_y 的引用 
}

/*!  
 * @brief 设置成员 z 的值  
 * @param _z 成员 z 的新值  
 */
void geometry_msgs::msg::Point32::z(
        float _z)
{
    m_z = _z;// 将新值赋给 m_z 
}

/*!  
 * @brief 返回成员 z 的值  
 * @return 成员 z 的值  
 */ 
float geometry_msgs::msg::Point32::z() const
{
    return m_z;// 返回 m_z 的值
}

/*!  
 * @brief 返回成员 z 的引用  
 * @return 成员 z 的引用  
 */ 
float& geometry_msgs::msg::Point32::z()
{
    return m_z;// 返回 m_z 的引用 
}
// 获取键的最大 CDR 序列化大小
size_t geometry_msgs::msg::Point32::getKeyMaxCdrSerializedSize(
        size_t current_alignment)
{
    static_cast<void>(current_alignment);// 忽略当前对齐值
    return geometry_msgs_msg_Point32_max_key_cdr_typesize;// 返回最大键的 CDR 序列化大小
}
// 检查键是否已定义
bool geometry_msgs::msg::Point32::isKeyDefined()
{
    return false;// 此类型不支持键，因此返回 false
}
// 序列化键 
void geometry_msgs::msg::Point32::serializeKey(
        eprosima::fastcdr::Cdr& scdr) const
{
    (void) scdr;// 忽略序列化上下文 
}
