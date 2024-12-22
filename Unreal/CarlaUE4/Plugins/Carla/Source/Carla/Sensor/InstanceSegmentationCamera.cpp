// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 引入Carla相关的主头文件，可能包含了整个项目的一些基础定义、类型等信息
#include "Carla.h"  
// 引入实例分割相机相关的头文件，应该定义了与实例分割相机功能相关的类等内容
#include "Carla/Sensor/InstanceSegmentationCamera.h"  
// 引入带有标签组件相关的头文件，可能用于处理游戏中带有特定标签的组件相关功能
#include "Carla/Game/TaggedComponent.h"  
// 引入用于在Actor蓝图中操作函数库的头文件，方便进行与Actor蓝图相关的一些操作，比如创建定义等
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"  
// 引入像素读取相关的头文件，可能用于处理从传感器等获取的像素数据相关操作
#include "Carla/Sensor/PixelReader.h"  
// 引入二维场景捕获组件相关的头文件，用于在场景中捕获相关画面等操作
#include "Components/SceneCaptureComponent2D.h"  

// 定义函数AInstanceSegmentationCamera::GetSensorDefinition，用于获取实例分割相机的传感器定义
// 该函数返回一个FActorDefinition类型的定义信息，大概率是用于描述该相机传感器在整个系统中的相关属性定义
FActorDefinition AInstanceSegmentationCamera::GetSensorDefinition()
{
  // 通过调用UActorBlueprintFunctionLibrary中的MakeCameraDefinition函数来创建并返回相机的定义信息
  // 传入的参数"instance_segmentation"可能是用于标识该相机是实例分割相机的特定字符串，用于区分不同类型的相机定义
  return UActorBlueprintFunctionLibrary::MakeCameraDefinition(TEXT("instance_segmentation"));
}

// AInstanceSegmentationCamera类的构造函数，接受一个FObjectInitializer类型的参数用于初始化对象
// 该参数通常包含了创建对象时所需的各种初始化设置信息
AInstanceSegmentationCamera::AInstanceSegmentationCamera(
    const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  // 调用AddPostProcessingMaterial函数添加一个后处理材质，传入的字符串是材质的路径
  // 这里添加的材质路径对应的材质用于物理镜头畸变相关的后处理效果（具体功能取决于材质本身实现）
  AddPostProcessingMaterial(TEXT("Material'/Carla/PostProcessingMaterials/PhysicLensDistortion.PhysicLensDistortion'"));
  
  // TODO注释表示此处有待办事项，计划是设置OnActorSpawnHandler，目的是能够在有Actor生成时刷新相关组件
  // 目前这行代码被注释掉了，可能还未完成相关功能的实现或者暂时不需要此功能开启
  // World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateRaw(this, &AInstanceSegmentationCamera::OnActorSpawned));
}

// 函数用于设置二维场景捕获组件（SceneCapture）的相关属性和配置
void AInstanceSegmentationCamera::SetUpSceneCaptureComponent(USceneCaptureComponent2D &SceneCapture)
{
  // 调用父类的SetUpSceneCaptureComponent函数，先执行父类中关于场景捕获组件设置的通用逻辑
  Super::SetUpSceneCaptureComponent(SceneCapture);
  
  // 应用视图模式，传入的参数VMI_Unlit表示无光照视图模式，后面两个参数表示是否强制应用以及对应的显示标志（ShowFlags）
  // 这行代码的作用是将场景捕获组件设置为无光照的视图模式
  ApplyViewMode(VMI_Unlit, true, SceneCapture.ShowFlags);
  
  // 设置显示标志（ShowFlags），让带有标签的组件（TaggedComponent）能够被绘制（默认是不绘制的情况）
  // 这里的TaggedComponent应该会检测到这个设置并根据情况设置代理材质的视图相关性
  SceneCapture.ShowFlags.SetNotDrawTaggedComponents(false); 
  
  // 设置显示标志（ShowFlags），关闭大气效果显示，意味着在该场景捕获中不会渲染大气相关的视觉效果
  SceneCapture.ShowFlags.SetAtmosphere(false);
  
  // 设置场景捕获组件的原始渲染模式，这里设置为使用仅显示列表（PRM_UseShowOnlyList）的模式
  // 表示只会渲染在显示列表中的组件，具体显示列表的内容后续应该会进行设置
  SceneCapture.PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
  
  // 创建一个UObject类型的数组，用于存储带有标签的组件（TaggedComponents）
  TArray<UObject *> TaggedComponents;
  // 通过调用GetObjectsOfClass函数获取所有类型为UTaggedComponent的对象，并存储到TaggedComponents数组中
  // 参数中的false表示不包含子类对象，后面的参数分别是对象标志和内部对象标志，用于更精确地筛选对象
  GetObjectsOfClass(UTaggedComponent::StaticClass(), TaggedComponents, false, EObjectFlags::RF_ClassDefaultObject, EInternalObjectFlags::AllFlags);
  
  // 创建一个UPrimitiveComponent类型的数组，用于存储要显示的组件（ShowOnlyComponents）
  TArray<UPrimitiveComponent *> ShowOnlyComponents;
  // 遍历获取到的带有标签的组件（TaggedComponents）
  for (UObject *Object : TaggedComponents) {
    // 将UObject类型的对象尝试转换为UPrimitiveComponent类型，如果转换成功则进行后续操作
    UPrimitiveComponent *Component = Cast<UPrimitiveComponent>(Object);
    // 将转换成功的组件添加到要显示的组件列表（ShowOnlyComponents）中，用于后续场景捕获组件按此列表进行渲染显示
    SceneCapture.ShowOnlyComponents.Emplace(Component);
  }
}

// 函数在物理模拟更新后的每帧（Tick）被调用，用于执行一些与实例分割相机相关的每帧更新操作
// 参数World表示当前的游戏世界，TickType表示当前的关卡Tick类型，DeltaSeconds表示自上一帧经过的时间
void AInstanceSegmentationCamera::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  // 使用TRACE_CPUPROFILER_EVENT_SCOPE宏定义一个CPU性能分析的事件范围，用于标记该函数执行的范围，方便性能分析工具进行分析
  TRACE_CPUPROFILER_EVENT_SCOPE(AInstanceSegmentationCamera::PostPhysTick);
  
  // 获取二维场景捕获组件的指针，如果获取成功则后续可以对其进行相关操作
  USceneCaptureComponent2D* SceneCapture = GetCaptureComponent2D();
  // 创建一个UObject类型的数组，用于存储带有标签的组件（TaggedComponents）
  TArray<UObject *> TaggedComponents;
  // 通过调用GetObjectsOfClass函数获取所有类型为UTaggedComponent的对象，并存储到TaggedComponents数组中
  // 参数含义与前面类似，用于筛选出特定类型的对象
  GetObjectsOfClass(UTaggedComponent::StaticClass(), TaggedComponents, false, EObjectFlags::RF_ClassDefaultObject, EInternalObjectFlags::AllFlags);
  
  // 清空场景捕获组件的显示列表（ShowOnlyComponents），为重新设置要显示的组件做准备
  SceneCapture->ClearShowOnlyComponents();
  // 再次遍历获取到的带有标签的组件（TaggedComponents）
  for (UObject *Object : TaggedComponents) {
    // 将UObject类型的对象尝试转换为UPrimitiveComponent类型，如果转换成功则进行后续操作
    UPrimitiveComponent *Component = Cast<UPrimitiveComponent>(Object);
    // 将转换成功的组件添加到场景捕获组件的显示列表（ShowOnlyComponents）中，用于本帧的渲染显示
    SceneCapture->ShowOnlyComponents.Emplace(Component);
  }
  
  // 通过FPixelReader模板函数，在渲染线程中发送像素数据，这里指定了发送的数据类型为FColor（颜色相关的数据类型）
  // 具体发送的像素数据来源以及发送的目的等逻辑依赖于FPixelReader类和相关函数的实现
  FPixelReader::SendPixelsInRenderThread<AInstanceSegmentationCamera, FColor>(*this);
}
