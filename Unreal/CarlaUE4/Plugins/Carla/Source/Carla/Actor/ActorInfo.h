// 版权信息
// 版权所有 (c) 2017 巴塞罗那自治大学 (UAB) 计算机视觉中心 (CVC)。
//
// 本工作根据 MIT 许可证的条款进行授权。
// 许可证副本见 <https://opensource.org/licenses/MIT>。
 
#pragma once
 
// 包含必要的头文件
#include "Carla/Actor/ActorDescription.h"  // 包含演员（Actor）描述的定义
#include "Carla/Game/Tagger.h"              // 包含标签器的定义，用于给游戏对象打标签
 
#include "Math/DVector.h"                   // 包含数学向量定义
 
// 禁用 Unreal Engine 4 的宏，以避免与 carla::rpc 命名空间中的宏冲突
#include <compiler/disable-ue4-macros.h>
#include <carla/rpc/Actor.h>                // 包含 RPC（远程过程调用）中演员的定义
#include <carla/rpc/ObjectLabel.h>          // 包含 RPC 中对象标签的定义
// 启用 Unreal Engine 4 的宏
#include <compiler/enable-ue4-macros.h>
 
// 为 carla::rpc 命名空间创建一个别名，以便简化代码
namespace crp = carla::rpc;
 
/// 一个结构体，用于表示演员及其属性的视图。
struct FActorInfo
{
public:
 
  // 演员的描述信息
  FActorDescription Description;
 
  // 演员的语义标签集合，用于表示演员的类型或属性（例如，车辆、行人、交通标志等）
  TSet<crp::CityObjectLabel> SemanticTags;
 
  // 演员的边界框，用于表示演员在物理世界中的位置和大小
  FBoundingBox BoundingBox;
 
  // 演员的序列化数据，包含从 RPC 接收到的演员信息
  crp::Actor SerializedData;
 
  // @todo 仅供 FWorldObserver 使用。用于存储演员的速度信息。
  // 这是一个可变成员，意味着它可以在不改变对象逻辑状态的情况下被修改。
  mutable FVector Velocity = {0.0f, 0.0f, 0.0f};  // 初始化为零向量
 
};
