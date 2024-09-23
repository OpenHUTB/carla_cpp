// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/detail/ActorVariant.h"   // 引入ActorVariant头文件

#include <boost/iterator/transform_iterator.hpp>   // 引入Boost库中的transform_iterator

#include <vector>  // 引入vector库

namespace carla {   // 定义命名空间carla
namespace client {  // 定义命名空间client

  class ActorList : public EnableSharedFromThis<ActorList> {   // 定义ActorList类，继承自EnableSharedFromThis
  private:

    template <typename It>  // 模板函数，接收任意类型的迭代器
    auto MakeIterator(It it) const {   // 创建变换迭代器
      return boost::make_transform_iterator(it, [this](auto &v) {   // 使用Boost创建变换迭代器
        return v.Get(_episode);  // 从Actor中获取当前episode
      });
    }

  public:

    /// 根据actor_id查找演员
    SharedPtr<Actor> Find(ActorId actor_id) const;   // 查找演员的声明

    ///过滤与给定模式匹配的演员列表
    SharedPtr<ActorList> Filter(const std::string &wildcard_pattern) const;   // 过滤演员的声明

    SharedPtr<Actor> operator[](size_t pos) const {   // 重载下标操作符
      return _actors[pos].Get(_episode);  // 根据位置获取演员
    }

    SharedPtr<Actor> at(size_t pos) const {  // 访问指定位置的演员
      return _actors.at(pos).Get(_episode);  // 使用at方法获取演员
    }

    auto begin() const {   // 获取开始迭代器
      return MakeIterator(_actors.begin());  // 调用MakeIterator生成开始迭代器
    }

    auto end() const {   // 获取结束迭代器
      return MakeIterator(_actors.end());  // 调用MakeIterator生成结束迭代器
    }

    bool empty() const {   // 检查演员列表是否为空
      return _actors.empty();   // 返回演员列表是否为空
    }

    size_t size() const {   // 获取演员列表的大小
      return _actors.size();   // 返回演员数量
    }

  private:

    friend class World;  // 声明World类为友元类

    ActorList(detail::EpisodeProxy episode, std::vector<rpc::Actor> actors);  // 构造函数声明

    detail::EpisodeProxy _episode;   // 存储当前的episode

    std::vector<detail::ActorVariant> _actors;  // 存储演员的变体列表
  };

} // namespace client
} // namespace carla
