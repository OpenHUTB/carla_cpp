#pragma once
// 引入标准库中的向量容器，用于存储动态大小的元素序列，在这里可能用于存储点坐标、三角形顶点索引等数据
#include <vector>
// 引入标准库中的数组容器，用于表示固定大小的数组，此处用于定义三角形（固定三个顶点索引）
#include <array>
// 引入标准库中的函数对象包装器，用于创建可调用对象，方便传递函数作为参数等操作
#include <functional>

namespace MeshReconstruction
{
    // 定义一个名为Vec3的结构体，用于表示三维空间中的向量（或点坐标）
    struct Vec3
    {
        double x, y, z;

        // 重载加法运算符，实现两个Vec3类型向量的相加操作
        // 返回一个新的Vec3对象，其各分量分别是两个操作数对应分量相加的结果
        Vec3 operator+(Vec3 const &o) const
        {
            return {x + o.x, y + o.y, z + o.z};
        }

        // 重载减法运算符，实现两个Vec3类型向量的相减操作
        // 返回一个新的Vec3对象，其各分量分别是两个操作数对应分量相减的结果
        Vec3 operator-(Vec3 const &o) const
        {
            return {x - o.x, y - o.y, z - o.z};
        }

        // 重载乘法运算符，实现向量与一个标量（double类型数字）的乘法操作
        // 返回一个新的Vec3对象，其各分量分别是原向量对应分量与标量相乘的结果
        Vec3 operator*(double c) const
        {
            return {c * x, c * y, c * z};
        }

        // 计算向量的模（长度），根据向量模的计算公式，返回sqrt(x * x + y * y + z * z)的值
        double Norm() const
        {
            return sqrt(x * x + y * y + z * z);
        }

        // 对向量进行归一化（标准化）操作，先计算向量的模，然后将向量的每个分量除以模
        // 返回一个与原向量方向相同但模为1的新向量
        Vec3 Normalized() const
        {
            auto n = Norm();
            return {x / n, y / n, z / n};
        }
    };

    // 定义一个名为Rect3的结构体，用于表示三维空间中的矩形区域
    struct Rect3
    {
        Vec3 min;  // 矩形区域的最小坐标点（左下角点等，具体取决于应用场景）
        Vec3 size; // 矩形区域的尺寸（长、宽、高，沿x、y、z方向的长度）
    };

    // 使用std::array定义Triangle类型，它表示一个三角形，通过存储三个整数索引来指定在顶点数组中的三个顶点
    using Triangle = std::array<int, 3>;

    // 定义一个名为Mesh的结构体，用于表示一个网格模型
    struct Mesh
    {
        // 存储网格模型的顶点坐标信息，每个元素是一个Vec3类型的点坐标
        std::vector<Vec3> vertices;
        // 存储构成网格模型的三角形信息，每个元素是一个Triangle类型，表示一个三角形的三个顶点索引
        std::vector<Triangle> triangles;
        // 存储网格模型中每个顶点的法向量信息，每个元素是一个Vec3类型的向量，用于表示顶点的法向方向
        std::vector<Vec3> vertexNormals;
    };

    // 定义Fun3s类型别名，它是一个函数类型，接受一个Vec3类型的常量引用作为参数，返回一个double类型的值
    // 可用于表示在三维空间中某个基于点坐标计算得到的标量函数（比如场函数等）
    using Fun3s = std::function<double(Vec3 const &)>;
    // 定义Fun3v类型别名，它是一个函数类型，接受一个Vec3类型的常量引用作为参数，返回一个Vec3类型的向量
    // 可用于表示在三维空间中基于点坐标进行某种向量运算得到的向量函数（比如力场、速度场等向量函数）
    using Fun3v = std::function<Vec3(Vec3 const &)>;
}