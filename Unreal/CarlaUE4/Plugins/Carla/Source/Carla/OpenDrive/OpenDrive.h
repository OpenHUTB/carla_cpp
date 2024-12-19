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
//test
public:

  /// ������@a MapName������OpenDrive XML������Ҳ����ļ����򷵻ؿա�
  UFUNCTION(BlueprintCallable, Category="CARLA|OpenDrive")
  static FString GetXODR(const UWorld *World);

  ///������@a MapName������OpenDrive XML������Ҳ����ļ����򷵻ؿա�
  UFUNCTION(BlueprintCallable, Category="CARLA|OpenDrive")
  static FString GetXODRByPath(FString XODRPath, FString MapName);

  UFUNCTION(BlueprintCallable, Category="CARLA|OpenDrive")
  static FString FindPathToXODRFile(const FString &InMapName);

  UFUNCTION(BlueprintCallable, Category="CARLA|OpenDrive")
  static FString LoadXODR(const FString &MapName);

  /// ���������MapName������OpenDriveMap������Ҳ���������ͬMapName��ODR���򷵻�nullptr��
  UFUNCTION(BlueprintCallable, Category="CARLA|OpenDrive")
  static UOpenDriveMap *LoadOpenDriveMap(const FString &MapName);

  /// �����뵱ǰ���ص�ӳ�������OpenDriveMap������Ҳ����뵱ǰӳ��ƥ���XODR���򷵻�nullptr��
  UFUNCTION(BlueprintPure, Category="CARLA|OpenDrive", meta=(WorldContext="WorldContextObject"))
  static UOpenDriveMap *LoadCurrentOpenDriveMap(const UObject *WorldContextObject);
};
