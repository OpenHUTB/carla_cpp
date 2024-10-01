// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/opendrive/OpenDriveParser.h"

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

#include <pugixml/pugixml.hpp>

namespace carla {
namespace opendrive {

  boost::optional<road::Map> OpenDriveParser::Load(const std::string &opendrive) {
    pugi::xml_document xml;
    pugi::xml_parse_result parse_result = xml.load_string(opendrive.c_str());  // 使用 pugixml XML 处理工具加载OpenDrive文件

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
    parser::LaneParser::Parse(xml, map_builder);
    parser::ProfilesParser::Parse(xml, map_builder);
    parser::TrafficGroupParser::Parse(xml, map_builder);
    parser::SignalParser::Parse(xml, map_builder);
    parser::ObjectParser::Parse(xml, map_builder);
    parser::ControllerParser::Parse(xml, map_builder);

    return map_builder.Build();
  }

} // namespace opendrive
} // namespace carla
