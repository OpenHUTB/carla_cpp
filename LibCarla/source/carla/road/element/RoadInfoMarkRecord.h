// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/road/element/RoadInfo.h" // 包含RoadInfo头文件
#include "carla/road/element/RoadInfoMarkTypeLine.h" // 包含RoadInfoMarkTypeLine头文件
#include <string> // 包含字符串类
#include <vector> // 包含向量类
#include <memory> // 包含智能指针

namespace carla {
namespace road {
namespace element {

  /// 每条车道在道路横截面内可以提供多个道路标记条目。
  /// 道路标记信息定义了车道外边界的线条样式。
  /// 对于左侧车道，这是左边界；对于右侧车道，这是右边界。
  /// 左右车道之间的分隔线样式由零号车道（即中央车道）的道路标记条目决定。
  class RoadInfoMarkRecord final : public RoadInfo {
  public:

    /// 可用作标志
    enum class LaneChange : uint8_t {
      None     = 0x00, // 无
      Increase = 0x01, // 增加
      Decrease = 0x02, // 减少
      Both     = 0x03  // 两者
    };

    /// 构造函数，初始化基本属性
    RoadInfoMarkRecord(
        double s, // 路段位置
        int road_mark_id) // 道路标记ID
      : RoadInfo(s), // 调用基类构造函数
        _road_mark_id(road_mark_id), // 初始化道路标记ID
        _type(""), // 初始化类型为空
        _weight(""), // 初始化重量为空
        _color("white"), // 初始化颜色为白色
        _material("standard"), // 初始化材料为标准
        _width(0.15), // 初始化宽度为0.15米
        _lane_change(LaneChange::None), // 初始化车道变更为无
        _height(0.0), // 初始化高度为0.0米
        _type_name(""), // 初始化类型名称为空
        _type_width(0.0) {} // 初始化类型宽度为0.0

    /// 构造函数，初始化所有属性
    RoadInfoMarkRecord(
        double s, // 路段位置
        int road_mark_id, // 道路标记ID
        std::string type, // 类型
        std::string weight, // 重量
        std::string color, // 颜色
        std::string material, // 材料
        double width, // 宽度
        LaneChange lane_change, // 车道变更
        double height, // 高度
        std::string type_name, // 类型名称
        double type_width) // 类型宽度
      : RoadInfo(s), // 调用基类构造函数
        _road_mark_id(road_mark_id), // 初始化道路标记ID
        _type(type), // 初始化类型
        _weight(weight), // 初始化重量
        _color(color), // 初始化颜色
        _material(material), // 初始化材料
        _width(width), // 初始化宽度
        _lane_change(lane_change), // 初始化车道变更
        _height(height), // 初始化高度
        _type_name(type_name), // 初始化类型名称
        _type_width(type_width) {} // 初始化类型宽度

    /// 接受访问者
    void AcceptVisitor(RoadInfoVisitor &v) final {
      v.Visit(*this); // 调用访问者的Visit方法
    }

    /// 获取道路标记的唯一标识符。
    int GetRoadMarkId() const {
      return _road_mark_id; // 返回道路标记ID
    }

    /// 获取道路标记的类型。
    const std::string &GetType() const {
      return _type; // 返回类型
    }

    /// 获取道路标记的重量。
    const std::string &GetWeight() const {
      return _weight; // 返回重量
    }

    /// 获取道路标记的颜色。
    const std::string &GetColor() const {
      return _color; // 返回颜色
    }

    /// 获取道路标记的材料（标识符待定义，目前使用“标准”）。
    const std::string &GetMaterial() const {
      return _material; // 返回材料
    }

    /// 获取道路标记的宽度 – 可选。
    double GetWidth() const {
      return _width; // 返回宽度
    }

    /// 允许在指定方向上进行车道变更，考虑到车道按升序编号从右到左。
    /// 如果缺少此属性，假定“双方”均有效。
    LaneChange GetLaneChange() const {
      return _lane_change; // 返回车道变更状态
    }

    /// 获取道路标记顶部边缘与车道参考平面之间的物理距离。
    double GetHeight() const {
      return _height; // 返回高度
    }

    /// 获取道路标记类型的名称（如果有的话）。
    const std::string &GetTypeName() const {
      return _type_name; // 返回类型名称
    }

    /// 获取道路标记类型的宽度（如果有的话）。
    double GetTypeWidth() const {
      return _type_width; // 返回类型宽度
    }

    /// 获取道路标记线条的集合
    std::vector<std::unique_ptr<RoadInfoMarkTypeLine>> &GetLines() {
      return _lines; // 返回线条集合
    }

  private:

    const int _road_mark_id; // 道路标记ID

    const std::string _type; // 标记类型

    const std::string _weight; // 标记重量

    const std::string _color; // 标记颜色

    const std::string _material; // 标记材料

    const double _width; // 标记宽度

    const LaneChange _lane_change; // 车道变更状态

    const double _height; // 标记高度

    const std::string _type_name; // 标记类型名称

    const double _type_width; // 标记类型宽度

    std::vector<std::unique_ptr<RoadInfoMarkTypeLine>> _lines; // 道路标记线条集合
  };
} // namespace element
} // namespace road
} // namespace carla
