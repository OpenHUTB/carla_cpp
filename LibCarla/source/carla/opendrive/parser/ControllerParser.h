// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once 
///@brief pugi XML解析库的命名空间
namespace pugi {
    ///@class xml_document
    ///@brief 表示一个XML文档
  class xml_document; 
} // namespace pugi
///@brief Carla模拟器的命名空间
namespace carla {
 ///@brief Carla中道路相关功能的命名空间
namespace road {
    ///@class MapBuilder
    ///@brief 用于在Carla模拟器中构建地图的类
  class MapBuilder;
} // namespace road
///@brief 处理OpenDrive格式地图数据的命名空间
namespace opendrive {
namespace parser {
///@class ControllerParser
///@brief 用于解析Opendrive控制器数据的类
  class ControllerParser {
  public:
      ///@brief 解析XML文档并使用提供的MapBuilder构建地图
      ///@param xml 要解析的XML文档的常量引用
      ///@param map_builder 将用于构建地图的MapBuilder对象的引用
    static void Parse(
        const pugi::xml_document &xml,
        carla::road::MapBuilder &map_builder);

  };

} // namespace parser
} // namespace opendrive
} // namespace carla
