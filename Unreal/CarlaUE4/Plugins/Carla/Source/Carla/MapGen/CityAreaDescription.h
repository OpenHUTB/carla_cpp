// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GraphTypes.h"

#include <vector>

// 命名空间 MapGen
namespace MapGen {

// 类 CityAreaDescription，不可复制
  class CARLA_API CityAreaDescription : private NonCopyable
  {
  public:

// 显式构造函数，接受一个 GraphFace 类型的参数
    explicit CityAreaDescription(const GraphFace &Face) : _face(&Face) {}

 // 添加节点到节点容器
    void Add(const GraphNode &Node) {
      _nodes.emplace_back(&Node);
    }

// 获取面（GraphFace）
    const GraphFace &GetFace() const {
      return *_face;
    }

 // 获取指定索引的节点
    const GraphNode &GetNodeAt(size_t i) const {
      return *_nodes[i];
    }

// 获取节点数量
    size_t NodeCount() const {
      return _nodes.size();
    }

  private:

 // 面（GraphFace）
    const GraphFace *_face;

// 节点容器
    std::vector<const GraphNode *> _nodes;
  };

} // namespace MapGen
