// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

namespace pugi { // 声明命名空间 pugi
  class xml_document; // 前向声明 xml_document 类
} // namespace pugi

namespace carla { // 声明命名空间 carla

namespace road { // 声明命名空间 road
  class MapBuilder; // 前向声明 MapBuilder 类
} // namespace road

namespace opendrive { // 声明命名空间 opendrive
namespace parser { // 声明命名空间 parser

  class ControllerParser { // 声明 ControllerParser 类
  public:

    static void Parse( // 静态成员函数 Parse
        const pugi::xml_document &xml, // 参数：常量引用 pugi 的 xml_document 对象
        carla::road::MapBuilder &map_builder); // 参数：引用 carla::road 的 MapBuilder 对象

  };

} // namespace parser
} // namespace opendrive
} // namespace carla
