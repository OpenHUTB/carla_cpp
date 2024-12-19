// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/NormalsCamera.h"

#include "Carla/Sensor/PixelReader.h"

#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"

// 返回传感器定义，使用 MakeNormalsCameraDefinition 函数创建该定义
FActorDefinition ANormalsCamera::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeNormalsCameraDefinition();
}

// ANormalsCamera 类的构造函数
// 主要进行后处理材质的添加和16位格式的启用或禁用
ANormalsCamera::ANormalsCamera(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  Enable16BitFormat(false);
  AddPostProcessingMaterial(
      TEXT("Material'/Carla/PostProcessingMaterials/PhysicLensDistortion.PhysicLensDistortion'"));
  AddPostProcessingMaterial(
      TEXT("Material'/Carla/PostProcessingMaterials/NormalsEffectMaterial.NormalsEffectMaterial'"));
}

// ANormalsCamera 的 PostPhysTick 函数
// 该函数在物理计算完成后调用，用于图像或像素的处理
void ANormalsCamera::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ANormalsCamera::PostPhysTick);
  FPixelReader::SendPixelsInRenderThread<ANormalsCamera, FColor>(*this);
}
