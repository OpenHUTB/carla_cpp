// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 这是一个预处理指令，用于确保该头文件在整个编译单元中只会被编译一次，避免因重复包含导致的编译错误，例如重复定义类型、变量等问题。


#include <type_traits>
// 引入 C++ 标准库中的 <type_traits> 头文件，该头文件提供了一系列用于在编译时进行类型特征检查、类型转换等操作的工具模板，
// 在后续代码中可能会利用其中的功能来处理枚举类型相关的底层类型操作等情况。

#include "VehicleInputPriority.generated.h"
// 包含一个由代码生成工具自动生成的头文件（通常与虚幻引擎等相关项目中类或枚举等的反射、序列化等机制配合使用），
// 里面包含了本枚举类型（EVehicleInputPriority）相关的一些额外代码，以帮助项目能更好地管理和使用这个枚举，例如在蓝图中进行可视化操作等。

UENUM(BlueprintType)
// 使用 UENUM 宏定义一个枚举类型，同时指定了该枚举具有 BlueprintType 属性，意味着这个枚举类型可以在虚幻引擎的蓝图可视化编程环境中使用，
// 方便在蓝图中进行变量声明、逻辑判断等操作，将枚举与蓝图相关功能进行了集成。
enum class EVehicleInputPriority : uint8
// 定义一个名为 EVehicleInputPriority 的强类型枚举（enum class），其底层存储类型被指定为无符号 8 位整数（uint8），
// 使用强类型枚举可以避免枚举值在不同作用域下的命名冲突等问题，使代码更加健壮和清晰。
{
  Null = 0, // Workarround for UE4.24 issue with enums
// 定义一个名为 Null 的枚举值，并初始化为 0，注释表明这是针对虚幻引擎 4.24 版本中枚举相关问题的一种变通解决办法（Workarround），
    // 具体解决的问题需要结合该版本引擎中枚举使用的实际情况来进一步了解。
  INVALID = 0u  UMETA(Hidden),
 // 定义一个名为 INVALID 的枚举值，同样初始化为 0（0u 表示无符号的 0），并且通过 UMETA 宏附加了元数据（Metadata），这里设置为 Hidden，
    // 意味着在某些虚幻引擎相关的可视化展示（比如在蓝图编辑器中）等场景下，这个枚举值可能会被隐藏起来，不直接展示给用户，通常用于一些特殊的、不希望用户直接操作的内部逻辑表示。

  Lowest        UMETA(DisplayName = "Lowest Priority", ToolTip = "Use for debugging purposes only"),
// 定义一个名为 Lowest 的枚举值，通过 UMETA 宏添加了显示名称（DisplayName）和提示信息（ToolTip）的元数据，
    // 显示名称用于在虚幻引擎相关界面（如蓝图编辑器中显示该枚举值的友好名称）为 "Lowest Priority"，提示信息提示此枚举值仅用于调试目的，方便开发人员了解其用途。
  Relaxation    UMETA(DisplayName = "Control Relaxation Input", ToolTip = "Control relaxation when no other input is provided (non-sticky control)"),
 // 定义名为 Relaxation 的枚举值，同样设置了显示名称和提示信息的元数据，其显示名称是 "Control Relaxation Input"，
    // 提示信息说明了该枚举值对应的情况是在没有其他输入提供时用于控制放松（非粘性控制，可能是一种特定的控制逻辑相关概念），便于理解该枚举值在项目中的具体含义。
  Autopilot     UMETA(DisplayName = "Autopilot Input", ToolTip = "Input provided by the built-in autopilot"),
 // 定义 Autopilot 枚举值，元数据中显示名称为 "Autopilot Input"，提示信息表明该枚举值代表由内置自动驾驶系统提供的输入，明确了其在项目中对应的功能场景。
  User          UMETA(DisplayName = "User Input", ToolTip = "Input provided by an user playing in the simulator"),
// 定义 User 枚举值，显示名称是 "User Input"，提示信息指出该枚举值表示由在模拟器中进行操作的用户所提供的输入，体现了其与用户交互相关的含义。
  Client        UMETA(DisplayName = "Client Input", ToolTip = "Input provided by an RPC client connected to the simulator"),
// 定义 Client 枚举值，其显示名称设定为 "Client Input"，提示信息说明该枚举值对应的是由连接到模拟器的远程过程调用（RPC）客户端所提供的输入，
    // 涉及到与外部客户端交互并获取输入的相关功能场景。
  Highest       UMETA(DisplayName = "Highest Priority", ToolTip = "Use for debugging purposes only")
 // 定义 Highest 枚举值，显示名称为 "Highest Priority"，提示信息提示该枚举值仅用于调试目的，与前面的 Lowest 枚举值在调试相关应用场景上形成对应关系。
};

inline static bool operator<=(EVehicleInputPriority Lhs, EVehicleInputPriority Rhs)
// 定义一个内联（inline）的全局静态函数（operator<=），用于重载小于等于（<=）运算符，使其可以对 EVehicleInputPriority 类型的枚举值进行比较操作，
// 这样在代码中可以直接使用 <= 运算符来比较两个 EVehicleInputPriority 枚举值的大小关系，符合 C++ 中对运算符重载的语法和使用习惯。
{
  auto Cast = [](auto e) { return static_cast<typename std::underlying_type<decltype(e)>::type>(e); };
// 使用了 C++11 中的 lambda 表达式定义了一个匿名函数（在这里赋值给 Cast），该函数接收一个任意类型（auto 表示自动推导类型）的参数 e，
    // 函数体内部通过 std::underlying_type 模板（来自 <type_traits> 头文件）获取传入枚举类型（通过 decltype(e) 获取传入参数的类型，即枚举类型）的底层类型，
    // 然后将传入的枚举值 e 转换为其底层类型并返回，目的是为了能基于枚举的底层类型（这里是 uint8）来进行比较操作，因为直接比较枚举值可能不符合预期的语义或者有类型不匹配等问题。
  return Cast(Lhs) <= Cast(Rhs);
// 调用上面定义的 Cast 匿名函数，将左操作数（Lhs）和右操作数（Rhs）都转换为它们对应的底层类型（uint8），然后使用 <= 运算符比较这两个转换后的底层类型值，
    // 最终返回比较结果，以此实现了对 EVehicleInputPriority 枚举类型进行 <= 比较的逻辑，使得代码可以像比较普通数值类型一样比较这个枚举类型的大小关系。
}
