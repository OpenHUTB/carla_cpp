// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "DoublyConnectedEdgeList.h"

namespace MapGen {

  /// 随机双连通边链表 DoublyConnectedEdgeList 生成器。
  class GraphGenerator : private NonCopyable
  {
  public:

    /// 创建一个大小为 @a SizeX 乘以  @a SizeY 的平方双连通边链表 DoublyConnectedEdgeList，
    /// 并使用固定的随机数生成种子 @a Seed 在内部生成随机连接。
    static TUniquePtr<DoublyConnectedEdgeList> Generate(uint32 SizeX, uint32 SizeY, int32 Seed);
  };

} // namespace MapGen
