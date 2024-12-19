
#pragma once

#include "UObject/Object.h"
#include "CoreMinimal.h"
#include "USDCARLAInterface.generated.h"

USTRUCT(BlueprintType)
struct FUSDCARLAWheelData
{
  GENERATED_BODY()
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")
  float MaxBrakeTorque = 0;#定义了最大的手刹扭距成员变量，为float类型，用于表示车辆车轮手刹能施加的最大扭矩值
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")
  float MaxHandBrakeTorque = 0;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")
  float MaxSteerAngle = 0;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")
  float SpringStrength = 0;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")
  float MaxCompression = 0;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")
  float MaxDroop = 0;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")
  float LateralStiffnessX = 0;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")
  float LateralStiffnessY = 0;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")
  float LongitudinalStiffness = 0;
};

USTRUCT(BlueprintType)
struct FUSDCARLALight
{
  GENERATED_BODY()
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Light")
  FString Name;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Light")
  FVector Location;
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Light")
  FLinearColor Color;
};

UCLASS()
class OMNIVERSEUSD_API UUSDCARLAInterface : public UObject
{
  GENERATED_BODY()

public:
  UFUNCTION(BlueprintCallable, Category="USD Carla Interface")
  static bool ImportUSD(
      const FString& Path, const FString& Dest,
      bool bImportUnusedReferences, bool bImportAsBlueprint);
  UFUNCTION(BlueprintCallable, Category="USD Carla Interface")
  static TArray<FUSDCARLALight> GetUSDLights(const FString& Path);
  UFUNCTION(BlueprintCallable, Category="USD Carla Interface")
  static TArray<FUSDCARLAWheelData> GetUSDWheelData(const FString& Path);
};
