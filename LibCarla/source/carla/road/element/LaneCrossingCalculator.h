// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 指示编译器只包含一次这个头文件，防止重复包含

#include "carla/road/element/LaneMarking.h" // 包含LaneMarking类的定义,这个类包含了车道标记的相关信息。

#include <vector> // 包含标准模板库中的vector容器，这是一个动态数组，用于存储可变长度的元素序列。

namespace carla {
namespace geom { class Location; } // Location类用于表示二维或三维空间中的一个点。
namespace road {

  class Map; // Map类可能用于表示道路的地图信息，包括道路、车道、交通标志等。

namespace element { // element命名空间用于封装与道路元素相关的类和函数。

  class LaneCrossingCalculator { // LaneCrossingCalculator类是一个用于计算车道穿越的静态工具类。
  public:

    static std::vector<LaneMarking> Calculate( // 静态成员函数，用于计算从起点到终点的车道标记
        const Map &map, // 地图对象的引用
        const geom::Location &origin,  // 起点位置
        const geom::Location &destination);  // 终点位置
  };

} // namespace element
} // namespace road
} // namespace carla
