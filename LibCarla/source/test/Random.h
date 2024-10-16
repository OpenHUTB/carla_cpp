// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once// 确保此头文件在编译过程中只包含一次

#include <carla/geom/Location.h>// 引入 Carla 库中的 Location 类

#include <algorithm>// 提供算法操作
#include <random> // 提供随机数生成的功能

namespace util {

  class Random {
  public:
// 生成范围 [min, max) 内的随机双精度浮点数
    static double Uniform(double min, double max) {
      std::uniform_real_distribution<double> distribution(min, max);
      return distribution(_engine); // 使用随机引擎生成并返回分布中的随机数
    }
// 生成坐标范围 [min, max) 内的随机 carla::geom::Location 对象
    static carla::geom::Location Location(float min, float max) {
      std::uniform_real_distribution<float> distribution(min, max);
      return {distribution(_engine), distribution(_engine), distribution(_engine)};// 创建一个随机的 Location 对象，包含随机的 x, y, z 坐标值
    }
// 使用随机引擎对提供的范围进行洗牌

    template <typename RangeT>
    static void Shuffle(RangeT &range) {
      std::shuffle(std::begin(range), std::end(range), _engine);
    }
  private:
 // 线程局部随机数引擎，确保每个线程都有自己的实例
    // Defined in test.cpp.
// 在 test.cpp 中定义，以便进行适当的初始化
    static thread_local std::mt19937_64 _engine;
  };

} // namespace util
