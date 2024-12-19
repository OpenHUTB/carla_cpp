// 版权所有 （c） 2023 Universitat Autonoma 计算机视觉中心 （CVC）
// de Barcelona (UAB).
//
// 本作品根据 MIT 许可证的条款获得许可.
// 有关副本，请参阅 <https://opensource.org/licenses/MIT>。

#编译一次

#包括 "Carla/Actor/ActorSpawnResult.h"
#包括 "Carla/Actor/CarlaActorFactory.h"

#包括 "UtilActorFactory.generated.h"

/// 负责生成静态网格体的工厂。该工厂能够生成
/// 内容中的任何网格
UCLASS()
class CARLA_API AUtilActorFactory : public ACarlaActorFactory
{
  GENERATED_BODY()

  /// 获取静态网格体演员的定义
  TArray<FActorDefinition> GetDefinitions() final;

  FActorSpawnResult SpawnActor(
      const FTransform &SpawnAtTransform,
      const FActorDescription &ActorDescription) final;
};
