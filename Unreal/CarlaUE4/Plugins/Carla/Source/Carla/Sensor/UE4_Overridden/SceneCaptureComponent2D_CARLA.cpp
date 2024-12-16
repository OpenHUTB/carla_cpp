// 版权所有 (c) 2022  (CVC)  Universitat Autonoma 计算机视觉中心
// 巴塞罗那 (UAB).
//
// 本作品根据 MIT 许可证的条款进行许可。
// 有关副本, 请参阅 <https://opensource.org/licenses/MIT>。

#include "SceneCaptureComponent2D_CARLA.h"



USceneCaptureComponent2D_CARLA::USceneCaptureComponent2D_CARLA(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer)
{
}

const AActor* USceneCaptureComponent2D_CARLA::GetViewOwner() const
{
  return ViewActor;
}
