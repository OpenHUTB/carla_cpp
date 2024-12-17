// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// 以下是为了解决Windows系统下的函数名称冲突问题的 workaround（变通方法）。
// 在Windows中，会根据是否定义了UNICODE宏，将一些函数（如DrawText、LoadLibrary等）
// 重命名为对应的Unicode版本（DrawTextW、LoadLibraryW等）或者ANSI版本（DrawTextA、LoadLibraryA等），
// 这种改变是对整个编译器全局生效的。而在一些头文件深处，Windows有类似这样的定义逻辑：
// #ifdef UNICODE
//   #define DrawText  DrawTextW
//   #define LoadLibrary LoadLibraryW
// #else
//   #define DrawText  DrawTextA
//   #define LoadLibrary LoadLibraryA
// #endif
// 这样一来，链接器在外部DLL中查找DrawTextW等函数时，就会出现无法解析的外部错误，
// 因为Unreal中只有DrawText这个函数名（没有DrawTextW这种形式），所以通过下面这行代码
// 取消定义DrawText这个与Windows API在Unicode情况下有冲突的函数名定义，来避免该问题。
#undef DrawText

// 引入Carla相关的头文件，可能包含了该项目中自定义的一些基础类、结构体、函数等的声明和定义，
// 具体内容取决于Carla这个模块的实现，是整个项目构建的基础依赖。
#include "Carla.h"
// 引入数组容器相关的头文件，用于操作动态大小的数组类型，方便存储和管理多个同类型元素。
#include "Containers/Array.h"
// 引入游戏框架中HUD（ Heads-Up Display，即平视显示器）相关的头文件，
// 应该包含了与游戏界面显示相关的基类等内容，为当前类继承自AHUD做准备。
#include "GameFramework/HUD.h"
// 引入轮式车辆移动组件相关的头文件，可能用于处理车辆的移动逻辑等相关功能，
// 比如获取车辆的速度、转向等信息，这里应该是和车辆调试等功能相关的使用场景。
#include "WheeledVehicleMovementComponent.h"
// 这是一个生成代码相关的头文件，用于和虚幻引擎的反射系统配合，自动生成一些必要的代码，
// 比如类的序列化、属性的编辑相关代码等，一般是基于虚幻引擎的特定宏机制实现的。
#include "CarlaHUD.generated.h"

// 定义一个结构体HUDString，用于在HUD上显示字符串相关的信息。
// 包含要显示的字符串内容（FString类型，是虚幻引擎中用于处理字符串的类型）、
// 显示位置（用三维向量FVector表示，指定在屏幕空间或者游戏世界空间中的位置）、
// 颜色（FColor类型，用于指定字符串显示的颜色）以及存活时间（TimeToDie，
// 可以理解为经过多长时间后该字符串显示信息应该被移除，用于控制显示时长）。
struct HUDString
{
  FString Str { "" };
  FVector Location;
  FColor Color;
  double TimeToDie;
};

// 定义一个结构体HUDLine，用于在HUD上显示线条相关的信息。
// 包含线条的起始点（Begin，用三维向量表示起始位置）、终点（End，同样是三维向量表示结束位置）、
// 线条的粗细（Thickness，以浮点数表示粗细程度）、线条的颜色（Color，指定线条显示的颜色）
// 以及存活时间（TimeToDie，用于控制该线条显示多长时间后应该被移除）。
struct HUDLine
{
  FVector Begin;
  FVector End;
  float Thickness;
  FColor Color;
  double TimeToDie;
};

// 使用UCLASS宏声明这是一个可以被虚幻引擎反射系统识别的类，是面向对象编程中类的定义部分，
// 该类继承自AHUD，意味着它可以复用AHUD类的功能并在此基础上进行扩展，用于在游戏中实现自定义的HUD显示逻辑。
// CARLA_API可能是用于控制该类的导出等相关属性，方便在不同模块间使用，具体取决于项目中的定义。
UCLASS()
class CARLA_API ACarlaHUD : public AHUD
{
  // 这个宏用于和虚幻引擎的代码生成系统配合，生成必要的类相关代码，比如默认构造函数、
  // 序列化相关代码等，是虚幻引擎中自定义类时的标准写法，用于保证类的正确构建和运行。
  GENERATED_BODY()

public:
  // 自定义的构造函数，接受一个FObjectInitializer类型的参数，用于初始化对象的属性等。
  // 通过调用父类（Super）的构造函数，并传入ObjectInitializer参数来完成基类部分的初始化。
  // 同时设置了PrimaryActorTick的bCanEverTick属性为false，表示该Actor（在虚幻引擎中，
  // 继承自AActor的类实例通常被视为Actor，这里的AHUD间接继承自AActor）默认情况下不进行每帧更新，
  // 可能是因为该HUD的显示内容不是每帧都需要动态改变，或者更新逻辑由其他手动调用的方法来控制。
  ACarlaHUD(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
  {
    PrimaryActorTick.bCanEverTick = false;
  }

  // 重写了AHUD类中的DrawHUD虚函数，用于实现自定义的在HUD上绘制内容的逻辑，
  // 具体的绘制操作应该会在这个函数的实现中编写，比如绘制文本、图形等。
  virtual void DrawHUD() override;

  // 一个指向轮式车辆移动组件的指针，用于调试车辆相关信息，初始化为nullptr，
  // 可以通过后面的函数来设置具体指向的车辆移动组件，方便获取车辆的运行状态等信息用于显示在HUD上。
  UWheeledVehicleMovementComponent* DebugVehicle{nullptr};

  // 函数用于添加一个车辆移动组件，以便进行车辆相关的遥测调试（Telemetry通常涉及收集车辆运行数据等功能），
  // 将传入的车辆移动组件指针赋值给DebugVehicle成员变量，使得该HUD可以关联到具体要调试的车辆。
  void AddDebugVehicleForTelemetry(UWheeledVehicleMovementComponent* Veh) { DebugVehicle = Veh; }

  // 函数用于向HUD添加要显示的字符串信息，接受字符串内容、显示位置、颜色以及存活时间作为参数，
  // 会将这些信息封装到HUDString结构体中，并添加到StringList数组里，以便后续在DrawHUD等函数中根据这些信息进行绘制显示。
  void AddHUDString(const FString Str, const FVector Location, const FColor Color, double LifeTime);

  // 函数用于向HUD添加要显示的线条信息，接受线条的起始点、终点、粗细、颜色以及存活时间作为参数，
  // 会将这些信息封装到HUDLine结构体中，并添加到LineList数组里，方便后续在DrawHUD等函数中根据这些参数进行实际的线条绘制显示。
  void AddHUDLine(const FVector Begin, const FVector End, const float Thickness, const FColor Color, double LifeTime);

private:
  // 定义一个数组，用于存储要在HUD上显示的字符串相关信息，通过前面的AddHUDString函数添加元素，
  // 在DrawHUD等函数中遍历该数组来进行字符串的绘制显示操作。
  TArray<HUDString> StringList;
  // 定义一个数组，用于存储要在HUD上显示的线条相关信息，通过前面的AddHUDLine函数添加元素，
  // 在DrawHUD等函数中遍历该数组来进行线条的绘制显示操作。
  TArray<HUDLine> LineList;
};
