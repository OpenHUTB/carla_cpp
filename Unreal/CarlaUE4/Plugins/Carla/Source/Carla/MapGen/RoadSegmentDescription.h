// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// 引入图相关的类型定义头文件，可能包含了图中半边等相关结构体等的定义
#include "GraphTypes.h"  
// 引入不可复制的工具类相关头文件，用于限制类对象不能被拷贝
#include "Util/NonCopyable.h"  
#include <vector>

namespace MapGen {

  // RoadSegmentDescription类用于描述道路段相关信息，继承自NonCopyable，意味着该类对象不能被复制
  class RoadSegmentDescription : private NonCopyable
  {
  public:
    // 将一个GraphHalfEdge类型的边添加到道路段描述中
    void Add(const GraphHalfEdge &Edge) {
      // 如果角度指针Angle目前为空（即还未设置角度信息）
      if (Angle == nullptr) {
        // 使用MakeUnique创建一个指向float类型的智能指针，其值为传入边的角度值，用于记录该道路段的角度信息
        Angle = MakeUnique<float>(Edge.Angle);
      } else if (*Angle!= Edge.Angle) { /// @todo Use a scale.
        // 如果当前已记录的角度与传入边的角度不一致，将角度指针置为nullptr，表示该道路段角度不统一（不再是直线等情况）
        Angle = nullptr;
      }
      // 将传入边的指针添加到向量_vect中，用于存储组成该道路段的边的信息
      _vect.emplace_back(&Edge);
    }

    // 重载了下标运算符，用于通过索引获取存储在_vect中的GraphHalfEdge对象的引用，方便访问道路段中的边信息
    const GraphHalfEdge &operator[](size_t i) const {
      return *_vect[i];
    }

    // 返回道路段中边的数量，即存储在_vect中的元素个数，反映了该道路段由多少个半边组成
    size_t Size() const {
      return _vect.size();
    }

    // 判断该道路段是否是直线（通过角度是否有效来判断，即Angle指针是否非空）
    bool IsStraight() const {
      return Angle.IsValid();
    }

    // 如果道路段是直线，返回指向其角度值的指针，否则返回nullptr，表示该道路段不是直线，没有统一的角度
    /// @return nullptr if the road segment is not straight.
    const float *GetAngle() const {
      return Angle.Get();
    }

  private:
    // 智能指针，用于指向一个float类型的角度值，如果该道路段是直线，则指向其角度值，否则为nullptr
    TUniquePtr<float> Angle = nullptr;

    // 向量，用于存储指向GraphHalfEdge类型的指针，这些指针指向组成该道路段的各个半边
    std::vector<const GraphHalfEdge *> _vect;
  };
} // namespace MapGen
