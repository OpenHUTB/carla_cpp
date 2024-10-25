// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
/// @brief 提供XML文档处理功能的命名空间，包含XML文档的加载、解析和遍历等功能
namespace pugi {
    /// @brief 表示一个XML文档的类，用于存储和解析XML数据
  class xml_document;
} // namespace pugi
/// @brief Carla自动驾驶仿真框架的命名空间
namespace carla {
    /// @brief 道路建模和地图构建的命名空间
namespace road {
    /// @brief 用于构建道路地图的类，提供地图元素的添加和地图构建的接口
  class MapBuilder;
} // namespace road
 /// @brief OpenDRIVE格式道路数据处理的命名空间
namespace opendrive {
    /// @brief 包含用于解析OpenDRIVE文件的解析器类的命名空间
namespace parser {
    /// @brief 用于解析OpenDRIVE文件中的道路设计配置文件（Profiles）信息的解析器类
  class ProfilesParser {
  public:
    /// @brief 解析XML文档中的道路设计配置文件信息，并将其用于构建道路地图
    /// 该函数读取XML文档中的道路设计配置文件数据，如道路的水平和垂直曲线信息 
    /// 并使用这些数据来构建或更新道路地图中的道路设计配置文件部分  
    /// @param xml [const pugi::xml_document &] 要解析的XML文档对象，包含道路设计配置文件信息 
    /// @param map_builder [carla::road::MapBuilder &] 用于构建道路地图的构建器对象
    static void Parse(
        const pugi::xml_document &xml,
        carla::road::MapBuilder &map_builder);

  };

} // namespace parser
} // namespace opendrive
} // namespace carla
