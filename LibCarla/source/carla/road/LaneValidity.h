// 版权所有 (c) 2019 巴萨大学计算机视觉中心 (CVC)。
// 本作品遵循 MIT 许可证进行许可。
// 详情请参见 <https://opensource.org/licenses/MIT>。

#pragma once

// 包含必要的头文件
#include "carla/NonCopyable.h" // 引入不可复制类的定义
#include <string>              // 引入字符串支持
#include <vector>             // 引入向量支持
#include "carla/road/RoadTypes.h" // 引入与道路类型相关的定义

namespace carla {
namespace road {

// 定义表示车道有效性的结构体
struct LaneValidity {
public:
    // 构造函数，接受起始车道和目标车道的 ID
    LaneValidity(LaneId from_lane, LaneId to_lane)
      : _from_lane(from_lane), _to_lane(to_lane) {}

    // 私有成员变量，表示起始车道的 ID
    road::LaneId _from_lane;
    
    // 私有成员变量，表示目标车道的 ID
    road::LaneId _to_lane;
};

} // namespace road
} // namespace carla
