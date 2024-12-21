﻿// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <random>

#include "RandomEngine.generated.h"

UCLASS(Blueprintable,BlueprintType)
class CARLA_API URandomEngine : public UObject
{
  GENERATED_BODY()

public:

  // ===========================================================================
  /// @name Generate Ids
  // ===========================================================================
  /// @{

  /// 生成一个非确定性随机种子。
  static uint64 GenerateRandomId();

  /// @}
  // ===========================================================================
  /// @name Seed
  // ===========================================================================
  /// @{

  /// 生成一个非确定性随机数种子。
  UFUNCTION(BlueprintCallable)
  static int32 GenerateRandomSeed();

  /// 基于前一个种子生成一个新的种子。
  UFUNCTION(BlueprintCallable)
  int32 GenerateSeed();

  /// 播种随机引擎/初始化随机数生成器
  UFUNCTION(BlueprintCallable)
  void Seed(int32 InSeed)
  {
    Engine.seed(InSeed);
  }

  /// @}
  // ===========================================================================
  /// @name Uniform distribution
  // ===========================================================================
  /// @{

  UFUNCTION(BlueprintCallable)
  float GetUniformFloat()  //返回一个在0到1之间（包括0但不包括1）均匀分布的随机浮点数
  {
    return std::uniform_real_distribution<float>()(Engine);
  }

  UFUNCTION(BlueprintCallable)
  float GetUniformFloatInRange(float Minimum, float Maximum)//返回一个在指定的最小值和最大值之间均匀分布的随机浮点数
  {
    return std::uniform_real_distribution<float>(Minimum, Maximum)(Engine);
  }

  UFUNCTION(BlueprintCallable)
  int32 GetUniformIntInRange(int32 Minimum, int32 Maximum)
  {
    return std::uniform_int_distribution<int32>(Minimum, Maximum)(Engine);
  }

  UFUNCTION(BlueprintCallable)
  bool GetUniformBool()
  {
    return (GetUniformIntInRange(0, 1) == 1);
  }

  /// @}
  // ===========================================================================
  /// @name Other distributions
  // ===========================================================================
  /// @{

  UFUNCTION(BlueprintCallable)
  bool GetBernoulliDistribution(float P)
  {
    return std::bernoulli_distribution(P)(Engine);
  }

  UFUNCTION(BlueprintCallable)
  int32 GetBinomialDistribution(int32 T, float P)
  {
    return std::binomial_distribution<int32>(T, P)(Engine);
  }

  UFUNCTION(BlueprintCallable)
  int32 GetPoissonDistribution(float Mean)
  {
    return std::poisson_distribution<int32>(Mean)(Engine);
  }

  UFUNCTION(BlueprintCallable)
  float GetExponentialDistribution(float Lambda)
  {
    return std::exponential_distribution<float>(Lambda)(Engine);
  }

  UFUNCTION(BlueprintCallable)
  float GetNormalDistribution(float Mean, float StandardDeviation)
  {
    return std::normal_distribution<float>(Mean, StandardDeviation)(Engine);
  }

  /// @}
  // ===========================================================================
  /// @name Sampling distributions
  // ===========================================================================
  /// @{

  UFUNCTION(BlueprintCallable)
  bool GetBoolWithWeight(float Weight)
  {
    return (Weight >= GetUniformFloat());
  }

  UFUNCTION(BlueprintCallable)
  int32 GetIntWithWeight(const TArray<float> &Weights)
  {
    return std::discrete_distribution<int32>(
        Weights.GetData(),
        Weights.GetData() + Weights.Num())(Engine);
  }

  /// @}
  // ===========================================================================
  /// @name Elements in TArray
  // ===========================================================================
  /// @{

  template <typename T>
  auto &PickOne(const TArray<T> &Array)//函数参数是 const TArray<T> &，这意味着数组本身在函数内部是不可修改的
  {
    check(Array.Num() > 0);
    return Array[GetUniformIntInRange(0, Array.Num() - 1)];
  }

  template <typename T>
  void Shuffle(TArray<T> &Array)//std::shuffle 来打乱数组的顺序
  {
    std::shuffle(Array.GetData(), Array.GetData() + Array.Num(), Engine);
  }

  /// @}

private:

  std::minstd_rand Engine;
};
