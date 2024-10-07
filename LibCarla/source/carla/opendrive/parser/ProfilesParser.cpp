// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/opendrive/parser/ProfilesParser.h"
// 包含Carla的OpenDRIVE解析器中的ProfilesParser头文件，用于解析道路剖面相关信息

#include "carla/road/MapBuilder.h"
// 包含Carla中构建地图的MapBuilder头文件

#include <pugixml/pugixml.hpp>
// 包含pugixml库的头文件，用于解析XML文件


namespace carla {
namespace opendrive {
namespace parser {


  struct ElevationProfile {
    carla::road::Road *road { nullptr };
    // 指向carla::road::Road类型的指针，用于表示与高程剖面相关的道路，初始化为nullptr
    double s { 0.0 };
    // 沿着道路的距离，初始化为0.0
    double a { 0.0 };
    // 多项式系数a，初始化为0.0
    double b { 0.0 };
    // 多项式系数b，初始化为0.0
    double c { 0.0 };
    // 多项式系数c，初始化为0.0
    double d { 0.0 };
    // 多项式系数d，初始化为0.0
  };


  struct LateralCrossfall {
    std::string side { "both" };
    // 表示横向横坡的侧边，初始化为"both"
  };


  struct LateralShape {
    double t { 0.0 };
    // 横向形状相关的变量t，初始化为0.0
  };


  struct LateralProfile {
    carla::road::Road *road { nullptr };
    // 指向carla::road::Road类型的指针，用于表示与横向剖面相关的道路，初始化为nullptr
    double s { 0.0 };
    // 沿着道路的距离，初始化为0.0
    double a { 0.0 };
    // 多项式系数a，初始化为0.0
    double b { 0.0 };
    // 多项式系数b，初始化为0.0
    double c { 0.0 };
    // 多项式系数c，初始化为0.0
    double d { 0.0 };
    // 多项式系数d，初始化为0.0
    std::string type { "superelevation" };
    // 横向剖面的类型，初始化为"superelevation"
    LateralCrossfall cross;
    // 横向横坡信息
    LateralShape shape;
    // 横向形状信息
  };


  void ProfilesParser::Parse(
      const pugi::xml_document &xml,
      carla::road::MapBuilder &map_builder) {
    // ProfilesParser类中的Parse函数，用于解析XML中的道路剖面信息并构建地图相关内容
    // 输入为一个pugi::xml_document类型的XML文档对象和一个carla::road::MapBuilder类型的地图构建器对象


    std::vector<ElevationProfile> elevation_profile;
    // 用于存储高程剖面信息的向量
    std::vector<LateralProfile> lateral_profile;
    // 用于存储横向剖面信息的向量


    for (pugi::xml_node node_road : xml.child("OpenDRIVE").children("road")) {
      // 遍历XML文档中<OpenDRIVE>下的所有<road>节点
      // 每个<road>节点代表一条道路


      // 解析高程剖面
      pugi::xml_node node_profile = node_road.child("elevationProfile");
      // 获取当前<road>节点下的<elevationProfile>子节点，该节点包含高程剖面信息
      uint64_t number_profiles = 0;
      // 用于统计高程剖面的数量，初始化为0


      if (node_profile) {
        // 如果存在<elevationProfile>节点


        // 所有的几何
        for (pugi::xml_node node_elevation : node_profile.children("elevation")) {
          // 遍历<elevationProfile>节点下的所有<elevation>子节点，每个<elevation>节点包含一个高程段的信息
          ElevationProfile elev;


          // 获取道路 id
          road::RoadId road_id = node_road.attribute("id").as_uint();
          // 从<road>节点的"id"属性获取道路的ID
          elev.road = map_builder.GetRoad(road_id);
          // 通过地图构建器根据道路ID获取对应的道路对象，并赋值给elev.road


          // 获取常用属性
          elev.s = node_elevation.attribute("s").as_double();
          // 从<elevation>节点的"s"属性获取沿着道路的距离
          elev.a = node_elevation.attribute("a").as_double();
          // 从<elevation>节点的"a"属性获取多项式系数a
          elev.b = node_elevation.attribute("b").as_double();
          // 从<elevation>节点的"b"属性获取多项式系数b
          elev.c = node_elevation.attribute("c").as_double();
          // 从<elevation>节点的"c"属性获取多项式系数c
          elev.d = node_elevation.attribute("d").as_double();
          // 从<elevation>节点的"d"属性获取多项式系数d


          // 添加它
          elevation_profile.emplace_back(elev);
          // 将解析得到的高程剖面信息添加到elevation_profile向量中
          number_profiles++;
          // 高程剖面数量加1
        }
      }
      // 如果没有找到，则添加默认配置
      if (number_profiles == 0) {
        ElevationProfile elev;
        road::RoadId road_id = node_road.attribute("id").as_uint();
        elev.road = map_builder.GetRoad(road_id);


        // 获取常用属性
        elev.s = 0;
        // 沿着道路的距离设置为0
        elev.a = 0;
        // 多项式系数a设置为0
        elev.b = 0;
        // 多项式系数b设置为0
        elev.c = 0;
        // 多项式系数c设置为0
        elev.d = 0;
        // 多项式系数d设置为0


        // 添加它
        elevation_profile.emplace_back(elev);
        // 将默认的高程剖面信息添加到elevation_profile向量中
      }


      // 解析横向剖面
      node_profile = node_road.child("lateralProfile");
      // 获取当前<road>节点下的<lateralProfile>子节点，该节点包含横向剖面信息
      if (node_profile) {
        for (pugi::xml_node node : node_profile.children()) {
          // 遍历<lateralProfile>节点下的所有子节点


          LateralProfile lateral;


          // 获取路 id
          road::RoadId road_id = node_road.attribute("id").as_uint();
          // 从<road>节点的"id"属性获取道路的ID
          lateral.road = map_builder.GetRoad(road_id);
          // 通过地图构建器根据道路ID获取对应的道路对象，并赋值给lateral.road


          // 获取常用属性
          lateral.s = node.attribute("s").as_double();
          // 从子节点的"s"属性获取沿着道路的距离
          lateral.a = node.attribute("a").as_double();
          // 从子节点的"a"属性获取多项式系数a
          lateral.b = node.attribute("b").as_double();
          // 从子节点的"b"属性获取多项式系数b
          lateral.c = node.attribute("c").as_double();
          // 从子节点的"c"属性获取多项式系数c
          lateral.d = node.attribute("d").as_double();
          // 从子节点的"d"属性获取多项式系数d


          // 处理不同类型
          lateral.type = node.name();
          // 将子节点的名称作为横向剖面的类型
          if (lateral.type == "crossfall") {
            lateral.cross.side = node.attribute("side").value();
            // 如果类型是"crossfall"，从子节点的"side"属性获取横向横坡的侧边信息并赋值给lateral.cross.side
          } else if (lateral.type == "shape") {
            lateral.shape.t = node.attribute("t").as_double();
            // 如果类型是"shape"，从子节点的"t"属性获取横向形状相关的变量t的值并赋值给lateral.shape.t
          }


          // 添加它
          lateral_profile.emplace_back(lateral);
          // 将解析得到的横向剖面信息添加到lateral_profile向量中
        }
      }
    }


    // 调用地图构建器map_builder
    for (auto const pro : elevation_profile) {
      map_builder.AddRoadElevationProfile(pro.road, pro.s, pro.a, pro.b, pro.c, pro.d);
      // 遍历elevation_profile向量，调用地图构建器的AddRoadElevationProfile函数，将高程剖面信息添加到地图构建器中
    }


    /// @todo: RoadInfo classes must be created to fit this information
    // for (auto const pro : lateral_profile) {
    //   if (pro.type == "superelevation")
    //     map_builder.AddRoadLateralSuperElevation(pro.road, pro.s, pro.a, pro.b, pro.c, pro.d);
    //   else if (pro.type == "crossfall")
    //     map_builder.AddRoadLateralCrossfall(pro.road, pro.s, pro.a, pro.b, pro.c, pro.d, pro.cross.side);
    //   else if (pro.type == "shape")
    //     map_builder.AddRoadLateralShape(pro.road, pro.s, pro.a, pro.b, pro.c, pro.d, pro.shape.t);
    // }


  }


} // namespace parser
} // namespace opendrive
} // namespace carla