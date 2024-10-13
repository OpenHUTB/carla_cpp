// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/TrafficSign.h"   // 包含 TrafficSign 类的头文件
#include "carla/client/detail/Simulator.h"  // 包含 Simulator 类的详细实现的头文件
#include "carla/client/ActorList.h"   // 包含 ActorList 类的头文件

namespace carla {  // 开始 carla 命名空间
namespace client {  // 开始 client 命名空间
  // 定义 TrafficSign 类的成员函数 GetSignId
  carla::road::SignId TrafficSign::GetSignId() const {   // 返回当前交通标志的标识符，从当前剧集获取相关的交通灯数据
    return std::string(GetEpisode().Lock()->GetActorSnapshot(*this).state.traffic_light_data.sign_id);
  }

} // namespace client
} // namespace carla
