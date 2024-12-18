// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/geom/Vector3D.h>
#include <carla/geom/Math.h>
#include <carla/geom/BoundingBox.h>
#include <carla/geom/Transform.h>
#include <limits>
// 定义一个名为carla的命名空间，用于组织相关的代码和类型
namespace carla {
// 在carla命名空间内部，再定义一个名为geom的子命名空间
namespace geom {
// 重载std::ostream的插入运算符(<<)，使其能够处理const Vector3D&类型的参数  
// 这个函数允许我们将Vector3D对象的内容输出到std::ostream对象（如std::cout）中
  std::ostream &operator<<(std::ostream &out, const Vector3D &vector3D) {
  // 使用输出流out输出Vector3D对象的坐标值  
  // 输出格式为：{x=x值, y=y值, z=z值}  
  // 其中x、y、z是Vector3D对象中存储的三个坐标分量
      out << "{x=" << vector3D.x << ", y=" << vector3D.y << ", z=" << vector3D.z << '}';
      // 返回输出流out的引用，这允许进行链式调用
      return out;
  }

} // namespace geom
} // namespace carla

using namespace carla::geom;

//验证当变换矩阵是单位矩阵（即没有任何变换）时，一个点经过变换后是否保持不变
TEST(geom, single_point_no_transform) {
  constexpr double error = 0.001;

  Location translation (0.0, 0.0, 0.0);//平移变换
  Rotation rotation(0.0, 0.0, 0.0);//旋转变换
  Transform transform (translation, rotation);//单位变换

  Location point (1.0,1.0,1.0);//创建一个点
  transform.TransformPoint(point);//将 point 点应用变换
  Location result_point(1.0, 1.0, 1.0);//表示变换后的点，这里变换后的点与原点相同

  //检查它们是否在指定的误差范围内相等
  ASSERT_NEAR(point.x, result_point.x, error);
  ASSERT_NEAR(point.y, result_point.y, error);
  ASSERT_NEAR(point.z, result_point.z, error);

}
// 使用 TEST 宏定义一个名为 "geom" 的测试组中的 "single_point_translation" 测试用例，用于验证当仅进行平移变换时，点的坐标是否按照预期进行了正确的平移变化。
TEST(geom, single_point_translation) {
  constexpr double error = 0.001;
      // 创建一个表示平移变换的 Location 对象，坐标分别为 (2.0, 5.0, 7.0)，意味着在 x、y、z 方向上分别要进行相应单位的平移操作。
  Location translation (2.0,5.0,7.0);
      // 创建一个表示旋转变换的 Rotation 对象，角度值均为 0.0，表示没有旋转操作，因为本测试用例只关注平移变换的效果。
  Rotation rotation (0.0, 0.0, 0.0);
      // 使用上述的平移和旋转对象创建一个 Transform 对象，表示仅包含平移操作的坐标变换。
  Transform transform (translation, rotation);
  
    // 创建一个坐标为 (0.0, 0.0, 0.0) 的 Location 对象作为原始点，用于观察经过平移变换后的坐标变化情况。
  Location point (0.0, 0.0, 0.0);
      // 对原始点应用坐标变换，使其按照设定的平移量进行平移。
  transform.TransformPoint(point);
      // 创建一个表示预期平移后结果的 Location 对象，坐标为 (2.0, 5.0, 7.0)，即按照前面设定的平移量应该得到的坐标值。
  Location result_point(2.0, 5.0, 7.0);
  
    // 检查实际变换后的点（point）和预期结果（result_point）在 x 坐标上是否在指定的误差范围内相等，若超出误差范围则测试失败并输出提示信息。
  ASSERT_NEAR(point.x, result_point.x, error);
      // 检查 y 坐标是否在误差范围内相等。
  ASSERT_NEAR(point.y, result_point.y, error);
      // 检查 z 坐标是否在误差范围内相等，以此来验证平移变换是否正确生效。
  ASSERT_NEAR(point.z, result_point.z, error);
}


TEST(geom, single_point_transform_inverse_transform_coherence) {
  constexpr double error = 0.001；
  
    // 创建一个表示空间中某一点的 Location 对象，坐标值分别为 (-3.14f, 1.337f, 4.20f)，作为原始点，这里使用 float 类型的数值进行初始化，后续会根据实际情况进行相应的类型转换和计算。
  const Location point(-3.14f, 1.337f, 4.20f);
      // 创建一个表示平移变换的 Location 对象，坐标分别为 (1.41f, -4.7f, 9.2f)，用于构建坐标变换中的平移部分。
  const Location translation (1.41f, -4.7f, 9.2f);
  const Rotation rotation (-47.0f, 37.0f, 250.2f);
      // 使用上述的平移和旋转对象创建一个 Transform 对象，表示完整的坐标变换操作。
  const Transform transform (translation, rotation);
    // 创建一个变量 transformed_point，并初始化为原始点 point 的值，用于存储经过坐标变换后的点坐标。
  auto transformed_point = point;
      // 对 transformed_point 应用坐标变换，使其按照设定的变换操作进行坐标变换。
  transform.TransformPoint(transformed_point);
    // 创建一个变量 point_back_to_normal，并初始化为变换后的点 transformed_point 的值，用于存储经过逆变换后的点坐标，目的是看能否通过逆变换恢复到原始点坐标。
  auto point_back_to_normal = transformed_point;
      // 对 point_back_to_normal 应用逆变换操作，尝试将其恢复到原始坐标位置。
  transform.InverseTransformPoint(point_back_to_normal);
  
    // 使用 ASSERT_NEAR 宏检查经过逆变换后的点（point_back_to_normal）和原始点（point）在 x 坐标上是否在指定的误差范围内相等，如果不相等则输出相应的提示信息，提示实际得到的 x 坐标值以及预期的 x 坐标值，方便排查问题。
  ASSERT_NEAR(point.x, point_back_to_normal.x, error) << "result.x is " << point_back_to_normal.x << " but expected " << point.x;
      // 同理，检查 y 坐标是否在误差范围内相等，并输出相应提示信息。
  ASSERT_NEAR(point.y, point_back_to_normal.y, error) << "result.y is " << point_back_to_normal.y << " but expected " << point.y;
      // 检查 z 坐标是否在误差范围内相等，并输出相应提示信息，通过对三个坐标维度的检查来全面验证变换和逆变换操作的连贯性和正确性。
  ASSERT_NEAR(point.z, point_back_to_normal.z, error) << "result.z is " << point_back_to_normal.z << " but expected " << point.z;
}

// 使用 TEST 宏定义一个名为 "geom" 的测试组中的 "bbox_get_local_vertices_get_world_vertices_coherence" 测试用例，用于验证包围盒（BoundingBox）的本地顶点坐标和经过坐标变换后的世界坐标顶点之间的一致性，即检查通过不同方式获取的顶点坐标是否符合坐标变换的逻辑关系。
TEST(geom, bbox_get_local_vertices_get_world_vertices_coherence) {
  constexpr double error = 0.001;
    // 创建一个 BoundingBox 对象，通过给定一个中心点坐标 (10.2f, -32.4f, 15.6f) 和一个表示尺寸的 Vector3D 对象 (9.2f, 13.5f, 20.3f) 来初始化包围盒，确定了包围盒在本地坐标系下的位置和大小情况。
  const BoundingBox bbox (Location(10.2f, -32.4f, 15.6f), Vector3D(9.2f, 13.5f, 20.3f));
    // 创建一个表示坐标变换中平移部分的 Location 对象，坐标值分别为 (-3.14f, 1.337f, 4.20f)，用于确定包围盒在世界坐标系中的平移情况。
  const Location bbox_location(-3.14f, 1.337f, 4.20f);
      // 创建一个表示坐标变换中旋转部分的 Rotation 对象，角度值分别为 (-59.0f, 17.0f, -650.2f)，用于确定包围盒在世界坐标系中的旋转情况，这样就构建了一个针对包围盒的坐标变换操作。
  const Rotation bbox_rotation (-59.0f, 17.0f, -650.2f);
      // 使用上述的平移和旋转对象创建一个 Transform 对象，表示包围盒从本地坐标系到世界坐标系的坐标变换操作。
  const Transform bbox_transform(bbox_location, bbox_rotation);
  
    // 调用 BoundingBox 对象的 GetLocalVertices 方法，获取包围盒在本地坐标系下的所有顶点坐标，返回的是一个包含多个 Vector3D 类型顶点的容器（具体类型取决于 GetLocalVertices 方法的实现，这里推测是某种向量容器，比如 std::vector<Vector3D>）。
  const auto local_vertices = bbox.GetLocalVertices();
      // 调用 BoundingBox 对象的 GetWorldVertices 方法，传入前面构建的坐标变换对象 bbox_transform，获取包围盒经过坐标变换后在世界坐标系下的所有顶点坐标，同样返回一个顶点坐标的容器。
  const auto world_vertices = bbox.GetWorldVertices(bbox_transform);
  
      // 通过循环遍历本地顶点坐标容器中的每个顶点，依次检查每个本地顶点经过坐标变换后是否与对应的世界坐标顶点在误差范围内相等，以此来验证坐标变换的正确性和顶点坐标的一致性。
  for (auto i = 0u; i < local_vertices.size(); ++i){
       // 获取当前遍历到的本地顶点坐标的引用，方便后续操作和避免不必要的拷贝。
      const auto &local_vertex = local_vertices[i];
    
      // 创建一个变量 transformed_local_vertex，并初始化为当前的本地顶点坐标 local_vertex，用于存储经过坐标变换后的本地顶点坐标，目的是与对应的世界坐标顶点进行比较。
      auto transformed_local_vertex = local_vertex;
      // 对 transformed_local_vertex 应用前面定义的包围盒坐标变换操作，使其转换到世界坐标系下的坐标位置。
      bbox_transform.TransformPoint(transformed_local_vertex);
    
      // 获取当前遍历到的世界坐标顶点坐标的引用，用于和经过变换后的本地顶点坐标进行比较。
      const auto &world_vertex = world_vertices[i];

      ASSERT_NEAR(transformed_local_vertex.x, world_vertex.x, error) << "result.x is " << transformed_local_vertex.x << " but expected " << world_vertex.x;
      ASSERT_NEAR(transformed_local_vertex.y, world_vertex.y, error) << "result.y is " << transformed_local_vertex.y << " but expected " << world_vertex.y;
      ASSERT_NEAR(transformed_local_vertex.z, world_vertex.z, error) << "result.z is " << transformed_local_vertex.z << " but expected " << world_vertex.z;
  }
}


TEST(geom, single_point_rotation) {
  constexpr double error = 0.001;

  Location translation (0.0,0.0,0.0);
  Rotation rotation (0.0,180.0,0.0); // y z x
  Transform transform (translation, rotation);

  Location point (0.0, 0.0, 1.0);
  transform.TransformPoint(point);
  Location result_point(0.0, 0.0, 1.0);
  ASSERT_NEAR(point.x, result_point.x, error);
  ASSERT_NEAR(point.y, result_point.y, error);
  ASSERT_NEAR(point.z, result_point.z, error);
}
      // 使用 TEST 宏定义一个名为 "geom" 的测试组中的 "single_point_rotation" 测试用例，用于验证当仅进行旋转变换时，点的坐标是否按照预期进行了正确的旋转变化，这里测试的是绕 y 轴旋转 180 度的情况。
TEST(geom, single_point_translation_and_rotation) {
  constexpr double error = 0.001;

  Location translation (0.0,0.0,-1.0); // x y z
  Rotation rotation (90.0,0.0,0.0); // y z x
  Transform transform (translation, rotation);

  Location point (0.0, 0.0, 2.0);
  transform.TransformPoint(point);
  Location result_point(-2.0, 0.0, -1.0);
  ASSERT_NEAR(point.x, result_point.x, error);
  ASSERT_NEAR(point.y, result_point.y, error);
  ASSERT_NEAR(point.z, result_point.z, error);
}

TEST(geom, distance) {
  constexpr double error = .01;
  ASSERT_NEAR(Math::Distance({0, 0, 0}, {0, 0, 0}), 0.0, error);
  ASSERT_NEAR(Math::Distance({1, 1, 1}, {0, 0, 0}), 1.732051, error);
  ASSERT_NEAR(Math::Distance({0, 0, 0}, {1, 1, 1}), 1.732051, error);
  ASSERT_NEAR(Math::Distance({-1, -1, -1}, {0, 0, 0}), 1.732051, error);
  ASSERT_NEAR(Math::Distance({0, 0, 0}, {-1, -1, -1}), 1.732051, error);
  ASSERT_NEAR(Math::Distance({7, 4, 3}, {17, 6, 2}), 10.246951, error);
  ASSERT_NEAR(Math::Distance({7, -4, 3}, {-17, 6, 2}), 26.019224, error);
  ASSERT_NEAR(Math::Distance({5, 6, 7}, {-6, 3, -4}), 15.84298, error);
  ASSERT_NEAR(Math::Distance({7, 4, 3}, {17, 6, 2}), 10.246951, error);
}

TEST(geom, nearest_point_segment) {
  const float segment[] = {
     0,  0, 10,  0,
     2,  5, 10,  8,
    -6,  8,  8, -2,
     8,  2,-10,  3,
     3,  3, -6, -5,
     3, -3,  2,  5,
     4, -6,  5,  4,
    -1, -4,-10,  8,
    -7, -5,  5,  5,
    -5,  6,  3, -9
  };

  const Vector3D point[] = {
    {  1,  -1, 0},
    { 10,  10, 0},
    {-10,  10, 0},
    { 10, -10, 0},
    {-10, -10, 0},
    {  0,   5, 0},
    {  0,  -5, 0},
    {  1,   4, 0},
    { -1,   1, 0},
    {  3, 2.5, 0}
  };

  const int results[] = {
    0, 1, 7, 9, 8, 2, 9, 2, 8, 3
  };

  for (int i = 0; i < 10; ++i) {
    double min_dist = std::numeric_limits<double>::max();
    int id = -1;
    for (int j = 0; j < 40; j += 4) {
      const double dist = Math::DistanceSegmentToPoint(
          point[i],
          {segment[j + 0], segment[j + 1], 0},
          {segment[j + 2], segment[j + 3], 0}).second;
      if (dist < min_dist) {
        min_dist = dist;
        id = j / 4;
      }
    }
    ASSERT_EQ(id, results[i]) << "Fails point number: " << i;
  }
}

TEST(geom, forward_vector) {
  auto compare = [](Rotation rotation, Vector3D expected) {
    constexpr float eps = 2.0f * std::numeric_limits<float>::epsilon();
    auto result = rotation.GetForwardVector();
    EXPECT_TRUE(
            (std::abs(expected.x - result.x) < eps) &&
            (std::abs(expected.y - result.y) < eps) &&
            (std::abs(expected.z - result.z) < eps))
        << "result   = " << result << '\n'
        << "expected = " << expected;
  };
  //        pitch     yaw    roll       x     y     z
  compare({  0.0f,   0.0f,   0.0f}, {1.0f, 0.0f, 0.0f});
  compare({  0.0f,   0.0f, 123.0f}, {1.0f, 0.0f, 0.0f});
  compare({360.0f, 360.0f,   0.0f}, {1.0f, 0.0f, 0.0f});
  compare({  0.0f,  90.0f,   0.0f}, {0.0f, 1.0f, 0.0f});
  compare({  0.0f, -90.0f,   0.0f}, {0.0f,-1.0f, 0.0f});
  compare({ 90.0f,   0.0f,   0.0f}, {0.0f, 0.0f, 1.0f});
  compare({180.0f, -90.0f,   0.0f}, {0.0f, 1.0f, 0.0f});
}

TEST(geom, nearest_point_arc) {
  ASSERT_NEAR(Math::DistanceArcToPoint(Vector3D(1,0,0),
      Vector3D(0,0,0), 1.57f, 0, 1).second, 0.414214f, 0.01f);
  ASSERT_NEAR(Math::DistanceArcToPoint(Vector3D(2,1,0),
      Vector3D(0,0,0), 1.57f, 0, 1).second, 1.0f, 0.01f);
  ASSERT_NEAR(Math::DistanceArcToPoint(Vector3D(0,1,0),
      Vector3D(0,0,0), 1.57f, 0, 1).second, 1.0f, 0.01f);
  ASSERT_NEAR(Math::DistanceArcToPoint(Vector3D(1,2,0),
      Vector3D(0,0,0), 1.57f, 0, 1).second, 1.0f, 0.01f);
}
