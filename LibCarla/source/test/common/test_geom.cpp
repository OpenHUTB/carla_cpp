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

//这段代码包含了两个单元测试，分别测试了旋转变换和平移加旋转变换对 3D 点的影响。
//第一个测试验证了绕 Y 轴旋转 180 度时，点的 Z 坐标是否正确变换。
//第二个测试验证了绕 X 轴旋转 90 度并平移后的点坐标是否正确变化。
//每个测试用例都使用 ASSERT_NEAR 来确保变换后的坐标与预期值相符，允许浮点数计算上的小误差。
TEST(geom, single_point_translation_and_rotation) {
  constexpr double error = 0.001;  // 设定允许的误差范围
  // 创建平移变换，(0.0, 0.0, -1.0) 表示点在 Z 轴上向下平移 1 单位
  Location translation (0.0, 0.0, -1.0); 
  // 创建旋转变换，(90.0, 0.0, 0.0) 表示绕 X 轴旋转 90 度
  Rotation rotation (90.0, 0.0, 0.0); 
  // 创建一个变换对象，将平移和旋转组合在一起
  Transform transform (translation, rotation);
  // 定义一个点 (0.0, 0.0, 2.0)，该点位于 Z 轴上，离原点有 2 个单位
  Location point (0.0, 0.0, 2.0);
  // 应用变换到该点，transform.TransformPoint(point) 修改 `point` 的值
  transform.TransformPoint(point);
  // 定义预期的结果点，绕 X 轴旋转 90 度后，点应该变为 (-2.0, 0.0, -1.0)
  Location result_point(-2.0, 0.0, -1.0);
  // 使用 ASSERT_NEAR 来比较实际的点坐标与预期坐标，误差范围是 0.001
  ASSERT_NEAR(point.x, result_point.x, error);  // 检查 x 坐标
  ASSERT_NEAR(point.y, result_point.y, error);  // 检查 y 坐标
  ASSERT_NEAR(point.z, result_point.z, error);  // 检查 z 坐标
}

TEST(geom, distance) {
  // 定义一个常量 error，用于在断言中指定容忍的误差范围
  constexpr double error = .01;
  // 测试计算两个点之间的距离是否为 0.0，使用 ASSERT_NEAR 检查实际值与期望值的差异是否在误差范围内
  ASSERT_NEAR(Math::Distance({0, 0, 0}, {0, 0, 0}), 0.0, error);
  // 测试计算点 (1, 1, 1) 和 (0, 0, 0) 之间的欧几里得距离，期望值为 1.732051（即 sqrt(3)）
  ASSERT_NEAR(Math::Distance({1, 1, 1}, {0, 0, 0}), 1.732051, error);
  // 测试计算点 (0, 0, 0) 和 (1, 1, 1) 之间的欧几里得距离，期望值与上一测试相同
  ASSERT_NEAR(Math::Distance({0, 0, 0}, {1, 1, 1}), 1.732051, error);
 // 测试计算点 (-1, -1, -1) 和 (0, 0, 0) 之间的欧几里得距离，期望值为 1.732051
  ASSERT_NEAR(Math::Distance({-1, -1, -1}, {0, 0, 0}), 1.732051, error);
 // 测试计算点 (0, 0, 0) 和 (-1, -1, -1) 之间的欧几里得距离，期望值与上一测试相同
  ASSERT_NEAR(Math::Distance({0, 0, 0}, {-1, -1, -1}), 1.732051, error);
   // 测试计算点 (7, 4, 3) 和 (17, 6, 2) 之间的欧几里得距离，期望值为 10.246951
  ASSERT_NEAR(Math::Distance({7, 4, 3}, {17, 6, 2}), 10.246951, error);
    // 测试计算点 (7, -4, 3) 和 (-17, 6, 2) 之间的欧几里得距离，期望值为 26.019224
  ASSERT_NEAR(Math::Distance({7, -4, 3}, {-17, 6, 2}), 26.019224, error);
  // 测试计算点 (5, 6, 7) 和 (-6, 3, -4) 之间的欧几里得距离，期望值为 15.84298
  ASSERT_NEAR(Math::Distance({5, 6, 7}, {-6, 3, -4}), 15.84298, error);
    // 再次测试计算点 (7, 4, 3) 和 (17, 6, 2) 之间的欧几里得距离，期望值与之前相同
  ASSERT_NEAR(Math::Distance({7, 4, 3}, {17, 6, 2}), 10.246951, error);
}

//求最近点到线段的距离
TEST(geom, nearest_point_segment) {
  // 定义一个包含 3D 空间中多个线段的数组。每个线段由两个点表示，
  // 每个点有 3 个坐标 (x, y, z)，所以每个线段有 6 个数字表示。
  const float segment[] = {
     0,  0, 10,  0,// 第一个线段的起点 (0, 0, 10)，终点 (0, 0, 0)
     2,  5, 10,  8,// 第二个线段的起点 (2, 5, 10)，终点 (10, 8)
    -6,  8,  8, -2,// 第三个线段的起点 (-6, 8, 8)，终点 (8, -2)
     8,  2,-10,  3,// 第四个线段的起点 (8, 2, -10)，终点 (3, 3)
     3,  3, -6, -5,// 第五个线段的起点 (3, 3, -6)，终点 (-5, -5)
     3, -3,  2,  5,// 第六个线段的起点 (3, -3, 2)，终点 (2, 5)
     4, -6,  5,  4,// 第七个线段的起点 (4, -6, 5)，终点 (4, 4)
    -1, -4,-10,  8, // 第八个线段的起点 (-1, -4, -10)，终点 (8, 8)
    -7, -5,  5,  5, // 第九个线段的起点 (-7, -5, 5)，终点 (5, 5)
    -5,  6,  3, -9 // 第十个线段的起点 (-5, 6, 3)，终点 (-9, -9)
  };

  //这段代码定义了一个名为 point 的数组，其中包含了 10 个二维点，每个点都是一个 Vector3D 类型的对象。虽然这些点在 3D 空间中定义，但由于 z 坐标全部为零，所以它们实际上都位于 XY 平面内
  const Vector3D point[] = {
    {  1,  -1, 0},  // 第一个点，坐标为 (1, -1, 0)
    { 10,  10, 0},  // 第二个点，坐标为 (10, 10, 0)
    {-10,  10, 0},  // 第三个点，坐标为 (-10, 10, 0)
    { 10, -10, 0},  // 第四个点，坐标为 (10, -10, 0)
    {-10, -10, 0},  // 第五个点，坐标为 (-10, -10, 0)
    {  0,   5, 0},  // 第六个点，坐标为 (0, 5, 0)
    {  0,  -5, 0},  // 第七个点，坐标为 (0, -5, 0)
    {  1,   4, 0},  // 第八个点，坐标为 (1, 4, 0)
    { -1,   1, 0},  // 第九个点，坐标为 (-1, 1, 0)
    {  3,  2.5, 0}  // 第十个点，坐标为 (3, 2.5, 0)
};

  //这段代码定义了一个常量整型数组 results，它包含了 10 个整数，初始化为 { 0, 1, 7, 9, 8, 2, 9, 2, 8, 3 }。由于是常量数组，数组的元素在程序中不可更改。这个数组可能用于存储某种固定的结果或用于其他计算。
  const int results[] = {
    0, 1, 7, 9, 8, 2, 9, 2, 8, 3
  };

//这段代码的主要任务是计算每个点到一系列线段的最短距离，并将计算出的最近线段的索引与预期的 results[i] 进行比较。
//如果计算的结果与预期不符，ASSERT_EQ 会使测试失败并输出详细的错误信息，帮助开发人员定位问题
for (int i = 0; i < 10; ++i) {  // 外层循环：遍历每个点（共有 10 个点）
    double min_dist = std::numeric_limits<double>::max();  // 初始化 min_dist 为一个非常大的值，表示最小距离
    int id = -1;  // 用于记录对应线段的索引

    for (int j = 0; j < 40; j += 4) {  // 内层循环：遍历所有线段，每个线段由 4 个元素表示（起点和终点的 x, y 坐标）
      
      // 计算点 point[i] 到当前线段的最小距离
      const double dist = Math::DistanceSegmentToPoint(
          point[i],  // 当前的点
          {segment[j + 0], segment[j + 1], 0},  // 当前线段的起点 (x1, y1, 0)，注意 z 坐标固定为 0
          {segment[j + 2], segment[j + 3], 0}   // 当前线段的终点 (x2, y2, 0)，注意 z 坐标固定为 0
      ).second;  // DistanceSegmentToPoint 返回的是一个 pair，dist 为返回的第二个元素，即最小距离

      // 如果当前计算的距离比已有的最小距离还小，则更新 min_dist 和 id
      if (dist < min_dist) {
        min_dist = dist;
        id = j / 4;  // 通过 j / 4 获取当前线段的索引（每个线段由 4 个数字表示）
      }
    }

    // 断言：检查计算的线段索引 id 是否与预期的结果 matches `results[i]`
    ASSERT_EQ(id, results[i]) << "Fails point number: " << i;  // 如果 id 不等于预期的 results[i]，会输出失败信息
}

  //这段代码的目标是测试 Rotation 类的 GetForwardVector() 方法是否正确地计算了旋转后的前向向量。
  //因为浮点数的精度问题，直接使用 == 来比较浮点数是不可靠的，所以这里使用了绝对差值比较，确保两者差异在可以接受的范围内。
  //通过 EXPECT_TRUE 宏进行断言，如果比较失败，将输出相关的调试信息，帮助开发人员诊断问题
  TEST(geom, forward_vector) {
    // 定义一个 lambda 函数 compare，用于比较旋转向量与预期值是否相等
    auto compare = [](Rotation rotation, Vector3D expected) {
        // 设置一个浮动误差范围 epsilon，通常用于浮点数比较
        constexpr float eps = 2.0f * std::numeric_limits<float>::epsilon();

        // 获取由 rotation 对象计算出的前向向量（即旋转后的方向向量）
        auto result = rotation.GetForwardVector();

        // 使用 EXPECT_TRUE 来判断两个浮点数向量 result 和 expected 是否足够接近
        // 通过绝对值的差异与 eps 比较，判断是否在误差范围内
        EXPECT_TRUE(
            (std::abs(expected.x - result.x) < eps) &&
            (std::abs(expected.y - result.y) < eps) &&
            (std::abs(expected.z - result.z) < eps))
            << "result   = " << result << '\n'  // 如果断言失败，输出实际的计算结果
            << "expected = " << expected;      // 如果断言失败，输出预期的值
    };

    // 这里通常会继续调用 compare() 函数进行具体的测试，例如：
    // compare(rotationInstance, expectedVector);
}
  //       物体旋转的角度（姿态）    x     y     z
 // 比较两个向量：{0.0f, 0.0f, 0.0f} 和 {1.0f, 0.0f, 0.0f}
// 这里测试的是零向量和单位向量的比较，期望的结果是 {1.0f, 0.0f, 0.0f}
//这段代码测试的是不同的旋转角度下物体的方向向量（通常是前向向量）的变化。
  compare({0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f});
// 比较两个向量：{0.0f, 0.0f, 123.0f} 和 {1.0f, 0.0f, 0.0f}
// 这里测试的是 {0.0f, 0.0f, 123.0f} 和单位向量的比较，期望结果为 {1.0f, 0.0f, 0.0f}
compare({0.0f, 0.0f, 123.0f}, {1.0f, 0.0f, 0.0f});
// 比较两个向量：{360.0f, 360.0f, 0.0f} 和 {1.0f, 0.0f, 0.0f}
// 这里测试的是包含大角度旋转（360度）的向量，期望的结果是 {1.0f, 0.0f, 0.0f}。
// 注意，360度的旋转会回到原始位置。
compare({360.0f, 360.0f, 0.0f}, {1.0f, 0.0f, 0.0f});
// 比较两个向量：{0.0f, 90.0f, 0.0f} 和 {0.0f, 1.0f, 0.0f}
// 这里测试的是绕 Y 轴旋转 90 度后的结果，期望结果是 {0.0f, 1.0f, 0.0f}。
// 这是因为 90度的旋转将会改变方向，使得向量沿 Y 轴指向上方。
compare({0.0f, 90.0f, 0.0f}, {0.0f, 1.0f, 0.0f});

// 比较两个向量：{0.0f, -90.0f, 0.0f} 和 {0.0f, -1.0f, 0.0f}
// 这里测试的是绕 Y 轴旋转 -90 度后的结果，期望结果是 {0.0f, -1.0f, 0.0f}。
// 旋转 -90度会使得向量指向 Y 轴的负方向。
compare({0.0f, -90.0f, 0.0f}, {0.0f, -1.0f, 0.0f});
// 比较两个向量：{90.0f, 0.0f, 0.0f} 和 {0.0f, 0.0f, 1.0f}
// 这里测试的是绕 X 轴旋转 90 度后的结果，期望结果是 {0.0f, 0.0f, 1.0f}。
// 旋转 90度后，前向向量会指向 Z 轴的正方向。
compare({90.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});
// 比较两个向量：{180.0f, -90.0f, 0.0f} 和 {0.0f, 1.0f, 0.0f}
// 这里测试的是绕 Y 轴旋转 -90度并且绕 X 轴旋转 180度后的结果。
// 期望的结果是 {0.0f, 1.0f, 0.0f}，这意味着旋转后的前向向量沿 Y 轴的正方向。
compare({180.0f, -90.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
}

///检查 Math::DistanceArcToPoint 函数能否准确计算点到圆弧的距离，确保其功能在各种情况下都能如预期工作
TEST(geom, nearest_point_arc) {
  // 测试 1：计算点 (1, 0, 0) 到圆弧的距离
  // 圆弧起点在 (0, 0, 0)，圆弧的角度跨度是 1.57 弧度，圆弧的半径为 1
  // 期望距离值为 0.414214，误差容忍范围为 0.01
  ASSERT_NEAR(Math::DistanceArcToPoint(Vector3D(1, 0, 0),  // 点坐标 (1, 0, 0)
      Vector3D(0, 0, 0),  // 圆弧起点 (0, 0, 0)
      1.57f,  // 圆弧角度跨度 (弧度)
      0,  // 圆弧的起始角度
      1).second,  // 圆弧的半径为 1
      0.414214f,  // 预期的距离值
      0.01f);  // 容忍的误差范围

  // 测试 2：计算点 (2, 1, 0) 到圆弧的距离
  // 圆弧起点在 (0, 0, 0)，圆弧的角度跨度是 1.57 弧度，圆弧的半径为 1
  // 期望距离值为 1.0，误差容忍范围为 0.01
  ASSERT_NEAR(Math::DistanceArcToPoint(Vector3D(2, 1, 0),  // 点坐标 (2, 1, 0)
      Vector3D(0, 0, 0),  // 圆弧起点 (0, 0, 0)
      1.57f,  // 圆弧角度跨度 (弧度)
      0,  // 圆弧的起始角度
      1).second,  // 圆弧的半径为 1
      1.0f,  // 预期的距离值
      0.01f);  // 容忍的误差范围

  // 测试 3：计算点 (0, 1, 0) 到圆弧的距离
  // 圆弧起点在 (0, 0, 0)，圆弧的角度跨度是 1.57 弧度，圆弧的半径为 1
  // 期望距离值为 1.0，误差容忍范围为 0.01
  ASSERT_NEAR(Math::DistanceArcToPoint(Vector3D(0, 1, 0),  // 点坐标 (0, 1, 0)
      Vector3D(0, 0, 0),  // 圆弧起点 (0, 0, 0)
      1.57f,  // 圆弧角度跨度 (弧度)
      0,  // 圆弧的起始角度
      1).second,  // 圆弧的半径为 1
      1.0f,  // 预期的距离值
      0.01f);  // 容忍的误差范围

  // 测试 4：计算点 (1, 2, 0) 到圆弧的距离
  // 圆弧起点在 (0, 0, 0)，圆弧的角度跨度是 1.57 弧度，圆弧的半径为 1
  // 期望距离值为 1.0，误差容忍范围为 0.01
  ASSERT_NEAR(Math::DistanceArcToPoint(Vector3D(1, 2, 0),  // 点坐标 (1, 2, 0)
      Vector3D(0, 0, 0),  // 圆弧起点 (0, 0, 0)
      1.57f,  // 圆弧角度跨度 (弧度)
      0,  // 圆弧的起始角度
      1).second,  // 圆弧的半径为 1
      1.0f,  // 预期的距离值
      0.01f);  // 容忍的误差范围
}
