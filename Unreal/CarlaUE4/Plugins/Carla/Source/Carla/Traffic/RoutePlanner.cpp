// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "RoutePlanner.h"

#include "Util/RandomEngine.h"
#include "Vehicle/CarlaWheeledVehicle.h"
#include "Vehicle/WheeledVehicleAIController.h"

#include "Engine/CollisionProfile.h"
#include "DrawDebugHelpers.h"

// 判断给定的样条组件是否有效，有效条件是组件不为空且样条点数大于1
static bool IsSplineValid(const USplineComponent *SplineComponent)
{
    return (SplineComponent!= nullptr) &&
           (SplineComponent->GetNumberOfSplinePoints() > 1);
}

// 获取给定Actor对应的车辆AI控制器，如果Actor即将被销毁则返回nullptr，
// 先尝试将Actor转换为ACarlaWheeledVehicle类型，若成功再获取其控制器并转换为AWheeledVehicleAIController类型返回
static AWheeledVehicleAIController *GetVehicleController(AActor *Actor)
{
    auto *Vehicle = (Actor->IsPendingKill()? nullptr : Cast<ACarlaWheeledVehicle>(Actor));
    return (Vehicle!= nullptr?
            Cast<AWheeledVehicleAIController>(Vehicle->GetController()) :
            nullptr);
}

// 根据给定的随机引擎、路径数组（USplineComponent类型）以及对应的概率数组，
// 按照概率随机选择一条路径（USplineComponent）并返回。
// 如果路径数组只有一个元素，则直接返回该元素；
// 否则根据概率权重选择对应的路径索引，并返回对应路径
static const USplineComponent *PickARoute(
    URandomEngine &RandomEngine,
    const TArray<USplineComponent *> &Routes,
    const TArray<float> &Probabilities)
{
    check(Routes.Num() > 0);

    if (Routes.Num() == 1)
    {
        return Routes[0];
    }

    auto Index = RandomEngine.GetIntWithWeight(Probabilities);
    check((Index >= 0) && (Index < Routes.Num()));
    return Routes[Index];
}

// ARoutePlanner类的构造函数，初始化根组件、触发体组件等相关属性
ARoutePlanner::ARoutePlanner(const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer)
{
    // 创建默认的场景根组件作为根组件，设置其移动性为静态
    RootComponent =
        ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneRootComponent"));
    RootComponent->SetMobility(EComponentMobility::Static);

    // 创建一个盒体组件作为触发体，用于检测车辆等Actor的重叠事件
    TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
    TriggerVolume->SetupAttachment(RootComponent);
    TriggerVolume->SetHiddenInGame(true);
    TriggerVolume->SetMobility(EComponentMobility::Static);
    TriggerVolume->SetCollisionProfileName(FName("OverlapAll"));
    TriggerVolume->SetGenerateOverlapEvents(true);

    // 设置触发体盒体的大小，此处不要随意更改默认值，因为自动驾驶相关功能依赖于此
    TriggerVolume->SetBoxExtent(FVector{32.0f, 32.0f, 32.0f});
}

// 在对象即将销毁时调用，清理已有的路径相关数据
void ARoutePlanner::BeginDestroy()
{
    CleanRoute();
    Super::BeginDestroy();
}

#if WITH_EDITOR
// 在编辑器中属性发生变化后调用，用于处理路径和概率数组相关的逻辑
// 如果属性发生变化且路径数组和概率数组大小不一致，会重置概率数组并为新的路径创建默认的样条组件等初始化操作
void ARoutePlanner::PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    const auto Size = Routes.Num();
    if (PropertyChangedEvent.Property && (Size!= Probabilities.Num()))
    {
        Probabilities.Reset(Size);
        for (auto i = 0; i < Size; ++i)
        {
            Probabilities.Add(1.0f / static_cast<float>(Size));
            if (Routes[i] == nullptr)
            {
                Routes[i] = NewObject<USplineComponent>(this);
                Routes[i]->SetupAttachment(RootComponent);
                Routes[i]->SetHiddenInGame(true);
                Routes[i]->SetMobility(EComponentMobility::Static);
                Routes[i]->RegisterComponent();
            }
        }
    }
}
#endif // WITH_EDITOR

// 添加一条新的路径，根据给定的概率以及路径上的点数组创建一个新的样条组件表示路径，
// 并将其添加到路径数组Routes中，同时将对应的概率添加到概率数组Probabilities中
void ARoutePlanner::AddRoute(float probability, const TArray<FVector> &routePoints)
{
    USplineComponent *NewSpline = NewObject<USplineComponent>(this);
    NewSpline->bHiddenInGame = true;

#if WITH_EDITOR
    // 在编辑器中设置未选中样条线段的颜色
    NewSpline->EditorUnselectedSplineSegmentColor = FLinearColor(1.f, 0.15f, 0.15f);
#endif // WITH_EDITOR

    // 设置样条组件的起始点位置
    NewSpline->SetLocationAtSplinePoint(0, routePoints[0], ESplineCoordinateSpace::World, true);
    NewSpline->SetLocationAtSplinePoint(1, routePoints[1], ESplineCoordinateSpace::World, true);

    // 依次添加路径上剩余的点到样条组件中
    for (int i = 2; i < routePoints.Num(); ++i)
    {
        NewSpline->AddSplinePoint(routePoints[i], ESplineCoordinateSpace::World, true);
    }

    Routes.Add(NewSpline);
    Probabilities.Add(probability);
}

// 清空已有的路径数组Routes和概率数组Probabilities，释放相关内存
void ARoutePlanner::CleanRoute()
{
    Routes.Empty();
    Probabilities.Empty();
}

// 为给定的车辆AI控制器分配一条随机路径，前提是控制器未处于即将销毁状态且其随机引擎有效
// 通过随机选择一条路径，提取路径上的路点信息，并设置给控制器作为固定路径
void ARoutePlanner::AssignRandomRoute(AWheeledVehicleAIController &Controller) const
{
    if (!Controller.IsPendingKill() && (Controller.GetRandomEngine()!= nullptr))
    {
        auto *RandomEngine = Controller.GetRandomEngine();
        auto *Route = PickARoute(*RandomEngine, Routes, Probabilities);

        TArray<FVector> WayPoints;
        const auto Size = Route->GetNumberOfSplinePoints();
        if (Size > 1)
        {
            WayPoints.Reserve(Size);
            for (auto i = 1; i < Size; ++i)
            {
                WayPoints.Add(Route->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World));
            }

            Controller.SetFixedRoute(WayPoints);
        }
        else
        {
            // 如果路径的样条点数小于等于1，输出错误日志提示
            UE_LOG(LogCarla, Error, TEXT("ARoutePlanner '%s' has a route with zero way-points."), *GetName());
        }
    }

}

// 初始化函数，用于检查路径相关的合法性以及注册触发体的重叠开始事件委托
// 如果没有分配路径，输出警告日志；
// 遍历已有的路径，检查路径的样条组件是否有效，无效则输出错误日志并返回；
// 如果触发体的重叠开始事件委托未绑定当前类的对应函数，则进行绑定
void ARoutePlanner::Init()
{
    if (Routes.Num() < 1)
    {
        UE_LOG(LogCarla, Warning, TEXT("ARoutePlanner '%s' has no route assigned."), *GetName());
        return;
    }

    for (auto &&Route : Routes)
    {
        if (!IsSplineValid(Route))
        {
            UE_LOG(LogCarla, Error, TEXT("ARoutePlanner '%s' has a route with zero way-points."), *GetName());
            return;
        }
    }

    // 注册委托，当触发体开始与其他组件重叠时调用OnTriggerBeginOverlap函数
    if (!TriggerVolume->OnComponentBeginOverlap.IsAlreadyBound(this, &ARoutePlanner::OnTriggerBeginOverlap))
    {
        TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &ARoutePlanner::OnTriggerBeginOverlap);
    }
}

// 在游戏开始时调用，调用父类的BeginPlay函数后执行初始化操作
void ARoutePlanner::BeginPlay()
{
    Super::BeginPlay();
    Init();
}

// 在游戏结束时调用，用于移除触发体重叠开始事件的委托，然后调用父类的EndPlay函数
void ARoutePlanner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 移除委托，取消绑定OnComponentBeginOverlap事件与OnTriggerBeginOverlap函数的关联
    if (TriggerVolume->OnComponentBeginOverlap.IsAlreadyBound(this, &ARoutePlanner::OnTriggerBeginOverlap))
    {
        TriggerVolume->OnComponentBeginOverlap.RemoveDynamic(this, &ARoutePlanner::OnTriggerBeginOverlap);
    }

    Super::EndPlay(EndPlayReason);
}

// 当触发体与其他Actor开始重叠时调用的函数，获取重叠的Actor对应的车辆AI控制器，
// 如果控制器存在，则为其分配一条随机路径
void ARoutePlanner::OnTriggerBeginOverlap(
    UPrimitiveComponent * /*OverlappedComp*/,
    AActor *OtherActor,
    UPrimitiveComponent * /*OtherComp*/,
    int32 /*OtherBodyIndex*/,
    bool /*bFromSweep*/,
    const FHitResult & /*SweepResult*/)
{
    auto *Controller = GetVehicleController(OtherActor);
    if (Controller!= nullptr)
    {
        AssignRandomRoute(*Controller);
    }
}

// 在编辑器中绘制路径，遍历所有路径的样条点，根据一定规则绘制线段来表示路径，
// 颜色根据是否是交叉点（bIsIntersection）有所不同，线段粗细也会根据点的位置动态变化
void ARoutePlanner::DrawRoutes()
{
#if WITH_EDITOR
    for (int i = 0, lenRoutes = Routes.Num(); i < lenRoutes; ++i)
    {
        for (int j = 0, lenNumPoints = Routes[i]->GetNumberOfSplinePoints() - 1; j < lenNumPoints; ++j)
        {
            const FVector p0 = Routes[i]->GetLocationAtSplinePoint(j + 0, ESplineCoordinateSpace::World);
            const FVector p1 = Routes[i]->GetLocationAtSplinePoint(j + 1, ESplineCoordinateSpace::World);

            static const float MinThickness = 3.f;
            static const float MaxThickness = 15.f;

            const float Dist = (float) j / (float) lenNumPoints;
            const float OneMinusDist = 1.f - Dist;
            const float Thickness = OneMinusDist * MaxThickness + MinThickness;

            if (bIsIntersection)
            {
                // 从蓝色到黑色渐变绘制线段（如果是交叉点情况）
                DrawDebugLine(
                    GetWorld(), p0, p1, FColor(0, 0, 255 * OneMinusDist),
                    true, -1.f, 0, Thickness);
            }
            else
            {
                // 从绿色到黑色渐变绘制线段（非交叉点情况）
                DrawDebugLine(
                    GetWorld(), p0, p1, FColor(0, 255 * OneMinusDist, 0),
                    true, -1.f, 0, Thickness);
            }
        }
    }
#endif
}
