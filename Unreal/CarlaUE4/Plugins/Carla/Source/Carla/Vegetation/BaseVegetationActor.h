// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 这是一个预编译头文件保护指令，确保该头文件内容在一个编译单元中只被包含一次
#pragma once

// 包含游戏框架中Actor类的头文件，Actor是游戏场景中可放置、具有行为等特性的基础对象类
#include "GameFramework/Actor.h"
// 包含基于弹簧的植被组件相关的头文件，推测该组件用于处理植被相关的模拟逻辑等
#include "SpringBasedVegetationComponent.h"

// 这个宏用于声明一个可以被虚幻引擎蓝图系统识别和使用的结构体或类等，此处用于声明接下来的结构体
#include "BaseVegetationActor.generated.h"

// 使用USTRUCT宏定义一个名为FSpringBasedVegetationParameters的结构体，并且标记它可以在蓝图中使用（BlueprintType）
USTRUCT(BlueprintType)
struct FSpringBasedVegetationParameters
{
    // 这个宏用于生成结构体相关的必要代码，比如反射相关的代码等，是虚幻引擎特定的代码生成机制要求
    GENERATED_BODY()

    // 使用UPROPERTY宏声明一个可编辑（EditAnywhere）、能在蓝图中读写（BlueprintReadWrite）的属性Beta，
    // 它属于"Spring Based Vegetation Component"分类，默认值为0.5f，用于相关植被模拟等逻辑中的某个参数（具体含义需结合具体业务）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
    float Beta = 0.5f;

    // 类似地，声明属性Alpha，默认值为0.f，用于相关植被模拟等逻辑中的某个参数，同样可在蓝图中编辑和读写
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
    float Alpha = 0.f;

    // 声明一个三维向量类型的属性Gravity，用于表示重力方向和大小等信息，默认值为向下的单位向量(0, 0, -1)，用于植被模拟中受重力影响的相关计算等
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
    FVector Gravity = FVector(0, 0, -1);

    // 声明属性BaseSpringStrength，代表基础弹簧强度，默认值为10000.f，用于植被模拟中涉及弹簧力学相关的计算参数设定
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
    float BaseSpringStrength = 10000.f;

    // 声明属性MinSpringStrength，代表最小弹簧强度，默认值为2000.f，可能用于限制弹簧强度下限等相关计算
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
    float MinSpringStrength = 2000.f;

    // 声明属性HorizontalFallof，水平衰减系数，默认值为0.1f，可能用于模拟植被在水平方向上某个属性随距离等因素的衰减情况
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
    float HorizontalFallof = 0.1f;

    // 声明属性VerticalFallof，垂直衰减系数，默认值为0.1f，类似地可能用于模拟植被在垂直方向上某个属性随距离等因素的衰减情况
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
    float VerticalFallof = 0.1f;

    // 声明属性RestFactor，休息因子（具体含义需结合具体业务逻辑），默认值为0.5f，用于植被模拟中相关的状态计算等
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
    float RestFactor = 0.5f;

    // 声明属性DeltaTimeOverride，时间步长覆盖值，默认值为 -1.f，可能用于在特定情况下覆盖默认的时间步长计算，用于植被模拟的时间相关逻辑
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
    float DeltaTimeOverride = -1.f;

    // 声明属性CollisionForceParameter，碰撞力参数，默认值为10.f，用于在植被模拟中计算碰撞相关的力的作用等情况
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
    float CollisionForceParameter = 10.f;

    // 声明属性CollisionForceMinVel，碰撞力最小速度，默认值为1.f，可能用于判断碰撞产生有效力的最小速度阈值等情况
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
    float CollisionForceMinVel = 1.f;

    // 声明属性ForceDistanceFalloffExponent，力距离衰减指数，默认值为1.f，用于描述力随距离衰减的数学关系中的指数参数
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
    float ForceDistanceFalloffExponent = 1.f;

    // 声明属性ForceMaxDistance，最大力作用距离，默认值为180.f，用于限制力在一定距离范围内起作用，超出此距离则不考虑该力的影响
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
    float ForceMaxDistance = 180.f;

    // 声明属性MinForceFactor，最小力因子，默认值为0.01，可能用于对力的大小进行下限限制等相关计算
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
    float MinForceFactor = 0.01;

    // 声明属性LineTraceMaxDistance，线追踪最大距离，默认值为180.f，可能用于射线检测等操作的最大距离限制，比如检测植被与其他物体的距离等情况
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
    float LineTraceMaxDistance = 180.f;

    // 声明属性CapsuleRadius，胶囊体半径，默认值为6.0f，可能用于将植被模拟相关的物体用胶囊体进行碰撞等物理模拟时的形状参数设定
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
    float CapsuleRadius = 6.0f;

    // 声明属性MinBoneLength，最小骨骼长度，默认值为10.f，可能在涉及植被骨骼结构模拟等情况时用于限制骨骼长度下限
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
    float MinBoneLength = 10.f;

    // 声明一个三维向量类型的属性SpringStrengthMulFactor，弹簧强度乘法因子，默认值为(1, 1, 1)，用于对弹簧强度进行按向量各分量的缩放等操作
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
    FVector SpringStrengthMulFactor = FVector(1, 1, 1);

    // 声明属性VehicleCenterZOffset，车辆中心Z轴偏移量，默认值为120.f，可能用于在涉及车辆与植被交互场景中，调整车辆相对植被在垂直方向上的位置关系
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
    float VehicleCenterZOffset = 120.f;

    // 声明一个FSkeletonHierarchy类型的属性Skeleton，用于存储植被相关的骨骼层次结构信息（具体FSkeletonHierarchy结构体定义需看对应头文件）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
    FSkeletonHierarchy Skeleton;

    // 声明属性bAutoComputeStrength，是否自动计算强度，默认值为true，用于控制植被模拟中强度相关参数是否自动计算的逻辑开关
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation Component")
    bool bAutoComputeStrength = true;
};

// 使用UCLASS宏定义一个名为ABaseVegetationActor的类，它继承自AActor类，表明它是游戏场景中的一个可放置、可交互的对象
// 并且该类可以被虚幻引擎的蓝图系统识别和使用（CARLA_API可能是用于控制该类的导出等相关功能的宏定义，具体要看相关配置）
UCLASS()
class CARLA_API ABaseVegetationActor : public AActor
{
    // 这个宏用于生成类相关的必要代码，比如反射相关的代码等，是虚幻引擎特定的代码生成机制要求
    GENERATED_BODY()

public:
    // 重写AActor类中的BeginPlay虚函数，该函数在游戏开始时（Actor被创建并开始参与游戏逻辑时）会被调用，
    // 通常用于进行一些初始化等操作，子类可以根据自身需求实现具体逻辑
    virtual void BeginPlay() override;

    // 使用UPROPERTY宏声明一个可编辑（EditAnywhere）、能在蓝图中读写（BlueprintReadWrite）的属性SpringParameters，
    // 它属于"Spring Based Vegetation"分类，用于存储基于弹簧的植被模拟相关的参数，通过这个属性可以在外部（比如蓝图中）方便地设置和获取这些参数
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spring Based Vegetation")
    FSpringBasedVegetationParameters SpringParameters;

    // 使用UFUNCTION宏声明一个可在蓝图中调用（BlueprintCallable）、能在编辑器中调用（CallInEditor）的函数GetParametersFromComponent，
    // 属于"Spring Based Vegetation"分类，该函数可能用于从相关的植被组件中获取参数并赋值给SpringParameters属性等操作
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Spring Based Vegetation")
    void GetParametersFromComponent();

    // 类似地，声明一个可在蓝图中调用的函数SetParametersToComponent，用于将SpringParameters属性中的参数设置到相关的植被组件中，实现参数传递等功能
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Spring Based Vegetation")
    void SetParametersToComponent();

    // 声明一个可在蓝图中调用的函数UpdateSkeletonAndParameters，用于更新植被相关的骨骼结构以及参数，可能涉及到根据游戏运行时情况动态调整相关数据等操作
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Spring Based Vegetation")
    void UpdateSkeletonAndParameters();
};
