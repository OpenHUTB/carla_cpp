// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// 版权声明，表明该代码受 MIT 许可证条款的许可，如需查看许可证内容，可通过给定的网址访问
// For a copy, see <https://opensource.org/licenses/MIT>.

// 预编译指令，确保头文件在多次被包含时只被编译一次，避免重复定义等问题
#pragma once

// 引入 "Kismet/BlueprintFunctionLibrary.h" 头文件，该头文件用于定义蓝图函数库相关的类和功能，
// 使得当前定义的类可以作为蓝图中可调用的函数库，方便在虚幻引擎的蓝图系统中使用相关功能
#include "Kismet/BlueprintFunctionLibrary.h"

// 引入 "GameFramework/Actor.h" 头文件，该头文件定义了 Actor 类相关的内容，Actor 是虚幻引擎中游戏对象的基类，
// 这里可能后续会与游戏世界中的各种对象交互等操作相关（虽然在当前代码片段中未明确体现其具体使用方式）
#include "GameFramework/Actor.h"

// 引入 <compiler/disable-ue4-macros.h> 和 <compiler/enable-ue4-macros.h>，这两个头文件通常用于控制是否启用虚幻引擎特定的一些宏定义，
// 可能在编译过程中根据需要来开启或关闭某些与引擎相关的编译时特性或代码逻辑
#include <compiler/disable-ue4-macros.h>
#include "carla/rpc/ObjectLabel.h"
#include "carla/rpc/LabelledPoint.h"
#include <compiler/enable-ue4-macros.h>

// 引入 <vector> 头文件，用于使用 C++ 标准库中的向量容器 std::vector，方便存储一组相关元素，比如这里用于存储 LabelledPoint 类型的元素
// 引入 <utility> 头文件，用于使用一些实用工具，例如 std::pair 类型，它可以将两个不同类型的值组合在一起，在代码中用于返回特定的组合结果
#include <vector>
#include <utility>

// 引入 "RayTracer.generated.h" 头文件，这是一个由虚幻引擎的代码生成工具生成的头文件，通常包含了与当前类相关的反射等元数据信息，
// 对于类能够在虚幻引擎中被正确识别、序列化、蓝图调用等操作起到关键作用
#include "RayTracer.generated.h"

// 使用 UCLASS() 宏定义，这是虚幻引擎中用于标记一个类可以被引擎的反射系统识别的宏，使得该类可以在蓝图、序列化等引擎相关功能中使用
// 这里定义了一个名为 URayTracer 的类，它继承自 UBlueprintFunctionLibrary，意味着它将作为一个蓝图函数库类，提供一些可在蓝图中调用的静态函数
UCLASS()
class CARLA_API URayTracer : public UBlueprintFunctionLibrary
{
    // 使用 GENERATED_BODY() 宏，这是虚幻引擎要求的，用于指示编译器在这里生成与反射、序列化等相关的必要代码，
    // 保证类在引擎中的各种特性能够正常工作
    GENERATED_BODY()

public:
    // 定义一个名为 CastRay 的静态函数，它接受起点位置（FVector 类型，在虚幻引擎中通常用于表示三维空间中的向量，这里表示射线起点坐标）、
    // 终点位置（同样是 FVector 类型，表示射线终点坐标）以及指向游戏世界的指针（UWorld * 类型，用于与游戏世界进行交互，比如获取世界中的对象等）作为参数，
    // 函数返回一个 std::vector<carla::rpc::LabelledPoint> 类型的结果，推测其功能可能是沿着给定的射线方向（从起点到终点）进行一些检测或计算，
    // 并返回一系列带有标签的点（LabelledPoint 类型）的集合，这些点可能是射线与游戏世界中某些对象相交的位置等情况对应的点
    static std::vector<carla::rpc::LabelledPoint> CastRay(
        FVector StartLocation, FVector EndLocation, UWorld * World);

    // 定义一个名为 ProjectPoint 的静态函数，它接受起点位置（FVector 类型）、方向向量（FVector 类型，表示射线的方向）、
    // 最大距离（float 类型，限制射线延伸的最大长度）以及指向游戏世界的指针（UWorld * 类型）作为参数，
    // 函数返回一个 std::pair<bool, carla::rpc::LabelledPoint> 类型的结果，其中 bool 类型的值可能用于表示是否成功投影（比如射线是否在最大距离内与某个对象相交等情况），
    // 而 carla::rpc::LabelledPoint 类型则可能是投影点对应的带有标签的点信息，整体功能推测是将一个点按照给定的方向和最大距离投影到游戏世界中，
    // 并返回投影结果的相关信息
    static std::pair<bool, carla::rpc::LabelledPoint> ProjectPoint(
        FVector StartLocation, FVector Direction, float MaxDistance, UWorld * World);
};
