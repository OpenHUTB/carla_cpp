// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/OpenDrive/OpenDriveMap.h"

#include "Kismet/BlueprintFunctionLibrary.h"

#include "OpenDrive.generated.h"

UCLASS()
class CARLA_API UOpenDrive : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()

public:

  /// 返回与@a MapName关联的OpenDrive XML，如果找不到文件，则返回空。
  UFUNCTION(BlueprintCallable, Category="CARLA|OpenDrive")
  static FString GetXODR(const UWorld *World);

  ///返回与@a MapName关联的OpenDrive XML，如果找不到文件，则返回空。
  UFUNCTION(BlueprintCallable, Category="CARLA|OpenDrive")
  static FString GetXODRByPath(FString XODRPath, FString MapName);

  UFUNCTION(BlueprintCallable, Category="CARLA|OpenDrive")
  static FString FindPathToXODRFile(const FString &InMapName);

  UFUNCTION(BlueprintCallable, Category="CARLA|OpenDrive")
  static FString LoadXODR(const FString &MapName);

  /// 加载与给定MapName关联的OpenDriveMap。如果找不到具有相同MapName的ODR，则返回nullptr。
  UFUNCTION(BlueprintCallable, Category="CARLA|OpenDrive")
  static UOpenDriveMap *LoadOpenDriveMap(const FString &MapName);

  /// 加载与当前加载的映射关联的OpenDriveMap。如果找不到与当前映射匹配的XODR，则返回nullptr。
  UFUNCTION(BlueprintPure, Category="CARLA|OpenDrive", meta=(WorldContext="WorldContextObject"))
  static UOpenDriveMap *LoadCurrentOpenDriveMap(const UObject *WorldContextObject);
};
