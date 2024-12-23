// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 预处理指令，用于保证该头文件在整个编译过程中只会被编译一次，防止因重复包含此头文件而引发编译错误，这是一种常见的避免头文件重复包含问题的做法。

#include "CarlaWheeledVehicle.h"
// 引入 "CarlaWheeledVehicle.h" 头文件，该头文件中应该定义了 ACarlaWheeledVehicle 类相关的内容，
// 意味着当前要定义的 ACarlaWheeledVehicleNW 类很可能会继承自或者依赖这个头文件里所定义的类的功能、成员等。

#include "CarlaWheeledVehicleNW.generated.h"
// 包含 "CarlaWheeledVehicleNW.generated.h" 头文件，在使用虚幻引擎开发时，这类由引擎自动生成代码相关的头文件非常关键，
// 它会配合虚幻引擎的代码生成工具，生成诸如反射、序列化等相关的必要代码，以保障类能在虚幻引擎体系下正常工作，比如能在蓝图中使用、进行对象持久化等操作。

/// CARLA轮式车辆的基类
// 这是一种文档注释（通常符合 Doxygen 等文档生成工具的注释规范），用于简要描述下面定义的类的基本性质，
// 这里说明 ACarlaWheeledVehicleNW 类是 CARLA 项目中轮式车辆相关的基类，方便代码阅读者快速了解类的大致用途。
UCLASS()
// 使用 UCLASS 宏来声明这是一个虚幻引擎中的类，通过这个宏，虚幻引擎能够对该类进行一系列的处理，
// 比如将类注册到引擎中，使其能够按照引擎的规则进行实例化、在蓝图中进行可视化编辑、参与游戏运行时的各种机制等，这里括号内没有添加额外的参数，意味着采用默认的类相关设置。
class CARLA_API ACarlaWheeledVehicleNW : public ACarlaWheeledVehicle
{
  GENERATED_BODY()
// 这个宏用于告知虚幻引擎的代码生成工具在当前位置生成相应的类相关代码体，这些代码主要涉及反射等机制相关的内容，
    // 是保证类在虚幻引擎环境下能正常实现诸如属性暴露、蓝图交互、序列化与反序列化等功能的关键部分。

  // ===========================================================================
  /// @name Constructor and destructor
  // ===========================================================================
  /// @{
// 以上这部分是一种代码文档注释风格，用于对下面的代码块进行分组和命名说明，这里表明接下来的代码是关于构造函数和析构函数相关的部分，
    // 方便阅读代码时快速定位和理解相关函数的作用范围及所属类别，@{ 和后面对应的 @} 用于界定这个分组注释的范围。

public:
// 以下是类的公有部分，外部代码可以访问这些成员，通常用于提供类的构造、析构以及对外公开的接口等功能。

  ACarlaWheeledVehicleNW(const FObjectInitializer &ObjectInitializer);
// 声明类的构造函数，它接受一个 FObjectInitializer 类型的常量引用作为参数，
    // FObjectInitializer 一般用于在创建对象时传递一些初始化相关的配置信息，这个构造函数用于初始化 ACarlaWheeledVehicleNW 类的对象实例。


  ~ACarlaWheeledVehicleNW();
// 声明类的析构函数，析构函数用于在对象生命周期结束时（例如对象被销毁时）执行一些清理资源、释放内存等必要的操作，确保程序的正确运行和资源的合理利用。
};
