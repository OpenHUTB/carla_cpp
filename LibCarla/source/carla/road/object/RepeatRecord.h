// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // 确保这个头文件在一个编译单元中只被包含一次 

#include "carla/NonCopyable.h"  // 包含carla框架中的NonCopyable类，用于禁止拷贝操作 
#include <string>  // 包含标准库中的string类 
#include <vector>  // 包含标准库中vector类，用于处理动态数组 
#include "carla/road/RoadTypes.h"  // 包含carla框架中road模块的路型定义 

namespace carla {
namespace road {
namespace object {
  // RepeatRecord类定义，它继承自MovableNonCopyable,后者私有继承自NonCopyable,  
  // 意味着RepeatRecord类也不能被拷贝，但可以移动（如果MovableNonCopyable类实现了移动语义） 
  // 这个类被设计为只包含私有成员，表明其不打算被外部直接使用 
  class RepeatRecord : private MovableNonCopyable {
  public:
    // 默认构造函数，不执行任何操作 
    RepeatRecord() {}

  private:

    ObjId _id;
  };

} // object
} // road
} // carla
