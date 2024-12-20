// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "BaseVegetationActor.h"
#include "Carla/Carla.h"

// ABaseVegetationActor类的成员函数BeginPlay，重写了UE4中Actor类的BeginPlay函数，在游戏开始时执行初始化相关操作，同时记录性能分析事件便于后续性能分析。
void ABaseVegetationActor::BeginPlay()
{
    // 使用TRACE_CPUPROFILER_EVENT_SCOPE宏（应该是用于性能分析的工具，用于标记一个代码块的开始，方便统计该函数执行过程中的CPU使用等性能相关数据），
    // 这里标记了ABaseVegetationActor::BeginPlay函数的执行范围，括号内传入函数名称作为标识，方便在性能分析工具中查看该函数相关的性能情况。
    TRACE_CPUPROFILER_EVENT_SCOPE(ABaseVegetationActor::BeginPlay);
    // 调用父类的BeginPlay函数，确保先执行父类中已有的初始化逻辑，这是在UE4等游戏开发框架中重写函数时的常见做法，以保证继承链上的功能完整性。
    Super::BeginPlay();
}

// ABaseVegetationActor类的成员函数GetParametersFromComponent，用于从USpringBasedVegetationComponent组件中获取各种参数，并复制到SpringParameters结构体中。
void ABaseVegetationActor::GetParametersFromComponent()
{
    // 使用GetComponentByClass函数（UE4中Actor类提供的用于获取指定类型组件的函数），尝试获取当前Actor上类型为USpringBasedVegetationComponent的组件，
    // 返回一个通用的UActorComponent类型指针，后续需要进一步转换为具体的USpringBasedVegetationComponent类型才能使用其特有的属性和函数。
    UActorComponent* Component = 
        GetComponentByClass(USpringBasedVegetationComponent::StaticClass());
    // 使用Cast函数（UE4中的类型转换函数，进行安全的向下转型，确保转换的类型是正确的）将获取到的通用组件指针Component转换为USpringBasedVegetationComponent类型指针，
    // 如果转换成功，SpringComponent指针将指向对应的组件实例，方便后续操作该组件的属性；如果转换失败（组件不存在或者类型不匹配），则SpringComponent为nullptr。
    USpringBasedVegetationComponent* SpringComponent = 
        Cast<USpringBasedVegetationComponent>(Component);
    
    // 判断SpringComponent是否为nullptr，即是否成功获取并转换到对应的组件实例，如果为nullptr，说明没有找到指定类型的组件。
    if (!SpringComponent)
    {
        // 使用UE_LOG宏输出错误日志，日志分类为LogCarla，错误级别为Error，日志内容为提示文本"ABaseVegetationActor::GetParametersFromComponent Component not found"，
        // 用于在开发过程中方便排查组件获取失败的问题，告知开发者找不到对应的组件，无法进行参数获取操作，然后直接返回，不再执行后续代码。
        UE_LOG(LogCarla, Error, 
            TEXT("ABaseVegetationActor::GetParametersFromComponent Component not found"));
        return;
    }

    // 将SpringComponent组件的Beta属性值复制到SpringParameters结构体的Beta成员变量中，实现参数的获取和赋值操作，以下类似的代码行都是依次将组件的各个参数复制到结构体对应成员中。
    SpringParameters.Beta = SpringComponent->Beta;
    SpringParameters.Alpha = SpringComponent->Alpha;
    SpringParameters.Gravity = SpringComponent->Gravity;
    SpringParameters.BaseSpringStrength = SpringComponent->BaseSpringStrength;
    SpringParameters.MinSpringStrength = SpringComponent->MinSpringStrength;
    SpringParameters.HorizontalFallof = SpringComponent->HorizontalFallof;
    SpringParameters.VerticalFallof = SpringComponent->VerticalFallof;
    SpringParameters.RestFactor = SpringComponent->RestFactor;
    SpringParameters.DeltaTimeOverride = SpringComponent->DeltaTimeOverride;
    SpringParameters.CollisionForceParameter = SpringComponent->CollisionForceParameter;
    SpringParameters.CollisionForceMinVel = SpringComponent->CollisionForceMinVel;
    SpringParameters.ForceDistanceFalloffExponent = SpringComponent->ForceDistanceFalloffExponent;
    SpringParameters.ForceMaxDistance = SpringComponent->ForceMaxDistance;
    SpringParameters.MinForceFactor = SpringComponent->MinForceFactor;
    SpringParameters.LineTraceMaxDistance = SpringComponent->LineTraceMaxDistance;
    SpringParameters.CapsuleRadius = SpringComponent->CapsuleRadius;
    SpringParameters.MinBoneLength = SpringComponent->MinBoneLength;
    SpringParameters.SpringStrengthMulFactor = SpringComponent->SpringStrengthMulFactor;
    SpringParameters.VehicleCenterZOffset = SpringComponent->VehicleCenterZOffset;
    SpringParameters.Skeleton = SpringComponent->Skeleton;
    SpringParameters.bAutoComputeStrength = SpringComponent->bAutoComputeStrength;

    // 使用UE_LOG宏输出普通日志，日志分类为LogCarla，日志级别为Log（一般用于记录正常的操作信息），日志内容为提示文本"ABaseVegetationActor::GetParametersFromComponent Copy successful"，
    // 告知开发者参数复制操作成功完成，方便在开发过程中了解函数执行情况。
    UE_LOG(LogCarla, Log, 
        TEXT("ABaseVegetationActor::GetParametersFromComponent Copy successful"));
}

// ABaseVegetationActor类的成员函数SetParametersToComponent，用于将SpringParameters结构体中的各个参数设置到USpringBasedVegetationComponent组件中，实现参数的反向赋值。
void ABaseVegetationActor::SetParametersToComponent()
{
    // 与GetParametersFromComponent函数中类似的操作，先使用GetComponentByClass函数获取当前Actor上类型为USpringBasedVegetationComponent的组件，返回通用的UActorComponent类型指针。
    UActorComponent* Component = 
        GetComponentByClass(USpringBasedVegetationComponent::StaticClass());
    // 再使用Cast函数将获取到的通用组件指针转换为USpringBasedVegetationComponent类型指针，方便后续操作该组件的属性，如果转换失败则SpringComponent为nullptr。
    USpringBasedVegetationComponent* SpringComponent = 
        Cast<USpringBasedVegetationComponent>(Component);
    // 判断SpringComponent是否为nullptr，如果为nullptr，说明没有找到指定类型的组件，无法进行参数设置操作。
    if (!SpringComponent)
    {
        // 使用UE_LOG宏输出错误日志，日志分类为LogCarla，错误级别为Error，日志内容为提示文本"ABaseVegetationActor::SetParametersToComponent Component not found"，
        // 用于告知开发者找不到对应的组件，然后直接返回，不再执行后续代码。
        UE_LOG(LogCarla, Error, TEXT("ABaseVegetationActor::SetParametersToComponent Component not found"));
        return;
    }

    // 将SpringParameters结构体的Beta成员变量的值设置到SpringComponent组件的Beta属性中，以下类似的代码行都是依次将结构体中的各个参数设置到组件对应属性中，实现参数的传递和更新。
    SpringComponent->Beta = SpringParameters.Beta;
    SpringComponent->Alpha = SpringParameters.Alpha;
    SpringComponent->Gravity = SpringParameters.Gravity;
    SpringComponent->BaseSpringStrength = SpringParameters.BaseSpringStrength;
    SpringComponent->MinSpringStrength = SpringParameters.MinSpringStrength;
    SpringComponent->HorizontalFallof = SpringParameters.HorizontalFallof;
    SpringComponent->VerticalFallof = SpringParameters.VerticalFallof;
    SpringComponent->RestFactor = SpringParameters.RestFactor;
    SpringComponent->DeltaTimeOverride = SpringParameters.DeltaTimeOverride;
    SpringComponent->CollisionForceParameter = SpringParameters.CollisionForceParameter;
    SpringComponent->CollisionForceMinVel = SpringParameters.CollisionForceMinVel;
    SpringComponent->ForceDistanceFalloffExponent = SpringParameters.ForceDistanceFalloffExponent;
    SpringComponent->ForceMaxDistance = SpringParameters.ForceMaxDistance;
    SpringComponent->MinForceFactor = SpringParameters.MinForceFactor;
    SpringComponent->LineTraceMaxDistance = SpringParameters.LineTraceMaxDistance;
    SpringComponent->CapsuleRadius = SpringParameters.CapsuleRadius;
    SpringComponent->MinBoneLength = SpringParameters.MinBoneLength;
    SpringComponent->SpringStrengthMulFactor = SpringParameters.SpringStrengthMulFactor;
    SpringComponent->VehicleCenterZOffset = SpringParameters.VehicleCenterZOffset;
    SpringComponent->Skeleton = SpringParameters.Skeleton;
    SpringComponent->bAutoComputeStrength = SpringParameters.bAutoComputeStrength;

}

// ABaseVegetationActor类的成员函数UpdateSkeletonAndParameters，用于更新USpringBasedVegetationComponent组件的骨架和相关参数，并计算分支的弹簧强度，整合了多个相关操作。
void ABaseVegetationActor::UpdateSkeletonAndParameters()
{
    // 同样先使用GetComponentByClass函数获取当前Actor上类型为USpringBasedVegetationComponent的组件，返回通用的UActorComponent类型指针。
    UActorComponent* Component = 
        GetComponentByClass(USpringBasedVegetationComponent::StaticClass());
    // 再使用Cast函数将获取到的通用组件指针转换为USpringBasedVegetationComponent类型指针，如果转换失败则SpringComponent为nullptr，用于后续操作该组件。
    USpringBasedVegetationComponent* SpringComponent = 
        Cast<USpringBasedVegetationComponent>(Component);
    // 判断SpringComponent是否为nullptr，如果为nullptr，说明没有找到指定类型的组件，无法进行后续的更新操作。
    if (!SpringComponent)
    {
        // 使用UE_LOG宏输出错误日志，日志分类为LogCarla，错误级别为Error，日志内容为提示文本"ABaseVegetationActor::UpdateSkeletonAndParameters Component not found"，
        // 告知开发者找不到对应的组件，然后直接返回，不再执行后续代码。
        UE_LOG(LogCarla, Error, TEXT("ABaseVegetationActor::UpdateSkeletonAndParameters Component not found"));
        return;
    }
    // 调用SetParametersToComponent函数，将SpringParameters结构体中的参数设置到SpringComponent组件中，实现参数的更新，确保组件使用最新的参数值。
    SetParametersToComponent();
    // 调用SpringComponent组件的GenerateSkeletonHierarchy函数（应该是用于生成组件相关的骨架层次结构的函数，具体功能取决于该函数的实现逻辑），更新组件的骨架结构。
    SpringComponent->GenerateSkeletonHierarchy();
    // 调用SpringComponent组件的ComputeSpringStrengthForBranches函数（用于计算分支的弹簧强度，根据组件内的相关参数和逻辑进行计算），完成强度计算操作。
    SpringComponent->ComputeSpringStrengthForBranches();
    // 最后调用GetParametersFromComponent函数，重新从更新后的SpringComponent组件中获取参数，可能用于同步组件参数和SpringParameters结构体中的值，或者进行后续的其他相关操作。
    GetParametersFromComponent();
}
