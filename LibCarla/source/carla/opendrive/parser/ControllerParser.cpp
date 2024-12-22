// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/opendrive/parser/ControllerParser.h"  // 引入 ControllerParser 的头文件

#include "carla/road/MapBuilder.h"  // 引入 MapBuilder 的头文件

#include <pugixml/pugixml.hpp>  // 引入 pugixml 库的头文件

namespace carla {
namespace opendrive {
namespace parser {

  void ControllerParser::Parse(  // 定义解析函数
      const pugi::xml_document &xml,  // 输入的 XML 文档
      carla::road::MapBuilder &map_builder) {  // 地图构建器

    // 提取 OpenDRIVE
    const pugi::xml_node opendrive_node = xml.child("OpenDRIVE");  // 获取 OpenDRIVE 节点
    for (pugi::xml_node controller_node = opendrive_node.child("controller");  // 遍历 controller 节点
        controller_node;  // 如果 controller_node 存在
        controller_node = controller_node.next_sibling("controller")) {  // 获取下一个 sibling 节点

      const road::ContId controller_id = controller_node.attribute("id").value();  // 获取控制器 ID
      const std::string controller_name = controller_node.attribute("name").value();  // 获取控制器名称
      const uint32_t controller_sequence = controller_node.attribute("sequence").as_uint();  // 获取控制器序列号

      log_debug("Controller: ",  // 日志输出控制器信息
        controller_id,
        controller_name,
        controller_sequence);

      std::set<road::SignId> signals;  // 创建信号 ID 集合

      for (pugi::xml_node control_node : controller_node.children("control")) {  // 遍历控制节点
        const road::SignId signal_id = control_node.attribute("signalId").value();  // 获取信号 ID
        // controller_type 包含在 OpenDrive 格式中，但尚未使用
        // 获取控制器类型（尚未使用）
        signals.insert(signal_id);  // 将信号 ID 插入集合
      }

      map_builder.CreateController(  // 在地图构建器中创建控制器
        controller_id,
        controller_name,
        controller_sequence,
        std::move(signals)  // 移动信号集合
      );
    }

  }

} // namespace parser
} // namespace opendrive
} // namespace carla
