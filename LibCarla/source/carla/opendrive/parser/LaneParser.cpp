// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// 车道解析器
// 
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/opendrive/parser/LaneParser.h"

#include "carla/road/MapBuilder.h"

#include <pugixml/pugixml.hpp>

namespace carla {
namespace opendrive {
namespace parser {

  // 解析车道
  static void ParseLanes(
      road::RoadId road_id,
      double s,
      const pugi::xml_node &parent_node,
      carla::road::MapBuilder &map_builder) {
    for (pugi::xml_node lane_node : parent_node.children("lane")) {

      road::LaneId lane_id = lane_node.attribute("id").as_int();

      road::Lane *lane = map_builder.GetLane(road_id, lane_id, s);

      // 车道宽度
      int width_count = 0;
      for (pugi::xml_node lane_width_node : lane_node.children("width")) {
        const double s_offset = lane_width_node.attribute("sOffset").as_double();
        const double a = lane_width_node.attribute("a").as_double();
        const double b = lane_width_node.attribute("b").as_double();
        const double c = lane_width_node.attribute("c").as_double();
        const double d = lane_width_node.attribute("d").as_double();

        // 调用地图构建器创建车道宽度函数
        map_builder.CreateLaneWidth(lane, s_offset + s, a, b, c, d);
        width_count++;
      }
      if (width_count == 0 && lane->GetId() != 0) {
        map_builder.CreateLaneWidth(lane, s, 0.0, 0.0, 0.0, 0.0);
        std::cout << "WARNING: In road " << lane->GetRoad()->GetId() << " lane " << lane->GetId() <<
        " no \"<width>\" parameter found under \"<lane>\" tag. Using default values." << std::endl;
      }

      // 车道边界
      for (pugi::xml_node lane_border_node : lane_node.children("border")) {
        const double s_offset = lane_border_node.attribute("sOffset").as_double();
        const double a = lane_border_node.attribute("a").as_double();
        const double b = lane_border_node.attribute("b").as_double();
        const double c = lane_border_node.attribute("c").as_double();
        const double d = lane_border_node.attribute("d").as_double();

        // 调用地图构建器创建车道边界函数
        map_builder.CreateLaneBorder(lane, s_offset + s, a, b, c, d);
      }

      // Lane Road Mark
      // 在OpenDrive中，<roadMark>元素定义了车道外边界的车道线样式，即左边车道就定义其左边界，右边车道就定义其右边界。
      int road_mark_id = 0;
      for (pugi::xml_node lane_road_mark : lane_node.children("roadMark")) {
        pugi::xml_node road_mark_type;
        {
          const double s_offset = lane_road_mark.attribute("sOffset").as_double();
          const std::string type = lane_road_mark.attribute("type").value();
          const std::string weight = lane_road_mark.attribute("weight").value();
          const std::string color = lane_road_mark.attribute("color").value();
          const std::string material = lane_road_mark.attribute("material").value();
          const double width = lane_road_mark.attribute("width").as_double();
          const std::string lane_change = lane_road_mark.attribute("laneChange").value();
          const double height = lane_road_mark.attribute("height").as_double();

          // 为 LaneRoadMarkType 调用地图构建器

          std::string type_name = "";
          double type_width = 0.0;
          road_mark_type = lane_road_mark.child("type");
          if (road_mark_type) {
            type_name = road_mark_type.attribute("name").value();
            type_width = road_mark_type.attribute("width").as_double();
          }

          // 为 LaneRoadMark 调用地图构建器
          map_builder.CreateRoadMark(
              lane,
              road_mark_id,
              s_offset + s,
              type,
              weight,
              color,
              material,
              width,
              lane_change,
              height,
              type_name,
              type_width);
        }

        for (pugi::xml_node road_mark_type_line_node : road_mark_type.children("line")) {

          const double length = road_mark_type_line_node.attribute("length").as_double();
          const double space = road_mark_type_line_node.attribute("space").as_double();
          const double t = road_mark_type_line_node.attribute("tOffset").as_double();
          const double s_offset = road_mark_type_line_node.attribute("sOffset").as_double();
          const std::string rule = road_mark_type_line_node.attribute("rule").value();
          const double width = road_mark_type_line_node.attribute("width").as_double();

          // 为 LaneRoadMarkType LaneRoadMarkTypeLine 调用地图构建器 
          map_builder.CreateRoadMarkTypeLine(
              lane,
              road_mark_id,
              length,
              space,
              t,
              s_offset + s,
              rule,
              width);
        }
        ++road_mark_id;
      }

      // 车道材质
      for (pugi::xml_node lane_material_node : lane_node.children("material")) {

        const double s_offset = lane_material_node.attribute("sOffset").as_double();
        const std::string surface = lane_material_node.attribute("surface").value();
        const double friction = lane_material_node.attribute("friction").as_double();
        const double roughness = lane_material_node.attribute("roughness").as_double();

        // 为车道材质 Lane Material 创建地图构建器
        map_builder.CreateLaneMaterial(lane, s_offset + s, surface, friction, roughness);
      }

      // 车道可见性
      for (pugi::xml_node lane_visibility_node : lane_node.children("visibility")) {
        const double s_offset = lane_visibility_node.attribute("sOffset").as_double();
        const double forward = lane_visibility_node.attribute("forward").as_double();
        const double back = lane_visibility_node.attribute("back").as_double();
        const double left = lane_visibility_node.attribute("left").as_double();
        const double right = lane_visibility_node.attribute("right").as_double();

        // 为车道可见性 Lane Visibility 创建地图构建器
        map_builder.CreateLaneVisibility(lane, s_offset + s, forward, back, left, right);
      }

      // Lane Speed
      // 单独车道可以拥有不同于所属道路的速度限制，其将被定义为<laneSpeed>。
      for (pugi::xml_node lane_speed_node : lane_node.children("speed")) {
        const double s_offset = lane_speed_node.attribute("sOffset").as_double();
        const double max = lane_speed_node.attribute("max").as_double();
        std::string unit = lane_speed_node.attribute("unit").value();

        // 为车道速度限制 Lane Speed 创建地图构建器
        map_builder.CreateLaneSpeed(lane, s_offset + s, max, unit);
      }

      // Lane Access
      // 在 <lane> 元素内提供了 <access> 元素,以便描述车道使用规则
      for (pugi::xml_node lane_access_node : lane_node.children("access")) {
        const double s_offset = lane_access_node.attribute("sOffset").as_double();
        const std::string restriction = lane_access_node.attribute("restriction").value();

        // 为车道使用 Lane Access 创建地图构建器
        map_builder.CreateLaneAccess(lane, s_offset + s, restriction);
      }

      // Lane Height
      for (pugi::xml_node lane_height_node : lane_node.children("height")) {
        const double s_offset = lane_height_node.attribute("sOffset").as_double();
        const double inner = lane_height_node.attribute("inner").as_double();
        const double outer = lane_height_node.attribute("outer").as_double();

        // Create map builder for Lane Height
        map_builder.CreateLaneHeight(lane, s_offset + s, inner, outer);
      }

      // Lane Rule
      for (pugi::xml_node lane_rule_node : lane_node.children("rule")) {
        const double s_offset = lane_rule_node.attribute("sOffset").as_double();
        const std::string value = lane_rule_node.attribute("value").value();

        // Create map builder for Lane Height
        map_builder.CreateLaneRule(lane, s_offset + s, value);
      }

    }
  }

  void LaneParser::Parse(
      const pugi::xml_document &xml,
      carla::road::MapBuilder &map_builder) {

    pugi::xml_node open_drive_node = xml.child("OpenDRIVE"); // 获取OpenDRIVE根节点

    // 车道
    for (pugi::xml_node road_node : open_drive_node.children("road")) { // 遍历每个道路节点
      road::RoadId road_id = road_node.attribute("id").as_uint(); // 获取道路ID

      for (pugi::xml_node lanes_node : road_node.children("lanes")) { // 遍历每个车道节点

        for (pugi::xml_node lane_section_node : lanes_node.children("laneSection")) { // 遍历每个车道段节点
          double s = lane_section_node.attribute("s").as_double(); // 获取车道段的s属性，表示沿道路的距离
          pugi::xml_node left_node = lane_section_node.child("left"); // 解析左侧车道
          if (left_node) {
            ParseLanes(road_id, s, left_node, map_builder);
          }

          pugi::xml_node center_node = lane_section_node.child("center"); // 解析中间车道
          if (center_node) {
            ParseLanes(road_id, s, center_node, map_builder);
          }

          pugi::xml_node right_node = lane_section_node.child("right"); // 解析右侧车道
          if (right_node) {
            ParseLanes(road_id, s, right_node, map_builder);
          }
        }
      }
    }
  }

} // namespace parser
} // namespace opendrive
} // namespace carla
