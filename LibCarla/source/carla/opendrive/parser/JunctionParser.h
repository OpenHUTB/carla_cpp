// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
/// @brief 提供XML文档处理功能的命名空间
namespace pugi {
    /// @brief 表示一个XML文档的类，用于解析和存储XML数据
  class xml_document;
} // namespace pugi
/// @brief Carla自动驾驶仿真框架的命名空间
namespace carla {
/// @brief 包含道路建模和地图构建功能的命名空间
namespace road {
/// @brief 用于构建道路地图的类，提供地图构建的接口
  class MapBuilder;
} // namespace road
/// @brief 包含OpenDrive格式道路数据解析功能的命名空间
namespace opendrive {
    /// @brief 包含用于解析OpenDrive文件的解析器类的命名空间
namespace parser {
    /// @brief 用于解析OpenDrive文件中的交叉路口（Junction）信息的解析器类
  class JunctionParser {
  public:
      /// @brief 解析XML文档中的交叉路口信息，并构建道路地图中的交叉路口部分
      /// 该函数读取XML文档中的交叉路口数据，如连接点、车道信息等
      /// 并使用这些数据来构建或更新道路地图中的交叉路口部分
      /// @param xml [const pugi::xml_document &] 要解析的XML文档对象 
      /// @param map_builder [carla::road::MapBuilder &] 用于构建道路地图的构建器对象
    static void Parse(
        const pugi::xml_document &xml,
        carla::road::MapBuilder &map_builder);

  };

} // namespace parser
} // namespace opendrive
} // namespace carla
