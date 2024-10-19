// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
/// @brief 提供XML文档处理功能的命名空间，包含用于解析和操作XML文档的类
namespace pugi {
    /// @brief 表示一个XML文档的类，用于加载、解析和访问XML数据
  class xml_document;
} // namespace pugi
/// @brief CARLA自动驾驶仿真框架的命名空间，包含自动驾驶相关的各种类和函数
namespace carla {
    /// @brief 道路建模和地图构建的命名空间，包含道路元素和地图构建工具的定义
namespace road {
    /// @brief 用于构建道路地图的类，提供添加道路元素和构建地图的接口
  class MapBuilder;
} // namespace road
/// @brief OpenDRIVE格式道路数据处理的命名空间，包含OpenDRIVE文件的解析和转换功能
namespace opendrive {
    /// @brief 包含用于解析OpenDRIVE文件的解析器类的命名空间
namespace parser {
    /// @brief 用于解析OpenDRIVE文件中的交通信号信息的解析器类
  class SignalParser {
  public:
  /// @brief 解析XML文档中的交通信号信息，并将其用于构建道路地图。  
  /// 该函数读取XML文档中的交通信号数据，如信号灯的位置、类型和控制逻辑，  
  /// 并使用这些数据来构建或更新道路地图中的交通信号部分。  
  /// @param xml [const pugi::xml_document &] 要解析的XML文档对象，包含交通信号信息。  
   /// @param map_builder [carla::road::MapBuilder &] 用于构建道路地图的构建器对象
    static void Parse(
        const pugi::xml_document &xml,
        carla::road::MapBuilder &map_builder);
  };

} // namespace parser
} // namespace opendrive
} // namespace carla
