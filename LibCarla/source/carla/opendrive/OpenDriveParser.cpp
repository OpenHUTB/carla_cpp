// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/opendrive/OpenDriveParser.h"
 // 引入OpenDriveParser类的声明

#include "carla/Logging.h"
#include "carla/opendrive/parser/ControllerParser.h"
#include "carla/opendrive/parser/GeoReferenceParser.h"
#include "carla/opendrive/parser/GeometryParser.h"
#include "carla/opendrive/parser/JunctionParser.h"
#include "carla/opendrive/parser/LaneParser.h"
#include "carla/opendrive/parser/ObjectParser.h"
#include "carla/opendrive/parser/ProfilesParser.h"
#include "carla/opendrive/parser/RoadParser.h"
#include "carla/opendrive/parser/SignalParser.h"
#include "carla/opendrive/parser/TrafficGroupParser.h"
#include "carla/road/MapBuilder.h"
// 引入CARLA项目中其他相关头文件，这些文件提供了日志记录、OpenDrive解析的各个部分（如控制器、地理参考、几何形状等）以及地图构建的功能。
#include <pugixml/pugixml.hpp>

// 引入pugixml库的头文件，这是一个用于处理XML的轻量级C++库。

namespace carla {
namespace opendrive {
// 声明CARLA的命名空间，以便在代码中使用简短的类名而不需要前缀。

  boost::optional<road::Map> OpenDriveParser::Load(const std::string &opendrive) {
      // OpenDriveParser类的Load成员函数，用于加载并解析OpenDrive格式的地图数据。
    pugi::xml_document xml;
     // 创建一个pugixml的xml_document对象，用于存储和解析XML数据。
    pugi::xml_parse_result parse_result = xml.load_string(opendrive.c_str()); 
    // 尝试从字符串加载XML数据，opendrive参数是包含OpenDrive XML数据的字符串。
    // load_string函数返回一个xml_parse_result对象，用于检查加载是否成功。
    // 使用 pugixml XML 处理工具加载OpenDrive文件
    // 通常，您会检查parse_result的状态来确定是否成功加载了XML数据。
    // 如果加载成功，您将继续解析XML并构建CARLA的road::Map对象。
    // 如果加载失败，您可能会返回一个空的boost::optional对象或抛出一个异常
    // ...（此处应添加解析XML和构建road::Map的逻辑）
    // 假设解析和构建成功，您可能会返回构建的road::Map对象包装在boost::optional中。
    // 如果发生错误，则返回boost::nullopt以表示没有有效的Map对象。

    if (parse_result == false) {
      log_error("unable to parse the OpenDRIVE XML string");
      return {};
    }
// 创建MapBuilder对象，用于构建地图
    carla::road::MapBuilder map_builder;
 // 使用GeoReferenceParser解析器解析XML中的地理参考信息（如坐标系统），并将这些信息传递给map_builder对象以构建地图的地理基础  
    parser::GeoReferenceParser::Parse(xml, map_builder);
 // 使用RoadParser解析器解析XML中的道路信息（如道路形状、类型等）， 并将这些信息添加到map_builder对象中 
    parser::RoadParser::Parse(xml, map_builder);
  // 使用JunctionParser解析器解析XML中的交叉路口信息， 并将这些信息添加到map_builder对象中 
    parser::JunctionParser::Parse(xml, map_builder);
  // 使用GeometryParser解析器解析XML中的几何信息（如道路曲率、边界等）， 并将这些信息添加到map_builder对象中
    parser::GeometryParser::Parse(xml, map_builder);
  // 使用LaneParser解析器解析XML中的车道信息（如车道宽度、方向等）， 并将这些信息添加到map_builder对象中  
    parser::LaneParser::Parse(xml, map_builder);
  // 使用ProfilesParser解析器解析XML中的道路属性信息（如速度限制、类型等）， 并将这些信息添加到map_builder对象中  
    parser::ProfilesParser::Parse(xml, map_builder);
  // 使用TrafficGroupParser解析器解析XML中的交通组信息（如公交专用道、自行车道等） ，并将这些信息添加到map_builder对象中  
    parser::TrafficGroupParser::Parse(xml, map_builder);
  // 使用SignalParser解析器解析XML中的交通信号信息（如红绿灯、停车标志等） ，并将这些信息添加到map_builder对象中  
    parser::SignalParser::Parse(xml, map_builder);
  // 使用ObjectParser解析器解析XML中的静态物体信息（如树木、建筑物等） ，并将这些信息添加到map_builder对象中  
    parser::ObjectParser::Parse(xml, map_builder);
  // 使用ControllerParser解析器解析XML中可能存在的控制器配置信息  ，并将这些信息添加到map_builder对象中  
    parser::ControllerParser::Parse(xml, map_builder);

    return map_builder.Build();
  }

} // namespace opendrive
} // namespace carla
