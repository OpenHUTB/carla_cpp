/////////////////////////////////////////////
//
// Mesh Simplification Tutorial
//
// (C) by Sven Forstmann in 2014
//
// License : MIT
// http://opensource.org/licenses/MIT
//
// https://github.com/sp4cerat/Fast-Quadric-Mesh-Simplification
//
// 5/2016: Chris Rorden created minimal version for OSX/Linux/Windows compile
/**
 * @brief 标准输入输出流库
 *
 * 提供输入输出功能，用于在控制台打印信息或读取用户输入。
 */
#include <iostream>
// 包含标准输入输出流库，用于输入输出操作
 /**
  * @brief 文件流库
  *
  * 提供文件读写功能，支持文本文件和二进制文件的操作。
  */
#include <fstream>
// 包含文件流库，用于文件的读写
  /**
   * @brief 算法库
   *
   * 提供各种常用算法的实现，如排序、查找等。
   */
#include <algorithm>
// 包含算法库，提供各种常用算法的实现
   /**
    * @brief 字符串操作函数库（C风格）
    *
    * 提供一系列用于处理C风格字符串的函数，如复制、比较等。
    */
#include <string.h>
// 包含字符串操作函数库（C 风格）
    /**
     * @brief 标准输入输出库（C风格）
     *
     * 提供基本的输入输出功能，如打印、读取等，通常用于控制台应用程序。
     */
#include <stdio.h>
// 包含标准输入输出库（C 风格）
     /**
      * @brief 标准库头文件（C风格）
      *
      * 提供内存分配、程序控制、环境访问等功能。
      */
#include <stdlib.h>
// 包含标准库头文件（C 风格），提供内存分配等功能
      /**
       * @brief 关联数组（映射）库
       *
       * 提供了一种存储键值对的数据结构，可以快速地根据键查找对应的值。
       */
#include <map>
// 包含关联数组（映射）库
       /**
        * @brief 动态数组库
        *
        * 提供了一种可以动态调整大小的数组数据结构，支持随机访问和高效的元素添加/删除。
        */
#include <vector>
// 包含动态数组库
        /**
         * @brief 字符串库（C++风格）
         *
         * 提供了一种表示和操作字符串的类，比C风格的字符串提供了更多的功能和安全性。
         */
#include <string>
// 包含字符串库（C++ 风格）
         /**
          * @brief 数学库
          *
          * 提供各种数学函数，如三角函数、对数函数、幂函数等。
          */
#include <math.h>
// 包含数学库
          /**
           * @brief 浮点型数值限制库
           *
           * 定义了浮点型数值的一些极限值，如最小正数（FLT_EPSILON, DBL_EPSILON）等。
           */
#include <float.h> 
// 包含浮点型数值限制库
           /**
            * @def loopi
            * @brief 定义一个循环宏，使用变量i从start_l迭代到end_l（不包括end_l）。
            *
            * @param start_l 循环起始值（包含）。
            * @param end_l 循环结束值（不包含）。
 // 定义一个循环宏，使用变量 i 从 start_l 迭代到 end_l（不包括 end_l）
           */

#define loopi(start_l, end_l) for (int i = start_l; i < end_l; ++i)
            /**
             * @def loopj
             * @brief 定义一个循环宏，使用变量j从start_l迭代到end_l（不包括end_l）。
             *
             * @param start_l 循环起始值（包含）。
             * @param end_l 循环结束值（不包含）。
             */
#define loopi(start_l, end_l) for (int i = start_l; i < end_l; ++i)
#define loopj(start_l, end_l) for (int j = start_l; j < end_l; ++j)
             /**
              * @def loopk
              * @brief 定义一个循环宏，使用变量k从start_l迭代到end_l（不包括end_l）。
              *
              * @param start_l 循环起始值（包含）。
              * @param end_l 循环结束值（不包含）。
              */
#define loopk(start_l, end_l) for (int k = start_l; k < end_l; ++k)
              /**
               * @struct vector3
               * @brief 一个简单的三维向量结构体，包含x、y、z三个坐标。
               */
// 定义一个简单的三维向量结构体
struct vector3
{
  double x, y, z;
};
/**
 * @struct vec3f
 * @brief 一个包含三维浮点数坐标的向量结构体，提供了一系列向量运算。
 */

// 定义一个包含更多功能的三维浮点数坐标的向量结构体
struct vec3f
{
// 默认构造函数
  double x, y, z;
  /**
   * @brief 默认构造函数。
   */
  inline vec3f(void) {}

  /**
    * @brief 拷贝构造函数，从另一个vector3对象构造。
    *
    * @param a 一个vector3对象。
    */
  inline vec3f(vector3 a)
  {
    x = a.x;
    y = a.y;
    z = a.z;
  }
  /**
   * @brief 参数化构造函数，从三个浮点数构造。
   *
   * @param X x坐标。
   * @param Y y坐标。
   * @param Z z坐标。
   */
  inline vec3f(const double X, const double Y, const double Z)
  {
    x = X;
    y = Y;
    z = Z;
  }

  inline vec3f operator+(const vec3f &a) const
  {
    return vec3f(x + a.x, y + a.y, z + a.z);
  }

  inline vec3f operator+=(const vec3f &a) const
  {
    return vec3f(x + a.x, y + a.y, z + a.z);
  }

  inline vec3f operator*(const double a) const
  {
    return vec3f(x * a, y * a, z * a);
  }

  inline vec3f operator*(const vec3f a) const
  {
    return vec3f(x * a.x, y * a.y, z * a.z);
  }

  inline vec3f v3() const
  {
    return vec3f(x, y, z);
  }
  /**
   * @brief 赋值运算符重载，从另一个vec3f对象赋值。
   *
   * @param a 另一个vec3f对象。
   * @return 返回当前对象的引用。
   */
  inline vec3f operator=(const vector3 a)
  {
    x = a.x;
    y = a.y;
    z = a.z;
    return *this;
  }
  /**
 * @brief 赋值运算符重载，用于将另一个vec3f对象的值赋给当前对象。
 *
 * @param a 另一个vec3f对象，其值将被赋给当前对象。
 * @return 返回当前对象的引用。
 */
  inline vec3f operator=(const vec3f a)
  {
    x = a.x;
    y = a.y;
    z = a.z;
    return *this;
  }
  /**
 * @brief 向量除法运算符重载，计算当前对象与另一个vec3f对象对应分量相除的结果。
 *
 * @param a 另一个vec3f对象，其对应分量将用作除数。
 * @return 返回一个新vec3f对象，包含除法运算的结果。
 */
  inline vec3f operator/(const vec3f a) const
  {
    return vec3f(x / a.x, y / a.y, z / a.z);
  }
  /**
 * @brief 向量减法运算符重载，计算当前对象与另一个vec3f对象对应分量相减的结果。
 *
 * @param a 另一个vec3f对象，其对应分量将从当前对象的对应分量中减去。
 * @return 返回一个新vec3f对象，包含减法运算的结果。
 */
  inline vec3f operator-(const vec3f &a) const
  {
    return vec3f(x - a.x, y - a.y, z - a.z);
  }
  /**
 * @brief 向量与标量除法运算符重载，计算当前对象与给定标量相除的结果。
 *
 * @param a 一个标量，将用作除数。
 * @return 返回一个新vec3f对象，包含除法运算的结果。
 */
  inline vec3f operator/(const double a) const
  {
    return vec3f(x / a, y / a, z / a);
  }
  /**
 * @brief 计算当前对象与另一个vec3f对象的点积。
 *
 * @param a 另一个vec3f对象。
 * @return 返回点积的结果。
 */
  inline double dot(const vec3f &a) const
  {
    return a.x * x + a.y * y + a.z * z;
  }
  /**
 * @brief 计算两个三维向量的叉积
 *
 * 计算两个三维向量a和b的叉积，并将结果存储在调用对象中。
 *
 * @param a 第一个三维向量
 * @param b 第二个三维向量
 * @return 调用对象的引用，其值被更新为叉积的结果
 */
  inline vec3f cross(const vec3f &a, const vec3f &b)
  {
    x = a.y * b.z - a.z * b.y;
    y = a.z * b.x - a.x * b.z;
    z = a.x * b.y - a.y * b.x;
    return *this;
  }
  /**
 * @brief 计算当前向量与另一个向量之间的角度
 *
 * 计算当前向量（*this）与向量v之间的夹角（弧度）。
 *
 * @param v 另一个三维向量
 * @return 夹角（以弧度为单位）
 */
  inline double angle(const vec3f &v)
  {
    vec3f a = v, b = *this;
    double dot = v.x * x + v.y * y + v.z * z;
    double len = a.length() * b.length();
    if (len == 0)
      len = 0.00001f;// 避免除以零
    double input = dot / len;
    if (input < -1)
      input = -1;// 防止acos参数超出范围
    if (input > 1)
      input = 1;
    return (double)acos(input);
  }
  /**
 * @brief 计算两个向量之间的角度，考虑第三个向量定义的平面
 *
 * 计算向量v和当前向量（*this）之间的夹角，但考虑由第三个向量w定义的平面。
 * 如果当前向量v在由w和b定义的平面的法线方向上投影为正，则返回负角度。
 *
 * @param v 第一个三维向量
 * @param w 用于定义平面的第三个三维向量
 * @return 夹角（以弧度为单位）
 */
  inline double angle2(const vec3f &v, const vec3f &w)
  {
    vec3f a = v, b = *this;
    double dot = a.x * b.x + a.y * b.y + a.z * b.z;
    double len = a.length() * b.length();
    if (len == 0)
      len = 1;

    vec3f plane;
    plane.cross(b, w);// 计算b和w的叉积，得到平面的法向量

    if (plane.x * a.x + plane.y * a.y + plane.z * a.z > 0)
      return (double)-acos(dot / len);// 如果v在法线方向上的投影为正，返回负角度

    return (double)acos(dot / len);
  }
  /**
 * @brief 绕X轴旋转当前向量
 *
 * 将当前向量绕X轴旋转a弧度。
 *
 * @param a 旋转角度（以弧度为单位）
 * @return 调用对象的引用，其值被更新为旋转后的结果
 */
  inline vec3f rot_x(double a)
  {
    double yy = cos(a) * y + sin(a) * z;
    double zz = cos(a) * z - sin(a) * y;
    y = yy;
    z = zz;
    return *this;
  }
  /**
 * @brief 绕Y轴旋转当前向量
 *
 * 将当前向量绕Y轴旋转a弧度。
 *
 * @param a 旋转角度（以弧度为单位）
 * @return 调用对象的引用，其值被更新为旋转后的结果
 */
  inline vec3f rot_y(double a)
  {
    double xx = cos(-a) * x + sin(-a) * z;
    double zz = cos(-a) * z - sin(-a) * x;
    x = xx;
    z = zz;
    return *this;
  }
  /**
 * @brief 将当前向量的分量限制在最小值和最大值之间
 *
 * 将当前向量的x、y、z分量限制在min和max之间。
 *
 * @param min 最小值
 * @param max 最大值
 */
  inline void clamp(double min, double max)
  {
    if (x < min)
      x = min;
    if (y < min)
      y = min;
    if (z < min)
      z = min;
    if (x > max)
      x = max;
    if (y > max)
      y = max;
    if (z > max)
      z = max;
  }
  /**
 * @brief 绕Z轴旋转当前向量
 *
 * 将当前向量绕Z轴旋转a弧度。
 *
 * @param a 旋转角度（以弧度为单位）
 * @return 调用对象的引用，其值被更新为旋转后的结果
 */
  inline vec3f rot_z(double a)
  {
    double yy = cos(a) * y + sin(a) * x;
    double xx = cos(a) * x - sin(a) * y;
    y = yy;
    x = xx;
    return *this;
  }
  /**
     * @brief 获取当前向量的相反向量
     *
     * 将当前向量的每个分量取反，并返回修改后的向量。
     *
     * @return 修改后的向量的引用。
     */
  inline vec3f invert()
  {
    x = -x;
    y = -y;
    z = -z;
    return *this;
  }
  /**
     * @brief 获取当前向量的分数部分
     *
     * 将当前向量的每个分量转换为小数部分（即去除整数部分）。
     *
     * @return 一个新的包含小数部分的向量。
     */
  inline vec3f frac()
  {
    return vec3f(
        x - double(int(x)),
        y - double(int(y)),
        z - double(int(z)));
  }
  /**
     * @brief 获取当前向量的整数部分
     *
     * 将当前向量的每个分量转换为整数部分（即去除小数部分）。
     *
     * @return 一个新的包含整数部分的向量。
     */
  inline vec3f integer()
  {
    return vec3f(
        double(int(x)),
        double(int(y)),
        double(int(z)));
  }
  /**
     * @brief 获取当前向量的长度
     *
     * 计算并返回当前向量的长度（欧几里得范数）。
     *
     * @return 向量的长度。
     */
  inline double length() const
  {
    return (double)sqrt(x * x + y * y + z * z);
  }
  /**
     * @brief 将当前向量归一化
     *
     * 将当前向量缩放为其单位长度（即长度为1），或者指定的长度。
     *
     * @param desired_length 希望的长度，默认为1。
     * @return 修改后的向量的引用。
     */
  inline vec3f normalize(double desired_length = 1)
  {
    double square = sqrt(x * x + y * y + z * z);
    /*
    if (square <= 0.00001f )
    {
      x=1;y=0;z=0;
      return *this;
    }*/
    // double len = desired_length / square;
    x /= square;
    y /= square;
    z /= square;

    return *this;
  }
  /**
     * @brief 静态方法：归一化一个向量
     *
     * 计算并返回一个向量的单位长度版本。
     *
     * @param a 要归一化的向量。
     * @return 归一化后的向量。
     */
  static vec3f normalize(vec3f a);
  /**
     * @brief 静态方法：初始化随机数生成器
     *
     * 设置随机数生成的初始状态。
     */
  static void random_init();
  /**
     * @brief 静态方法：生成一个随机双精度浮点数
     *
     * 生成一个在[0, 1)范围内的随机双精度浮点数。
     *
     * @return 随机双精度浮点数。
     */
  static double random_double();
  /**
     * @brief 静态方法：生成一个随机三维向量
     *
     * 生成一个每个分量都在[0, 1)范围内的随机三维向量。
     *
     * @return 随机三维向量。
     */
  static vec3f random();
  /**
     * @brief 静态成员变量：随机数生成器使用的随机数
     *
     * 用于随机数生成的内部状态变量。
     */
  static int random_number;
  /**
     * @brief 生成一个[0, 1)范围内的随机双精度浮点数，基于给定的输入进行变换
     *
     * 使用一个复杂的线性同余生成器（LCG）算法，基于给定的输入a生成一个随机数。
     *
     * @param a 输入值，用于影响生成的随机数。
     * @return [0, 1)范围内的随机双精度浮点数。
     */
  double random_double_01(double a)
  {
    double rnf = a * 14.434252 + a * 364.2343 + a * 4213.45352 + a * 2341.43255 + a * 254341.43535 + a * 223454341.3523534245 + 23453.423412;
    int rni = ((int)rnf) % 100000;
    return double(rni) / (100000.0f - 1.0f);
  }
  /**
     * @brief 将当前向量的每个分量设置为[0, 1)范围内的随机数
     *
     * 使用random_double_01方法为每个分量生成一个随机数。
     *
     * @return 修改后的向量的引用。
     */
  vec3f random01_fxyz()
  {
    x = (double)random_double_01(x);
    y = (double)random_double_01(y);
    z = (double)random_double_01(z);
    return *this;
  }
};
/**
 * @brief 计算点p相对于三角形abc的重心坐标
 *
 * 给定三角形abc和点p，计算点p的重心坐标(u, v, w)。
 * 重心坐标满足u + v + w = 1，并且表示点p相对于三角形abc的位置。
 *
 * @param p 需要计算重心坐标的点
 * @param a 三角形的第一个顶点
 * @param b 三角形的第二个顶点
 * @param c 三角形的第三个顶点
 * @return vec3f 包含重心坐标(u, v, w)的向量
 */
vec3f barycentric(const vec3f &p, const vec3f &a, const vec3f &b, const vec3f &c)
{
  vec3f v0 = b - a;
  vec3f v1 = c - a;
  vec3f v2 = p - a;
  double d00 = v0.dot(v0);
  double d01 = v0.dot(v1);
  double d11 = v1.dot(v1);
  double d20 = v2.dot(v0);
  double d21 = v2.dot(v1);
  double denom = d00 * d11 - d01 * d01;
  double v = (d11 * d20 - d01 * d21) / denom;
  double w = (d00 * d21 - d01 * d20) / denom;
  double u = 1.0 - v - w;
  return vec3f(u, v, w);
}
/**
 * @brief 根据重心坐标插值属性
 *
 * 给定三角形abc和点p的重心坐标，以及三角形abc三个顶点的属性attrs，
 * 计算点p处的插值属性。
 *
 * @param p 需要计算插值属性的点
 * @param a 三角形的第一个顶点
 * @param b 三角形的第二个顶点
 * @param c 三角形的第三个顶点
 * @param attrs 包含三角形三个顶点属性的数组
 * @return vec3f 插值后的属性向量
 */
vec3f interpolate(const vec3f &p, const vec3f &a, const vec3f &b, const vec3f &c, const vec3f attrs[3])
{
  vec3f bary = barycentric(p, a, b, c);
  vec3f out = vec3f(0, 0, 0);
  out = out + attrs[0] * bary.x;
  out = out + attrs[1] * bary.y;
  out = out + attrs[2] * bary.z;
  return out;
}
/**
 * @brief 返回两个数中的较小值
 *
 * @param v1 第一个数
 * @param v2 第二个数
 * @return double 返回v1和v2中的较小值
 */
double min(double v1, double v2)
{
  return fmin(v1, v2);
}
/**
 * @brief 对称矩阵类
 *
 * 表示一个4x4对称矩阵，只存储上三角矩阵的元素（包括对角线）。
 */
class SymetricMatrix
{

public:
    /**
       * @brief 构造函数，使用默认值初始化所有元素
       *
       * @param c 所有元素的默认值
       */
  SymetricMatrix(double c = 0) { loopi(0, 10) m[i] = c; }
  /**
     * @brief 构造函数，使用给定的值初始化矩阵
     *
     * @param m11, m12, m13, m14 上三角矩阵的第一行元素
     * @param m22, m23, m24 上三角矩阵的第二行元素（不包括m21，因为是对称矩阵）
     * @param m33, m34 上三角矩阵的第三行元素（不包括m31, m32，因为是对称矩阵）
     * @param m44 上三角矩阵的第四行第四列元素（不包括m41, m42, m43，因为是对称矩阵）
     */
  SymetricMatrix(double m11, double m12, double m13, double m14,
                 double m22, double m23, double m24,
                 double m33, double m34,
                 double m44)
  {
    m[0] = m11;
    m[1] = m12;
    m[2] = m13;
    m[3] = m14;
    m[4] = m22;
    m[5] = m23;
    m[6] = m24;
    m[7] = m33;
    m[8] = m34;
    m[9] = m44;
  }

  /**
     * @brief 使用平面方程的参数构造对称矩阵
     *
     * 给定平面方程ax + by + cz + d = 0的参数a, b, c, d，
     * 构造一个表示该平面点积矩阵的对称矩阵。
     *
     * @param a 平面方程的参数a
     * @param b 平面方程的参数b
     * @param c 平面方程的参数c
     * @param d 平面方程的参数d
     */
  SymetricMatrix(double a, double b, double c, double d)
  {
    m[0] = a * a;
    m[1] = a * b;
    m[2] = a * c;
    m[3] = a * d;
    m[4] = b * b;
    m[5] = b * c;
    m[6] = b * d;
    m[7] = c * c;
    m[8] = c * d;
    m[9] = d * d;
  }
  /**
     * @brief 访问矩阵元素
     *
     * @param c 要访问的元素索引
     * @return double 返回指定索引处的矩阵元素值
     */
  double operator[](int c) const { return m[c]; }

  /**
      * @brief 计算矩阵的子行列式
      *
      * 给定子行列式的元素索引，计算该子行列式的值。
      *
      * @param a11, a12, a13 子行列式的第一行元素索引
      * @param a21, a22, a23 子行列式的第二行元素索引
      * @param a31, a32, a33 子行列式的第三行元素索引
      * @return double 返回子行列式的值
      */
  double det(int a11, int a12, int a13,
             int a21, int a22, int a23,
             int a31, int a32, int a33)
  {
    double det = m[a11] * m[a22] * m[a33] + m[a13] * m[a21] * m[a32] + m[a12] * m[a23] * m[a31] - m[a13] * m[a22] * m[a31] - m[a11] * m[a23] * m[a32] - m[a12] * m[a21] * m[a33];
    return det;
  }
  /**
     * @brief 矩阵加法运算符重载
     *
     * @param n 要相加的另一个对称矩阵
     * @return SymetricMatrix 返回相加后的新对称矩阵
     */
  const SymetricMatrix operator+(const SymetricMatrix &n) const
  {
    return SymetricMatrix(m[0] + n[0], m[1] + n[1], m[2] + n[2], m[3] + n[3],
                          m[4] + n[4], m[5] + n[5], m[6] + n[6],
                          m[7] + n[7], m[8] + n[8],
                          m[9] + n[9]);
  }
  /**
     * @brief 矩阵加法赋值运算符重载
     *
     * @param n 要相加的另一个对称矩阵
     * @return SymetricMatrix& 返回当前矩阵的引用，以便链式调用
     */
  SymetricMatrix &operator+=(const SymetricMatrix &n)
  {
    m[0] += n[0];
    m[1] += n[1];
    m[2] += n[2];
    m[3] += n[3];
    m[4] += n[4];
    m[5] += n[5];
    m[6] += n[6];
    m[7] += n[7];
    m[8] += n[8];
    m[9] += n[9];
    return *this;
  }
  /**
     * @brief 存储矩阵元素的数组
     *
     * 只存储上三角矩阵的元素（包括对角线），共10个元素。
     */
  double m[10];
};
///////////////////////////////////////////
/**
 * @namespace Simplify
 *
 * @brief 命名空间，用于简化网格模型的工具和数据结构。
 */
namespace Simplify
{
    /**
   * @enum Attributes
   *
   * @brief 枚举类型，表示三角形的属性。
   */
  enum Attributes
  {
      /**
     * @brief 无属性。
     */
    NONE,
    /**
     * @brief 普通属性。
     */
    NORMAL = 2,
    /**
    * @brief 包含纹理坐标属性。
    */
    TEXCOORD = 4,
    /**
     * @brief 包含颜色属性。
     */
    COLOR = 8
  };
  /**
 * @struct Triangle
 *
 * @brief 表示三角形的数据结构。
 */
  struct Triangle
  {
      /**
     * @brief 三角形的三个顶点索引。
     */
    int v[3];
    /**
     * @brief 误差值数组，用于简化算法。
     */
    double err[4];
    /**
     * @brief 是否被删除的标志、是否需要重新计算的标志和三角形的属性。
     */
    int deleted, dirty, attr;
    /**
     * @brief 法向量。
     */
    vec3f n;
    /**
     * @brief 三个顶点的纹理坐标。
     */
    vec3f uvs[3];
    /**
     * @brief 材质索引。
     */
    int material;
  };
  /**
 * @struct Vertex
 *
 * @brief 表示顶点的数据结构。
 */
  struct Vertex
  {
      /**
     * @brief 顶点位置。
     */
    vec3f p;
    /**
     * @brief 第一个引用此顶点的三角形的索引和引用此顶点的三角形数量。
     */
    int tstart, tcount;
    /**
     * @brief 对称矩阵，用于存储顶点信息。
     */
    SymetricMatrix q;
    /**
     * @brief 是否为边界顶点的标志。
     */
    int border;
  };
  /**
 * @struct Ref
 *
 * @brief 表示三角形和顶点之间的引用的数据结构。
 */
  struct Ref
  {/**
     * @brief 三角形的索引和三角形中引用的顶点索引。
     */
    int tid, tvertex;
  };
  /**
 * @class SimplificationObject
 *
 * @brief 用于简化网格的类。
 */
  class SimplificationObject
  {
  public:
      /**
     * @brief 存储三角形的向量。
     */
    std::vector<Triangle> triangles;
    /**
     * @brief 存储顶点的向量。
     */
    std::vector<Vertex> vertices;
    /**
     * @brief 存储引用的向量。
     */
    std::vector<Ref> refs;
    /**
     * @brief 材质库的名称。
     */
    std::string mtllib;
    /**
     * @brief 存储材质名称的向量。
     */
    std::vector<std::string> materials;

    /**
     * @brief 主简化函数。
     *
     * @param target_count 目标三角形数量。
     * @param agressiveness 激进程度，用于增加阈值的锐度。5到8是较好的数值，更多迭代次数可以获得更高质量。
     * @param verbose 是否输出详细信息。
     */
    void simplify_mesh(int target_count, double agressiveness = 7, bool verbose = false)
    {
        // 初始化
      loopi(0, triangles.size())
      {
        triangles[i].deleted = 0;
      }

      // 主迭代循环
      int deleted_triangles = 0;
      std::vector<int> deleted0, deleted1;
      int triangle_count = triangles.size();
      // int iteration = 0;
      // loop(iteration,0,100)
      for (int iteration = 0; iteration < 100; iteration++)
      {
        if (triangle_count - deleted_triangles <= target_count)
          break;

        // 每隔一段时间更新网格
        if (iteration % 5 == 0)
        {
          update_mesh(iteration);
        }

        // clear dirty flag
        loopi(0, triangles.size()) triangles[i].dirty = 0;

        // 计算当前迭代的误差阈值
         // 以下数值对大多数模型有效，如果不适用，可以尝试调整以下三个参数
        double threshold = 0.000000001 * pow(double(iteration + 3), agressiveness);

        // 如果启用了详细输出，并且迭代次数是5的倍数，则打印当前状态
        if ((verbose) && (iteration % 5 == 0))
        {
          printf("iteration %d - triangles %d threshold %g\n", iteration, triangle_count - deleted_triangles, threshold);
        }

        // 遍历所有三角形，移除误差较小的三角形并标记已删除的三角形
        loopi(0, triangles.size())
        {
          Triangle &t = triangles[i];
          if (t.err[3] > threshold)
            continue;
          if (t.deleted)
            continue;
          if (t.dirty)
            continue;
          // 遍历三角形的三条边
          loopj(0, 3) if (t.err[j] < threshold)
          {

            int i0 = t.v[j];
            Vertex &v0 = vertices[i0];
            int i1 = t.v[(j + 1) % 3];
            Vertex &v1 = vertices[i1];
            // 如果任一顶点位于边界上，则跳过
            if (v0.border || v1.border)
              continue;

            // 计算要合并到的顶点位置
            vec3f p;
            calculate_error(i0, i1, p);
            // 临时存储顶点相关的三角形信息
            deleted0.resize(v0.tcount);
            deleted1.resize(v1.tcount); 
            // 如果合并后会导致翻转，则跳过
            if (flipped(p, i0, i1, v0, v1, deleted0))
              continue;

            if (flipped(p, i1, i0, v1, v0, deleted1))
              continue;
            // 如果三角形具有纹理坐标，则更新纹理坐标
            if ((t.attr & TEXCOORD) == TEXCOORD)
            {
              update_uvs(i0, v0, p, deleted0);
              update_uvs(i0, v1, p, deleted1);
            }

            // 如果没有翻转，则移除边
            v0.p = p;
            v0.q = v1.q + v0.q;
            // 更新三角形信息
            int tstart = refs.size();

            update_triangles(i0, v0, deleted0, deleted_triangles);
            update_triangles(i0, v1, deleted1, deleted_triangles);

            int tcount = refs.size() - tstart;
            // 如果更新后的三角形数量小于等于原始数量，则节省内存
            if (tcount <= v0.tcount)
            {
              
              if (tcount)
                memcpy(&refs[v0.tstart], &refs[tstart], tcount * sizeof(Ref));
            }
            else
                // 否则，追加新的三角形信息
              v0.tstart = tstart;

            v0.tcount = tcount;
            // 跳出内层循环，继续下一个三角形的检查
            break;
          }
          // 如果已达到目标三角形数量，则退出外层循环
          if (triangle_count - deleted_triangles <= target_count)
            break;
        }
      }
      // 清理网格，移除所有已标记为删除的三角形
      compact_mesh();
    } // simplify_mesh()
    /**
 * @brief 无损简化网格
 *
 * 该函数通过迭代地移除误差低于给定阈值的三角形来简化网格，同时保持网格的无损性。
 *
 * @param verbose 是否打印详细信息，默认为false不打印
 */
    void simplify_mesh_lossless(bool verbose = false)
    {
        // 初始化
     /// @todo 使用范围for循环替换宏定义的循环，以提高代码可读性和安全性
      loopi(0, triangles.size()) triangles[i].deleted = 0;

      // 主迭代循环
      int deleted_triangles = 0;
      std::vector<int> deleted0, deleted1;

      // 迭代处理网格
      for (int iteration = 0; iteration < 9999; iteration++)
      {
          // 不断更新网格
        update_mesh(iteration);
        // clear dirty flag
        loopi(0, triangles.size()) triangles[i].dirty = 0;
        // 移除误差低于阈值的边及其三角形
        double threshold = DBL_EPSILON; // 1.0E-3 EPS;用于确定哪些三角形应该被移除
        if (verbose)
        {
          printf("lossless iteration %d\n", iteration);
        }

        // 遍历所有三角形，移除符合条件的三角形
        loopi(0, triangles.size())
        {
          Triangle &t = triangles[i];
          if (t.err[3] > threshold)
            continue;
          if (t.deleted)
            continue;
          if (t.dirty)
            continue;

          loopj(0, 3) if (t.err[j] < threshold)
          {
            int i0 = t.v[j];
            Vertex &v0 = vertices[i0];
            int i1 = t.v[(j + 1) % 3];
            Vertex &v1 = vertices[i1];

            // 边界检查，如果两个顶点不在同一边界上，则跳过
            if (v0.border != v1.border)
              continue;

            // 计算要合并到的顶点位置
            vec3f p;
            calculate_error(i0, i1, p);
            // 临时存储顶点相关的三角形索引
            deleted0.resize(v0.tcount); 
            deleted1.resize(v1.tcount); 

            // 如果合并后导致法线翻转，则跳过
            if (flipped(p, i0, i1, v0, v1, deleted0))
              continue;
            if (flipped(p, i1, i0, v1, v0, deleted1))
              continue;
            // 如果三角形有纹理坐标，则更新纹理坐标
            if ((t.attr & TEXCOORD) == TEXCOORD)
            {
              update_uvs(i0, v0, p, deleted0);
              update_uvs(i0, v1, p, deleted1);
            }

            // 合并顶点并更新相关三角形
            v0.p = p;
            v0.q = v1.q + v0.q;
            int tstart = refs.size();

            update_triangles(i0, v0, deleted0, deleted_triangles);
            update_triangles(i0, v1, deleted1, deleted_triangles);

            int tcount = refs.size() - tstart;
            // 如果合并后的三角形数量小于等于原数量，则节省内存
            if (tcount <= v0.tcount)
            {
              if (tcount)
                memcpy(&refs[v0.tstart], &refs[tstart], tcount * sizeof(Ref));
            }
            else
                // 否则，追加新的三角形索引
              v0.tstart = tstart;

            v0.tcount = tcount;
            break;// 找到一个可合并的边后跳出内层循环
          }
        }// 如果没有三角形被删除，则跳出循环
        if (deleted_triangles <= 0)
          break;
        deleted_triangles = 0;// 重置计数器，为下一轮迭代准备
      } // 迭代结束
      // 清理网格，移除所有被标记为删除的三角形
      compact_mesh();
    } // simplify_mesh_lossless()

    /**
 * @brief 检查移除指定边后三角形是否会翻转
 *
 * 遍历与顶点v0相关联的所有三角形，检查如果移除边(p, v0) -> (p, v1)后，
 * 是否有三角形发生翻转。翻转的条件包括：
 * 1. 边(p, v0)相邻的两向量几乎共线（点积接近1或-1）。
 * 2. 三角形的法向量与由边(p, v0)相邻的两向量构成的平面的法向量差异较大。
 *
 * @param p       与v0和v1形成边的另一个端点
 * @param i0      顶点p在顶点数组中的索引
 * @param i1      顶点v1在顶点数组中的索引
 * @param v0      顶点v0的引用
 * @param v1      顶点v1的引用
 * @param deleted 用于标记被删除的三角形的数组
 * @return true 如果移除边后至少有一个三角形翻转，否则返回false
 */
    bool flipped(vec3f p, int i0, int i1, Vertex &v0, Vertex &v1, std::vector<int> &deleted)
    {

      loopk(0, v0.tcount)
      {
        Triangle &t = triangles[refs[v0.tstart + k].tid];
        if (t.deleted)
          continue;

        int s = refs[v0.tstart + k].tvertex;
        int id1 = t.v[(s + 1) % 3];
        int id2 = t.v[(s + 2) % 3];

        if (id1 == i1 || id2 == i1) // delete ?
        {

          deleted[k] = 1;
          continue;
        }
        vec3f d1 = vertices[id1].p - p;
        d1.normalize();
        vec3f d2 = vertices[id2].p - p;
        d2.normalize();
        if (fabs(d1.dot(d2)) > 0.999)
          return true;
        vec3f n;
        n.cross(d1, d2);
        n.normalize();
        deleted[k] = 0;
        if (n.dot(t.n) < 0.2)
          return true;
      }
      return false;
    }

    /**
 * @brief 更新UV坐标
 *
 * 遍历与顶点v相关联的所有三角形，对于未被删除且当前顶点未被标记为删除的三角形，
 * 更新其UV坐标。新的UV坐标通过插值计算得到。
 *
 * @param i0      顶点p在顶点数组中的索引
 * @param v       顶点v的引用
 * @param p       与v形成边的另一个端点
 * @param deleted 用于标记被删除的三角形的数组
 */
    void update_uvs(int i0, const Vertex &v, const vec3f &p, std::vector<int> &deleted)
    {
      loopk(0, v.tcount)
      {
        Ref &r = refs[v.tstart + k];
        Triangle &t = triangles[r.tid];
        if (t.deleted)
          continue;
        if (deleted[k])
          continue;
        vec3f p1 = vertices[t.v[0]].p;
        vec3f p2 = vertices[t.v[1]].p;
        vec3f p3 = vertices[t.v[2]].p;
        t.uvs[r.tvertex] = interpolate(p, p1, p2, p3, t.uvs);
      }
    }

    /**
  * @brief 更新三角形连接和边误差
  *
  * 遍历与顶点v相关联的所有三角形，更新其顶点连接和边误差。如果三角形被标记为删除，
  * 则将其标记为已删除并增加已删除三角形的计数。否则，更新三角形的顶点连接，
  * 并重新计算其误差。
  *
  * @param i0                顶点p在顶点数组中的索引
  * @param v                 顶点v的引用
  * @param deleted           用于标记被删除的三角形的数组
  * @param deleted_triangles 已删除三角形的计数，通过引用传递以便更新
  */
    void update_triangles(int i0, Vertex &v, std::vector<int> &deleted, int &deleted_triangles)
    {
      vec3f p;
      loopk(0, v.tcount)// 遍历与顶点v相关联的所有三角形
      {
        Ref &r = refs[v.tstart + k];// 获取三角形r的引用
        Triangle &t = triangles[r.tid];// 获取该三角形t
        if (t.deleted)// 如果三角形已经标记为删除，跳过
          continue;
        if (deleted[k]) // 如果当前三角形被标记为删除
        {
          t.deleted = 1;// 将该三角形标记为删除
          deleted_triangles++;// 增加删除的三角形计数
          continue;
        } // 如果三角形未被删除，更新三角形的顶点索引
        t.v[r.tvertex] = i0;// 用新顶点索引替换
        t.dirty = 1;// 标记三角形为错误·，需要重新计算
       // 计算三角形的误差值
        t.err[0] = calculate_error(t.v[0], t.v[1], p);
        t.err[1] = calculate_error(t.v[1], t.v[2], p);
        t.err[2] = calculate_error(t.v[2], t.v[0], p);
        t.err[3] = min(t.err[0], min(t.err[1], t.err[2]));// 获取最小的误差
        refs.push_back(r);// 将更新后的引用加入引用列表
      }
    }

    /**
 * @brief 更新网格：压缩三角形，并建立引用列表
 *
 * 该函数在指定的迭代次数大于0时，会压缩三角形数组，移除被标记为删除的三角形。
 * 接着，它会初始化顶点的三角形起始索引和三角形计数，然后遍历三角形数组来更新这些值。
 * 最后，它会构建一个引用列表，该列表将每个顶点与其相关联的三角形以及三角形中的顶点索引相关联。
 *
 *
 * @param iteration 当前迭代次数，用于判断是否进行三角形的压缩
 */
    void update_mesh(int iteration)
    {// 如果迭代次数大于0，则压缩三角形数组
      if (iteration > 0)  
      {
        int dst = 0;// 目标索引，用于记录非删除三角形的位置
        loopi(0, triangles.size()) if (!triangles[i].deleted)// 遍历三角形数组
        {
          triangles[dst++] = triangles[i];// 将非删除三角形移动到数组前面
        }
        triangles.resize(dst);// 调整三角形数组大小，移除被删除的三角形
      }
      //

      // 初始化顶点的三角形起始索引和三角形计数
      loopi(0, vertices.size())
      {
        vertices[i].tstart = 0;// 初始化三角形的起始索引为0
        vertices[i].tcount = 0;// 初始化与该顶点相关联的三角形数量为0
      }
      // 更新顶点的三角形计数
      loopi(0, triangles.size())
      {
        Triangle &t = triangles[i];// 引用当前三角形
        loopj(0, 3) vertices[t.v[j]].tcount++;// 遍历三角形的三个顶点，增加与该顶点相关联的三角形数量
      }
      int tstart = 0;// 用于计算每个顶点的三角形起始索引
      loopi(0, vertices.size())
      {
        Vertex &v = vertices[i];// 引用当前顶点
        v.tstart = tstart;// 设置该顶点的三角形起始索引
        tstart += v.tcount;// 更新下一个顶点的三角形起始索引
        v.tcount = 0;// 重置顶点的三角形计数（为构建引用列表做准备）
      }

      // Write References
      refs.resize(triangles.size() * 3); // 每个三角形有三个顶点，因此引用列表的大小是三角形数的三倍
      loopi(0, triangles.size())
      {
        Triangle &t = triangles[i];
        loopj(0, 3)
        {
          Vertex &v = vertices[t.v[j]];
          refs[v.tstart + v.tcount].tid = i;// 记录三角形ID
          refs[v.tstart + v.tcount].tvertex = j;// 记录该顶点在三角形中的位置
          v.tcount++;// 更新顶点的计数
        }
      }

      // Init Quadrics by Plane & Edge Errors
      //
      // required at the beginning ( iteration == 0 )
      // recomputing during the simplification is not required,
      // but mostly improves the result for closed meshes
      //
      if (iteration == 0)
      {    // 标记边界顶点
        // Identify boundary : vertices[].border=0,1

        std::vector<int> vcount, vids;

        loopi(0, vertices.size())
            vertices[i]
                .border = 0; // 初始化边界标志

        loopi(0, vertices.size())
        {
          Vertex &v = vertices[i];
          vcount.clear();
          vids.clear();
          loopj(0, v.tcount)
          {
            int k = refs[v.tstart + j].tid;// 获取三角形ID
            Triangle &t = triangles[k];
            loopk(0, 3)
            {
              int ofs = 0, id = t.v[k];// 获取三角形中的顶点ID
              while (ofs < vcount.size())// 检查该顶点是否已在vcount中，如果没有，则添加
              {
                if (vids[ofs] == id)
                  break;
                ofs++;
              }
              if (ofs == vcount.size())
              {
                vcount.push_back(1);
                vids.push_back(id);
              }
              else
                vcount[ofs]++;
            }
          } // 标记仅与一个三角形相邻的顶点为边界顶点
          loopj(0, vcount.size()) if (vcount[j] == 1)
              vertices[vids[j]]
                  .border = 1;
        }
        // initialize errors
        loopi(0, vertices.size())
            vertices[i]
                .q = SymetricMatrix(0.0);
       // 计算三角形法向量和误差矩阵
        loopi(0, triangles.size())
        {
          Triangle &t = triangles[i];
          vec3f n, p[3];
          loopj(0, 3) p[j] = vertices[t.v[j]].p;
          n.cross(p[1] - p[0], p[2] - p[0]);
          n.normalize();
          t.n = n;
          loopj(0, 3) vertices[t.v[j]].q =
              vertices[t.v[j]].q + SymetricMatrix(n.x, n.y, n.z, -n.dot(p[0]));
        }        // 计算每条边的误差

        loopi(0, triangles.size())
        {
          // Calc Edge Error
          Triangle &t = triangles[i];
          vec3f p;
          loopj(0, 3) t.err[j] = calculate_error(t.v[j], t.v[(j + 1) % 3], p);
          t.err[3] = min(t.err[0], min(t.err[1], t.err[2])); // 获取最小误差
        }
      }
    }

    // Finally compact mesh before exiting

    void compact_mesh()
    {
      int dst = 0;
      loopi(0, vertices.size())
      {
        vertices[i].tcount = 0;// 重置顶点的三角形计数
      }// 压缩三角形和顶点
      loopi(0, triangles.size()) if (!triangles[i].deleted)
      {
        Triangle &t = triangles[i];
        triangles[dst++] = t;// 将非删除三角形压缩到数组前面
        loopj(0, 3) vertices[t.v[j]].tcount = 1;// 更新顶点的三角形计数
      }
      triangles.resize(dst);// 调整三角形数组大小
      dst = 0;
      loopi(0, vertices.size()) if (vertices[i].tcount)
      {
        vertices[i].tstart = dst;// 更新顶点的起始索引
        vertices[dst].p = vertices[i].p;// 将顶点坐标复制到新的位置
        dst++;
      }
    // 更新三角形的顶点索引
      loopi(0, triangles.size())
      {
        Triangle &t = triangles[i];
        loopj(0, 3) t.v[j] = vertices[t.v[j]].tstart;// 更新三角形的顶点索引
      }
      vertices.resize(dst);
    }

    // Error between vertex and Quadric
   // 计算顶点的误差
    double vertex_error(SymetricMatrix q, double x, double y, double z)
    {
      return q[0] * x * x + 2 * q[1] * x * y + 2 * q[2] * x * z + 2 * q[3] * x + q[4] * y * y + 2 * q[5] * y * z + 2 * q[6] * y + q[7] * z * z + 2 * q[8] * z + q[9];
    }
// 计算并返回一个顶点 (x, y, z) 处的误差，误差公式根据对称矩阵 q 给定
   // 计算一个边缘的误差
    double calculate_error(int id_v1, int id_v2, vec3f &p_result)
    {
         // 计算两顶点 (id_v1 和 id_v2) 之间的误差并返回最小误差。
        // p_result 存储计算出的最优点。
       // 获取两个顶点的对称矩阵并求和
      SymetricMatrix q = vertices[id_v1].q + vertices[id_v2].q;
      // 判断是否为边界顶点
      bool border = vertices[id_v1].border & vertices[id_v2].border;
      double error = 0;
      double det = q.det(0, 1, 2, 1, 4, 5, 2, 5, 7);// 计算矩阵 q 的行列式
      if (det != 0 && !border)
      {// 如果行列式不为 0 并且不在边界，则计算最优插值顶点 p_result
        // q_delta 的逆可用

               p_result.x = -1 / det * (q.det(1, 2, 3, 4, 5, 6, 5, 7, 8)); // vx = A41/det(q_delta)
        p_result.y = 1 / det * (q.det(0, 2, 3, 1, 5, 6, 2, 7, 8));  // vy = A42/det(q_delta)
        p_result.z = -1 / det * (q.det(0, 1, 3, 1, 4, 6, 2, 5, 8)); // vz = A43/det(q_delta)
      // 计算此插值顶点的误差
        error = vertex_error(q, p_result.x, p_result.y, p_result.z);
      }
      else
      {// 如果行列式为 0（不可逆），则选择最小误差的点
        vec3f p1 = vertices[id_v1].p;// 获取第一个顶点的位置
        vec3f p2 = vertices[id_v2].p; // 获取第二个顶点的位置
        vec3f p3 = (p1 + p2) / 2;  // 计算两点的中点
           // 计算每个顶点的误差
        double error1 = vertex_error(q, p1.x, p1.y, p1.z);
        double error2 = vertex_error(q, p2.x, p2.y, p2.z);
        double error3 = vertex_error(q, p3.x, p3.y, p3.z);
         // 选择最小的误差，并将结果存储到 p_result
        error = min(error1, min(error2, error3));
        if (error1 == error)
          p_result = p1;
        if (error2 == error)
          p_result = p2;
        if (error3 == error)
          p_result = p3;
      }
      return error;
    }
// 修剪字符串两端的空格
    char *trimwhitespace(char *str)
    {
      char *end;
    // 修剪前导空格
            while (isspace((unsigned char)*str))
        str++;

      if (*str == 0) // 如果全部为空格
        return str;
// 解释：判断传入的字符串指针所指向的字符是否为 '\0'（字符串结束标志，在这里表示字符串为空或者全是空格的情况），
// 如果是，则直接返回该字符串指针，意味着不需要进行后续处理，直接将原字符串返回。
        // 修剪尾部空格
      end = str + strlen(str) - 1;
// 解释：先获取字符串的末尾指针位置，通过将字符串指针 `str` 加上字符串长度（`strlen(str)`）再减去 1，得到指向字符串最后一个字符的指针，
// 这里减 1 是因为字符串长度是从 1 开始计数，而指针指向的是字符在内存中的地址，要指向最后一个实际字符需要减去 1。
      while (end > str && isspace((unsigned char)*end))
        end--;
// 解释：从字符串末尾开始向前遍历，只要当前指针 `end` 大于起始指针 `str`（表示还没遍历到字符串开头）并且当前指针所指向的字符是空格（通过 `isspace` 函数判断，
// 将字符强制转换为 `unsigned char` 类型是为了符合函数参数要求），就将指针 `end` 向前移动一位（即 `end--`），目的是找到最后一个非空格字符的位置，从而去除尾部空格。
             // 写入新的终止符
      *(end + 1) = 0;

      return str;
    }
// 解释：返回处理后的字符串指针，这个指针指向的字符串已经去除了尾部空格，可用于后续需要使用该字符串的地方。
    // 加载 OBJ 文件
    void load_obj(const char *filename, bool process_uv = false)
    {
      
      vertices.clear();
      triangles.clear();
// 解释：清空存储顶点数据的 `vertices` 容器和存储三角形数据的 `triangles` 容器，通常这两个容器可能是 `vector` 等类型，
    // 这样做是为了在加载新的 `OBJ` 文件前清除之前可能存在的数据，避免数据混淆。
      // printf ( "Loading Objects %s ... \n",filename);
      FILE *fn;
      if (filename == NULL)
        return;
      if ((char)filename[0] == 0)
        return;
      if ((fn = fopen(filename, "rb")) == NULL)
      {
        printf("File %s not found!\n", filename);
        return;
      }
// 解释：尝试以二进制只读模式（"rb"）打开指定的文件，如果打开文件失败（`fopen` 函数返回 `NULL`），则输出提示文件未找到的信息，并直接返回，
    // 无法进行后续从文件读取数据的操作。
      char line[1000];
      memset(line, 0, 1000);
// 解释：创建一个长度为 1000 的字符数组 `line`，用于逐行读取文件中的内容，并通过 `memset` 函数将数组的所有元素初始化为 '\0'，
    // 确保数组内容初始为空，避免出现未初始化的垃圾数据影响后续操作。
      int vertex_cnt = 0;
      int material = -1;
      std::map<std::string, int> material_map;
      std::vector<vec3f> uvs;
      std::vector<std::vector<int>> uvMap;
// 解释：初始化一些变量和容器，`vertex_cnt` 可能用于记录顶点的数量计数，`material` 用于表示当前使用的材质编号（初始化为 -1 表示未确定），
    // `material_map` 是一个 `map` 容器，用于将材质名称映射到对应的编号，方便后续查找和管理材质信息；`uvs` 是一个存储纹理坐标的向量容器，
    // `uvMap` 是一个二维向量容器，可能用于存储纹理坐标与三角形顶点之间的映射关系等相关信息。
          // 逐行读取文件内容
      while (fgets(line, 1000, fn) != NULL)
      {
        Vertex v;
        vec3f uv;
 // 解释：在每次循环中，创建一个 `Vertex` 类型的对象 `v`（`Vertex` 类型应该是自定义的顶点结构体等类似类型）和一个 `vec3f` 类型的纹理坐标对象 `uv`，
        // 用于临时存储从文件中解析出的顶点和纹理坐标信息。

     // 解析材质库
        if (strncmp(line, "mtllib", 6) == 0)
        {
          mtllib = trimwhitespace(&line[7]);
        }
// 解释：判断当前读取的行内容是否以 "mtllib" 开头（通过 `strncmp` 函数比较前 6 个字符），如果是，表示这一行是材质库相关的信息，
        // 调用 `trimwhitespace` 函数（前面定义的用于去除空格的函数）处理该行从第 7 个字符开始的内容（即材质库文件名部分），去除可能存在的空格后赋值给 `mtllib` 变量，
        // `mtllib` 变量应该是用于存储材质库文件名的。

        if (strncmp(line, "usemtl", 6) == 0)
        {
          std::string usemtl = trimwhitespace(&line[7]);
          if (material_map.find(usemtl) == material_map.end())
          {
            material_map[usemtl] = materials.size();
            materials.push_back(usemtl);
          }
          material = material_map[usemtl];
        }
// 解释：判断当前行是否以 "usemtl" 开头，如果是，表示这一行指定了使用的材质名称。先调用 `trimwhitespace` 函数处理从第 7 个字符开始的材质名称部分，
        // 得到去除空格后的材质名称字符串，然后在 `material_map` 中查找该材质名称是否已经存在映射关系，如果不存在，则将该材质名称添加到 `material_map` 中，
        // 其编号为 `materials` 容器的当前大小（即新的材质编号），同时将材质名称也添加到 `materials` 容器中（`materials` 可能用于存储所有出现过的材质名称列表），
        // 最后通过 `material_map` 获取该材质名称对应的编号并赋值给 `material` 变量，用于记录当前使用的材质编号。
      // 解析纹理坐标
        if (line[0] == 'v' && line[1] == 't')
        {
          if (line[2] == ' ')
            if (sscanf(line, "vt %lf %lf",
                       &uv.x, &uv.y) == 2)
            {
              uv.z = 0;
              uvs.push_back(uv);
            }
            else if (sscanf(line, "vt %lf %lf %lf",
                            &uv.x, &uv.y, &uv.z) == 3)
            {
              uvs.push_back(uv);
            }
        }
// 解释：判断当前行是否以 "v" 开头且第二个字符是 "t"，表示这一行是纹理坐标信息。如果第三个字符是空格，说明格式符合简单的二维纹理坐标格式，
        // 尝试通过 `sscanf` 函数按照 "vt %lf %lf" 的格式从该行提取两个浮点数（分别赋值给 `uv` 对象的 `x` 和 `y` 成员变量），如果成功提取到两个值（`sscanf` 返回 2），
        // 则将 `uv` 对象的 `z` 成员变量设置为 0，并将该 `uv` 对象添加到 `uvs` 容器中；如果按照二维格式提取失败，但按照 "vt %lf %lf %lf" 格式能成功提取三个浮点数（`sscanf` 返回 3），
        // 则直接将 `uv` 对象添加到 `uvs` 容器中，说明这是三维纹理坐标信息。
        else if (line[0] == 'v')
        {         // 解析顶点坐标
          if (line[1] == ' ')
            if (sscanf(line, "v %lf %lf %lf",
                       &v.p.x, &v.p.y, &v.p.z) == 3)
            {
              vertices.push_back(v);
            }
        }
// 解释：判断当前行是否仅以 "v" 开头，表示这一行是顶点坐标信息。如果第二个字符是空格，说明格式符合常见的顶点坐标格式，
        // 尝试通过 `sscanf` 函数按照 "v %lf %lf %lf" 的格式从该行提取三个浮点数（分别赋值给 `v` 对象的 `p`（`p` 应该是 `Vertex` 结构体中表示坐标的成员变量，类型可能是 `vec3f` 等）的 `x`、`y`、`z` 成员变量），
        // 如果成功提取到三个值（`sscanf` 返回 3），则将该 `v` 对象添加到 `vertices` 容器中，用于存储所有解析出的顶点坐标信息。
        int integers[9];
// 判断读取的行首字符是否为'f'，通常用于识别某种特定格式的数据行（可能与三角形面相关，比如.obj文件中的面数据格式）
        if (line[0] == 'f')
        {
// 创建一个Triangle类型的对象t，用于存储三角形相关信息（Triangle类型应该是自定义的结构体或类，具体成员表示三角形的顶点等信息）
          Triangle t;
/ 用于标记三角形数据是否读取正确的布尔变量，初始化为false
          bool tri_ok = false;
// 用于标记是否包含UV坐标信息的布尔变量，初始化为false
          bool has_uv = false;
// 尝试按照"f %d %d %d"的格式从line中读取3个整数到integers数组中，若成功读取到3个整数（返回值为3），则表示三角形数据读取正确，将tri_ok置为true
          if (sscanf(line, "f %d %d %d",
                     &integers[0], &integers[1], &integers[2]) == 3)
          {
            tri_ok = true;
          }
// 尝试按照"f %d// %d// %d//"的格式从line中读取3个整数到integers数组中，若成功读取到3个整数（返回值为3），则表示三角形数据读取正确，将tri_ok置为true
          else if (sscanf(line, "f %d// %d// %d//",
                          &integers[0], &integers[1], &integers[2]) == 3)
          {
            tri_ok = true;
          }
// 尝试按照"f %d//%d %d//%d %d//%d"的格式从line中读取6个整数到integers数组中，若成功读取到6个整数（返回值为6），则表示三角形数据读取正确，将tri_ok置为true
          else if (sscanf(line, "f %d//%d %d//%d %d//%d",
                          &integers[0], &integers[3],
                          &integers[1], &integers[4],
                          &integers[2], &integers[5]) == 6)
          {
            tri_ok = true;
          }
          else if (sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
                          &integers[0], &integers[6], &integers[3],
                          &integers[1], &integers[7], &integers[4],
                          &integers[2], &integers[8], &integers[5]) == 9)
          {
            tri_ok = true;
            has_uv = true;
          }
          else // Add Support for v/vt only meshes
            if (sscanf(line, "f %d/%d %d/%d %d/%d",
                       &integers[0], &integers[6],
                       &integers[1], &integers[7],
                       &integers[2], &integers[8]) == 6)
            {
              tri_ok = true;
              has_uv = true;
            }
            else
            {
              printf("unrecognized sequence\n");
              printf("%s\n", line);
              while (1)
                ;
            }
// 如果三角形数据读取正确（tri_ok为true），进行以下处理
          if (tri_ok)
          {
            t.v[0] = integers[0] - 1 - vertex_cnt;
            t.v[1] = integers[1] - 1 - vertex_cnt;
            t.v[2] = integers[2] - 1 - vertex_cnt;
            t.attr = 0;
// 如果允许处理UV坐标（process_uv为true）且确实有UV坐标（has_uv为true），进行以下操作
            if (process_uv && has_uv)
            {
// 创建一个整型的动态数组indices，用于存储UV坐标相关的索引信息
              std::vector<int> indices;
              indices.push_back(integers[6] - 1 - vertex_cnt);
              indices.push_back(integers[7] - 1 - vertex_cnt);
              indices.push_back(integers[8] - 1 - vertex_cnt);
// 将indices数组添加到uvMap中，uvMap应该是一个存储UV坐标索引信息的容器
              uvMap.push_back(indices);
              t.attr |= TEXCOORD;
            }
// 将当前使用的材质赋值给三角形对象t的material成员，material应该是表示材质相关信息的变量
            t.material = material;
            // geo.triangles.push_back ( tri );
            triangles.push_back(t);
            // state_before = state;
            // state ='f';
          }
        }
      }
// 如果允许处理UV坐标（process_uv为true）且uvs容器中有元素（表示有UV坐标数据）
      if (process_uv && uvs.size())
      {
        loopi(0, triangles.size())
        {
// 内层循环遍历当前三角形的每个顶点（通常三角形有3个顶点），j表示当前顶点的索引
          loopj(0, 3)
              triangles[i]
                  .uvs[j] = uvs[uvMap[i][j]];
        }
      }
// 关闭文件指针fn指向的文件，fn应该是之前打开的用于读取数据的文件指针
      fclose(fn);

      // printf("load_obj: vertices = %lu, triangles = %lu, uvs = %lu\n", vertices.size(), triangles.size(), uvs.size() );
    } // load_obj()

    // Optional : Store as OBJ
// 将数据以OBJ文件格式进行存储的函数，filename为要存储的文件名
    void write_obj(const char *filename)
    {
// 以写入模式打开指定文件名的文件，如果打开失败返回NULL，file为文件指针
      FILE *file = fopen(filename, "w");
// 用于记录当前材质的索引，初始化为-1
      int cur_material = -1;
      bool has_uv = (triangles.size() && (triangles[0].attr & TEXCOORD) == TEXCOORD);
// 如果文件打开失败，输出错误信息并退出程序
      if (!file)
      {
        printf("write_obj: can't write data file \"%s\".\n", filename);
        exit(0);
      }
      if (!mtllib.empty())
      {
        fprintf(file, "mtllib %s\n", mtllib.c_str());
      }
      loopi(0, vertices.size())
      {
        // fprintf(file, "v %lf %lf %lf\n", vertices[i].p.x,vertices[i].p.y,vertices[i].p.z);
        fprintf(file, "v %g %g %g\n", vertices[i].p.x, vertices[i].p.y, vertices[i].p.z); // more compact: remove trailing zeros
      }
//如果有UV坐标
      if (has_uv)
      {
//写入UV坐标到文件
        loopi(0, triangles.size()) if (!triangles[i].deleted)
        {
          fprintf(file, "vt %g %g\n", triangles[i].uvs[0].x, triangles[i].uvs[0].y);
          fprintf(file, "vt %g %g\n", triangles[i].uvs[1].x, triangles[i].uvs[1].y);
          fprintf(file, "vt %g %g\n", triangles[i].uvs[2].x, triangles[i].uvs[2].y);
        }
      }
//初始化索引为1
      int uv = 1;
//如果有UV坐标且三角形未被删除
      loopi(0, triangles.size()) if (!triangles[i].deleted)
      {
        if (has_uv)
        {
          fprintf(file, "f %d/%d %d/%d %d/%d\n", triangles[i].v[0] + 1, uv, triangles[i].v[1] + 1, uv + 1, triangles[i].v[2] + 1, uv + 2);
          uv += 3;
        }
        else
        {
          fprintf(file, "f %d %d %d\n", triangles[i].v[0] + 1, triangles[i].v[1] + 1, triangles[i].v[2] + 1);
        }
        // fprintf(file, "f %d// %d// %d//\n", triangles[i].v[0]+1, triangles[i].v[1]+1, triangles[i].v[2]+1); //more compact: remove trailing zeros
      }
      fclose(file);
    }
  };

}
///////////////////////////////////////////
