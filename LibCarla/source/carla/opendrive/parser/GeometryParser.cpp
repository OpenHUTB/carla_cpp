// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// 参考：https://www.cnblogs.com/aixing/p/16225067.html
// 
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/opendrive/parser/GeometryParser.h"

#include "carla/road/MapBuilder.h"

#include <pugixml/pugixml.hpp>

namespace carla {
namespace opendrive {
namespace parser {

  using RoadId = road::RoadId;

  // 弧线描述了有着恒定曲率(curvature)的道路参考线。
  // 在OpenDRIVE中，弧线用<geometry> 元素里的<arc>元素来表示。
  // 正曲率：左曲线（逆时针运动）
  // 负曲率：右曲线（顺时针运动）
  struct GeometryArc {
    double curvature  { 0.0 }; // 贯穿元素的恒定曲率（单位：1/m）
  };

  // 螺旋线是以起始位置的曲率(@curvStart)和结束位置的曲率(@curvEnd)为特征。
  // 沿着螺旋线的弧形长度（见 <geometry> 元素@length），曲率从头至尾呈线性。
  struct GeometrySpiral {
    double curvStart  { 0.0 };  // 起始位置的曲率
    double curvEnd    { 0.0 };  // 结束位置的曲率
  };

  // 局部三次多项式表达式：v(u) = a + b*u + c*u^2 + d*u^3
  // 在OpenDRIVE中，三次多项式用 <geometry> 元素里的 <poly3> 元素来表示。
  struct GeometryPoly3 {
    double a    { 0.0 };
    double b    { 0.0 };
    double c    { 0.0 };
    double d    { 0.0 };
  };

  // 参数三次曲线表达式：
  // u(p) = aU + bU*p + cU*p^2 + dU*p^3
  // v(p) = aV + bV * p + cV * p^2 + dV * p^3
  // 在OpenDRIVE中，参数三次曲线用 <geometry> 元素里的 <paramPoly3> 元素来表示。
  struct GeometryParamPoly3 {
    double aU             { 0.0 };
    double bU             { 0.0 };
    double cU             { 0.0 };
    double dU             { 0.0 };
    double aV             { 0.0 };
    double bV             { 0.0 };
    double cV             { 0.0 };
    double dV             { 0.0 };
    std::string p_range   { "arcLength" };
  };

  struct Geometry {
    RoadId road_id      { 0u };
    double s            { 0.0 };
    double x            { 0.0 };
    double y            { 0.0 };
    double hdg          { 0.0 };
    double length       { 0.0 };
    std::string type    { "line" };
    GeometryArc arc;
    GeometrySpiral spiral;
    GeometryPoly3 poly3;
    GeometryParamPoly3 param_poly3;
  };

  // 几何构造解析器
  void GeometryParser::Parse(
      const pugi::xml_document &xml,
      carla::road::MapBuilder &map_builder) {

    std::vector<Geometry> geometry;

    for (pugi::xml_node node_road : xml.child("OpenDRIVE").children("road")) {

      // 解析规划视图
      pugi::xml_node node_plan_view = node_road.child("planView");
      if (node_plan_view) {
        // 所有的几何构造
        for (pugi::xml_node node_geo : node_plan_view.children("geometry")) {
          Geometry geo;

          // 获取路的 id
          geo.road_id = node_road.attribute("id").as_uint();

          // 获取常用属性：geometry 标签中共有 5 个属性：https://cniter.github.io/posts/b7d79231.html
          geo.s = node_geo.attribute("s").as_double();  // 该段 geometry 沿参考线起始(start)位置
          geo.x = node_geo.attribute("x").as_double();  // 该段 geometry 在惯性坐标系下起始横坐标
          geo.y = node_geo.attribute("y").as_double();  // 该段 geometry 在惯性坐标系下起始纵坐标
          geo.hdg = node_geo.attribute("hdg").as_double();  // 该段 geometry 在惯性坐标系下起始弧度
          geo.length = node_geo.attribute("length").as_double(); // 该段 geometry 长度

          // 检查几何构造的类型：直线、螺旋线、圆弧线、三次曲线、参数化三次曲线
          pugi::xml_node node = node_geo.first_child();
          geo.type = node.name();
          if (geo.type == "arc") { // 圆弧线 arcs
            geo.arc.curvature = node.attribute("curvature").as_double();
          } else if (geo.type == "spiral") { // 螺旋线 spirals
            geo.spiral.curvStart = node.attribute("curvStart").as_double();
            geo.spiral.curvEnd = node.attribute("curvEnd").as_double();
          } else if (geo.type == "poly3") {  // 三次曲线 cubic polynomials
            geo.poly3.a = node.attribute("a").as_double();
            geo.poly3.b = node.attribute("b").as_double();
            geo.poly3.c = node.attribute("c").as_double();
            geo.poly3.d = node.attribute("d").as_double();
          } else if (geo.type == "paramPoly3") {  // 参数化三次曲线 parametric cubic polynomials 
            geo.param_poly3.aU = node.attribute("aU").as_double();
            geo.param_poly3.bU = node.attribute("bU").as_double();
            geo.param_poly3.cU = node.attribute("cU").as_double();
            geo.param_poly3.dU = node.attribute("dU").as_double();
            geo.param_poly3.aV = node.attribute("aV").as_double();
            geo.param_poly3.bV = node.attribute("bV").as_double();
            geo.param_poly3.cV = node.attribute("cV").as_double();
            geo.param_poly3.dV = node.attribute("dV").as_double();
            geo.param_poly3.p_range = node.attribute("pRange").value();
          }

          // add it
          geometry.emplace_back(geo);
        }
      }
    }

    // map_builder calls
    for (auto const geo : geometry) {
      carla::road::Road *road = map_builder.GetRoad(geo.road_id);
      if (geo.type == "line") {
        map_builder.AddRoadGeometryLine(road, geo.s, geo.x, geo.y, geo.hdg, geo.length);
      } else if (geo.type == "arc") {
        map_builder.AddRoadGeometryArc(road, geo.s, geo.x, geo.y, geo.hdg, geo.length, geo.arc.curvature);
      } else if (geo.type == "spiral") {
        map_builder.AddRoadGeometrySpiral(road,
            geo.s,
            geo.x,
            geo.y,
            geo.hdg,
            geo.length,
            geo.spiral.curvStart,
            geo.spiral.curvEnd);
      } else if (geo.type == "poly3") {
        map_builder.AddRoadGeometryPoly3(road,
            geo.s,
            geo.x,
            geo.y,
            geo.hdg,
            geo.length,
            geo.poly3.a,
            geo.poly3.b,
            geo.poly3.c,
            geo.poly3.d);
      } else if (geo.type == "paramPoly3") {
        map_builder.AddRoadGeometryParamPoly3(road,
            geo.s,
            geo.x,
            geo.y,
            geo.hdg,
            geo.length,
            geo.param_poly3.aU,
            geo.param_poly3.bU,
            geo.param_poly3.cU,
            geo.param_poly3.dU,
            geo.param_poly3.aV,
            geo.param_poly3.bV,
            geo.param_poly3.cV,
            geo.param_poly3.dV,
            geo.param_poly3.p_range);
      }
    }
  }

} // namespace parser
} // namespace opendrive
} // namespace carla
