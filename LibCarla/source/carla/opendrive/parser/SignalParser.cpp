// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
// 包含Carla项目中OpenDRIVE相关的信号解析器（SignalParser）头文件，推测其中定义了SignalParser类的声明等内容，用于解析OpenDRIVE格式中的信号相关信息
#include "carla/opendrive/parser/SignalParser.h"// 包含Carla项目中道路地图构建器（MapBuilder）相关的头文件，用于构建道路地图，可能涉及创建道路、添加各种道路元素等操作

#include "carla/road/MapBuilder.h"
// 包含pugixml库的头文件，pugixml是一个用于解析XML文件的C++库，在这里用于解析OpenDRIVE格式对应的XML文件内容
#include <pugixml/pugixml.hpp>
// 定义在Carla项目的opendrive命名空间下的parser命名空间中，表明这些函数和类是用于OpenDRIVE解析相关的具体实现部分
namespace carla {
namespace opendrive {
namespace parser {
// 定义一个名为AddValidity的静态函数，用于为信号引用添加有效性范围信息
  static void AddValidity(
    road::element::RoadInfoSignal* signal_reference,
    pugi::xml_node parent_node,
    const std::string &node_name,
    road::MapBuilder &map_builder) {
    	// 遍历父节点下所有名为"validity"的子节点
    for (pugi::xml_node validity_node = parent_node.child(node_name.c_str());
        validity_node;
        validity_node = validity_node.next_sibling("validity")) {
        	// 获取有效性范围的起始车道和结束车道
      const auto from_lane = validity_node.attribute("fromLane").as_int();
      const auto to_lane = validity_node.attribute("toLane").as_int();
      // 将有效性范围添加到信号引用中
      map_builder.AddValidityToSignalReference(signal_reference, from_lane, to_lane);
    }
  }

  void SignalParser::Parse(
      const pugi::xml_document &xml,
      carla::road::MapBuilder &map_builder) {

    // 提取 OpenDRIVE
    const pugi::xml_node opendrive_node = xml.child("OpenDRIVE");
    const std::string validity = "validity";
    // 遍历OpenDRIVE节点下的所有道路节点
    for (pugi::xml_node road_node = opendrive_node.child("road");
        road_node;
        road_node = road_node.next_sibling("road")) {
// 获取道路ID
      road::RoadId road_id = road_node.attribute("id").as_uint();
// 获取道路节点下的信号节点
      const pugi::xml_node signals_node = road_node.child("signals");
      if(signals_node){
      	 // 遍历信号节点下的所有信号元素
        for (pugi::xml_node signal_node : signals_node.children("signal")) {
        	 // 获取信号的各种属性
          const double s_position = signal_node.attribute("s").as_double();// 获取信号在道路上的横向位置（t坐标），从"signal"子节点的属性"t"中获取双精度浮点数值
          const double t_position = signal_node.attribute("t").as_double();// 获取信号的标识符（SignId），从"signal"子节点的属性"id"中获取对应的值，具体类型由road::SignId定义
          const road::SignId signal_id = signal_node.attribute("id").value();// 获取信号的名称，从"signal"子节点的属性"name"中获取对应的值，类型为std::string
          const std::string name = signal_node.attribute("name").value();// 获取信号是否动态的标识，从"signal"子节点的属性"dynamic"中获取对应的值，类型为std::string
          const std::string dynamic =  signal_node.attribute("dynamic").value();// 获取信号的朝向信息，从"signal"子节点的属性"orientation"中获取对应的值，类型为std::string
          const std::string orientation =  signal_node.attribute("orientation").value();// 获取信号的垂直偏移量（z轴偏移），从"signal"子节点的属性"zOffSet"中获取双精度浮点数值
          const double zOffset = signal_node.attribute("zOffSet").as_double();// 获取信号所属的国家信息，从"signal"子节点的属性"country"中获取对应的值，类型为std::string
          const std::string country =  signal_node.attribute("country").value();// 获取信号的类型信息，从"signal"子节点的属性"type"中获取对应的值，类型为std::string
          const std::string type =  signal_node.attribute("type").value();// 获取信号的子类型信息，从"signal"子节点的属性"subtype"中获取对应的值，类型为std::string
          const std::string subtype =  signal_node.attribute("subtype").value();// 获取信号的值，从"signal"子节点的属性"value"中获取双精度浮点数值
          const double value = signal_node.attribute("value").as_double();// 获取信号的单位信息，从"signal"子节点的属性"unit"中获取对应的值，类型为std::string
          const std::string unit =  signal_node.attribute("unit").value();// 获取信号的高度信息，从"signal"子节点的属性"height"中获取双精度浮点数值
          const double height = signal_node.attribute("height").as_double(); // 获取信号的宽度信息，从"signal"子节点的属性"width"中获取双精度浮点数值
          const double width = signal_node.attribute("width").as_double();// 获取信号显示的文本信息，从"signal"子节点的属性"text"中获取对应的值，类型为std::string
          const std::string text =  signal_node.attribute("text").value();// 获取信号的水平偏移量（h轴偏移），从"signal"子节点的属性"hOffset"中获取双精度浮点数值
          const double hOffset = signal_node.attribute("hOffset").as_double();// 获取信号的俯仰角信息，从"signal"子节点的属性"pitch"中获取双精度浮点数值
          const double pitch = signal_node.attribute("pitch").as_double(); // 获取信号的翻滚角信息，从"signal"子节点的attribute("roll")中获取双精度浮点数值
          const double roll = signal_node.attribute("roll").as_double(); // 输出调试信息，显示正在添加的信号的各种属性信息，方便调试时查看解析到的信号具体情况
         
          log_debug("Road: ",
              road_id,
              "Adding Signal: ",
              s_position,
              t_position,
              signal_id,
              name,
              dynamic,
              orientation,
              zOffset,
              country,
              type,
              subtype,
              value,
              unit,
              height,
              width,
              text,
              hOffset,
              pitch,
              roll);
// 通过地图构建器根据道路ID获取对应的道路对象指针，后续操作需要基于此道路对象来添加信号等元素
          carla::road::Road *road = map_builder.GetRoad(road_id);
        // 使用地图构建器添加信号，传入道路对象指针以及信号的各种属性信息，返回添加后的信号引用对象，
                    // 后续可以基于此信号引用对象进行更多相关操作，比如添加有效性范围等
          auto signal_reference = map_builder.AddSignal(road,
              signal_id,
              s_position,
              t_position,
              name,
              dynamic,
              orientation,
              zOffset,
              country,
              type,
              subtype,
              value,
              unit,
              height,
              width,
              text,
              hOffset,
              pitch,
              roll);
             // 调用AddValidity函数，为刚添加的信号引用对象添加有效性范围信息，传入信号引用对象、当前"signal"子节点（作为父节点）以及有效性范围节点名称等信息
          AddValidity(signal_reference, signal_node, "validity", map_builder);
// 处理信号的依赖节点
          for (pugi::xml_node dependency_node : signal_node.children("dependency")) {
            const std::string dependency_id = dependency_node.attribute("id").value();
            const std::string dependency_type = dependency_node.attribute("type").value();
            // 打印调试信息，显示为信号添加的依赖
            log_debug("Added dependency to signal ", signal_id, ":", dependency_id, dependency_type);
            // 将依赖添加到信号中
            map_builder.AddDependencyToSignal(signal_id, dependency_id, dependency_type);
          }
          // 处理信号的惯性位置节点
          for (pugi::xml_node position_node : signal_node.children("positionInertial")) {
            const double x = position_node.attribute("x").as_double(); // 获取惯性位置的y坐标值，从"positionInertial"子节点的属性"y"中获取双精度浮点数值
            const double y = position_node.attribute("y").as_double();// 获取惯性位置的z坐标值，从"positionInertial"子节点的属性"z"中获取双精度浮点数值
            const double z = position_node.attribute("z").as_double();// 获取惯性位置的航向角（heading）值，从"positionInertial"子节点的属性"hdg"中获取双精度浮点数值
            const double hdg = position_node.attribute("hdg").as_double();// 获取惯性位置的俯仰角（pitch）值，从"positionInertial"子节点的属性"pitch"中获取双精度浮点数值
            const double inertial_pitch = position_node.attribute("pitch").as_double();// 获取惯性位置的翻滚角（roll）值，从"positionInertial"子节点的attribute("roll")中获取双精度浮点数值
            const double inertial_roll = position_node.attribute("roll").as_double();// 通过地图构建器将获取到的信号惯性位置信息添加到对应的信号中，传入信号标识符以及惯性位置的各个坐标和角度信息
       
            map_builder.AddSignalPositionInertial(
                signal_id,
                x, y, z,
                hdg, inertial_pitch, inertial_roll);
          }
        }
      // 处理信号引用节点，遍历"signals"节点下所有名为"signalReference"的子节点，每个子节点代表一个信号引用的相关信息
        for (pugi::xml_node signal_reference_node : signals_node.children("signalReference")) {// 获取信号引用在道路上的纵向位置（s坐标），从"signalReference"子节点的属性"s"中获取双精度浮点数值
          const double s_position = signal_reference_node.attribute("s").as_double();// 获取信号引用在道路上的横向位置（t坐标），从"signalReference"子节点的属性"t"中获取双精度浮点数值
          const double t_position = signal_reference_node.attribute("t").as_double();// 获取信号引用的标识符（SignId），从"signalReference"子节点的属性"id"中获取对应的值，具体类型由road::SignId定义
          const road::SignId signal_id = signal_reference_node.attribute("id").value();// 获取信号引用的朝向信息，从"signalReference"子节点的属性"orientation"中获取对应的值，类型为std::string
          const std::string signal_reference_orientation =
              signal_reference_node.attribute("orientation").value();
             // 输出调试信息，显示正在添加的信号引用的相关信息，方便调试时查看信号引用情况
          log_debug("Road: ",
              road_id,
              "Added SignalReference ",
              s_position,
              t_position,
              signal_reference_orientation);
               // 通过地图构建器根据道路ID获取对应的道路对象指针，后续操作需要基于此道路对象来添加信号引用等元素
          carla::road::Road *road = map_builder.GetRoad(road_id);
        // 使用地图构建器添加信号引用，传入道路对象指针以及信号引用的各种属性信息，返回添加后的信号引用对象，
                    // 后续可以基于此信号引用对象进行更多相关操作，比如添加有效性范围等
          auto signal_reference = map_builder.AddSignalReference(
              road,
              signal_id,
              s_position,
              t_position,
              signal_reference_orientation);
          // 调用AddValidity函数，为刚添加的信号引用对象添加有效性范围信息，传入信号引用对象、当前"signalReference"子节点（作为父节点）以及有效性范围节点名称等信息
          AddValidity(signal_reference, signal_reference_node, "validity", map_builder);
        }
      }
    }
  }
} // namespace parser
} // namespace opendrive
} // namespace carla
