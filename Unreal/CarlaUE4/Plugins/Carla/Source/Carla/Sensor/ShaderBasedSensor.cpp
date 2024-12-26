// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/ShaderBasedSensor.h"

#include "ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Actor/ActorBlueprintFunctionLibrary.h"

// 添加后处理材质
bool AShaderBasedSensor::AddPostProcessingMaterial(const FString &Path)
{
  // 使用路径加载材质
  ConstructorHelpers::FObjectFinder<UMaterial> Loader(*Path);
  if (Loader.Succeeded())
  {
    // 如果加载成功，将材质添加到已找到的材质列表中
    MaterialsFound.Add(Loader.Object);
  }
  return Loader.Succeeded(); // 返回是否加载成功
}

// 配置场景捕捉组件
void AShaderBasedSensor::SetUpSceneCaptureComponent(USceneCaptureComponent2D &SceneCapture)
{
  for (const auto &MaterialFound : MaterialsFound)
  {
    // 创建材质（着色器）的动态实例
    AddShader({UMaterialInstanceDynamic::Create(MaterialFound, this), 1.0});
  }

  for (const auto &Shader : Shaders)
  {
    // 将动态实例附加到后期处理的混合设置中
    SceneCapture.PostProcessSettings.AddBlendable(Shader.PostProcessMaterial, Shader.Weight);
  }

  // 设置着色器中每个浮点参数的值
  for (const auto &ParameterValue : FloatShaderParams)
  {
    Shaders[ParameterValue.ShaderIndex].PostProcessMaterial->SetScalarParameterValue(
        ParameterValue.ParameterName,
        ParameterValue.Value);
  }
}

// 设置实体属性
void AShaderBasedSensor::Set(const FActorDescription &Description)
{
  Super::Set(Description); // 调用父类的 Set 方法
  // 设置摄像机属性
  UActorBlueprintFunctionLibrary::SetCamera(Description, this);
}

// 设置浮点类型的着色器参数
void AShaderBasedSensor::SetFloatShaderParameter(
    uint8_t ShaderIndex,
    const FName &ParameterName,
    float Value)
{
  // 将参数添加到浮点着色器参数列表中
  FloatShaderParams.Add({ShaderIndex, ParameterName, Value});
}

