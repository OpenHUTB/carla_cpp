// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"
#ifdef LIBCARLA_INCLUDED_FROM_UE4
#include <compiler/enable-ue4-macros.h>
#include "Vehicle/VehiclePhysicsControl.h"
#include <compiler/disable-ue4-macros.h>
#endif
// 定义 `carla` 命名空间及其子命名空间 `rpc`，通过这样的命名空间结构来组织代码，避免不同模块间的命名冲突，清晰地划分功能所属的范围，当前定义的 `GearPhysicsControl` 类就在 `rpc` 这个子命名空间下
namespace carla {
namespace rpc {
    // 定义一个名为 `GearPhysicsControl` 的类，从名字来看，它可能用于控制与齿轮相关的物理特性参数，比如在车辆模拟等场景中，涉及到不同挡位对应的传动比等物理量的控制和管理
  class GearPhysicsControl {
  public:
   // 默认构造函数，使用 `= default` 语法让编译器自动生成默认的构造函数实现，这个构造函数会按照成员变量的默认初始化列表进行初始化（这里就是将各成员变量初始化为各自的默认值，如下文定义的 `ratio` 为 `1.0f`，`down_ratio` 为 `0.5f`，`up_ratio` 为 `0.65f`），方便创建一个具有默认参数状态的 `GearPhysicsControl` 对象
    GearPhysicsControl() = default;
  // 自定义的构造函数，接受三个 `float` 类型的参数 `in_ratio`、`in_down_ratio`、`in_up_ratio`，用于创建一个可以指定各物理参数值的 `GearPhysicsControl` 对象，通过初始化列表将传入的参数分别赋值给对应的成员变量 `ratio`、`down_ratio`、`up_ratio`，使得对象可以按照传入的具体数值来初始化相关的齿轮物理参数
    GearPhysicsControl(
    float in_ratio,
    float in_down_ratio,
    float in_up_ratio)
      : ratio(in_ratio),
        down_ratio(in_down_ratio),
        up_ratio(in_up_ratio) {}
   // 定义一个 `float` 类型的成员变量 `ratio`，并初始化为 `1.0f`，从名字推测它可能表示某个挡位的传动比之类的物理参数，用于描述齿轮在传动过程中的速度、扭矩等转换关系，初始值 `1.0f` 可能是一个默认的标准传动比设定，具体数值会根据实际的物理模型和应用场景有所不同
    float ratio = 1.0f;
    // 定义一个 `float` 类型的成员变量 `down_ratio`，并初始化为 `0.5f`，可能表示降挡相关的比例参数，比如在车辆换挡操作中，用于控制降挡时速度、扭矩等变化的比例关系，不同挡位之间的降挡比例会有所差异，这里的 `0.5f` 是其默认的初始设定值
    float down_ratio = 0.5f;
    // 定义一个 `float` 类型的成员变量 `up_ratio`，并初始化为 `0.65f`，同理，它可能表示升挡相关的比例参数，用于控制升挡时各物理量的变化比例，`0.65f` 为其默认的初始设定值，在实际应用中可以根据具体的车辆或机械装置的特性进行调整
    float up_ratio = 0.65f;
    // 重载的不等运算符（`!=`），用于比较两个 `GearPhysicsControl` 对象是否不相等。比较逻辑是依次对比 `ratio`、`down_ratio`、`up_ratio` 这三个成员变量的值是否分别相等，如果有任何一个成员变量的值不相等，则两个对象被认为是不相等的，返回 `true`；只有当所有成员变量都相等时，才返回 `false`，表示两个对象相等
    bool operator!=(const GearPhysicsControl &rhs) const {
      return
      ratio != rhs.ratio ||
      down_ratio != rhs.down_ratio ||
      up_ratio != rhs.up_ratio;
    }
    // 重载的相等运算符（`==`），通过调用上面定义的不等运算符（`!=`）并取反来实现相等判断逻辑。即如果两个对象通过 `!=` 运算符判断为不相等，则返回 `false`；如果 `!=` 运算符返回 `false`（意味着所有成员变量都相等），则这里返回 `true`，表示两个对象相等
    bool operator==(const GearPhysicsControl &rhs) const {
      return !(*this != rhs);
    }
    // 判断是否是从 UE4 中包含进来的这个头文件，如果是，则定义下面这些与 UE4 中相关类型相互转换的函数和构造函数，用于在 `carla` 代码与 UE4 相关代码交互时方便地进行数据类型的转换操作
#ifdef LIBCARLA_INCLUDED_FROM_UE4
            // 定义一个构造函数，接受一个 UE4 中的 `FGearPhysicsControl` 类型的参数 `Gear`，用于将 UE4 中的对应齿轮物理控制类型转换为当前的 `GearPhysicsControl` 类型。它通过将 `FGearPhysicsControl` 类型中各成员变量（如 `Ratio`、`DownRatio`、`UpRatio`）的值分别赋值给当前对象的对应成员变量（`ratio`、`down_ratio`、`up_ratio`），实现从 UE4 类型到自定义类型的转换，方便在不同环境下对齿轮物理控制参数进行统一管理和操作

    GearPhysicsControl(const FGearPhysicsControl &Gear)
      : ratio(Gear.Ratio),
        down_ratio(Gear.DownRatio),
        up_ratio(Gear.UpRatio) {}
            // 定义一个类型转换运算符（`operator`），用于将当前的 `GearPhysicsControl` 对象隐式转换为 UE4 中的 `FGearPhysicsControl` 类型。当在代码中需要将 `GearPhysicsControl` 对象当作 `FGearPhysicsControl` 类型使用时（比如作为参数传递给接受 `FGearPhysicsControl` 类型的函数等情况），编译器会自动调用这个转换运算符进行转换，返回一个具有相同物理参数值的 `FGearPhysicsControl` 对象，方便在与 UE4 相关的项目中进行数据的交互和兼容使用

    operator FGearPhysicsControl() const {
      FGearPhysicsControl Gear;
      Gear.Ratio = ratio;
      Gear.DownRatio = down_ratio;
      Gear.UpRatio = up_ratio;
      return Gear;
    }
#endif
        // 使用 `MSGPACK_DEFINE_ARRAY` 宏（来自于之前引入的 `carla/MsgPack.h` 文件相关的序列化功能）来定义如何将这个 `GearPhysicsControl` 类的对象序列化为数组形式。这里指定了按照 `ratio`、`down_ratio`、`up_ratio` 的顺序将成员变量进行序列化，方便在网络传输、存储等场景下对该类对象进行相应的处理，使得对象可以方便地在不同地方进行恢复和使用，例如在不同的车辆模拟模块或者与远程服务器交互车辆物理参数时，可以利用序列化后的对象进行数据传递和处理

    MSGPACK_DEFINE_ARRAY(ratio,
    down_ratio,
    up_ratio)
  };

}
}
