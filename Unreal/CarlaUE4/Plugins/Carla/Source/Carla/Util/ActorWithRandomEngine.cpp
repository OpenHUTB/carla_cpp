// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "ActorWithRandomEngine.h"

#include "Util/RandomEngine.h"

// 初始化 AActorWithRandomEngine 对象，并创建一个默认的随机引擎子对象
AActorWithRandomEngine::AActorWithRandomEngine(const FObjectInitializer& ObjectInitializer) :
  Super(ObjectInitializer)
{
  RandomEngine = CreateDefaultSubobject<URandomEngine>(TEXT("RandomEngine"));
}

// 确保随机引擎已初始化并设置种子值
void AActorWithRandomEngine::OnConstruction(const FTransform &Transform)
{
  Super::OnConstruction(Transform);
  check(RandomEngine != nullptr);
  RandomEngine->Seed(Seed);
}

#if WITH_EDITOR
// 编辑器中属性更改后调用：在属性修改后，检查是否需要生成新的随机种子并重新设置随机引擎的种子。
void AActorWithRandomEngine::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
  Super::PostEditChangeProperty(PropertyChangedEvent);
  if (PropertyChangedEvent.Property) {
    if (bGenerateRandomSeed) {
      Seed = URandomEngine::GenerateRandomSeed();
      bGenerateRandomSeed = false;
    }
    check(RandomEngine != nullptr);
    RandomEngine->Seed(Seed);
  }
}
#endif // WITH_EDITOR

// 设置随机引擎的种子值
void AActorWithRandomEngine::SetSeed(const int32 InSeed)
{
  check(RandomEngine != nullptr);
  Seed = InSeed;
  RandomEngine->Seed(InSeed);
}
