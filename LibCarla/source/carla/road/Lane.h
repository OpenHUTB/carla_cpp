// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Mesh.h" // 引入Mesh类的头文件
#include "carla/geom/Transform.h" // 引入Transform类的头文件
#include "carla/road/InformationSet.h" // 引入InformationSet类的头文件
#include "carla/road/RoadTypes.h" // 引入RoadTypes类的头文件

#include <vector> // 引入vector容器
#include <iostream> // 引入输入输出流
#include <memory> // 引入智能指针

namespace carla { // 定义carla命名空间
namespace road { // 定义road命名空间

  class LaneSection; // 前向声明LaneSection类
  class MapBuilder; // 前向声明MapBuilder类
  class Road; // 前向声明Road类

  class Lane : private MovableNonCopyable { // 定义Lane类，继承自MovableNonCopyable
  public:

    /// 可以作为标志使用
    enum class LaneType : int32_t { // 定义车道类型的枚举
      None          = 0x1, // 无
      Driving       = 0x1 << 1, // 行驶车道
      Stop          = 0x1 << 2, // 停止车道
      Shoulder      = 0x1 << 3, // 应急车道
      Biking        = 0x1 << 4, // 自行车道
      Sidewalk      = 0x1 << 5, // 人行道
      Border        = 0x1 << 6, // 边界
      Restricted    = 0x1 << 7, // 限制
      Parking       = 0x1 << 8, // 停车场
      Bidirectional = 0x1 << 9, // 双向车道
      Median        = 0x1 << 10, // 中央车道
      Special1      = 0x1 << 11, // 特殊类型1
      Special2      = 0x1 << 12, // 特殊类型2
      Special3      = 0x1 << 13, // 特殊类型3
      RoadWorks     = 0x1 << 14, // 道路施工
      Tram          = 0x1 << 15, // 有轨电车车道
      Rail          = 0x1 << 16, // 铁路车道
      Entry         = 0x1 << 17, // 入口
      Exit          = 0x1 << 18, // 出口
      OffRamp       = 0x1 << 19, // 下匝道
      OnRamp        = 0x1 << 20, // 上匝道
      Any           = -2 // 任意类型
    };

  public:

    Lane() = default; // 默认构造函数

    Lane( // 带参数的构造函数
        LaneSection *lane_section, // 车道段指针
        LaneId id, // 车道ID
        std::vector<std::unique_ptr<element::RoadInfo>> &&info) // 路面信息的唯一指针向量
      : _lane_section(lane_section), // 初始化车道段指针
        _id(id), // 初始化车道ID
        _info(std::move(info)) { // 移动初始化路面信息
      DEBUG_ASSERT(lane_section != nullptr); // 调试断言：车道段指针不能为空
    }

    const LaneSection *GetLaneSection() const; // 获取车道段指针

    Road *GetRoad() const; // 获取道路指针

    LaneId GetId() const; // 获取车道ID

    LaneType GetType() const; // 获取车道类型

    bool GetLevel() const; // 获取车道等级

    template <typename T>
    const T *GetInfo(const double s) const { // 根据位置s获取信息
      DEBUG_ASSERT(_lane_section != nullptr); // 调试断言：车道段指针不能为空
      return _info.GetInfo<T>(s); // 返回指定类型的信息
    }

    template <typename T>
    std::vector<const T*> GetInfos() const { // 获取所有信息
      DEBUG_ASSERT(_lane_section != nullptr); // 调试断言：车道段指针不能为空
      return _info.GetInfos<T>(); // 返回所有指定类型的信息
    }

    const std::vector<Lane *> &GetNextLanes() const { // 获取下一车道的引用
      return _next_lanes; // 返回下一车道列表
    }

    const std::vector<Lane *> &GetPreviousLanes() const { // 获取前一车道的引用
      return _prev_lanes; // 返回前一车道列表
    }

    LaneId GetSuccessor() const { // 获取后继车道ID
      return _successor; // 返回后继车道ID
    }

    LaneId GetPredecessor() const { // 获取前驱车道ID
      return _predecessor; // 返回前驱车道ID
    }

    double GetDistance() const; // 获取车道的距离

    double GetLength() const; // 获取车道的长度

    /// 返回给定位置s的车道总宽度
    double GetWidth(const double s) const; // 获取给定位置s的车道宽度

    /// 检查几何形状是否是直线
    bool IsStraight() const; // 检查车道是否直

    geom::Transform ComputeTransform(const double s) const; // 计算给定位置s的变换

    /// 计算给定位置s的车道边缘位置
    std::pair<geom::Vector3D, geom::Vector3D> GetCornerPositions( // 获取车道角落位置
      const double s, const float extra_width = 0.f) const; // 可选额外宽度

  private:

    friend MapBuilder; // 友元类：MapBuilder

    LaneSection *_lane_section = nullptr; // 车道段指针

    LaneId _id = 0; // 车道ID

    InformationSet _info; // 路面信息集合

    LaneType _type = LaneType::None; // 车道类型

    bool _level = false; // 车道等级

    LaneId _successor = 0; // 后继车道ID

    LaneId _predecessor = 0; // 前驱车道ID

    std::vector<Lane *> _next_lanes; // 下一车道列表

    std::vector<Lane *> _prev_lanes; // 前一车道列表
  };

} // road
} // carla