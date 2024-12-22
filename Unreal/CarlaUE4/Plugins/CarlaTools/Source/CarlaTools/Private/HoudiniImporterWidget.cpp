// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "HoudiniImporterWidget.h"

#include "EditorLevelUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "FileHelpers.h"
#include "EditorLevelLibrary.h"
#include "Components/PrimitiveComponent.h"

void UHoudiniImporterWidget::CreateSubLevels(ALargeMapManager* LargeMapManager)
{
// 函数体暂时为空
}

void UHoudiniImporterWidget::MoveActorsToSubLevelWithLargeMap(TArray<AActor*> Actors, ALargeMapManager* LargeMapManager)
{
// 存储每个地图瓦片对应的要移动的参与者数组的映射
  TMap<FCarlaMapTile*, TArray<AActor*>> ActorsToMove;
  for (AActor* Actor : Actors)
  {
   // 若参与者为空则跳过
    if (Actor == nullptr) {
      continue;
    }
    // 将参与者的 HierarchicalInstancedStaticMeshComponent 组件转换出来
    UHierarchicalInstancedStaticMeshComponent* Component
        = Cast<UHierarchicalInstancedStaticMeshComponent>(
          Actor->GetComponentByClass(
          UHierarchicalInstancedStaticMeshComponent::StaticClass()));
    // 获取参与者的位置
    FVector ActorLocation = Actor->GetActorLocation();
    if (Component)
    {
     // 如果有 HierarchicalInstancedStaticMeshComponent 组件，将位置设为 (0,0,0) 并计算平均位置
      ActorLocation = FVector(0);
      for(int32 i = 0; i < Component->GetInstanceCount(); ++i)
      {
        FTransform Transform;
        Component->GetInstanceTransform(i, Transform, true);
        ActorLocation = ActorLocation + Transform.GetTranslation();
      }
      ActorLocation = ActorLocation / Component->GetInstanceCount();
    }
    // 输出参与者位置的日志信息
    UE_LOG(LogCarlaTools, Log, TEXT("Actor at location %s"),
          *ActorLocation.ToString());
    // 根据参与者位置获取对应的地图瓦片
    FCarlaMapTile* Tile = LargeMapManager->GetCarlaMapTile(ActorLocation);
    if(!Tile)
    {
    // 若无法找到对应瓦片，输出错误日志并跳过
      UE_LOG(LogCarlaTools, Error, TEXT("Error: actor %s in location %s is outside the map"),
          *Actor->GetName(),*ActorLocation.ToString());
      continue;
    }

    if(Component)
    {
    // 若有 HierarchicalInstancedStaticMeshComponent 组件，更新实例化网格坐标
      UpdateInstancedMeshCoordinates(Component, Tile->Location);
    }
    else
    {
     // 否则更新普通参与者坐标
      UpdateGenericActorCoordinates(Actor, Tile->Location);
    }
   // 将参与者添加到要移动的映射中
    ActorsToMove.FindOrAdd(Tile).Add(Actor);
  }

  for (auto& Element : ActorsToMove)
  {
    FCarlaMapTile* Tile = Element.Key;
    TArray<AActor*> ActorList = Element.Value;
    if(!ActorList.Num())
    {
      continue;
    }
    // 获取编辑器世界
    UWorld* World = UEditorLevelLibrary::GetEditorWorld();
    ULevelStreamingDynamic* StreamingLevel = Tile->StreamingLevel;
    // 设置加载阻塞，设置可见和加载状态
    StreamingLevel->bShouldBlockOnLoad = true;
    StreamingLevel->SetShouldBeVisible(true);
    StreamingLevel->SetShouldBeLoaded(true);
    ULevelStreaming* Level =
        UEditorLevelUtils::AddLevelToWorld(
        World, *Tile->Name, ULevelStreamingDynamic::StaticClass(), 
FTransform());
   // 将参与者移动到关卡中并获取移动的参与者数量
    int MovedActors = UEditorLevelUtils::MoveActorsToLevel(ActorList, Level, false, false);
    // StreamingLevel->SetShouldBeLoaded(false);
    UE_LOG(LogCarlaTools, Log, TEXT("Moved %d actors"), MovedActors);
    FEditorFileUtils::SaveDirtyPackages(false, true, true, false, false, false, nullptr);
    UEditorLevelUtils::RemoveLevelFromWorld(Level->GetLoadedLevel());
  }
  // 执行垃圾回收和清理参与者
  GEngine->PerformGarbageCollectionAndCleanupActors();
  FText TransResetText(FText::FromString("Clean up after Move actors to sublevels"));
  if ( GEditor->Trans )
  {
  // 重置事务
    GEditor->Trans->Reset(TransResetText);
    GEditor->Cleanse(true, true, TransResetText);
  }
}

void UHoudiniImporterWidget::ForceStreamingLevelsToUnload( ALargeMapManager* LargeMapManager )
{
  // 获取游戏世界
  UWorld* World = UEditorLevelLibrary::GetGameWorld();

  FIntVector NumTilesInXY  = LargeMapManager->GetNumTilesInXY();

  for(int x = 0; x < NumTilesInXY.X; ++x)
  {
    for(int y = 0; y < NumTilesInXY.Y; ++y)
    {
      FIntVector CurrentTileVector(x, y, 0);
      // 获取当前瓦片
      FCarlaMapTile CarlaTile = LargeMapManager->GetCarlaMapTile(CurrentTileVector);
      ULevelStreamingDynamic* StreamingLevel = CarlaTile.StreamingLevel;
       // 将关卡添加到世界中
      ULevelStreaming* Level =
        UEditorLevelUtils::AddLevelToWorld(
        World, *CarlaTile.Name, ULevelStreamingDynamic::StaticClass(), FTransform());
       // 保存脏包
      FEditorFileUtils::SaveDirtyPackages(false, true, true, false, false, false, nullptr);
      // 从世界中移除关卡
      UEditorLevelUtils::RemoveLevelFromWorld(Level->GetLoadedLevel());
    }
  }

}

void UHoudiniImporterWidget::MoveActorsToSubLevel(TArray<AActor*> Actors, ULevelStreaming* Level)
{
  // 将参与者移动到关卡中并获取移动的参与者数量
  int MovedActors = UEditorLevelUtils::MoveActorsToLevel(Actors, Level, false, false);
  // 输出移动的参与者数量日志
  UE_LOG(LogCarlaTools, Log, TEXT("Moved %d actors"), MovedActors);
  // 保存脏包
  FEditorFileUtils::SaveDirtyPackages(false, true, true, false, false, false, nullptr);
  // 从世界中移除关卡
  UEditorLevelUtils::RemoveLevelFromWorld(Level->GetLoadedLevel());
  FText TransResetText(FText::FromString("Clean up after Move actors to sublevels"));
  if ( GEditor->Trans )
  {
  // 重置事务
    GEditor->Trans->Reset(TransResetText);
  }
}

void UHoudiniImporterWidget::UpdateGenericActorCoordinates(
    AActor* Actor, FVector TileOrigin)
{
  // 计算相对位置
  FVector LocalLocation = Actor->GetActorLocation() - TileOrigin;
  // 设置参与者的新位置
  Actor->SetActorLocation(LocalLocation);
  // 输出新位置日志
  UE_LOG(LogCarlaTools, Log, TEXT("%s New location %s"),
    *Actor->GetName(), *LocalLocation.ToString());
}

void UHoudiniImporterWidget::UpdateInstancedMeshCoordinates(
    UHierarchicalInstancedStaticMeshComponent* Component,  FVector TileOrigin)
{
  TArray<FTransform> NewTransforms;
  for(int32 i = 0; i < Component->GetInstanceCount(); ++i)
  {
    FTransform Transform;
    // 获取实例的变换
    Component->GetInstanceTransform(i, Transform, true);
    // 平移变换
    Transform.AddToTranslation(-TileOrigin);
    NewTransforms.Add(Transform);
    // 输出新实例位置日志
    UE_LOG(LogCarlaTools, Log, TEXT("New instance location %s"),
        *Transform.GetTranslation().ToString());
  }
  // 批量更新实例的变换
  Component->BatchUpdateInstancesTransforms(0, NewTransforms, true, true, true);
}

void UHoudiniImporterWidget::UseCOMasActorLocation(TArray<AActor*> Actors)
{
  for (AActor* Actor : Actors)
  {
   // 获取参与者的 PrimitiveComponent 组件
    UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(
        Actor->GetComponentByClass(UPrimitiveComponent::StaticClass()));
    if(Primitive)
    {
    // 获取质心
      FBodyInstance* BodyInstance = Primitive->GetBodyInstance();
      FVector CenterOfMass = BodyInstance->COMNudge;
      // 设置参与者位置为质心
      Actor->SetActorLocation(CenterOfMass);
    }
  }
}

bool UHoudiniImporterWidget::GetNumberOfClusters(
    TArray<AActor*> ActorList, int& OutNumClusters)
{

  for (AActor* Actor : ActorList)
  {
  // 获取对象名称
    FString ObjectName = UKismetSystemLibrary::GetObjectName(Actor);
     // 输出正在搜索的字符串日志
    UE_LOG(LogCarlaTools, Log, TEXT("Searching in string %s"), *ObjectName);
    if(ObjectName.StartsWith("b"))
    {

 // 查找 "of" 子串的索引    
 int Index = ObjectName.Find("of");
      if(Index == -1)
      {
        continue;
      }
      // 输出找到 "of" 子串的日志
      UE_LOG(LogCarlaTools, Log, TEXT("found of substr at %d"), Index);
      // 截取 "of" 之后的数字字符串
      FString NumClusterString = ObjectName.Mid(Index+2, ObjectName.Len());
      // 将数字字符串转换为整数
      OutNumClusters = FCString::Atoi(*NumClusterString);
      return true;
    }
  }
  // 输出未找到集群数量的警告日志
  UE_LOG(LogCarlaTools, Warning, TEXT("Num clusters not found"));
  OutNumClusters = -1;
  return false;
}
