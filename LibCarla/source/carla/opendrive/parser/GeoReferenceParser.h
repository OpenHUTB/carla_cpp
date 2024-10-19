// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
/// @brief 一个提供XML文档的类，用于储存和解析XML数据
namespace pugi {
  class xml_document;
} // namespace pugi
/// @brief Carla自动驾驶仿真框架的命名空间
namespace carla {
/// @brief 包含道路建模和地图构建相关功能的命名空间
namespace road {
  class MapBuilder;/// @brief 用于构建道路地图的类，提供地图构建的接口和方法
} // namespace road
/// @brief 包含OpenDrive格式道路数据解析和处理功能的命名空间
namespace opendrive {
 /// @brief 包含用于解析OpenDrive文件的解析器类的命名空间
namespace parser {
 /// @brief 用于解析OpenDrive文件中的地理参考信息的解析器类
  class GeoReferenceParser {
  public:
/// @brief 解析XML文档中的地理参考信息并构建道路地图
/// 该函数读取XML文档中的地理参考数据，如坐标系统、投影信息
/// 并使用这些信息来构建或更新道路地图
/// @param xml [const pugi::xml_document &] 要解析的XML文档对象
/// @param map_builder [carla::road::MapBuilder &] 用于构建道路地图的构建器对象
    static void Parse(
        const pugi::xml_document &xml,
        carla::road::MapBuilder &map_builder);
  };

} // namespace parser
} // namespace opendrive
} // namespace carla
