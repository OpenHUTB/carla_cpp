// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"

namespace carla {
namespace rpc {

  /// Seting for map generation from opendrive without additional geometry
  struct OpendriveGenerationParameters {
    OpendriveGenerationParameters(){}
    OpendriveGenerationParameters(
// 这是一个构造函数的定义部分，可能是某个类的构造函数（从代码片段上下文推测），接受多个参数来初始化类中的成员变量
// 参数 v_distance 类型为 double，可能用于表示顶点距离相关的数值（具体含义需结合完整代码上下文确定）
// 参数 max_road_len 类型为 double，可能与道路最大长度相关（具体含义依完整项目而定）
// 参数 w_height 类型为 double，推测和墙的高度相关（具体用途结合整体来看）
// 参数 a_width 类型为 double，也许是额外宽度相关的数值（确切含义要参考更多代码）
// 参数 smooth_junc 类型为 bool，可能用于控制是否平滑路口（只是推测，具体功能看整体逻辑）
// 参数 e_visibility 类型为 bool，大概率和是否启用网格可见性相关（需结合完整代码判断准确含义）
// 参数 e_pedestrian 类型为 bool，可能是用于控制是否启用行人导航相关功能（从命名和常见场景推测）
        double v_distance,
        double max_road_len,
        double w_height,
        double a_width,
        bool smooth_junc,
        bool e_visibility,
        bool e_pedestrian)
      : vertex_distance(v_distance), // 使用传入的参数 v_distance 初始化成员变量 vertex_distance
        max_road_length(max_road_len),// 使用传入的参数 max_road_len 初始化成员变量 max_road_length
        wall_height(w_height), // 使用传入的参数 w_height 初始化成员变量 wall_height
        additional_width(a_width),// 使用传入的参数 a_width 初始化成员变量 additional_width
        smooth_junctions(smooth_junc),// 使用传入的参数 smooth_junc 初始化成员变量 smooth_junctions
        enable_mesh_visibility(e_visibility),// 使用传入的参数 e_visibility 初始化成员变量 enable_mesh_visibility
        enable_pedestrian_navigation(e_pedestrian)// 使用传入的参数 e_pedestrian 初始化成员变量 enable_pedestrian_navigation
        {}
// 声明并初始化成员变量 vertex_distance，初始值设为 2.0，具体含义可能与某种距离相关（结合整体代码判断）
    double vertex_distance = 2.0;
// 声明并初始化成员变量 max_road_length，初始值设为 50.0，推测与道路长度的最大值有关（要根据完整功能确定
    double max_road_length = 50.0;
// 声明并初始化成员变量 wall_height，初始值设为 1.0，可能代表墙的高度（依整体情况判断）
    double wall_height = 1.0；
// 声明并初始化成员变量 additional_width，初始值设为 0.6，也许是额外的宽度数值（结合具体场景确定）

    double additional_width = 0.6;
// 声明并初始化成员变量 vertex_width_resolution，初始值设为 4.0f（注意这里是单精度浮点数类型），具体作用需结合完整代码知晓
    double vertex_width_resolution = 4.0f;
// 声明并初始化成员变量 simplification_percentage，初始值设为 20.0f（单精度浮点数），可能与简化比例之类的功能相关（看整体逻辑）
    float simplification_percentage = 20.0f;
// 声明并初始化成员变量 smooth_junctions，初始值设为 true，表示默认情况下启用平滑路口相关功能（具体功能结合代码上下文）
    bool smooth_junctions = true;
/ /声明并初始化成员变量 enable_mesh_visibility，初始值设为 true，意味着默认开启网格可见性相关设置（准确含义参考完整代码
    bool enable_mesh_visibility = true;
// 声明并初始化成员变量 enable_pedestrian_navigation，初始值设为 true，说明默认启用行人导航相关功能（要结合整体项目判断）
    bool enable_pedestrian_navigation = true;
// 使用 MSGPACK_DEFINE_ARRAY 宏（可能来自某个特定的序列化库，比如 msgpack-c 等）来定义一个数组结构，将下面列出的这些成员变量包含在这个数组相关的定义中
// 这样做可能是为了方便对这些变量进行序列化或者其他与数据组织相关的操作（具体取决于这个宏的实现和使用场景）
    MSGPACK_DEFINE_ARRAY(
        vertex_distance,
        max_road_length,
        wall_height,
        additional_width,
        smooth_junctions,
        enable_mesh_visibility,
        enable_pedestrian_navigation);
  };

}
}
