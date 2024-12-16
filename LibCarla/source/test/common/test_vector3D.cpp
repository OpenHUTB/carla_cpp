// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"
#include "gtest/gtest-death-test.h"

#include <carla/geom/Vector3D.h>
#include <carla/geom/Math.h>

using namespace carla::geom;

// 统一测试向量归一化后的结果值
TEST(vector3D, make_unit_vec) {
  ASSERT_EQ(Vector3D(10,0,0).MakeUnitVector(), Vector3D(1,0,0));//测试向量 (10,0,0) 归一化后是否等于 (1,0,0)
  ASSERT_NE(Vector3D(10,0,0).MakeUnitVector(), Vector3D(0,1,0));//测试向量 (10,0,0) 归一化后是否不等于 (0,1,0)
  ASSERT_EQ(Vector3D(0,10,0).MakeUnitVector(), Vector3D(0,1,0));//测试向量 (0,10,0) 归一化后是否等于 (0,1,0)
  ASSERT_EQ(Vector3D(0,0,512).MakeUnitVector(), Vector3D(0,0,1));//测试向量 (0,0,512) 归一化后是否等于 (0,0,1)
  ASSERT_NE(Vector3D(0,1,512).MakeUnitVector(), Vector3D(0,0,1));//测试向量 (0,1,512) 归一化后是否不等于 (0,0,1)
#ifdef LIBCARLA_NO_EXCEPTIONS
  ASSERT_DEATH_IF_SUPPORTED(
      Vector3D().MakeUnitVector(),
      "length > 2.0f \\* std::numeric_limits<float>::epsilon()");//如果支持死亡测试,测试空向量调用MakeUnitVector()方法时程序是否会异常终止,并检查异常信息是否符合预期
#else
  ASSERT_THROW(
      Vector3D().MakeUnitVector(),
      std::runtime_error);//测试空向量调用 MakeUnitVector()方法时是否会抛出 std::runtime_error 异常
#endif // LIBCARLA_NO_EXCEPTIONS//用于处理在不使用异常的情况下的测试
}
