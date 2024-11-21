// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#include "MapGen/SoilTypeManager.h"
// 引入 "MapGen/SoilTypeManager.h" 头文件，通常其中包含了当前类（ASoilTypeManager）的定义以及相关的类型、函数等声明

#include "Kismet/GameplayStatics.h"
// 引入 "Kismet/GameplayStatics.h" 头文件，该头文件提供了一系列用于游戏玩法相关的静态函数，比如获取场景中的Actor等操作

// 设置默认值
ASoilTypeManager::ASoilTypeManager()
{
   // 设置此 Actor 使其每一帧都调用 Tick () 函数。如果你不需要它（每帧调用 Tick () 函数的功能），可以将其关闭以提高性能。
  PrimaryActorTick.bCanEverTick = false;
}

// 在游戏开始或（Actor）生成时被调用。
void ASoilTypeManager::BeginPlay()
{
  Super::BeginPlay();
  // 调用父类的BeginPlay函数，确保父类在游戏开始或Actor被生成时的初始化逻辑先执行，
  // 这里虽然没有添加额外的逻辑，但在继承体系中保留这种调用是很重要的，以保证正确的初始化顺序
  
}

void ASoilTypeManager::Tick(float DeltaTime)
{
#if WITH_EDITOR // 仅用于调试目的。需要在构造函数中激活 Tick（函数调用）。
  // 这是一个预编译指令，只有在定义了WITH_EDITOR宏的情况下（通常在编辑器相关的编译配置中会定义），
  // 下面的代码才会被编译，主要用于调试目的，并且需要在构造函数中开启Tick函数调用（前面已经设置默认关闭）
  if((int)DeltaTime % 2000 == 0)
  {
    // 每隔2000个时间单位（这里DeltaTime的单位应该是与游戏帧率相关的时间单位，比如秒）执行一次下面的逻辑
    ALargeMapManager* LargeMapManager = (ALargeMapManager*) UGameplayStatics::GetActorOfClass(GetWorld(), ALargeMapManager::StaticClass());
    // 使用UGameplayStatics的静态函数GetActorOfClass从当前世界（GetWorld获取）中获取ALargeMapManager类型的Actor，
    // 并将其转换为ALargeMapManager* 类型，用于后续操作与地图相关的逻辑
    AActor* Car = UGameplayStatics::GetActorOfClass(GetWorld(), CarClass);
    // 同样使用GetActorOfClass函数从当前世界中获取指定类型（CarClass，这里CarClass应该是在其他地方定义的表示车辆的Actor类）的Actor，
    // 并存储为AActor* 类型，用于后续获取车辆相关信息

    if(Car != nullptr)
    {
      // 如果成功获取到了表示车辆的Actor（即Car不为nullptr），则执行下面的操作
      FVector CarPos = Car->GetActorLocation();
      // 获取车辆Actor的位置信息，存储在FVector类型的CarPos变量中，FVector通常用于表示三维空间中的向量（这里表示位置向量）

      FVector GlobalCarPos = LargeMapManager->LocalToGlobalLocation(CarPos);
      // 通过获取到的LargeMapManager对象，调用其LocalToGlobalLocation函数将车辆的本地位置（CarPos）转换为全局位置，
      // 并存储在GlobalCarPos变量中，以便在全局坐标系下进行后续操作
      FIntVector TileVector = LargeMapManager->GetTileVectorID(GlobalCarPos);
      // 使用LargeMapManager对象的GetTileVectorID函数，根据车辆的全局位置（GlobalCarPos）获取对应的地图瓦片向量ID，
      // 该ID可能用于在地图瓦片相关的数据结构中索引特定的瓦片信息，存储在FIntVector类型的TileVector变量中
      uint64 TileIndex = LargeMapManager->GetTileID(GlobalCarPos);
      // 类似地，通过LargeMapManager对象的GetTileID函数，根据车辆的全局位置获取对应的地图瓦片索引（TileIndex），
      // 具体的索引格式可能根据地图数据结构的实现而定

      FString TypeStr = GetTerrainPropertiesAtGlobalLocation(GlobalCarPos).ToString();

      // 调用当前类（ASoilTypeManager）的GetTerrainPropertiesAtGlobalLocation函数获取车辆所在全局位置的地形属性，
      // 并将其转换为字符串形式存储在TypeStr变量中，以便后续输出日志
      UE_LOG(LogCarla, Log, TEXT("Current Tile Index %d ----> (%d, %d, %d) with position L[%f, %f, %f] G[%f, %f, %f] Terrain Type: %s"),
        TileIndex, TileVector.X, TileVector.Y, TileVector.Z, CarPos.X, CarPos.Y, CarPos.Z, GlobalCarPos.X, GlobalCarPos.Y, GlobalCarPos.Z,
        *TypeStr);
      // 使用UE_LOG输出日志信息，记录当前瓦片索引、瓦片向量ID、车辆的本地位置和全局位置以及地形类型等相关信息，
      // 方便在调试时查看游戏运行过程中的相关数据状态
    }
  }
#endif
}

FSoilTerramechanicsProperties ASoilTypeManager::GetGeneralTerrainProperties()
{
  return GeneralTerrainProperties;
  // 返回当前类的成员变量GeneralTerrainProperties，该变量应该存储了一些通用的地形力学属性相关信息，
  // 返回的类型是FSoilTerramechanicsProperties，应该是一个自定义的结构体类型用于表示土壤地形力学属性
}

FSoilTerramechanicsProperties ASoilTypeManager::GetTerrainPropertiesAtGlobalLocation(FVector VehicleLocation)
{
  // 从位置获取索引
  FIntVector TileVectorID = LargeMapManager->GetTileVectorID(VehicleLocation);
  // 根据传入的车辆在全局坐标系下的位置（VehicleLocation），通过LargeMapManager对象的GetTileVectorID函数获取对应的地图瓦片向量ID，
  // 以便后续在地图相关的数据结构中查找该位置对应的地形属性

  // 查询地图，如果（所需信息）不在地图中，则返回通用（属性）。
  if(TilesTerrainProperties.Contains(TileVectorID))
    return TilesTerrainProperties[TileVectorID];  // Tile properties
  else
    return GeneralTerrainProperties;    // General properties
  // 检查地图瓦片地形属性的容器（TilesTerrainProperties，应该是一个以FIntVector为键，FSoilTerramechanicsProperties为值的容器）
  // 是否包含根据位置获取到的瓦片向量ID，如果包含则返回该ID对应的具体地形属性（即该瓦片位置的地形属性），
  // 如果不包含则返回通用的地形属性（GeneralTerrainProperties）
}

FSoilTerramechanicsProperties ASoilTypeManager::GetTerrainPropertiesAtLocalLocation(FVector VehicleLocation)
{
  FVector GlobalVehiclePosition = LargeMapManager->LocalToGlobalLocation(VehicleLocation);
  // 首先将传入的车辆在本地坐标系下的位置（VehicleLocation）通过LargeMapManager对象的LocalToGlobalLocation函数转换为全局位置，
  // 存储在GlobalVehiclePosition变量中
  return GetTerrainPropertiesAtGlobalLocation(GlobalVehiclePosition);
  // 然后调用GetTerrainPropertiesAtGlobalLocation函数获取该全局位置对应的地形属性，并返回结果
}

void ASoilTypeManager::SetGeneralTerrainProperties(FSoilTerramechanicsProperties TerrainProperties)
{
  const FString TerrainPropertiesStr = TerrainProperties.ToString();
  // 将传入的地形属性参数（TerrainProperties）转换为字符串形式，存储在TerrainPropertiesStr变量中，
  // 以便后续输出日志使用
  UE_LOG(LogCarla, Log, TEXT("Setting General Terrain Settings %s"), *TerrainPropertiesStr);
  // 使用UE_LOG输出日志信息，记录正在设置通用地形设置的相关信息，包括设置的具体地形属性内容（通过TerrainPropertiesStr）
  GeneralTerrainProperties = TerrainProperties;
  // 将传入的地形属性参数赋值给当前类的成员变量GeneralTerrainProperties，完成对通用地形属性的设置操作
}

void ASoilTypeManager::AddTerrainPropertiesToTile(int TileX, int TileY, FSoilTerramechanicsProperties TerrainProperties)
{
  // 根据 X、Y 坐标计算 ID。
  check(LargeMapManager != nullptr)
  // 使用check宏来确保LargeMapManager指针不为空，这是一种在开发过程中进行断言检查的方式，
  // 如果LargeMapManager为空则会触发断言失败，通常在调试版本中会给出相应的错误提示

  FIntVector TileVectorID(TileX, TileY, 0);
  // 根据传入的瓦片坐标（TileX和TileY）创建一个FIntVector类型的瓦片向量ID，这里Z坐标设置为0，
  // 可能表示二维地图瓦片的索引方式（具体取决于地图数据结构的设计）

  // 添加到地图（中）
  if(TerrainProperties.TerrainType == ESoilTerramechanicsType::NONE_SOIL)
    TilesTerrainProperties.Add(TileVectorID, GeneralTerrainProperties);
  else
    TilesTerrainProperties.Add(TileVectorID, TerrainProperties);
  // 根据传入的地形属性中的地形类型（TerrainProperties.TerrainType）判断，
  // 如果是NONE_SOIL类型，则将通用地形属性（GeneralTerrainProperties）添加到地图瓦片地形属性的容器（TilesTerrainProperties）中，
  // 对应的键是刚刚创建的瓦片向量ID；如果不是NONE_SOIL类型，则将传入的具体地形属性（TerrainProperties）添加到容器中，
  // 同样以瓦片向量ID为键，这样就完成了将特定地形属性添加到指定地图瓦片位置的操作
}

void ASoilTypeManager::ClearTerrainPropertiesMap()
{
  TilesTerrainProperties.Empty(TilesTerrainProperties.Num());
  // 清空地图瓦片地形属性的容器（TilesTerrainProperties），通过调用Empty函数并传入容器当前的元素数量，
  // 实现将容器中的所有元素移除，重置容器为空状态的操作
}
