// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
// 这是一个头文件保护，防止头文件被重复包含
#pragma once
// 引入Carla项目中的相关调试头文件
#include "carla/Debug.h"// 引入Carla项目中的内存相关头文件，可能涉及智能指针等内存管理相关的功能定义
#include "carla/Memory.h"// 引入Carla客户端中表示游戏世界里的“演员”（如车辆、行人等实体）的头文件
#include "carla/client/Actor.h"// 引入Carla客户端中与“情节”（可以理解为一段模拟场景的过程）代理相关的头文件，
// 可能用于和服务器交互、获取或设置情节相关的信息等操作
#include "carla/client/detail/EpisodeProxy.h" // 引入Carla项目中远程过程调用（RPC）相关的Actor定义，用于网络通信等场景下对Actor的描述等
#include "carla/rpc/Actor.h"
// 如果是使用微软的编译器（MSVC），进行一些警告相关的处理
#ifdef _MSC_VER// 保存当前的警告状态
#pragma warning(push)// 禁用特定的警告编号4583，可能是和类模板相关的一些编译器警告
#pragma warning(disable:4583)// 禁用特定的警告编号4582，同样可能是和类模板相关的警告
#pragma warning(disable:4582)// 引入Boost库的variant2中的variant类型，它可以在一个变量中存储多种不同类型的值
#include <boost/variant2/variant.hpp>// 恢复之前保存的警告状态
#pragma warning(pop)// 如果不是微软编译器，直接引入Boost库的variant2中的variant类型
#else
#include <boost/variant2/variant.hpp>
#endif
// 定义在Carla项目的client命名空间下的detail命名空间中，表明这个类可能是用于一些内部实现细节相关的功能
namespace carla {
namespace client {
namespace detail {

  /// 创建一个执行者, 只在需要的时候实例化.
  class ActorVariant {
  public:

    ActorVariant(rpc::Actor actor)
      : _value(actor) {}

    ActorVariant(SharedPtr<client::Actor> actor)
      : _value(actor) {}

    ActorVariant &operator=(rpc::Actor actor) {
      _value = actor;
      return *this;
    }
// 重载赋值运算符，接受指向client::Actor的智能指针（SharedPtr），将传入的智能指针赋值给内部的_value变量，
    // 并返回当前对象的引用，用于链式赋值
    ActorVariant &operator=(SharedPtr<client::Actor> actor) {
      _value = actor;
      return *this;
    }// 获取对应的client::Actor的智能指针，如果内部_value当前存储的是rpc::Actor类型，
    // 则会调用MakeActor函数进行相应的转换（可能是根据RPC信息创建对应的客户端Actor实例），
    // 最后返回指向client::Actor的智能指针，这里假设经过处理后_value中存储的是合适的client::Actor类型

    SharedPtr<client::Actor> Get(EpisodeProxy episode) const {
      if (_value.index() == 0u) {
        MakeActor(episode);
      }
      DEBUG_ASSERT(_value.index() == 1u);
      return boost::variant2::get<SharedPtr<client::Actor>>(_value);
    }

    const rpc::Actor &Serialize() const {
      return boost::variant2::visit(Visitor(), _value);
    }
// 获取Actor的ID，通过先调用Serialize函数获取对应的rpc::Actor，再获取其ID属性
    ActorId GetId() const {
      return Serialize().id;
    }
// 获取Actor的父ID，同样先调用Serialize函数获取rpc::Actor，再获取其parent_id属性
    ActorId GetParentId() const {
      return Serialize().parent_id;
    }
// 获取Actor的类型ID，通过先调用Serialize函数获取rpc::Actor，再获取其描述中的类型ID属性
    const std::string &GetTypeId() const {
      return Serialize().description.id;
    }

    bool operator==(ActorVariant rhs) const {
      return GetId() == rhs.GetId();
    }
// 重载不等运算符，基于相等运算符的结果取反来判断两个ActorVariant对象是否不等
    bool operator!=(ActorVariant rhs) const {
      return !(*this == rhs);
    }

  private:// 定义一个访问者结构体（Visitor），用于在访问variant类型的_value时，根据其实际存储的类型进行相应的操作

    struct Visitor {// 如果_value中存储的是rpc::Actor类型，直接返回该rpc::Actor对象
      const rpc::Actor &operator()(const rpc::Actor &actor) const {
        return actor;
      }// 如果_value中存储的是指向client::Actor的智能指针类型，
        // 则调用该智能指针所指向的client::Actor对象的Serialize函数来返回对应的rpc::Actor对象
      const rpc::Actor &operator()(const SharedPtr<const client::Actor> &actor) const {
        return actor->Serialize();
      }
    };
// 私有函数，用于将内部_value中存储的rpc::Actor类型转换为对应的client::Actor类型，
    // 具体实现应该在对应的cpp文件中（这里只有声明），可能会根据传入的EpisodeProxy等信息来创建客户端的Actor实例
    void MakeActor(EpisodeProxy episode) const;
// 使用Boost的variant2::variant类型来存储不同形式的Actor，可以是rpc::Actor类型或者是指向client::Actor的智能指针类型，
    // 并且使用mutable关键字修饰，意味着即使在const成员函数中也可以修改它的值（例如在Get函数中可能会根据情况修改它）
    mutable boost::variant2::variant<rpc::Actor, SharedPtr<client::Actor>> _value;
  };

} // namespace detail
} // namespace client
} // namespace carla
