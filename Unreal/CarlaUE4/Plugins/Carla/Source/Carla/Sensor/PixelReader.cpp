// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/PixelReader.h"

#include "Engine/TextureRenderTarget2D.h"
#include "Async/Async.h"
#include "HighResScreenshot.h"
#include "Runtime/ImageWriteQueue/Public/ImageWriteQueue.h"

// =============================================================================
// -- FPixelReader -------------------------------------------------------------
// =============================================================================

/**
 * 将像素数据从渲染目标异步写入缓冲区。
 * 
 * @param RenderTarget 要读取像素数据的纹理渲染目标。
 * @param Offset 缓冲区的字节偏移量。
 * @param RHICmdList 渲染操作的命令列表。
 * @param FuncForSending 处理像素数据的回调函数。
 */
void FPixelReader::WritePixelsToBuffer(
    const UTextureRenderTarget2D &RenderTarget,
    uint32 Offset,
    FRHICommandListImmediate &RHICmdList,
    FPixelReader::Payload FuncForSending)
{
  TRACE_CPUPROFILER_EVENT_SCOPE_STR("WritePixelsToBuffer");
  check(IsInRenderingThread());

  // 获取与渲染目标关联的渲染资源。
  auto RenderResource =
      static_cast<const FTextureRenderTarget2DResource *>(RenderTarget.Resource);
  FTexture2DRHIRef Texture = RenderResource->GetRenderTargetTexture();
  if (!Texture)
  {
    return; // 如果纹理不可用，则退出。
  }

  // 创建一个 GPU 纹理读取对象。
  auto BackBufferReadback = std::make_unique<FRHIGPUTextureReadback>(TEXT("CameraBufferReadback"));
  FIntPoint BackBufferSize = Texture->GetSizeXY();
  EPixelFormat BackBufferPixelFormat = Texture->GetFormat();

  // 将纹理数据复制到读取对象。
  {
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("EnqueueCopy");
    BackBufferReadback->EnqueueCopy(RHICmdList,
                                    Texture,
                                    FResolveRect(0, 0, BackBufferSize.X, BackBufferSize.Y));
  }

  // 强制 Vulkan 刷新 RHI 状态。
  {
    FRenderQueryRHIRef Query = RHICreateRenderQuery(RQT_AbsoluteTime);
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("create query");
    RHICmdList.EndRenderQuery(Query);
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("Flush");
    RHICmdList.ImmediateFlush(EImmediateFlushType::FlushRHIThread);
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("query result");
    uint64 OldAbsTime = 0;
    RHICmdList.GetRenderQueryResult(Query, OldAbsTime, true);
  }

  // 异步处理读取的数据。
  AsyncTask(ENamedThreads::HighTaskPriority, [=, Readback=std::move(BackBufferReadback)]() mutable {
    {
      TRACE_CPUPROFILER_EVENT_SCOPE_STR("Wait GPU transfer");
      while (!Readback->IsReady())
      {
        std::this_thread::yield(); // 在读取完成之前让线程让出资源。
      }
    }

    {
      TRACE_CPUPROFILER_EVENT_SCOPE_STR("Readback data");
      FPixelFormatInfo PixelFormat = GPixelFormats[BackBufferPixelFormat];
      uint32 ExpectedRowBytes = BackBufferSize.X * PixelFormat.BlockBytes;
      int32 Size = (BackBufferSize.Y * (PixelFormat.BlockBytes * BackBufferSize.X));
      void* LockedData = Readback->Lock(Size);
      if (LockedData)
      {
        // 将数据传递给回调函数。
        FuncForSending(LockedData, Size, Offset, ExpectedRowBytes);
      }
      Readback->Unlock();
      Readback.reset();
    }
  });
}

/**
 * 将像素数据从渲染目标写入数组。
 *
 * @param RenderTarget 要读取的纹理渲染目标。
 * @param BitMap 用于存储像素数据的数组。
 * @return 如果成功，返回 true；否则返回 false。
 */
bool FPixelReader::WritePixelsToArray(
    UTextureRenderTarget2D &RenderTarget,
    TArray<FColor> &BitMap)
{
  check(IsInGameThread());
  FTextureRenderTargetResource *RTResource =
      RenderTarget.GameThread_GetRenderTargetResource();
  if (RTResource == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT("FPixelReader: UTextureRenderTarget2D missing render target"));
    return false;
  }
  FReadSurfaceDataFlags ReadPixelFlags(RCM_UNorm);
  ReadPixelFlags.SetLinearToGamma(true);
  return RTResource->ReadPixels(BitMap, ReadPixelFlags);
}

/**
 * 从渲染目标提取像素数据，并将其封装为唯一指针。
 *
 * @param RenderTarget 要读取的纹理渲染目标。
 * @return 包含像素数据的唯一指针。
 */
TUniquePtr<TImagePixelData<FColor>> FPixelReader::DumpPixels(
    UTextureRenderTarget2D &RenderTarget)
{
  const FIntPoint DestSize(RenderTarget.GetSurfaceWidth(), RenderTarget.GetSurfaceHeight());
  TUniquePtr<TImagePixelData<FColor>> PixelData = MakeUnique<TImagePixelData<FColor>>(DestSize);
  TArray<FColor> Pixels(PixelData->Pixels.GetData(), PixelData->Pixels.Num());
  if (!WritePixelsToArray(RenderTarget, Pixels))
  {
    return nullptr; // 如果写入像素失败，则返回 null。
  }
  PixelData->Pixels = Pixels;
  return PixelData;
}

/**
 * 将渲染目标中的像素数据异步保存到磁盘。
 *
 * @param RenderTarget 要读取的纹理渲染目标。
 * @param FilePath 保存图像的文件路径。
 * @return 一个指示操作成功的未来值。
 */
TFuture<bool> FPixelReader::SavePixelsToDisk(
    UTextureRenderTarget2D &RenderTarget,
    const FString &FilePath)
{
  return SavePixelsToDisk(DumpPixels(RenderTarget), FilePath);
}

/**
 * 异步保存像素数据到磁盘。
 *
 * @param PixelData 要保存的像素数据。
 * @param FilePath 保存图像的文件路径。
 * @return 一个指示操作成功的未来值。
 */
TFuture<bool> FPixelReader::SavePixelsToDisk(
    TUniquePtr<TImagePixelData<FColor>> PixelData,
    const FString &FilePath)
{
  TUniquePtr<FImageWriteTask> ImageTask = MakeUnique<FImageWriteTask>();
  ImageTask->PixelData = MoveTemp(PixelData);
  ImageTask->Filename = FilePath;
  ImageTask->Format = EImageFormat::PNG;
  ImageTask->CompressionQuality = (int32) EImageCompressionQuality::Default;
  ImageTask->bOverwriteFile = true;
  ImageTask->PixelPreProcessors.Add(TAsyncAlphaWrite<FColor>(255));

  FHighResScreenshotConfig &HighResScreenshotConfig = GetHighResScreenshotConfig();
  return HighResScreenshotConfig.ImageWriteQueue->Enqueue(MoveTemp(ImageTask));
}
