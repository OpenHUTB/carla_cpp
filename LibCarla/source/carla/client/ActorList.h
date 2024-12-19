// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 确保该头文件只被包含一次

#include "carla/client/detail/ActorVariant.h" // 引入 ActorVariant 类定义

#include <boost/iterator/transform_iterator.hpp> // 引入 Boost 库中的 transform_iterator，用于创建变换迭代器

#include <vector> // 引入标准库中的 vector 容器，存储参与者数据

namespace carla { // 开始 carla 命名空间
namespace client { // 开始 client 命名空间
  // ActorList 类定义，表示一个包含多个参与者（Actors）的列表。
  // 支持 shared_from_this 以便对象能方便地管理生命周期。
  class ActorList : public EnableSharedFromThis<ActorList> { // 定义 ActorList 类，支持 shared_from_this
  private:

    // 创建变换迭代器，将元素从 ActorVariant 转换为 Actor
    template <typename It>
    auto MakeIterator(It it) const {
      return boost::make_transform_iterator(it, [this](auto &v) { // 使用 Boost 库的变换迭代器
        return v.Get(_episode); // 获取 Actor 对象，基于当前的 episode
      });
    }

  public:

    /// 根据指定的 actor_id 查找对应的参与者（Actor）。
    SharedPtr<Actor> Find(ActorId actor_id) const; // 查找指定 id 的参与者

    /// 根据提供的通配符模式（wildcard_pattern）过滤符合条件的参与者列表。
    SharedPtr<ActorList> Filter(const std::string &wildcard_pattern) const; // 根据通配符模式过滤参与者列表

    /// 重载 [] 运算符，返回指定位置的参与者（Actor）。
    SharedPtr<Actor> operator[](size_t pos) const { 
      return _actors[pos].Get(_episode); // 获取指定位置的 Actor
    }
    /// 提供 at() 函数用于安全访问指定位置的参与者，支持边界检查。
    SharedPtr<Actor> at(size_t pos) const { // 提供 at() 函数以安全访问元素
      return _actors.at(pos).Get(_episode); // 获取指定位置的 Actor，并进行边界检查
    }

    /// 返回指向列表中第一个元素的迭代器。
    auto begin() const { 
      return MakeIterator(_actors.begin()); // 创建并返回开始迭代器
    }

    /// 返回指向列表中最后一个元素后一个位置的迭代器。
    auto end() const { // 返回迭代器的结束位置
      return MakeIterator(_actors.end()); // 创建并返回结束迭代器
    }

    /// 检查列表是否为空。
    bool empty() const { 
      return _actors.empty(); // 返回列表是否为空
    }

    /// 返回列表中包含的参与者数量。
    size_t size() const { 
      return _actors.size(); // 返回参与者数量
    }

  private:

    friend class World; // 声明 World 类为 ActorList 的友元类

    // 构造函数，接受 EpisodeProxy 和包含多个 ActorVariant 对象的 vector。
    ActorList(detail::EpisodeProxy episode, std::vector<rpc::Actor> actors); 

    detail::EpisodeProxy _episode; // 存储 EpisodeProxy 对象，表示当前的场景或回合

    std::vector<detail::ActorVariant> _actors; // 存储 ActorVariant 对象的向量，表示多个参与者
  };

} // namespace client
} // namespace carla

