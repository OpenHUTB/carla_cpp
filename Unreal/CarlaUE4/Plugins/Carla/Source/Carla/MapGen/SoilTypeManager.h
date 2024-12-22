// 版权所有 （c） 2017 巴塞罗那自治大学 （UAB） 计算机视觉中心 （CVC）。本作品根据 MIT 许可证的条款进行许可。有关副本，请参阅 <https://opensource.org/licenses/MIT>。

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Carla/MapGen/LargeMapManager.h"
#include "SoilTypeManager.generated.h"
// 枚举类型，表示不同的土壤类型及其对应的地形力学类型
UENUM(BlueprintType)
enum ESoilTerramechanicsType
{
  NONE_SOIL = 0,
  DESERT   = 1,
  FOREST   = 2
};
// 结构体，包含土壤类型的属性
USTRUCT(BlueprintType)
struct CARLA_API FSoilTerramechanicsProperties
{
  GENERATED_BODY()
// 土壤类型枚举
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TEnumAsByte<ESoilTerramechanicsType> TerrainType;
// 将土壤类型转换为字符串
  const FString ToString() const
  {
    switch(TerrainType)
    {
      case ESoilTerramechanicsType::NONE_SOIL:
        return "None";
      case ESoilTerramechanicsType::DESERT:
        return "Desert";
      case ESoilTerramechanicsType::FOREST:
        return "Forest";
    }
    return "";
  };
};
// 管理土壤类型的Actor类
UCLASS()
class CARLA_API ASoilTypeManager : public AActor
{
  GENERATED_BODY()

private:
// 通用地形属性
  UPROPERTY(EditAnywhere)
  FSoilTerramechanicsProperties GeneralTerrainProperties;
// 每个瓦片的地形属性
  UPROPERTY(EditAnywhere)
  TMap<FIntVector, FSoilTerramechanicsProperties> TilesTerrainProperties;
 // 大地图管理器的引用
  UPROPERTY(EditAnywhere)
  ALargeMapManager* LargeMapManager;
  
public:  
  //设置此角色属性的默认值
  ASoilTypeManager();
// 可编辑的属性，车辆类
  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TSubclassOf<AActor> CarClass;

protected:
  //在游戏开始时或生成时调用
  virtual void BeginPlay() override;

public:  
 // 获取通用地形属性
  UFUNCTION(Category="MapGen|Soil Manager")
  FSoilTerramechanicsProperties GetGeneralTerrainProperties();
// 根据全局位置获取地形属性
  UFUNCTION(Category="MapGen|Soil Manager")
  FSoilTerramechanicsProperties GetTerrainPropertiesAtGlobalLocation(FVector VehicleLocation);
// 根据局部位置获取地形属性
  UFUNCTION(Category="MapGen|Soil Manager")
  FSoilTerramechanicsProperties GetTerrainPropertiesAtLocalLocation(FVector VehicleLocation);
// 设置通用地形属性
  UFUNCTION(Category="MapGen|Soil Manager")
  void SetGeneralTerrainProperties(FSoilTerramechanicsProperties TerrainProperties);
  // 为特定瓦片添加地形属性
  UFUNCTION(Category="MapGen|Soil Manager")
  void AddTerrainPropertiesToTile(int TileX, int TileY, FSoilTerramechanicsProperties TerrainProperties);
 // 清除所有地形属性
  UFUNCTION(Category="MapGen|Soil Manager")
  void ClearTerrainPropertiesMap();
 // 每帧调用
  virtual void Tick(float DeltaSeconds) override;

};
