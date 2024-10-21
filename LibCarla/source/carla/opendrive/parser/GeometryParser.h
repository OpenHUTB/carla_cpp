// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
/// @brief 一个用于处理XML文档的命名空间
namespace pugi {
  class xml_document;/// @brief 表示一个XML文档的类
} // namespace pugi
/// @brief Carla仿真器的相关功能实现的命名空间
namespace carla {
/// @brief 涉及道路建模和构建的命名空间
namespace road {
  class MapBuilder;/// @brief 用于构建道路地图的类 
} // namespace road
/// @brief 涉及OpenDrive文件的解析器类的命名空间
namespace opendrive {
namespace parser {
/// @brief 用于解析OpenDrive文件中的几何信息的解析器类
  class GeometryParser {
  public:
/// @brief 解析XML文档中的几何信息并构建道路地图
/// @param xml [const pugi::xml_document &] 要解析的XML文档
/// @param map_builder [carla::road::MapBuilder &] 用于构建道路地图的构建器对象
    static void Parse(
        const pugi::xml_document &xml,
        carla::road::MapBuilder &map_builder);

  };

} // namespace parser
} // namespace opendrive
} // namespace carla
