// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// 引入Carla的消息打包相关头文件
#include "carla/MsgPack.h"
// 引入Carla的几何数学相关头文件
#include "carla/geom/Math.h"
// 引入Carla的三维向量相关头文件
#include "carla/geom/Vector3D.h"

// 如果是从UE4中包含此文件（定义了相应宏），则引入UE4相关的宏启用和禁用头文件以及UE4的旋转器相关头文件
#ifdef LIBCARLA_INCLUDED_FROM_UE4
#include <compiler/enable-ue4-macros.h>
#include "Math/Rotator.h"
#include <compiler/disable-ue4-macros.h>
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace geom {

    // 旋转类，用于表示物体的旋转状态，包含俯仰角（pitch）、偏航角（yaw）和翻滚角（roll）三个角度信息
    class Rotation {
    public:

        // =========================================================================
        // -- 公开数据成员 ----------------------------------------------------------
        // =========================================================================

        // 俯仰角，绕X轴旋转的角度，初始化为0.0f，单位通常为弧度（根据使用场景可能会有转换）
        float pitch = 0.0f;

        // 偏航角，绕Y轴旋转的角度，初始化为0.0f，单位通常为弧度（根据使用场景可能会有转换）
        float yaw = 0.0f;

        // 翻滚角，绕Z轴旋转的角度，初始化为0.0f，单位通常为弧度（根据使用场景可能会有转换）
        float roll = 0.0f;

        // 使用MsgPack定义如何对这个类的成员（pitch、yaw、roll）进行序列化和反序列化操作
        MSGPACK_DEFINE_ARRAY(pitch, yaw, roll);

        // =========================================================================
        // -- 构造函数 --------------------------------------------------------------
        // =========================================================================

        // 默认构造函数，使用默认初始化，三个角度成员都为默认值（0.0f）
        Rotation() = default;

        // 带参数的构造函数，用于根据给定的俯仰角、偏航角和翻滚角来初始化旋转对象
        Rotation(float p, float y, float r)
            : pitch(p),
            yaw(y),
            roll(r) {}

        // =========================================================================
        // -- 其他方法 --------------------------------------------------------------
        // =========================================================================

        // 获取代表物体“前向”方向的向量，通过调用Math类中的相关函数，基于当前旋转状态来计算得到
        Vector3D GetForwardVector() const {
            return Math::GetForwardVector(*this);
        }

        // 获取代表物体“右向”方向的向量，通过调用Math类中的相关函数，基于当前旋转状态来计算得到
        Vector3D GetRightVector() const {
            return Math::GetRightVector(*this);
        }

        // 获取代表物体“上向”方向的向量，通过调用Math类中的相关函数，基于当前旋转状态来计算得到
        Vector3D GetUpVector() const {
            return Math::GetUpVector(*this);
        }

        // 对输入的三维向量进行旋转操作，按照先绕X轴（roll）、再绕Y轴（pitch）、最后绕Z轴（yaw）的顺序进行旋转变换
        // 具体的旋转计算是通过三角函数（正弦、余弦）以及相应的矩阵乘法规则来实现的
        void RotateVector(Vector3D &in_point) const {
            // 将角度从度数转换为弧度后获取对应的余弦值，用于后续旋转计算（偏航角相关）
            const float cy = std::cos(Math::ToRadians(yaw));
            // 将角度从度数转换为弧度后获取对应的正弦值，用于后续旋转计算（偏航角相关）
            const float sy = std::sin(Math::ToRadians(yaw));
            // 将角度从度数转换为弧度后获取对应的余弦值，用于后续旋转计算（翻滚角相关）
            const float cr = std::cos(Math::ToRadians(roll));
            // 将角度从度数转换为弧度后获取对应的正弦值，用于后续旋转计算（翻滚角相关）
            const float sr = std::sin(Math::ToRadians(roll));
            // 将角度从度数转换为弧度后获取对应的余弦值，用于后续旋转计算（俯仰角相关）
            const float cp = std::cos(Math::ToRadians(pitch));
            // 将角度从度数转换为弧度后获取对应的正弦值，用于后续旋转计算（俯仰角相关）
            const float sp = std::sin(Math::ToRadians(pitch));

            Vector3D out_point;
            // 根据旋转矩阵乘法规则计算旋转后向量的x坐标分量
            out_point.x =
                in_point.x * (cp * cy) +
                in_point.y * (cy * sp * sr - sy * cr) +
                in_point.z * (-cy * sp * cr - sy * sr);

            // 根据旋转矩阵乘法规则计算旋转后向量的y坐标分量
            out_point.y =
                in_point.x * (cp * sy) +
                in_point.y * (sy * sp * sr + cy * cr) +
                in_point.z * (-sy * sp * cr + cy * sr);

            // 根据旋转矩阵乘法规则计算旋转后向量的z坐标分量
            out_point.z =
                in_point.x * (sp) +
                in_point.y * (-cp * sr) +
                in_point.z * (cp * cr);

            in_point = out_point;
        }

        // 对输入的三维向量进行旋转操作，返回旋转后的新向量（内部调用了上面的RotateVector方法，先复制输入向量再进行旋转）
        Vector3D RotateVector(const Vector3D& in_point) const {
            Vector3D out_point = in_point;
            RotateVector(out_point);
            return out_point;
        }

        // 对输入的三维向量进行逆旋转操作，应用RotateVector函数中使用的旋转矩阵的转置来实现逆旋转效果
        void InverseRotateVector(Vector3D &in_point) const {
            // 将角度从度数转换为弧度后获取对应的余弦值，用于后续逆旋转计算（偏航角相关）
            const float cy = std::cos(Math::ToRadians(yaw));
            // 将角度从度数转换为弧度后获取对应的正弦值，用于后续逆旋转计算（偏航角相关）
            const float sy = std::sin(Math::ToRadians(yaw));
            // 将角度从度数转换为弧度后获取对应的余弦值，用于后续逆旋转计算（翻滚角相关）
            const float cr = std::cos(Math::ToRadians(roll));
            // 将角度从度数转换为弧度后获取对应的正弦值，用于后续逆旋转计算（翻滚角相关）
            const float sr = std::sin(Math::ToRadians(roll));
            // 将角度从度数转换为弧度后获取对应的余弦值，用于后续逆旋转计算（俯仰角相关）
            const float cp = std::cos(Math::ToRadians(pitch));
            // 将角度从度数转换为弧度后获取对应的正弦值，用于后续逆旋转计算（俯仰角相关）
            const float sp = std::sin(Math::ToRadians(pitch));

            Vector3D out_point;
            // 根据逆旋转矩阵乘法规则计算逆旋转后向量的x坐标分量
            out_point.x =
                in_point.x * (cp * cy) +
                in_point.y * (cp * sy) +
                in_point.z * (sp);

            // 根据逆旋转矩阵乘法规则计算逆旋转后向量的y坐标分量
            out_point.y =
                in_point.x * (cy * sp * sr - sy * cr) +
                in_point.y * (sy * sp * sr + cy * cr) +
                in_point.z * (-cp * sr);

            // 根据逆旋转矩阵乘法规则计算逆旋转后向量的z坐标分量
            out_point.z =
                in_point.x * (-cy * sp * cr - sy * sr) +
                in_point.y * (-sy * sp * cr + cy * sr) +
                in_point.z * (cp * cr);

            in_point = out_point;
        }

        // =========================================================================
        // -- 比较运算符 ------------------------------------------------------------
        // =========================================================================

        // 重载相等运算符，用于比较两个Rotation对象是否相等，即三个角度成员（pitch、yaw、roll）是否都相等
        bool operator==(const Rotation &rhs) const {
            return (pitch == rhs.pitch) && (yaw == rhs.yaw) && (roll == rhs.roll);
        }

        // 重载不等运算符，通过对相等运算符取反来判断两个Rotation对象是否不相等
        bool operator!=(const Rotation &rhs) const {
            return!(*this == rhs);
        }

        // =========================================================================
        // -- 转换为 UE4 类型 -------------------------------------------------------
        // =========================================================================

#ifdef LIBCARLA_INCLUDED_FROM_UE4
        // 从UE4的FRotator类型构造Rotation对象，将FRotator的三个角度成员赋值给当前Rotation对象的对应角度成员
        Rotation(const FRotator &rotator)
            : Rotation(rotator.Pitch, rotator.Yaw, rotator.Roll) {}

        // 类型转换运算符重载，将Rotation对象转换为UE4的FRotator类型，直接返回一个由当前Rotation对象的三个角度成员构造的FRotator对象
        operator FRotator() const {
            return FRotator{ pitch, yaw, roll };
        }
#endif // LIBCARLA_INCLUDED_FROM_UE4
    };

} // namespace geom
} // namespace carla
