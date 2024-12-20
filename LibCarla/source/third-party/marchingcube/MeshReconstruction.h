#pragma once
#include "DataStructs.h"
#include "Cube.h"
#include "Triangulation.h"

namespace MeshReconstruction// 命名空间 MeshReconstruction，用于网格重建相关的功能
{
  ///使用 <a href=“https://en.wikipedia.org/wiki/Marching_cubes”>Marching Cube 从给定的有向距离函数重建三角形网格</a>。
  ///@param sdf <a href=“http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm”>有符号距离函数</a>。
  /// @param 域 重建域。
  /// @returns 重建的网格。
  Mesh MarchCube(
      Fun3s const &sdf,
      Rect3 const &domain);

  /// 使用 <a href=“https://en.wikipedia.org/wiki/Marching_cubes”>Marching Cube 从给定的有向距离函数重建三角形网格</a>。
  ///@param sdf <a href=“http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm”>有符号距离函数</a>。
  ///@param 域 重建域。
  /// @param cubeSize 行进立方体的大小。立方体越小，生成的网格分辨率越高。
  /// @param应对其进行三角剖分的 SDF 的 isoLevel Level 集。更改此值将移动重建的表面。
  /// @param SDF 的 sdfGrad 梯度，可生成重建网格的顶点法线。如果未提供任何参数，则使用数值近似值。
  /// @returns 重建的网格。
  Mesh MarchCube(
      Fun3s const &sdf,
      Rect3 const &domain,
      Vec3 const &cubeSize,
      double isoLevel = 0,
      Fun3v sdfGrad = nullptr);
}

using namespace MeshReconstruction;
using namespace std;

// Adapted from here: http://paulbourke.net/geometry/polygonise/

namespace// 匿名命名空间，用于定义一些仅在当前翻译单元内可见的辅助函数
{
// 计算给定函数在某一点的数值梯度。
// @param f 一个三维函数。
// @param p 计算梯度的点。
// @returns 该点处的数值梯度
  Vec3 NumGrad(Fun3s const &f, Vec3 const &p)
  {
    auto const Eps = 1e-6;// 用于计算梯度的微小增量
    Vec3 epsX{Eps, 0, 0}, epsY{0, Eps, 0}, epsZ{0, 0, Eps};
// 通过中心差分法计算 x、y、z 方向的偏导数
    auto gx = (f(p + epsX) - f(p - epsX)) / 2;
    auto gy = (f(p + epsY) - f(p - epsY)) / 2;
    auto gz = (f(p + epsZ) - f(p - epsZ)) / 2;
    return {gx, gy, gz};
  }
}
// 实现命名空间 MeshReconstruction 中的 MarchCube 函数，当只传入有符号距离函数和区域时的版本。
// 默认情况下，使用一个固定数量的立方体进行重建，并调用另一个接受立方体大小参数的 MarchCube 函数。
Mesh MeshReconstruction::MarchCube(Fun3s const &sdf, Rect3 const &domain)
{
  auto const NumCubes = 50;// 固定的立方体数量
  auto cubeSize = domain.size * (1.0 / NumCubes);// 计算默认的立方体大小

  return MarchCube(sdf, domain, cubeSize);
}
// 实现命名空间 MeshReconstruction 中的 MarchCube 函数，接受更多参数的版本。
Mesh MeshReconstruction::MarchCube(
    Fun3s const &sdf,
    Rect3 const &domain,
    Vec3 const &cubeSize,
    double isoLevel,
    Fun3v sdfGrad)
{
  // Default value.
  sdfGrad = sdfGrad == nullptr
                ? [&sdf](Vec3 const &p)
  { return NumGrad(sdf, p); }
                : sdfGrad;

  auto const NumX = static_cast<int>(ceil(domain.size.x / cubeSize.x));// 计算 x 方向上的立方体数量
  auto const NumY = static_cast<int>(ceil(domain.size.y / cubeSize.y));// 计算 y 方向上的立方体数量
  auto const NumZ = static_cast<int>(ceil(domain.size.z / cubeSize.z));// 计算 z 方向上的立方体数量

  auto const HalfCubeDiag = cubeSize.Norm() / 2.0;// 计算立方体对角线长度的一半
  auto const HalfCubeSize = cubeSize * 0.5;// 计算立方体边长的一半

  Mesh mesh;// 创建一个空的网格对象
// 遍历所有的立方体
  for (auto ix = 0; ix < NumX; ++ix)
  {
    auto x = domain.min.x + ix * cubeSize.x;// 当前立方体的 x 坐标

    for (auto iy = 0; iy < NumY; ++iy)
    {
      auto y = domain.min.y + iy * cubeSize.y;// 当前立方体的 y 坐标

      for (auto iz = 0; iz < NumZ; ++iz)
      {
        auto z = domain.min.z + iz * cubeSize.z;// 当前立方体的 z 坐标
        Vec3 min{x, y, z};// 当前立方体的最小坐标

        // 仅当立方体位于表面周围的窄带内时才处理。
        auto cubeCenter = min + HalfCubeSize;// 计算立方体的中心坐标
        auto dist = abs(sdf(cubeCenter) - isoLevel);// 计算立方体中心到等值面的距离
        if (dist > HalfCubeDiag)
          continue;

        Cube cube({min, cubeSize}, sdf);// 创建一个立方体对象
        auto intersect = cube.Intersect(isoLevel); // 计算立方体与等值面的交点
        Triangulate(intersect, sdfGrad, mesh); // 对交点进行三角剖分，并将结果添加到网格中
      }
    }
  }

  return mesh;// 返回重建后的网格
}
