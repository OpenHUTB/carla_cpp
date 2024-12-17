// 版权所有 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).  
//  
// 根据 Apache 许可证第 2.0 版（“许可证”）授权；除非遵循许可证，否则您不得使用此文件。  
// 您可以在以下网址获得许可证的副本：  
//  
//     http://www.apache.org/licenses/LICENSE-2.0  
//  
// 除非适用法律另有规定或书面协议，否则根据许可证分发的软件是按“原样”基准提供的，  
// 不提供任何形式的明示或暗示的保证或条件。  
// 有关许可证所涉及的权限及限制的信息，请参见许可证。  
/*!  
 * @file CarlaEgoCarlaEgoVehicleControl.h  
 * 此头文件包含在 IDL 文件中描述的类型的声明。  
 *  
 * 此文件由工具 gen 生成。  
 */ 
#ifndef _FAST_DDS_GENERATED_CARLA_MSGS_MSG_CARLAEGOCarlaEgoVehicleControl_H_
#define _FAST_DDS_GENERATED_CARLA_MSGS_MSG_CARLAEGOCarlaEgoVehicleControl_H_

#include "Header.h"

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
#if defined(CarlaEgoCarlaEgoVehicleControl_SOURCE)
#define CarlaEgoCarlaEgoVehicleControl_DllAPI __declspec( dllexport )
#else
#define CarlaEgoCarlaEgoVehicleControl_DllAPI __declspec( dllimport )
#endif // CarlaEgoCarlaEgoVehicleControl_SOURCE
#else
#define CarlaEgoCarlaEgoVehicleControl_DllAPI
#endif  // EPROSIMA_USER_DLL_EXPORT
#else
#define CarlaEgoCarlaEgoVehicleControl_DllAPI
#endif // _WIN32

namespace eprosima {
namespace fastcdr {
class Cdr;
} // namespace fastcdr
} // namespace eprosima

namespace carla_msgs {
    namespace msg {
        /*!  
         * @brief 该类表示用户在 IDL 文件中定义的结构 CarlaEgoVehicleControl。  
         * @ingroup CarlaEgoVehicleControl  
         */ 
        class CarlaEgoVehicleControl
        {
        public:

            /*!  
             * @brief 默认构造函数。  
             */ 
            eProsima_user_DllExport CarlaEgoVehicleControl();
            /*!  
             * @brief 默认析构函数。  
             */ 
            eProsima_user_DllExport ~CarlaEgoVehicleControl();
            /*!  
             * @brief 复制构造函数。  
             * @param x 将被复制的 carla_msgs::msg::CarlaEgoVehicleControl 对象的引用。  
             */ 
            eProsima_user_DllExport CarlaEgoVehicleControl(
                    const CarlaEgoVehicleControl& x);
            /*!  
             * @brief 移动构造函数。  
             * @param x 将被复制的 carla_msgs::msg::CarlaEgoVehicleControl 对象的引用。  
             */
            eProsima_user_DllExport CarlaEgoVehicleControl(
                    CarlaEgoVehicleControl&& x) noexcept;
            /*!  
             * @brief 复制赋值运算符。  
             * @param x 将被复制的 carla_msgs::msg::CarlaEgoVehicleControl 对象的引用。  
             */ 
            eProsima_user_DllExport CarlaEgoVehicleControl& operator =(
                    const CarlaEgoVehicleControl& x);
            /*!  
             * @brief 移动赋值运算符。  
             * @param x 将被复制的 carla_msgs::msg::CarlaEgoVehicleControl 对象的引用。  
             */ 
            eProsima_user_DllExport CarlaEgoVehicleControl& operator =(
                    CarlaEgoVehicleControl&& x) noexcept;
            /*!  
             * @brief 比较运算符。  
             * @param x 要比较的 carla_msgs::msg::CarlaEgoVehicleControl 对象。  
             */
            eProsima_user_DllExport bool operator ==(
                    const CarlaEgoVehicleControl& x) const;
            /*!  
             * @brief 比较运算符。  
             * @param x 要比较的 carla_msgs::msg::CarlaEgoVehicleControl 对象。  
             */ 
            eProsima_user_DllExport bool operator !=(
                    const CarlaEgoVehicleControl& x) const;
            /*!  
             * @brief 该函数复制成员 header 的值。  
             * @param _header 要复制到成员 header 的新值。  
             */ 
            eProsima_user_DllExport void header(
                    const std_msgs::msg::Header& _header);
            /*!  
             * @brief 该函数移动成员 header 的值。  
             * @param _header 要移动到成员 header 的新值。  
             */ 
            eProsima_user_DllExport void header(
                    std_msgs::msg::Header&& _header);
            /*!  
             * @brief 该函数返回对成员 header 的常量引用。  
             * @return 对成员 header 的常量引用。  
             */  
            eProsima_user_DllExport const std_msgs::msg::Header& header() const;
            /*!  
             * @brief 该函数返回对成员 header 的引用。  
             * @return 对成员 header 的引用。  
             */ 
            eProsima_user_DllExport std_msgs::msg::Header& header();
            /*!  
             * @brief 该函数设置成员 throttle 的值。  
             * @param _throttle 成员 throttle 的新值。  
             */ 
            eProsima_user_DllExport void throttle(
                    float _throttle);
            /*!  
             * @brief 该函数返回成员 throttle 的值。  
             * @return 成员 throttle 的值。  
             */  
            eProsima_user_DllExport float throttle() const;
            /*!  
             * @brief 该函数返回对成员 throttle 的引用。  
             * @return 对成员 throttle 的引用。  
             */ 
            eProsima_user_DllExport float& throttle();
            /*!  
             * @brief 该函数设置成员 steer 的值。  
             * @param _steer 成员 steer 的新值。  
             */
            eProsima_user_DllExport void steer(
                    float _steer);
            /*!  
             * @brief 该函数返回成员 steer 的值。  
             * @return 成员 steer 的值。  
             */ 
            eProsima_user_DllExport float steer() const;
            /*!  
             * @brief 该函数返回对成员 steer 的引用。  
             * @return 对成员 steer 的引用。  
             */
            eProsima_user_DllExport float& steer();
            /*!  
             * @brief 该函数设置成员 brake 的值。  
             * @param _brake 成员 brake 的新值。  
             */ 
            eProsima_user_DllExport void brake(
                    float _brake);
            /*!  
             * @brief 该函数返回成员 brake 的值。  
             * @return 成员 brake 的值。  
             */ 
            eProsima_user_DllExport float brake() const;
            /*!  
             * @brief 该函数返回对成员 brake 的引用。  
             * @return 对成员 brake 的引用。  
             */ 
            eProsima_user_DllExport float& brake();
            /*!  
             * @brief 该函数设置成员 hand_brake 的值。  
             * @param _hand_brake 成员 hand_brake 的新值。  
             */  
            eProsima_user_DllExport void hand_brake(
                    bool _hand_brake);
            /*!  
             * @brief 该函数返回成员 hand_brake 的值。  
             * @return 成员 hand_brake 的值。  
             */ 
            eProsima_user_DllExport bool hand_brake() const;
            /*!  
             * @brief 该函数返回对成员 hand_brake 的引用。  
             * @return 对成员 hand_brake 的引用。  
             */ 
            eProsima_user_DllExport bool& hand_brake();
            /*!  
             * @brief 该函数设置成员 reverse 的值。  
             * @param _reverse 成员 reverse 的新值。  
             */ 
            eProsima_user_DllExport void reverse(
                    bool _reverse);
            /*!  
             * @brief 该函数返回成员 reverse 的值。  
             * @return 成员 reverse 的值。  
             */  
            eProsima_user_DllExport bool reverse() const;
            /*!  
             * @brief 该函数返回对成员 reverse 的引用。  
             * @return 对成员 reverse 的引用。  
             */ 
            eProsima_user_DllExport bool& reverse();
            /*!  
             * @brief 该函数设置成员 gear 的值。  
             * @param _gear 成员 gear 的新值。  
             */
            eProsima_user_DllExport void gear(
                    int32_t _gear);
            /*!  
             * @brief 该函数返回成员 gear 的值。  
             * @return 成员 gear 的值。  
             */  
            eProsima_user_DllExport int32_t gear() const;
            /*!  
             * @brief 该函数返回对成员 gear 的引用。  
             * @return 对成员 gear 的引用。  
             */  
            eProsima_user_DllExport int32_t& gear();
            /*!  
             * @brief 该函数设置成员 manual_gear_shift 的值。  
             * @param _manual_gear_shift 成员 manual_gear_shift 的新值。  
             */ 
            eProsima_user_DllExport void manual_gear_shift(
                    bool _manual_gear_shift);
            /*!  
             * @brief 该函数返回成员 manual_gear_shift 的值。  
             * @return 成员 manual_gear_shift 的值。  
             */ 
            eProsima_user_DllExport bool manual_gear_shift() const;
            /*!  
             * @brief 该函数返回对成员 manual_gear_shift 的引用。  
             * @return 对成员 manual_gear_shift 的引用。  
             */ 
            eProsima_user_DllExport bool& manual_gear_shift();
            /*!  
             * @brief 该函数返回对象的最大序列化大小，取决于缓冲区对齐方式。  
             * @param current_alignment 缓冲区对齐方式。  
             * @return 最大序列化大小。  
             */ 
            eProsima_user_DllExport static size_t getMaxCdrSerializedSize(
                    size_t current_alignment = 0);
            /*!  
             * @brief 该函数返回数据的序列化大小，取决于缓冲区对齐方式。  
             * @param data 要计算其序列化大小的数据。  
             * @param current_alignment 缓冲区对齐方式。  
             * @return 序列化大小。  
             */ 
            eProsima_user_DllExport static size_t getCdrSerializedSize(
                    const carla_msgs::msg::CarlaEgoVehicleControl& data,
                    size_t current_alignment = 0);
            /*!  
             * @brief 该函数使用 CDR 序列化序列化一个对象。  
             * @param cdr CDR 序列化对象。  
             */
            eProsima_user_DllExport void serialize(
                    eprosima::fastcdr::Cdr& cdr) const;
            /*!  
             * @brief 该函数使用 CDR 序列化反序列化一个对象。  
             * @param cdr CDR 序列化对象。  
             */ 
            eProsima_user_DllExport void deserialize(
                    eprosima::fastcdr::Cdr& cdr);
            /*!  
             * @brief 该函数返回对象键的最大序列化大小，取决于缓冲区对齐方式。  
             * @param current_alignment 缓冲区对齐方式。  
             * @return 最大序列化大小。  
             */ 
            eProsima_user_DllExport static size_t getKeyMaxCdrSerializedSize(
                    size_t current_alignment = 0);
            /*!  
             * @brief 该函数告诉您此类型的键是否已定义。  
             */ 
            eProsima_user_DllExport static bool isKeyDefined();
            /*!  
             * @brief 该函数使用 CDR 序列化序列化对象的键成员。  
             * @param cdr CDR 序列化对象。  
             */ 
            eProsima_user_DllExport void serializeKey(
                    eprosima::fastcdr::Cdr& cdr) const;
        private:
            std_msgs::msg::Header m_header;
            float m_throttle;
            float m_steer;
            float m_brake;
            bool m_hand_brake;
            bool m_reverse;
            int32_t m_gear;
            bool m_manual_gear_shift;

        };
    } // namespace msg
} // namespace carla_msgs

#endif // _FAST_DDS_GENERATED_CARLA_MSGS_MSG_CARLAEGOCarlaEgoVehicleControl_H_
