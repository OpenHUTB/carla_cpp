// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreGlobals.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Runtime/ImageWriteQueue/Public/ImagePixelData.h"

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <D3d12.h>
#endif

#include "Carla/Game/CarlaEngine.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/Logging.h>
#include <carla/Buffer.h>
#include <carla/BufferView.h>
#include <carla/sensor/SensorRegistry.h>
#include <compiler/enable-ue4-macros.h>

// =============================================================================
// -- FPixelReader -------------------------------------------------------------
// =============================================================================

/// 用于从 UTextureRenderTarget2D 读取像素的实用工具。
///
/// @todo 当前该类仅支持 PF_R8G8B8A8 格式。
class FPixelReader
{
public:

  using Payload = std::function<void(void *, uint32, uint32, uint32)>;

  /// 将 @a RenderTarget 中的像素复制到 @a BitMap 中。
  ///
  /// @pre 必须从游戏线程调用。
  static bool WritePixelsToArray(
      UTextureRenderTarget2D &RenderTarget,
      TArray<FColor> &BitMap);

  /// 转储 @a RenderTarget 中的像素。
  ///
  /// @pre 必须从游戏线程调用。
  static TUniquePtr<TImagePixelData<FColor>> DumpPixels(
      UTextureRenderTarget2D &RenderTarget);

  /// 异步保存 @a RenderTarget 中的像素到磁盘。
  ///
  /// @pre 必须从游戏线程调用。
  static TFuture<bool> SavePixelsToDisk(
      UTextureRenderTarget2D &RenderTarget,
      const FString &FilePath);

  /// 异步保存 @a PixelData 中的像素到磁盘。
  ///
  /// @pre 必须从游戏线程调用。
  static TFuture<bool> SavePixelsToDisk(
      TUniquePtr<TImagePixelData<FColor>> PixelData,
      const FString &FilePath);

  /// 方便函数，将渲染命令加入队列以通过 @a Sensor 的数据流发送像素。
  /// 它需要一个继承自 ASceneCaptureSensor 或兼容的传感器。
  ///
  /// 注意：序列化器需要定义一个 "header_offset"，分配在缓冲区前部。
  ///
  /// @pre 必须从游戏线程调用。
  template <typename TSensor, typename TPixel>
  static void SendPixelsInRenderThread(TSensor &Sensor, bool use16BitFormat = false, std::function<TArray<TPixel>(void *, uint32)> Conversor = {});

  /// 将 @a RenderTarget 中的像素复制到 @a Buffer。
  ///
  /// @pre 必须从渲染线程调用。
  static void WritePixelsToBuffer(
      const UTextureRenderTarget2D &RenderTarget,
      uint32 Offset,
      FRHICommandListImmediate &InRHICmdList,
      FPixelReader::Payload FuncForSending);

};

// =============================================================================
// -- FPixelReader::SendPixelsInRenderThread -----------------------------------
// =============================================================================

template <typename TSensor, typename TPixel>
void FPixelReader::SendPixelsInRenderThread(TSensor &Sensor, bool use16BitFormat, std::function<TArray<TPixel>(void *, uint32)> Conversor)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(FPixelReader::SendPixelsInRenderThread);
  check(Sensor.CaptureRenderTarget != nullptr);

  if (!Sensor.HasActorBegunPlay() || Sensor.IsPendingKill())
  {
    return;
  }

  /// 阻塞，直到渲染线程完成所有任务。
  Sensor.EnqueueRenderSceneImmediate();

  // 在渲染线程中加入命令，将图像缓冲区写入数据流。
  ENQUEUE_RENDER_COMMAND(FWritePixels_SendPixelsInRenderThread)
  (
    [&Sensor, use16BitFormat, Conversor = std::move(Conversor)](auto &InRHICmdList) mutable
    {
      TRACE_CPUPROFILER_EVENT_SCOPE_STR("FWritePixels_SendPixelsInRenderThread");

      /// @todo 确保传感器不会被销毁？
      if (!Sensor.IsPendingKill())
      {
        FPixelReader::Payload FuncForSending =
          [&Sensor, Frame = FCarlaEngine::GetFrameCounter(), Conversor = std::move(Conversor)](void *LockedData, uint32 Size, uint32 Offset, uint32 ExpectedRowBytes)
          {
            if (Sensor.IsPendingKill()) return;

            TArray<TPixel> Converted;

            // 可选的数据转换
            if (Conversor)
            {
              TRACE_CPUPROFILER_EVENT_SCOPE_STR("Data conversion");
              Converted = Conversor(LockedData, Size);
              LockedData = reinterpret_cast<void *>(Converted.GetData());
              Size = Converted.Num() * Converted.GetTypeSize();
            }

            auto Stream = Sensor.GetDataStream(Sensor);
            Stream.SetFrameNumber(Frame);
            auto Buffer = Stream.PopBufferFromPool();

            uint32 CurrentRowBytes = ExpectedRowBytes;

#ifdef _WIN32
            // DirectX 为了对齐每行到 256 字节边界而增加了额外字节，需要移除这些数据。
            if (IsD3DPlatform(GMaxRHIShaderPlatform, false))
            {
              CurrentRowBytes = Align(ExpectedRowBytes, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
              if (ExpectedRowBytes != CurrentRowBytes)
              {
                TRACE_CPUPROFILER_EVENT_SCOPE_STR("Buffer Copy (windows, row by row)");
                Buffer.reset(Offset + Size);
                auto DstRow = Buffer.begin() + Offset;
                const uint8 *SrcRow = reinterpret_cast<uint8 *>(LockedData);
                uint32 i = 0;
                while (i < Size)
                {
                  FMemory::Memcpy(DstRow, SrcRow, ExpectedRowBytes);
                  DstRow += ExpectedRowBytes;
                  SrcRow += CurrentRowBytes;
                  i += ExpectedRowBytes;
                }
              }
            }
#endif // _WIN32

            if (ExpectedRowBytes == CurrentRowBytes)
            {
              check(ExpectedRowBytes == CurrentRowBytes);
              TRACE_CPUPROFILER_EVENT_SCOPE_STR("Buffer Copy");
              Buffer.copy_from(Offset, boost::asio::buffer(LockedData, Size));
            }

            {
              // 发送数据
              TRACE_CPUPROFILER_EVENT_SCOPE_STR("Sending buffer");
              if(Buffer.data())
              {
                // 序列化数据
                carla::Buffer BufferReady(std::move(carla::sensor::SensorRegistry::Serialize(Sensor, std::move(Buffer))));
                carla::SharedBufferView BufView = carla::BufferView::CreateFrom(std::move(BufferReady));

                // ROS2 支持
                #if defined(WITH_ROS2)
                auto ROS2 = carla::ros2::ROS2::GetInstance();
                if (ROS2->IsEnabled())
                {
                  TRACE_CPUPROFILER_EVENT_SCOPE_STR("ROS2 Send PixelReader");
                  auto StreamId = carla::streaming::detail::token_type(Sensor.GetToken()).get_stream_id();
                  auto Res = std::async(std::launch::async, [&Sensor, ROS2, &Stream, StreamId, BufView]()
                  {
                    // 获取相机分辨率
                    int W = -1, H = -1;
                    float Fov = -1.0f;
                    auto WidthOpt = Sensor.GetAttribute("image_size_x");
                    if (WidthOpt.has_value())
                      W = FCString::Atoi(*WidthOpt->Value);
                    auto HeightOpt = Sensor.GetAttribute("image_size_y");
                    if (HeightOpt.has_value())
                      H = FCString::Atoi(*HeightOpt->Value);
                    auto FovOpt = Sensor.GetAttribute("fov");
                    if (FovOpt.has_value())
                      Fov = FCString::Atof(*FovOpt->Value);
                    // 将数据发送到 ROS2
                    AActor* ParentActor = Sensor.GetAttachParentActor();
                    if (ParentActor)
                    {
                      FTransform LocalTransformRelativeToParent = Sensor.GetActorTransform().GetRelativeTransform(ParentActor->GetActorTransform());
                      ROS2->ProcessDataFromCamera(Stream.GetSensorType(), StreamId, LocalTransformRelativeToParent, W, H, Fov, BufView, &Sensor);
                    }
                    else
                    {
                      ROS2->ProcessDataFromCamera(Stream.GetSensorType(), StreamId, Stream.GetSensorTransform(), W, H, Fov, BufView, &Sensor);
                    }
                  });
                }
                #endif

                // 网络传输
                SCOPE_CYCLE_COUNTER(STAT_CarlaSensorStreamSend);
                TRACE_CPUPROFILER_EVENT_SCOPE_STR("Stream Send");
                Stream.Send(Sensor, BufView);
              }
            }
          };

          WritePixelsToBuffer(
              *Sensor.CaptureRenderTarget,
              carla::sensor::SensorRegistry::get<TSensor *>::type::header_offset,
              InRHICmdList,
              std::move(FuncForSending));
        }
      }
    );

  // 阻塞，直到渲染线程完成所有任务。
  Sensor.WaitForRenderThreadToFinish();
}

