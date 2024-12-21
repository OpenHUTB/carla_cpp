#pragma once
// 引入相关的头文件，这些头文件中可能包含了自定义的数据结构、立方体相关以及三角剖分相关的定义等，用于后续代码中对应类型的使用
#include "DataStructs.h"
#include "Cube.h"
#include "Triangulation.h"

namespace MeshReconstruction // 定义名为MeshReconstruction的命名空间，用于将网格重建相关的功能代码组织在一起，避免命名冲突
{
    /// 使用 <a href=“https://en.wikipedia.org/wiki/Marching_cubes”>Marching Cube</a> 从给定的有向距离函数重建三角形网格。
    /// 这里通过文档注释说明了该函数的主要功能以及使用的算法（Marching Cube算法），并提供了相关的参考链接方便查看算法详细信息。
    /// @param sdf <a href=“http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm”>有符号距离函数</a>。
    /// 对参数sdf进行说明，它是有符号距离函数，同样提供了相关参考链接供进一步了解其概念。
    /// @param 域 重建域。
    /// 简单描述了另一个参数“域”的含义，但这里命名不太规范（最好使用英文或者更明确的中文表述），推测是指进行网格重建的空间范围相关概念。
    /// @returns 重建的网格。
    /// 说明了函数的返回值类型及含义，即返回重建好的三角形网格对象。
    Mesh MarchCube(
        Fun3s const &sdf,
        Rect3 const &domain);

    /// 使用 <a href=“https://en.wikipedia.org/wiki/Marching_cubes”>Marching Cube</a> 从给定的有向距离函数重建三角形网格。
    /// 同样是基于Marching Cube算法进行网格重建的函数重载版本，以下是对各新增参数的说明。
    /// @param sdf <a href=“http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm”>有符号距离函数</a>。
    /// @param 域 重建域。
    /// @param cubeSize 行进立方体的大小。立方体越小，生成的网格分辨率越高。
    /// 解释了cubeSize参数的含义，表明它控制着行进立方体的大小，并且说明了其大小与生成网格分辨率之间的关系。
    /// @param 应对其进行三角剖分的SDF的isoLevel Level集。更改此值将移动重建的表面。
    /// 对isoLevel参数进行说明，指出它与三角剖分以及重建表面位置的关联。
    /// @param SDF的sdfGrad梯度，可生成重建网格的顶点法线。如果未提供任何参数，则使用数值近似值。
    /// 说明了sdfGrad参数的作用，即用于生成重建网格顶点法线，同时说明了如果不传入该参数时的默认处理方式（使用数值近似值）。
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
// 以下是一个匿名命名空间，用于定义一些仅在当前翻译单元内可见的辅助函数，不会与其他文件中的同名函数产生命名冲突
namespace
{
    // 计算给定函数在某一点的数值梯度。
    // 该函数的功能注释，清晰表明它的作用是通过数值方法计算给定三维函数在特定点的梯度。
    // @param f 一个三维函数。
    // 对参数f的说明，它代表一个三维函数，具体类型应该是符合代码中Fun3s定义的函数类型。
    // @param p 计算梯度的点。
    // 说明参数p的含义，即要计算梯度的空间中的点，类型应该是符合代码中Vec3定义的向量类型，表示三维坐标点。
    // @returns 该点处的数值梯度
    // 指出函数的返回值为该点处计算得到的数值梯度，类型同样为Vec3，即三维向量，用于表示梯度在各个方向上的分量。
    Vec3 NumGrad(Fun3s const &f, Vec3 const &p)
    {
        auto const Eps = 1e-6; // 用于计算梯度的微小增量
        // 定义在x、y、z方向上的微小偏移量，用于中心差分法计算偏导数，初始化为仅对应方向有微小增量，其他方向为0
        Vec3 epsX{Eps, 0, 0}, epsY{0, Eps, 0}, epsZ{0, 0, Eps};
        // 通过中心差分法计算x、y、z方向的偏导数，利用函数在微小偏移前后的值差来近似计算偏导数
        auto gx = (f(p + epsX) - f(p - epsX)) / 2;
        auto gy = (f(p + epsY) - f(p - epsY)) / 2;
        auto gz = (f(p + epsZ) - f(p - epsZ)) / 2;
        return {gx, gy, gz}; // 返回计算得到的数值梯度向量，包含x、y、z三个方向的偏导数分量
    }
}

// 实现命名空间MeshReconstruction中的MarchCube函数，当只传入有符号距离函数和区域时的版本。
// 此函数是对只传入两个参数的MarchCube函数的具体实现，默认情况下，使用一个固定数量的立方体进行重建，并调用另一个接受立方体大小参数的MarchCube函数。
Mesh MeshReconstruction::MarchCube(Fun3s const &sdf, Rect3 const &domain)
{
    auto const NumCubes = 50; // 固定的立方体数量，用于默认情况下划分重建区域的立方体数量设定
    auto cubeSize = domain.size * (1.0 / NumCubes); // 计算默认的立方体大小，通过将重建域的大小按固定立方体数量进行均分来得到每个立方体的大小

    return MarchCube(sdf, domain, cubeSize); // 调用另一个参数更全的MarchCube函数进行实际的网格重建操作，传入计算好的默认立方体大小
}

// 实现命名空间MeshReconstruction中的MarchCube函数，接受更多参数的版本。
Mesh MeshReconstruction::MarchCube(
    Fun3s const &sdf,
    Rect3 const &domain,
    Vec3 const &cubeSize,
    double isoLevel,
    Fun3v sdfGrad)
{
    // Default value.
    // 根据传入的sdfGrad参数是否为nullptr来决定使用何种方式获取梯度函数，如果传入为nullptr，则使用前面定义的数值梯度计算函数NumGrad，否则使用传入的sdfGrad函数。
    sdfGrad = sdfGrad == nullptr
                 ? [&sdf](Vec3 const &p)
    { return NumGrad(sdf, p); }
                  : sdfGrad;

    auto const NumX = static_cast<int>(ceil(domain.size.x / cubeSize.x)); // 计算x方向上的立方体数量，通过将重建域在x方向的大小除以立方体在x方向的大小，并向上取整得到立方体数量
    auto const NumY = static_cast<int>(ceil(domain.size.y / cubeSize.y)); // 计算y方向上的立方体数量，原理同x方向
    auto const NumZ = static_cast<int>(ceil(domain.size.z / cubeSize.z)); // 计算z方向上的立方体数量，原理同x方向

    auto const HalfCubeDiag = cubeSize.Norm() / 2.0; // 计算立方体对角线长度的一半，可能用于后续判断立方体与等值面的关系等操作
    auto const HalfCubeSize = cubeSize * 0.5; // 计算立方体边长的一半，同样可能用于一些与立方体相关的坐标计算等操作

    Mesh mesh; // 创建一个空的网格对象，用于存储后续重建得到的网格数据

    // 遍历所有的立方体，通过三层嵌套循环遍历在x、y、z三个方向上的所有立方体位置
    for (auto ix = 0; ix < NumX; ++ix)
    {
        auto x = domain.min.x + ix * cubeSize.x; // 当前立方体的x坐标，通过重建域的最小x坐标加上当前在x方向上的立方体序号乘以立方体在x方向的大小来计算得到

        for (auto iy = 0; iy < NumY; ++iy)
        {
            auto y = domain.min.y + iy * cubeSize.y; // 当前立方体的y坐标，计算原理同x坐标

            for (auto iz = 0; iz < NumZ; ++iz)
            {
                auto z = domain.min.z + iz * cubeSize.z; // 当前立方体的z坐标，计算原理同x坐标
                Vec3 min{x, y, z}; // 当前立方体的最小坐标，将计算得到的x、y、z坐标组合成一个表示立方体最小坐标的向量

                // 仅当立方体位于表面周围的窄带内时才处理。
                // 以下是判断当前立方体是否在需要处理的范围内（即表面周围的窄带内），通过计算立方体中心到等值面的距离，并与立方体对角线长度一半进行比较来判断
                auto cubeCenter = min + HalfCubeSize; // 计算立方体的中心坐标，通过立方体最小坐标加上边长一半的偏移量得到
                auto dist = abs(sdf(cubeCenter) - isoLevel); // 计算立方体中心到等值面的距离，利用有符号距离函数计算中心坐标处到指定isoLevel对应的等值面的距离，并取绝对值
                if (dist > HalfCubeDiag)
                    continue; // 如果距离大于立方体对角线长度一半，则跳过当前立方体，不进行后续处理

                Cube cube({min, cubeSize}, sdf); // 创建一个立方体对象，传入立方体的最小坐标和大小以及有符号距离函数，用于后续与等值面的相交等操作判断
                auto intersect = cube.Intersect(isoLevel); // 计算立方体与等值面的交点，具体的相交计算逻辑应该在Cube类的Intersect函数中实现
                Triangulate(intersect, sdfGrad, mesh); // 对交点进行三角剖分，并将结果添加到网格中，Triangulate函数实现具体的三角剖分及添加到网格的操作逻辑
            }
        }
    }

    return mesh; // 返回重建后的网格，即将经过一系列处理得到的最终网格对象返回给调用者
}
