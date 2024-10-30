// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
/// @brief 提供XML文档处理功能的命名空间，包含XML文档的表示和解析等功能
namespace pugi {
    /// @brief 表示一个XML文档的类，包含XML数据的加载、解析、遍历和修改等功能
  class xml_document;
} // namespace pugi
/// @brief Carla自动驾驶仿真框架的命名空间，包含道路、车辆、传感器等仿真元素的定义和接口
namespace carla {
    /// @brief 道路建模和地图构建的命名空间，包含道路元素的定义、地图的构建和更新等功能
namespace road {
    /// @brief 用于构建和更新道路地图的类，提供地图元素的添加、删除和修改等接口
  class MapBuilder;
} // namespace road
/// @brief OpenDrive格式道路数据处理的命名空间，包含OpenDrive文件的解析、转换和存储等功能
namespace opendrive {
    /// @brief 包含用于解析OpenDrive文件的解析器类的命名空间
namespace parser {
    /// @brief 用于解析OpenDrive文件中的道路对象（如交通标志、交通灯、障碍物等）的解析器类
  class ObjectParser {
  public:
      /// @brief 解析XML文档中的道路对象信息，并将其添加到道路地图中
      /// 该函数读取XML文档中的道路对象数据，如位置、类型、属性等  
      /// 并使用这些数据来构建或更新道路地图中的道路对象部分 
      /// @param xml [const pugi::xml_document &] 要解析的XML文档对象，包含道路对象信息
      /// @param map_builder [carla::road::MapBuilder &] 用于构建和更新道路地图的构建器对象
    static void Parse(
        const pugi::xml_document &xml,
        carla::road::MapBuilder &map_builder);
  };

} // namespace parser
} // namespace opendrive
} // namespace carla
