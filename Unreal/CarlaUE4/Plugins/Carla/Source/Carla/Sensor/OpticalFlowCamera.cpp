#include "Carla.h"
#include "Carla/Sensor/OpticalFlowCamera.h"
#include "HAL/IConsoleManager.h"

#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"

#include "Carla/Sensor/PixelReader.h"

// 获取光流相机的传感器定义
FActorDefinition AOpticalFlowCamera::GetSensorDefinition()
{
  // 使用蓝图函数库创建光流相机的传感器定义
  return UActorBlueprintFunctionLibrary::MakeCameraDefinition(TEXT("optical_flow"));
}

// 构造函数，初始化光流相机
AOpticalFlowCamera::AOpticalFlowCamera(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  // 启用 16 位图像格式
  Enable16BitFormat(true);

  // 添加用于物理镜头畸变校正的后处理材质
  AddPostProcessingMaterial(
      TEXT("Material'/Carla/PostProcessingMaterials/PhysicLensDistortion.PhysicLensDistortion'"));

  // 添加用于计算光流的后处理材质
  AddPostProcessingMaterial(
          TEXT("Material'/Carla/PostProcessingMaterials/VelocityMaterial.VelocityMaterial'"));
}

// 在物理帧结束时处理光流数据
void AOpticalFlowCamera::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  // 获取控制光流输出的控制台变量
  auto CVarForceOutputsVelocity = IConsoleManager::Get().FindConsoleVariable(TEXT("r.BasePassForceOutputsVelocity"));
  
  // 保存控制台变量的原始值
  int32 OldValue = CVarForceOutputsVelocity->GetInt();
  
  // 设置控制台变量以强制输出速度（光流）信息
  CVarForceOutputsVelocity->Set(1);

  // 定义一个转换函数，将图像数据转换为浮点光流值
  std::function<TArray<float>(void *, uint32)> Conversor = [](void *Data, uint32 Size)
  {
    TArray<float> IntermediateBuffer;
    // 计算数据中的像素数量
    int32 Count = Size / sizeof(FFloat16Color);
    DEBUG_ASSERT(Count * sizeof(FFloat16Color) == Size);

    // 将数据解释为 FFloat16Color 类型的数组
    FFloat16Color *Buf = reinterpret_cast<FFloat16Color *>(Data);

    // 预分配缓冲区大小（每个像素有两个值：x 和 y）
    IntermediateBuffer.Reserve(Count * 2);

    // 遍历像素数据，计算光流的 x 和 y 分量
    for (int i = 0; i < Count; ++i)
    {
      float x = (Buf->R.GetFloat() - 0.5f) * 4.f; // 光流 x 分量
      float y = (Buf->G.GetFloat() - 0.5f) * 4.f; // 光流 y 分量
      IntermediateBuffer.Add(x);
      IntermediateBuffer.Add(y);
      ++Buf; // 移动到下一个像素
    }
    return IntermediateBuffer;
  };

  // 使用像素读取器将处理后的光流数据发送到渲染线程
  FPixelReader::SendPixelsInRenderThread<AOpticalFlowCamera, float>(*this, true, Conversor);
  
  // 恢复控制台变量的原始值
  CVarForceOutputsVelocity->Set(OldValue);
}
