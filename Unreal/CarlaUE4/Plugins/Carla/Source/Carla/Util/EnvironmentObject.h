//  版权所有 （c） 2020 Uniive 计算机视觉中心 （CVC）
// de Barcelona (UAB).
// 本作品根据 MIT 许可证的条款进行许可。
//有关副本，请参阅 <https://opensource.org/licenses/MIT>。

#编译一次

#包括"Carla/Util/BoundingBox.h"
#包括 <compiler/disable-ue4-macros.h>
#包括 <carla/rpc/ObjectLabel.h>
#包括 <compiler/enable-ue4-macros.h>

#包括 "BoundingBox.h"

#包括 "EnvironmentObject.generated.h"

namespace crp = carla::rpc;

enum EnvironmentObjectType
{
  Invalid,
  Vehicle,
  Character,
  TrafficLight,
  ISMComp,
  SMComp,
  SKMComp
};

// 姓名正在讨论中
USTRUCT(BlueprintType)
struct CARLA_API FEnvironmentObject
{
  GENERATED_BODY()

  AActor* Actor = nullptr;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString Name;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FString IdStr;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FTransform Transform;

  UPROPERTY(EditAnywhere, BlueprintReadWrite)
  FBoundingBox BoundingBox;

  uint64 Id = 0;

  EnvironmentObjectType Type = EnvironmentObjectType::Invalid;

  crp::CityObjectLabel ObjectLabel;

  bool CanTick = false;
};
