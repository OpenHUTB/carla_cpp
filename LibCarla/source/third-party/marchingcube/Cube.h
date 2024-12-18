#pragma once
#include "DataStructs.h"  // 包含自定义的数据结构头文件

namespace MeshReconstruction
{
  // IntersectInfo结构体用于存储与立方体相交的相关信息
  struct IntersectInfo
  {
    // signConfig表示符号配置，范围为0到255。用于存储立方体8个顶点的符号配置。
    int signConfig;

    // edgeVertIndices存储12条边相交的顶点位置（如果有交点的话）。
    // 每条边的两个端点通过插值计算得到交点。
    std::array<Vec3, 12> edgeVertIndices;
  };

  // Cube类表示一个立方体，包括8个顶点、符号场值（SDF）以及与等值面相交的相关计算方法。
  class Cube
  {
    Vec3 pos[8];     // 立方体的8个顶点的位置
    double sdf[8];   // 立方体8个顶点的符号场值（SDF）

    // LerpVertex函数用于计算两个顶点之间的插值顶点。它根据等值面（isoLevel）插值两个顶点。
    Vec3 LerpVertex(double isoLevel, int i1, int i2) const;

    // SignConfig函数根据8个顶点的SDF值计算符号配置（0到255之间的整数），
    // 用于描述哪些顶点在等值面之上，哪些在等值面之下。
    int SignConfig(double isoLevel) const;

  public:
    // Cube的构造函数，接收空间范围（Rect3）和符号场函数（Fun3s）来初始化立方体。
    Cube(Rect3 const &space, Fun3s const &sdf);

    // Intersect函数根据给定的isoLevel，计算立方体与等值面相交的顶点。
    IntersectInfo Intersect(double isoLevel = 0) const;
  };

  namespace
  {
    // signConfigToIntersectedEdges数组用于存储256种符号配置的边相交信息。
    // 每个整数代表一个符号配置，二进制位表示与12条边相交的情况（1表示交点存在，0表示没有交点）。
    const int signConfigToIntersectedEdges[256] = {
        0x0, 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,
        0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
        0x190, 0x99, 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
        0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
        // ...省略其他部分，直到256个配置
    };

    // Edge结构体表示一个立方体的边，每条边通过两个顶点（vert0和vert1）来定义。
    // edgeFlag是一个12位标志，每个位置表示该边是否与等值面相交。
    struct Edge
    {
      int edgeFlag : 12; // 12位标志位，用于表示每条边是否有交点
      int vert0;         // 边的起点，0-7表示8个顶点中的某个
      int vert1;         // 边的终点，0-7表示8个顶点中的某个
    };

    // edges数组定义了立方体的12条边。每个元素表示一条边，包含标志位（edgeFlag）和端点（vert0, vert1）。
    const Edge edges[12] =
        {
            {1, 0, 1}, // 边0：连接顶点0和顶点1
            {2, 1, 2}, // 边1：连接顶点1和顶点2
            {4, 2, 3}, // 边2：连接顶点2和顶点3
            {8, 3, 0}, // 边3：连接顶点3和顶点0
            {16, 4, 5}, // 边4：连接顶点4和顶点5
            {32, 5, 6}, // 边5：连接顶点5和顶点6
            {64, 6, 7}, // 边6：连接顶点6和顶点7
            {128, 7, 4}, // 边7：连接顶点7和顶点4
            {256, 0, 4}, // 边8：连接顶点0和顶点4
            {512, 1, 5}, // 边9：连接顶点1和顶点5
            {1024, 2, 6}, // 边10：连接顶点2和顶点6
            {2048, 3, 7}  // 边11：连接顶点3和顶点7
    };
  }

  // LerpVertex函数：用于计算在等值面isoLevel下，两个顶点之间的插值位置。
  Vec3 Cube::LerpVertex(double isoLevel, int i1, int i2) const
  {
    auto const Eps = 1e-5;  // 一个小的容差值，避免除零或计算误差

    auto const v1 = sdf[i1];  // 顶点1的符号场值
    auto const v2 = sdf[i2];  // 顶点2的符号场值
    auto const &p1 = pos[i1]; // 顶点1的位置
    auto const &p2 = pos[i2]; // 顶点2的位置

    // 如果符号场值接近isoLevel，则返回该顶点位置
    if (abs(isoLevel - v1) < Eps)
      return p1;
    if (abs(isoLevel - v2) < Eps)
      return p2;

    // 如果符号场值接近零，则返回顶点1
    if (abs(v1 - v2) < Eps)
      return p1;

    // 插值计算
    auto mu = (isoLevel - v1) / (v2 - v1);
    return p1 + (p2 - p1) * mu;
  }

  // Cube类的构造函数，根据传入的空间范围和符号场函数初始化立方体
  Cube::Cube(Rect3 const &space, Fun3s const &sdf)
  {
    auto mx = space.min.x;
    auto my = space.min.y;
    auto mz = space.min.z;

    auto sx = space.size.x;
    auto sy = space.size.y;
    auto sz = space.size.z;

    // 计算立方体8个顶点的坐标
    pos[0] = space.min;
    pos[1] = {mx + sx, my, mz};
    pos[2] = {mx + sx, my, mz + sz};
    pos[3] = {mx, my, mz + sz};
    pos[4] = {mx, my + sy, mz};
    pos[5] = {mx + sx, my + sy, mz};
    pos[6] = {mx + sx, my + sy, mz + sz};
    pos[7] = {mx, my + sy, mz + sz};

    // 计算每个顶点的符号场值
    for (auto i = 0; i < 8; ++i)
    {
      auto sd = sdf(pos[i]);
      if (sd == 0)  // 防止符号场值为零，添加一个小的偏移量
        sd += 1e-6;
      this->sdf[i] = sd;
    }
  }

  // SignConfig函数计算立方体的符号配置。每个顶点的符号场值与isoLevel进行比较，决定其是否在等值面上。
  int Cube::SignConfig(double isoLevel) const
  {
    auto edgeIndex = 0;

    // 比较8个顶点的符号场值与isoLevel，构建符号配置
    for (auto i = 0; i < 8; ++i)
    {
      if (sdf[i] < isoLevel)
      {
        edgeIndex |= 1 << i; // 如果该顶点在等值面以下，设置对应的二进制位
      }
    }

    return edgeIndex;
  }

  // Intersect函数根据符号配置找出与等值面相交的顶点，并计算它们的插值位置
  IntersectInfo Cube::Intersect(double iso) const
  {
    IntersectInfo intersect;
    intersect.signConfig = SignConfig(iso);  //
