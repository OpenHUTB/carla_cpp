// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "CityMapMeshHolder.h"
#include "Engine/StaticMeshActor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include <vector>
#include "Settings/CarlaSettings.h"
#include "Game/CarlaGameInstance.h"
#include "Kismet/KismetSystemLibrary.h"


// 引入标准库中的类型别名定义，用于将枚举类型ECityMapMeshTag转换为其底层类型
using tag_size_t = std::underlying_type<ECityMapMeshTag>::type;

// 定义一个常量表达式，获取ECityMapMeshTag枚举类型的标签数量
constexpr static tag_size_t NUMBER_OF_TAGS = CityMapMeshTag::GetNumberOfTags();

// =============================================================================
// -- Construction and update related methods ----------------------------------
// =============================================================================

// ACityMapMeshHolder类的构造函数，接受一个FObjectInitializer对象用于初始化对象
ACityMapMeshHolder::ACityMapMeshHolder(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
  // 设置该Actor的Tick函数是否可被调用，这里设置为false，表示不需要每帧更新
  PrimaryActorTick.bCanEverTick = false;

  // 使用传入的ObjectInitializer创建一个默认的场景组件作为根组件
  SceneRootComponent =
      ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneComponent"));
      // 设置场景组件的移动性为静态，即不可移动
  SceneRootComponent->SetMobility(EComponentMobility::Static);
  // 将创建的场景组件设置为Actor的根组件
  RootComponent = SceneRootComponent;

  // 遍历所有的标签类型，为每个标签类型添加一个静态网格持有者
  for (tag_size_t i = 0u; i < NUMBER_OF_TAGS; ++i) {
    // 添加静态网格物体支架。
    StaticMeshes.Add(CityMapMeshTag::FromUInt(i));
  }

}

/*void ACityMapMeshHolder::LayoutDetails(IDetailLayoutBuilder& DetailLayout)
{
 // 获取渲染类别细节构建器
 IDetailCategoryBuilder& DetailCategory = DetailLayout.EditCategory("Rendering");
 // 在渲染类别中添加一个属性行，属性名为"Generation"，显示文本为空字符串
 IDetailPropertyRow& row = DetailCategory.AddProperty("Generation", TEXT(""));

}*/

// 当该Actor在场景中被构建时调用此函数，会先调用父类的OnConstruction函数
void ACityMapMeshHolder::OnConstruction(const FTransform &Transform)
{
  Super::OnConstruction(Transform);

}

// 在所有组件初始化完成后调用此函数，会先调用父类的PostInitializeComponents函数
void ACityMapMeshHolder::PostInitializeComponents()
{
  Super::PostInitializeComponents();

  // 检查当前Actor所在的关卡是否有效且未被标记为待销毁
  if(IsValid(GetLevel())&&!GetLevel()->IsPendingKill())
  {
    // 创建一个数组用于存储附加到该Actor的道路部件Actor
	 TArray<AActor*> roadpieces;
   // 获取附加到该Actor的所有Actor并存储到roadpieces数组中
     GetAttachedActors(roadpieces);
   // 如果没有附加的道路部件Actor
	 if(roadpieces.Num()==0)
	 {
     // 输出错误日志，提示需要在编辑模式下重新生成道路
	   UE_LOG(LogCarla, Error, TEXT("Please regenerate the road in edit mode for '%s' actor"), *UKismetSystemLibrary::GetDisplayName(this));
     // 更新地图比例尺
	   UpdateMapScale();
       // 更新地图
       UpdateMap();
	 }
  }

}

// 仅在编辑器环境下编译时生效的函数，当某个属性被编辑修改后调用此函数，会先调用父类的对应函数
#if WITH_EDITOR
void ACityMapMeshHolder::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
  // 如果被修改的属性存在
  Super::PostEditChangeProperty(PropertyChangedEvent);
  if (PropertyChangedEvent.Property)
  {
    // 删除一些部件（具体实现见DeletePieces函数）
    DeletePieces();
    // 更新地图比例尺
    UpdateMapScale();
    // 更新地图
    UpdateMap();
   }
}
#endif // WITH_EDITOR

// =============================================================================
// -- Other protected methods --------------------------------------------------
// =============================================================================

// 根据给定的X和Y坐标值，返回一个在地图中的瓦片位置向量
FVector ACityMapMeshHolder::GetTileLocation(uint32 X, uint32 Y) const
{
  // 返回一个以地图比例尺为单位的坐标位置向量，Z坐标固定为0.0f
  return {X * MapScale, Y * MapScale, 0.0f};
}

// 设置指定标签对应的静态网格资源
void ACityMapMeshHolder::SetStaticMesh(ECityMapMeshTag Tag, UStaticMesh *Mesh)
{
  // 将指定标签对应的静态网格资源设置为传入的Mesh
  StaticMeshes[Tag] = Mesh;
  // 如果传入的Mesh不为空，则将Mesh与对应的标签添加到TagMap映射中
  if (Mesh != nullptr) {
    TagMap.Add(Mesh, Tag);
  }
}

// 获取指定标签对应的静态网格资源（非const版本）
UStaticMesh *ACityMapMeshHolder::GetStaticMesh(ECityMapMeshTag Tag)
{
  // 返回指定标签对应的静态网格资源
  return StaticMeshes[Tag];
}

// 获取指定标签对应的静态网格资源（const版本）
const UStaticMesh *ACityMapMeshHolder::GetStaticMesh(ECityMapMeshTag Tag) const
{
  // 返回指定标签对应的静态网格资源
  return StaticMeshes[Tag];
}

// 根据给定的静态网格资源获取对应的标签
ECityMapMeshTag ACityMapMeshHolder::GetTag(const UStaticMesh &StaticMesh) const
{
  // 在TagMap中查找给定静态网格资源对应的标签指针
  const ECityMapMeshTag *Tag = TagMap.Find(&StaticMesh);
  // 如果找到对应的标签指针，则返回该标签，否则返回INVALID标签
  return (Tag != nullptr ? *Tag : ECityMapMeshTag::INVALID);
}

// 在指定的瓦片坐标位置添加一个指定标签的实例（仅指定坐标，角度默认）
void ACityMapMeshHolder::AddInstance(ECityMapMeshTag Tag, uint32 X, uint32 Y)
{
  // 调用另一个AddInstance函数，传入根据坐标获取的变换信息
  AddInstance(Tag, FTransform(GetTileLocation(X, Y)));
}

// 在指定的瓦片坐标位置添加一个指定标签的实例，并指定角度
void ACityMapMeshHolder::AddInstance(ECityMapMeshTag Tag, uint32 X, uint32 Y, float Angle)
{
  // 根据给定的角度创建一个四元数表示旋转信息
  const FQuat rotation(FVector(0.0f, 0.0f, 1.0f), Angle);
  // 根据坐标获取位置信息
  const FVector location = GetTileLocation(X, Y);
  // 调用另一个AddInstance函数，传入包含旋转和位置信息的变换对象
  AddInstance(Tag, FTransform(rotation, location));
}

// 在指定的变换位置添加一个指定标签的实例
void ACityMapMeshHolder::AddInstance(ECityMapMeshTag Tag, FTransform Transform)
{
  // 创建一个Actor生成参数对象，用于指定生成Actor时的一些参数
  FActorSpawnParameters params;
  // 设置生成Actor时的碰撞处理方式为总是生成，即使有碰撞也生成
  params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
  // 获取变换对象中的位置信息
  FVector location = Transform.GetLocation();
  // 获取变换对象中的旋转信息并转换为旋转器类型
  FRotator rotation = Transform.Rotator();
  // 在世界中生成一个静态网格Actor，传入类类型、位置、旋转信息和生成参数
  AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(GetWorld()->SpawnActor(AStaticMeshActor::StaticClass(),&location,&rotation,params));
  // 将生成的静态网格Actor附加到当前Actor上，保持相对变换关系
  StaticMeshActor->AttachToActor(this,FAttachmentTransformRules::KeepRelativeTransform);
  // 设置静态网格Actor的移动性为静态
  StaticMeshActor->SetMobility(EComponentMobility::Static);
  // 获取静态网格Actor的静态网格组件
  UStaticMeshComponent* staticmeshcomponent = StaticMeshActor->GetStaticMeshComponent();
  // 设置静态网格组件的移动性为静态
  staticmeshcomponent->SetMobility(EComponentMobility::Static);
  // 设置静态网格组件的静态网格资源为指定标签对应的静态网格资源
  staticmeshcomponent->SetStaticMesh(GetStaticMesh(Tag));
  // 给生成的静态网格Actor添加一个特定的标签
  StaticMeshActor->Tags.Add(UCarlaSettings::CARLA_ROAD_TAG);
  // 启用自动生成细节层次（LOD）功能
  StaticMeshActor->bEnableAutoLODGeneration = true;
}

// =============================================================================
// -- Private methods ----------------------------------------------------------
// =============================================================================

// 更新地图的函数，这里为空实现，可能在子类中被重写以实现具体的更新逻辑
void ACityMapMeshHolder::UpdateMap() {}

// 删除一些部件的函数，具体实现如下
void ACityMapMeshHolder::DeletePieces()
{
  // 这部分将会被弃用：移除实例化静态网格体组件
  TArray<UInstancedStaticMeshComponent*> oldcomponents;
  // 获取当前Actor的所有组件，并将实例化静态网格组件存储到oldcomponents数组中
  GetComponents(oldcomponents);
  // 遍历数组，销毁每个实例化静态网格组件
  for(int32 i=0;i<oldcomponents.Num();i++)
  {
	  oldcomponents[i]->DestroyComponent();
  }

  // 创建一个数组用于存储附加到该Actor的道路部件Actor
  TArray<AActor*> roadpieces;
  // 获取附加到该Actor的所有Actor并存储到roadpieces数组中
  GetAttachedActors(roadpieces);

  // 从后往前遍历道路部件Actor数组
  for(int32 i=roadpieces.Num()-1; i>=0; i--)
  {
    // 如果Actor具有特定的道路标签
	if(roadpieces[i]->ActorHasTag(UCarlaSettings::CARLA_ROAD_TAG))
	{
    // 销毁该Actor
	   roadpieces[i]->Destroy();
	}
  }

}

// 更新地图比例尺的函数
void ACityMapMeshHolder::UpdateMapScale()
{
  // 获取基础网格标签
  auto Tag = CityMapMeshTag::GetBaseMeshTag();
  // 获取对应标签的静态网格资源
  auto *mesh = GetStaticMesh(Tag);
  // 如果获取不到对应的静态网格资源
  if (mesh == nullptr) {
    // 输出错误日志，提示找不到用于计算瓦片大小的网格资源
    UE_LOG(
        LogCarla,
        Error,
        TEXT("Cannot find mesh \"%s\" for computing tile size"),
        *CityMapMeshTag::ToString(Tag));
    // 将地图比例尺设置为1.0f
    MapScale = 1.0f;
  } else {
    // 获取静态网格资源的包围盒大小向量
    FVector size = mesh->GetBoundingBox().GetSize();
    // 将地图比例尺设置为静态网格资源包围盒的X轴大小
    MapScale = size.X;
  }
}


