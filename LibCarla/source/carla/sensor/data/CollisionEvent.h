// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// 表明该代码遵循MIT许可协议，可通过对应链接查看协议具体内容
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// 引入Carla项目中的Debug头文件，可能用于调试相关功能或输出调试信息等操作，具体取决于该头文件内的实现
#include "carla/Debug.h"
// 引入Carla项目中客户端细节相关的ActorVariant头文件，ActorVariant类型应该是用于处理不同种类的Actor（可能是游戏中的角色、物体等实体概念）的一种通用表示形式
#include "carla/client/detail/ActorVariant.h"
// 引入Carla项目中几何相关的Vector3D头文件，用于表示三维向量，在这里大概率是用于描述碰撞相关的矢量信息，比如碰撞的方向、冲量方向等
#include "carla/geom/Vector3D.h"
// 引入Carla项目中传感器数据相关的SensorData头文件，CollisionEvent类继承自SensorData，意味着它会继承SensorData类已有的属性和行为，以构建更具体的碰撞事件相关的数据结构
#include "carla/sensor/SensorData.h"
// 引入Carla项目中传感器序列化相关的CollisionEventSerializer头文件，用于对碰撞事件进行序列化和反序列化操作，方便数据的存储、传输以及恢复等处理
#include "carla/sensor/s11n/CollisionEventSerializer.h"

namespace carla {
namespace sensor {
namespace data {

  // 定义CollisionEvent类，表示一个已记录的碰撞事件，继承自SensorData类，从而能复用SensorData中的通用传感器数据相关逻辑，并在此基础上拓展与碰撞事件特定相关的功能
  /// A registered collision.
  class CollisionEvent : public SensorData {
    // 使用类型别名Super来指代SensorData类型，这样在后续代码中可以更简洁清晰地表示父类类型，方便代码书写和阅读
    using Super = SensorData;
  protected:
    // 使用类型别名Serializer来指代s11n::CollisionEventSerializer类型，便于后续调用该序列化器相关的函数，提高代码的可读性和简洁性
    using Serializer = s11n::CollisionEventSerializer;
    // 将Serializer声明为友元类，这意味着CollisionEventSerializer类可以访问CollisionEvent类的私有成员，方便在序列化和反序列化操作中直接操作类的内部数据
    friend Serializer;

    // 显式定义的构造函数，用于根据传入的原始数据（RawData类型）来创建一个CollisionEvent对象，初始化该碰撞事件相关的各项属性
    explicit CollisionEvent(const RawData &data)
      // 调用父类（SensorData）的构造函数，传递原始数据data，先完成父类部分的初始化工作，保证继承体系下的初始化顺序和完整性
      : Super(data),
        // 通过调用Serializer类的DeserializeRawData方法，从传入的原始数据中解析并获取“自身”演员（Actor）信息，然后用其初始化_self_actor成员变量，该成员变量存储了与此次碰撞事件中检测到碰撞的一方相关的Actor信息
        _self_actor(Serializer::DeserializeRawData(data).self_actor),
        // 同样通过Serializer类的DeserializeRawData方法，从原始数据中解析并获取“其他”演员信息，用于初始化_other_actor成员变量，该成员变量存储了与此次碰撞事件中与“自身”演员发生碰撞的另一方相关的Actor信息
        _other_actor(Serializer::DeserializeRawData(data).other_actor),
        // 还是借助Serializer类的DeserializeRawData方法，从原始数据中解析并获取碰撞的法向冲量信息，以此初始化_normal_impulse成员变量，该成员变量使用Vector3D类型来表示碰撞产生的法向冲量矢量，包含了冲量的大小和方向信息
        _normal_impulse(Serializer::DeserializeRawData(data).normal_impulse) {}

  public:
    // 获取“自身”演员的共享指针，这里的“自身”演员指的是在碰撞事件中负责检测或者说记录此次碰撞的那个Actor，通过调用_self_actor的Get方法，并传入相关的Episode（可能是游戏中的场景、情节等概念，具体取决于项目中的定义）信息来获取对应的Actor指针
    /// Get "self" actor. Actor that measured the collision.
    SharedPtr<client::Actor> GetActor() const {
      return _self_actor.Get(GetEpisode());
    }

    // 获取与“自身”演员发生碰撞的“其他”演员的共享指针，通过调用_other_actor的Get方法，并传入相关的Episode信息来获取对应的Actor指针，以此得到碰撞事件中涉及的另一方Actor信息
    /// Get the actor to which we collided.
    SharedPtr<client::Actor> GetOtherActor() const {
      return _other_actor.Get(GetEpisode());
    }

    // 获取碰撞产生的法向冲量结果，返回一个常量引用，指向表示法向冲量的geom::Vector3D对象，外部可以通过这个引用获取但不能修改该法向冲量的信息，比如用于后续的物理模拟计算、碰撞效果分析等操作
    /// Normal impulse result of the collision.
    const geom::Vector3D &GetNormalImpulse() const {
      return _normal_impulse;
    }

  private:
    // 用于存储碰撞事件中“自身”演员的相关信息，使用client::detail::ActorVariant类型可以灵活处理不同类型的Actor，以适应项目中各种可能的Actor表示形式，具体内容由Serializer从原始数据中反序列化得到
    client::detail::ActorVariant _self_actor;
    // 用于存储碰撞事件中与“自身”演员发生碰撞的“其他”演员的相关信息，同样使用ActorVariant类型来灵活表示不同的Actor情况，其值也是通过Serializer从原始数据中反序列化获取
    client::detail::ActorVariant _other_actor;
    // 用于存储碰撞产生的法向冲量信息，是一个geom::Vector3D类型的三维向量，包含了法向冲量的方向和大小，该向量信息从原始数据中反序列化而来，用于描述此次碰撞在物理层面的冲量情况
    geom::Vector3D _normal_impulse;
  };

} // namespace data
} // namespace sensor
} // namespace carla
