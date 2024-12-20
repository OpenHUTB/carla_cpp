#pragma once
// 引入名为“DataStructs.h”的头文件，通常其中包含了该代码所依赖的一些数据结构等相关的定义
#include "DataStructs.h"
// 引入名为“DataStructs.h”的头文件，通常其中包含了该代码所依赖的一些数据结构等相关的定义
namespace MeshReconstruction
{
// 定义名为“IntersectInfo”的结构体，用于存储与相交相关的信息
  struct IntersectInfo
  {
// 用于存储符号配置，取值范围是0 - 255，可能用于表示某种特定的状态或标识
    // 0 - 255
    int signConfig;

    // 如果存在，则第i条边的顶点存储在第i个位置
    //关于边的编号和位置，请参见numberings.png
    std::array<Vec3, 12> edgeVertIndices;
  };
//这段代码定义了一个名为 Cube 的类这段代码定义了一个名为 Cube 的类
  class Cube
  {
    Vec3 pos[8];// 存储立方体的8个顶点的位置。
    double sdf[8];
// 私有成员函数，用于对两个顶点进行线性插值计算得到新的顶点位置
    Vec3 LerpVertex(double isoLevel, int i1, int i2) const;
// 私有成员函数，根据给定的等值面水平值（isoLevel）计算符号配置
    int SignConfig(double isoLevel) const;

  public:
// 构造函数，用于初始化立方体对象，接受一个Rect3类型
    Cube(Rect3 const &space, Fun3s const &sdf);
// 成员函数，用于找出曲面与立方体相交的顶点信息，返回一个IntersectInfo结构体对象
    // 找出曲面与立方体相交的顶点。
    IntersectInfo Intersect(double isoLevel = 0) const;
  };
// 在“MeshReconstruction”命名空间内定义一个未命名的内部命名
  namespace
  {
    // Cube有8个顶点。每个顶点可以有正号或负号
    // 在每种情况下，2^8=256种可能的配置映射到相交的边
    // 这12条边被编号为1,2,4,2048，并为每个配置存储为一个12位的位串
    const int signConfigToIntersectedEdges[256] = {
        0x0, 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,
        0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
        0x190, 0x99, 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
        0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
        0x230, 0x339, 0x33, 0x13a, 0x636, 0x73f, 0x435, 0x53c,
        0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
        0x3a0, 0x2a9, 0x1a3, 0xaa, 0x7a6, 0x6af, 0x5a5, 0x4ac,
        0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
        0x460, 0x569, 0x663, 0x76a, 0x66, 0x16f, 0x265, 0x36c,
        0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
        0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0xff, 0x3f5, 0x2fc,
        0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
        0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55, 0x15c,
        0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
        0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc,
        0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
        0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc,
        0xcc, 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
        0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c,
        0x15c, 0x55, 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
        0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc,
        0x2fc, 0x3f5, 0xff, 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
        0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
        0x36c, 0x265, 0x16f, 0x66, 0x76a, 0x663, 0x569, 0x460,
        0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,
        0x4ac, 0x5a5, 0x6af, 0x7a6, 0xaa, 0x1a3, 0x2a9, 0x3a0,
        0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c,
        0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33, 0x339, 0x230,
        0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c,
        0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x99, 0x190,
        0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c,
        0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x0};
// 定义名为“Edge”的结构体，用于描述立方体的边相关信息
    struct Edge
    {
// 用于存储边的标志，使用位域（bit-field）定义，占12位
      int edgeFlag : 12; // flag: 1, 2, 4, ... 2048
// 存储边的一个顶点索引，取值范围是0 - 7，对应立方体8个顶点中的一个
      int vert0;         // 0-7
// 存储边的另一个顶点索引，取值范围同样是0 - 7
      int vert1;         // 0-7
    };
// 定义一个长度为12的Edge类型数组，用于表示立方体的12条边的具体信息
    const Edge edges[12] =
        {
            {1, 0, 1}, // edge 0
            {2, 1, 2}, // edge 1
            {4, 2, 3}, // ...
            {8, 3, 0},
            {16, 4, 5},
            {32, 5, 6},
            {64, 6, 7},
            {128, 7, 4},
            {256, 0, 4},
            {512, 1, 5},
            {1024, 2, 6},
            {2048, 3, 7} // edge 11
    };
  }
// “Cube”类的成员函数“LerpVertex”的定义，用于对两个顶点进行线性插值计算得到新的顶点位置
  Vec3 Cube::LerpVertex(double isoLevel, int i1, int i2) const 
  {
// 获取索引为i1的顶点对应的SDF值，用于后续插值计算判断等操作
    auto const Eps = 1e-5; // 定义一个用于比较浮点数的精度阈值
    auto const v1 = sdf[i1]; // 定义一个用于比较浮点数的精度阈值
// 获取索引为i2的顶点对应的SDF值
    auto const v2 = sdf[i2];
// 获取索引为i1的顶点位置引用，用于后续插值计算新顶点位置
    auto const &p1 = pos[i1]; // 定义一个用于比较浮点数的精度阈值
// 获取索引为i2的顶点位置引用
    auto const &p2 = pos[i2];
    // 如果isoLevel与其中一个顶点的SDF值非常接近，则直接返回该顶点位置
    if (abs(isoLevel - v1) < Eps)
      return p1;
    if (abs(isoLevel - v2) < Eps)
      return p2;
    if (abs(v1 - v2) < Eps)
      return p1;

    auto mu = (isoLevel - v1) / (v2 - v1); // 计算线性插值参数
 // 根据线性插值公式，返回插值后的位置，即由两个顶点位置（p1和p2）
   return p1 + (p2 - p1) * mu; // 返回插值后的位置
  }
// “Cube”类的构造函数定义，用于初始化立方体对象的顶点位置和对应的SDF值
  Cube::Cube(Rect3 const &space, Fun3s const &sdf)
  {
// 获取给定空间范围（Rect3类型）的最小x坐标值，用于初始化立方体顶点位置
    auto mx = space.min.x;
// 获取最小y坐标值
    auto my = space.min.y;
// 获取最小z坐标值
    auto mz = space.min.z;
// 获取给定空间范围的x方向尺寸大小，用于确定立方体在x方向的跨度
    auto sx = space.size.x;
// 获取y方向尺寸大小
    auto sy = space.size.y;
// 获取z方向尺寸大小
    auto sz = space.size.z;
    // 初始化立方体的8个顶点位置
// 初始化立方体的第一个顶点位置，即空间范围的最小坐标点位置，对应立方体的一个角顶点
    pos[0] = space.min;
// 初始化第二个顶点位置，在x方向增加空间的x方向尺寸大小
    pos[1] = {mx + sx, my, mz};
// 依次初始化其他顶点位置，按照立方体顶点的相对位置关系进行赋值
    pos[2] = {mx + sx, my, mz + sz};
    pos[3] = {mx, my, mz + sz};
    pos[4] = {mx, my + sy, mz};
    pos[5] = {mx + sx, my + sy, mz};
    pos[6] = {mx + sx, my + sy, mz + sz};
    pos[7] = {mx, my + sy, mz + sz};
    // 计算每个顶点的SDF值
    for (auto i = 0; i < 8; ++i)
    {
      auto sd = sdf(pos[i]);
// 如果计算得到的SDF值为0，为了避免后续可能出现除以零等问题，给其增加一个极小的值1e-6
      if (sd == 0)
        sd += 1e-6; // 避免除以零
// 将计算并处理后的SDF值存储到对应顶点的sdf数组元素中
      this->sdf[i] = sd;
    }
  }
// “Cube”类的成员函数“SignConfig”的定义，用于根据给定的等值面水平值（isoLevel）计算符号配置
  int Cube::SignConfig(double isoLevel) const
  {
// 初始化一个用于记录符号配置的索引值，初始为0
    auto edgeIndex = 0;

    for (auto i = 0; i < 8; ++i)
    {
      if (sdf[i] < isoLevel)
      {
        edgeIndex |= 1 << i; // 将对应的顶点标记为穿过
      }
    }

    return edgeIndex;
  }

  IntersectInfo Cube::Intersect(double iso) const
  {
    // idea:
    // 从立方体的8个角的符号中计算出一个符号配置(256个可能的)
    // 这个配置可以用来索引到一个表中，这个表告诉12条边中哪条是相交的
    // 找到与边相邻的顶点并插值切割顶点并将其存储在Intersectionlnfo对象中

    IntersectInfo intersect;
    intersect.signConfig = SignConfig(iso);

    for (auto e = 0; e < 12; ++e)
    {
      if (signConfigToIntersectedEdges[intersect.signConfig] & edges[e].edgeFlag)
      {
        auto v0 = edges[e].vert0;
        auto v1 = edges[e].vert1;
        auto vert = LerpVertex(iso, v0, v1);
        intersect.edgeVertIndices[e] = vert;
      }
    }

    return intersect;
  }

}
