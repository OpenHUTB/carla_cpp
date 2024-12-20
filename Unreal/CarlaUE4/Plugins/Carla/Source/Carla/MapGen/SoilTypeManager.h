// 版权所有 （c） 2017 巴塞罗那自治大学 （UAB） 计算机视觉中心 （CVC）。本作品根据 MIT 许可证的条款进行许可。有关副本，请参阅 <https://opensource.org/licenses/MIT>。

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Carla/MapGen/LargeMapManager.h"
#include "SoilTypeManager.generated.h"

UENUM(BlueprintType)
enum ESoilTerramechanicsType
{
  NONE_SOIL = 0,
  DESERT   = 1,
  FOREST   = 2
};

USTRUCT(BlueprintType)
struct CARLA_API FSoilTerramechanicsProperties
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TEnumAsByte<ESoilTerramechanicsType> TerrainType;

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

UCLASS()
class CARLA_API ASoilTypeManager : public AActor
{
  GENERATED_BODY()

private:
  UPROPERTY(EditAnywhere)
  FSoilTerramechanicsProperties GeneralTerrainProperties;

  UPROPERTY(EditAnywhere)
  TMap<FIntVector, FSoilTerramechanicsProperties> TilesTerrainProperties;

  UPROPERTY(EditAnywhere)
  ALargeMapManager* LargeMapManager;
  
public:  
  //设置此角色属性的默认值
  ASoilTypeManager();

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  TSubclassOf<AActor> CarClass;

protected:
  //在游戏开始时或生成时调用
  virtual void BeginPlay() override;

public:  
  UFUNCTION(Category="MapGen|Soil Manager")
  FSoilTerramechanicsProperties GetGeneralTerrainProperties();

  UFUNCTION(Category="MapGen|Soil Manager")
  FSoilTerramechanicsProperties GetTerrainPropertiesAtGlobalLocation(FVector VehicleLocation);

  UFUNCTION(Category="MapGen|Soil Manager")
  FSoilTerramechanicsProperties GetTerrainPropertiesAtLocalLocation(FVector VehicleLocation);

  UFUNCTION(Category="MapGen|Soil Manager")
  void SetGeneralTerrainProperties(FSoilTerramechanicsProperties TerrainProperties);

  UFUNCTION(Category="MapGen|Soil Manager")
  void AddTerrainPropertiesToTile(int TileX, int TileY, FSoilTerramechanicsProperties TerrainProperties);

  UFUNCTION(Category="MapGen|Soil Manager")
  void ClearTerrainPropertiesMap();

  virtual void Tick(float DeltaSeconds) override;

};
