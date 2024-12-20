// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
// 包含用于解析OpenDRIVE格式中交通组相关信息的头文件，其中应该定义了TrafficGroupParser类等相关内容，用于处理交通组解析逻辑
#include "carla/opendrive/parser/TrafficGroupParser.h"
// 包含用于构建地图相关的头文件，可能提供了创建、添加地图元素等操作的接口和类定义，例如这里可能用于向地图中添加交通组信息等功能
#include "carla/road/MapBuilder.h"
// 包含pugixml库的头文件，pugixml是一个用于解析XML文件的C++库，这里用于解析包含交通组信息的XML格式数据
#include <pugixml/pugixml.hpp>
// 定义在carla命名空间下的opendrive命名空间里的parser命名空间中，这样的命名空间嵌套有助于组织代码结构，避免命名冲突
namespace carla {
namespace opendrive {
namespace parser {

  /*
    <userData>
      <trafficGroup id="137" type="trafficGroup" redTime="10" yellowTime="5" greenTime="5"/>
      <trafficGroup id="138" type="trafficGroup" redTime="10" yellowTime="5" greenTime="5"/>
      <trafficGroup id="139" type="trafficGroup" redTime="10" yellowTime="5" greenTime="5"/>
      <trafficGroup id="140" type="trafficGroup" redTime="10" yellowTime="5" greenTime="5"/>
      <trafficGroup id="141" type="trafficGroup" redTime="10" yellowTime="5" greenTime="5"/>
    </userData>
  */

  void TrafficGroupParser::Parse(
      const pugi::xml_document & /*xml*/,
      carla::road::MapBuilder & /* map_builder */) {
    // pugi::xml_node opendrive_node = xml.child("OpenDRIVE");
    // for (pugi::xml_node userdata_node = opendrive_node.child("userData");
    //     userdata_node;
    //     userdata_node = userdata_node.next_sibling("userData")) {
    //   for (pugi::xml_node trafficgroup_node = userdata_node.child("trafficGroup");
    //       trafficgroup_node;
    //       trafficgroup_node = trafficgroup_node.next_sibling("trafficGroup")) {
    //     uint16_t id = trafficgroup_node.attribute("id").as_int(0);
    //     uint16_t redTime = trafficgroup_node.attribute("redTime").as_int(0);
    //     uint16_t yellowTime = trafficgroup_node.attribute("yellowTime").as_int(0);
    //     uint16_t greenTime = trafficgroup_node.attribute("greenTime").as_int(0);
    //     log_debug("Found TrafficGroup with ID: ",
    //         id,
    //         "Times (",
    //         redTime,
    //         ", ",
    //         yellowTime,
    //         ", ",
    //         greenTime,
    //         ")");
    //   }
    //   map_builder.AddTrafficGroup(id, redTime, yellowTime, greenTime);
    // }
  }

} // namespace parser
} // namespace opendrive
} // namespace carla
