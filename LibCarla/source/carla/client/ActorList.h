// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 确保该头文件只被包含一次

#include "carla/client/detail/ActorVariant.h" // 包含 ActorVariant 的定义

#include <boost/iterator/transform_iterator.hpp> // 引入 Boost 库中的变换迭代器

#include <vector> // 引入标准库中的 vector 容器

namespace carla { // 开始 carla 命名空间
namespace client { // 开始 client 命名空间

  class ActorList : public EnableSharedFromThis<ActorList> { // 定义 ActorList 类，支持 shared_from_this
  private:

    // 定义一个模板函数，用于生成变换迭代器
    template <typename It>
    auto MakeIterator(It it) const {
      return boost::make_transform_iterator(it, [this](auto &v) { // 使用 Boost 库的变换迭代器
        return v.Get(_episode); // 从 ActorVariant 获取 Actor 对象
      });
    }

  public:

    /// 根据 id 寻找参与者。
    SharedPtr<Actor> Find(ActorId actor_id) const; // 查找指定 id 的参与者

    /// 过滤类型 ID 与 @a wildcard_pattern 匹配的参与者列表。
    SharedPtr<ActorList> Filter(const std::string &wildcard_pattern) const; // 根据通配符模式过滤参与者列表

    SharedPtr<Actor> operator[](size_t pos) const { // 重载 [] 运算符
      return _actors[pos].Get(_episode); // 获取指定位置的参与者
    }

    SharedPtr<Actor> at(size_t pos) const { // 提供 at() 函数以安全访问元素
      return _actors.at(pos).Get(_episode); // 获取指定位置的参与者（使用 at 进行边界检查）
    }

    auto begin() const { // 返回迭代器的起始位置
      return MakeIterator(_actors.begin()); // 创建并返回起始迭代器
    }

    auto end() const { // 返回迭代器的结束位置
      return MakeIterator(_actors.end()); // 创建并返回结束迭代器
    }

    bool empty() const { // 检查列表是否为空
      return _actors.empty(); // 返回空状态
    }

    size_t size() const { // 获取参与者数量
      return _actors.size(); // 返回参与者数量
    }

  private:

    friend class World; // 声明 World 类为友元类

    ActorList(detail::EpisodeProxy episode, std::vector<rpc::Actor> actors); // 构造函数，接受 EpisodeProxy 和 Actor 向量

    detail::EpisodeProxy _episode; // 存储 EpisodeProxy 对象

    std::vector<detail::ActorVariant> _actors; // 存储 ActorVariant 对象的向量
  };

} // namespace client
} // namespace carla

