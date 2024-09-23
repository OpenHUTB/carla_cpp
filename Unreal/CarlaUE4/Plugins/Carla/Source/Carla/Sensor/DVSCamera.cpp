// Copyright (c) 2020 Robotics and Perception Group (GPR)
// University of Zurich and ETH Zurich
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.


#include <random>
#include <cmath>
#include <algorithm>

#include "Carla.h"
#include "Carla/Util/RandomEngine.h"
#include "Carla/Sensor/DVSCamera.h"
#include "Actor/ActorBlueprintFunctionLibrary.h"

#include <compiler/disable-ue4-macros.h>
#include "carla/ros2/ROS2.h"
#include <carla/Buffer.h>
#include <carla/BufferView.h>
#include <compiler/enable-ue4-macros.h>

// RGB图像的灰度值：I = 0.2989*R + 0.5870*G + 0.1140*B
static float FColorToGrayScaleFloat(FColor Color)
{
  return 0.2989 * Color.R + 0.587 * Color.G + 0.114 * Color.B;
}

ADVSCamera::ADVSCamera(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  EnablePostProcessingEffects(true);
  AddPostProcessingMaterial(
      TEXT("Material'/Carla/PostProcessingMaterials/PhysicLensDistortion.PhysicLensDistortion'"));

  RandomEngine = CreateDefaultSubobject<URandomEngine>(TEXT("RandomEngine"));
}

// 获得动态视觉传感器的定义
// 文档：https://openhutb.github.io/carla_doc/ref_sensors/#dvs-camera
FActorDefinition ADVSCamera::GetSensorDefinition()
{
  constexpr bool bEnableModifyingPostProcessEffects = true;
  auto Definition = UActorBlueprintFunctionLibrary::MakeCameraDefinition(TEXT("dvs"), bEnableModifyingPostProcessEffects);

  FActorVariation Cp;
  Cp.Id = TEXT("positive_threshold");      // 与亮度变化增量相关的正阈值 C，范围为 (0-1)。
  Cp.Type = EActorAttributeType::Float;    // 值的类型
  Cp.RecommendedValues = { TEXT("0.3") };  // 默认值
  Cp.bRestrictToRecommended = false;

  FActorVariation Cm;
  Cm.Id = TEXT("negative_threshold");       // 与亮度变化减少相关的负阈值 C，范围为(0-1)。
  Cm.Type = EActorAttributeType::Float;
  Cm.RecommendedValues = { TEXT("0.3") };
  Cm.bRestrictToRecommended = false;

  FActorVariation Sigma_Cp;
  Sigma_Cp.Id = TEXT("sigma_positive_threshold");  // 正事件的白噪声标准差，范围为 (0-1)。
  Sigma_Cp.Type = EActorAttributeType::Float;
  Sigma_Cp.RecommendedValues = { TEXT("0.0") };
  Sigma_Cp.bRestrictToRecommended = false;

  FActorVariation Sigma_Cm;
  Sigma_Cm.Id = TEXT("sigma_negative_threshold");  // 负事件的白噪声标准差，范围为 (0-1)。
  Sigma_Cm.Type = EActorAttributeType::Float;
  Sigma_Cm.RecommendedValues = { TEXT("0.0") };
  Sigma_Cm.bRestrictToRecommended = false;

  FActorVariation Refractory_Period;
  Refractory_Period.Id = TEXT("refractory_period_ns");  // 不应期（像素在触发事件后无法触发事件的时间），以纳秒为单位。它限制了触发事件的最高频率。
  Refractory_Period.Type = EActorAttributeType::Int;
  Refractory_Period.RecommendedValues = { TEXT("0") };
  Refractory_Period.bRestrictToRecommended = false;

  FActorVariation Use_Log;
  Use_Log.Id = TEXT("use_log");  // 是否以对数强度刻度工作
  Use_Log.Type = EActorAttributeType::Bool;
  Use_Log.RecommendedValues = { TEXT("True") };
  Use_Log.bRestrictToRecommended = false;

  FActorVariation Log_EPS;
  Log_EPS.Id = TEXT("log_eps");  // 用于将图像转换为日志的 Epsilon 值: L = log(eps + I / 255.0)
  Log_EPS.Type = EActorAttributeType::Float;
  Log_EPS.RecommendedValues = { TEXT("0.001") };
  Log_EPS.bRestrictToRecommended = false;

  Definition.Variations.Append({ Cp, Cm, Sigma_Cp, Sigma_Cm, Refractory_Period, Use_Log, Log_EPS });

  return Definition;
}

void ADVSCamera::Set(const FActorDescription &Description)
{
  Super::Set(Description);

  this->config.Cp = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "positive_threshold",
      Description.Variations,
      0.5f);

  this->config.Cm = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "negative_threshold",
      Description.Variations,
      0.5f);

  this->config.sigma_Cp = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "sigma_positive_threshold",
      Description.Variations,
      0.0f);

  this->config.sigma_Cm = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "sigma_negative_threshold",
      Description.Variations,
      0.0f);

  this->config.refractory_period_ns = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToInt(
      "refractory_period_ns",
      Description.Variations,
      0.0);

  this->config.use_log = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToBool(
      "use_log",
      Description.Variations,
      true);

  this->config.log_eps = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "log_eps",
      Description.Variations,
      1e-03);
}

// 物理节拍信号后处理
void ADVSCamera::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaTime)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ADVSCamera::PostPhysTick);
  check(CaptureRenderTarget != nullptr);
  if (!HasActorBegunPlay() || IsPendingKill())
  {
    return;
  }

  /// 在当前时刻，场景的立刻入队渲染命令。
  EnqueueRenderSceneImmediate();
  WaitForRenderThreadToFinish();

  //Super (ASceneCaptureSensor) Capture the Scene in a (UTextureRenderTarget2D) CaptureRenderTarge from the CaptureComponent2D
  /** 读取图像 **/
  TArray<FColor> RawImage;
  this->ReadPixels(RawImage);

  /** 将图像转换为灰度图 **/
  if (this->config.use_log)
  {
    this->ImageToLogGray(RawImage);
  }
  else
  {
    this->ImageToGray(RawImage);
  }

  /** 动态视觉传感器仿真器 **/
  ADVSCamera::DVSEventArray events = this->Simulation(DeltaTime);

  auto Stream = GetDataStream(*this);       // 获得数据流
  auto Buff = Stream.PopBufferFromPool();   // 从内存池中获取一个内存缓冲，用于存数据

  // 序列化数据：将动态视觉传感器仿真器中的时间 移动到 缓冲区中（std::move 本身并不移动任何东西;它只是将其参数转换为右值引用）
  carla::Buffer BufferReady(carla::sensor::SensorRegistry::Serialize(*this, events, std::move(Buff)));
  carla::SharedBufferView BufView = carla::BufferView::CreateFrom(std::move(BufferReady));

  // ROS2
  #if defined(WITH_ROS2)
  auto ROS2 = carla::ros2::ROS2::GetInstance();
  if (ROS2->IsEnabled())
  {
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("ROS2 Send");
    auto StreamId = carla::streaming::detail::token_type(GetToken()).get_stream_id();
    {
      // 获得相机分辨率
      int W = -1, H = -1;
      float Fov = -1.0f;
      auto WidthOpt = GetAttribute("image_size_x");
      if (WidthOpt.has_value())
        W = FCString::Atoi(*WidthOpt->Value);
      auto HeightOpt = GetAttribute("image_size_y");
      if (HeightOpt.has_value())
        H = FCString::Atoi(*HeightOpt->Value);
      auto FovOpt = GetAttribute("fov");
      if (FovOpt.has_value())
        Fov = FCString::Atof(*FovOpt->Value);
      AActor* ParentActor = GetAttachParentActor();
      if (ParentActor)
      {
        FTransform LocalTransformRelativeToParent = GetActorTransform().GetRelativeTransform(ParentActor->GetActorTransform());
        ROS2->ProcessDataFromDVS(Stream.GetSensorType(), StreamId, LocalTransformRelativeToParent, BufView, W, H, Fov, this);
      }
      else
      {
        ROS2->ProcessDataFromDVS(Stream.GetSensorType(), StreamId, Stream.GetSensorTransform(), BufView, W, H, Fov, this);
      }
    }
  }
  #endif
  if (events.size() > 0)
  {
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("ADVSCamera Stream Send");
    /** 发送事件 **/
    Stream.Send(*this, BufView);
  }
}

// 图像转为灰度图
void ADVSCamera::ImageToGray(const TArray<FColor> &image)
{
  /** 合理性检查 **/
  if (image.Num() != (this->GetImageHeight() * this->GetImageWidth()))
    return;

  /** 保存 HxW 个元素 **/
  last_image.SetNumUninitialized(image.Num());

  /** 将图像转化为灰度原始图像值 **/
  for (size_t i = 0; i < image.Num(); ++i)
  {
    last_image[i] = FColorToGrayScaleFloat(image[i]);
  }
}

// 将图像转化为 对数灰度图
void ADVSCamera::ImageToLogGray(const TArray<FColor> &image)
{
  /** 合理性检查 **/
  if (image.Num() != (this->GetImageHeight() * this->GetImageWidth()))
    return;

  /** 保存 HxW 个元素 **/
  last_image.SetNumUninitialized(image.Num());  // 最近（当前）的图像 vs 先前的图像：动态视觉传感器是计算两者的变化

  /** 将图像转化为灰度原始图像值 **/
  for (size_t i = 0; i < image.Num(); ++i)
  {
    // L = log(eps + I / 255.0)
    // log_eps = 1e-03
    last_image[i] = std::log(this->config.log_eps + (FColorToGrayScaleFloat(image[i]) / 255.0));
  }
}

// 执行仿真
ADVSCamera::DVSEventArray ADVSCamera::Simulation (float DeltaTime)
{
  /** 事件数组 **/
  ADVSCamera::DVSEventArray events;

  /** 合理性检查 **/
  if (this->last_image.Num() != (this->GetImageHeight() * this->GetImageWidth()))
    return events;

  /** 检查初始化 **/
  if(this->prev_image.Num() == 0)
  {
    /** 设置第一个渲染的图像 **/
    this->ref_values = this->last_image;  // 参考值 <- 最新图像
    this->prev_image = this->last_image;  // 先前图像 <- 最新图像

    /** 将数组大小调整为给定的元素数量。新元素将被归零。 **/
    this->last_event_timestamp.SetNumZeroed(this->last_image.Num());

    /** 重置当前时间 **/
    this->current_time = dvs::secToNanosec(this->GetEpisode().GetElapsedGameTime());

    return events;
  }

  static constexpr float tolerance = 1e-6;  // 判断前后图像像素亮度是否发生变化的阈值

  /** 以纳秒表示的时间增量 **/
  const std::uint64_t delta_t_ns = dvs::secToNanosec(
      this->GetEpisode().GetElapsedGameTime()) - this->current_time;

  /** 沿着图像大小循环 **/
  for (uint32 y = 0; y < this->GetImageHeight(); ++y)
  {
    for (uint32 x = 0; x < this->GetImageWidth(); ++x)
    {
      const uint32 i = (this->GetImageWidth() * y) + x;  // 将2维中的索引转换为1维中的索引
      const float itdt = this->last_image[i];  // 先前图像过了时间增量dt后的图像（即最新的图像）在索引为i位置的像素值
      const float it = this->prev_image[i];  // 先前的图像 在索引为i的像素值
      const float prev_cross = this->ref_values[i];

      if (std::fabs (it - itdt) > tolerance)  // 如果前后的像素亮度变化超过阈值
      {
        // 根据像素亮度变化的符号来判断事件的极性(polarity)。
        // `+1`当亮度增加时极性为正，`-1`当亮度减少时极性为负。
        const float pol = (itdt >= it) ? +1.0 : -1.0;
        float C = (pol > 0) ? this->config.Cp : this->config.Cm;  // Cp正事件(positive)，Cm负事件。对比度门限值（C,contrast threshold）
        const float sigma_C = (pol > 0) ? this->config.sigma_Cp : this->config.sigma_Cm;

        if(sigma_C > 0)
        {
          C += RandomEngine->GetNormalDistribution(0, sigma_C);
          constexpr float minimum_contrast_threshold = 0.01;
          C = std::max(minimum_contrast_threshold, C);  // 返回两个值中的最大值
        }
        float curr_cross = prev_cross;
        bool all_crossings = false;

        do
        {
          curr_cross += pol * C;

          if ((pol > 0 && curr_cross > it && curr_cross <= itdt)
              || (pol < 0 && curr_cross < it && curr_cross >= itdt))
          {
            const std::uint64_t edt = (curr_cross - it) * delta_t_ns / (itdt - it);
            const std::int64_t t = this->current_time + edt;

            // 检查像素(x,y)当前不处于“不应”状态
            // i.e. |t - that last_timestamp(x,y)| >= refractory_period
            const std::int64_t last_stamp_at_xy = dvs::secToNanosec(this->last_event_timestamp[i]);
            if (t >= last_stamp_at_xy)
            {
              const std::uint64_t dt = t - last_stamp_at_xy;
              if(this->last_event_timestamp[i] == 0 || dt >= this->config.refractory_period_ns)
              {
                events.push_back(::carla::sensor::data::DVSEvent(x, y, t, pol > 0));
                this->last_event_timestamp[i] = dvs::nanosecToSecTrunc(t);
              }
              else
              {
                /** 取消事件，因为距离上次事件的时间 小于 不应期refractory_period_ns **/
              }
              this->ref_values[i] = curr_cross;
            }
          }
          else
          {
            all_crossings = true;
          }
        } while (!all_crossings);
      } // end tolerance
    } // end for each pixel
  }

  /** 更新当前时间 **/
  this->current_time = dvs::secToNanosec(this->GetEpisode().GetElapsedGameTime());

  this->prev_image = this->last_image;

  // 通过增加时间戳对事件进行排序，因为这是大多数事件处理算法所期望的
  std::sort(events.begin(), events.end(), [](const ::carla::sensor::data::DVSEvent& it1, const ::carla::sensor::data::DVSEvent& it2){return it1.t < it2.t;});

  return events;
}
