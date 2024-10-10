// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 引入carla/road/element/Geometry.h头文件
#include "carla/road/element/Geometry.h"

// 引入carla/Debug.h头文件
#include "carla/Debug.h"
// 引入carla/Exception.h头文件
#include "carla/Exception.h"
// 引入carla/geom/Location.h头文件
#include "carla/geom/Location.h"
// 引入carla/geom/Math.h头文件
#include "carla/geom/Math.h"
// 引入carla/geom/Vector2D.h头文件
#include "carla/geom/Vector2D.h"

// 引入boost/array.hpp头文件
#include <boost/array.hpp>
// 引入boost/math/tools/rational.hpp头文件
#include <boost/math/tools/rational.hpp>

// 引入odrSpiral/odrSpiral.h头文件
#include <odrSpiral/odrSpiral.h>

// 引入algorithm标准库头文件
#include <algorithm>
// 引入cmath标准库头文件，用于数学运算
#include <cmath>
// 引入stdexcept标准库头文件，用于处理异常
#include <stdexcept>

// 定义命名空间carla
namespace carla {
// 定义命名空间road，在carla命名空间内
namespace road {
// 定义命名空间element，在road命名空间内
namespace element {

// 函数：DirectedPoint类的成员函数，用于应用横向偏移量
// lateral_offset: 横向偏移量
void DirectedPoint::ApplyLateralOffset(float lateral_offset) {
    /// @todo Z轴相关（这里可能是待处理的关于Z轴的内容）
    // 计算法向量的x分量，将弧度制的tangent转换为sin值
    auto normal_x =  std::sin(static_cast<float>(tangent));
    // 计算法向量的y分量，将弧度制的tangent转换为 -cos值
    auto normal_y = -std::cos(static_cast<float>(tangent));
    // 根据横向偏移量更新位置的x坐标
    location.x += lateral_offset * normal_x;
    // 根据横向偏移量更新位置的y坐标
    location.y += lateral_offset * normal_y;
}

// 函数：GeometryLine类的成员函数，根据距离获取位置点
// dist: 距离
DirectedPoint GeometryLine::PosFromDist(double dist) const {
    // 调试断言，确保_length大于0.0
    DEBUG_ASSERT(_length > 0.0);
    // 将距离限制在0.0到_length之间
    dist = geom::Math::Clamp(dist, 0.0, _length);
    // 创建一个DirectedPoint对象，初始位置为_start_position，方向为_heading
    DirectedPoint p(_start_position, _heading);
    // 根据距离和方向更新位置的x坐标
    p.location.x += static_cast<float>(dist * std::cos(p.tangent));
    // 根据距离和方向更新位置的y坐标
    p.location.y += static_cast<float>(dist * std::sin(p.tangent));
    // 返回计算后的DirectedPoint对象
    return p;
}

// 函数：GeometryArc类的成员函数，根据距离获取位置点
// dist: 距离
DirectedPoint GeometryArc::PosFromDist(double dist) const {
    // 将距离限制在0.0到_length之间
    dist = geom::Math::Clamp(dist, 0.0, _length);
    // 调试断言，确保_length大于0.0
    DEBUG_ASSERT(_length > 0.0);
    // 调试断言，确保曲率的绝对值大于1e - 15
    DEBUG_ASSERT(std::fabs(_curvature) > 1e-15);
    // 根据曲率计算半径
    const double radius = 1.0 / _curvature;
    // 定义常量pi_half，即圆周率的一半
    constexpr double pi_half = geom::Math::Pi<double>() / 2.0;
    // 创建一个DirectedPoint对象，初始位置为_start_position，方向为_heading
    DirectedPoint p(_start_position, _heading);
    // 根据半径和初始方向更新位置的x坐标
    p.location.x += static_cast<float>(radius * std::cos(p.tangent + pi_half));
    // 根据半径和初始方向更新位置的y坐标
    p.location.y += static_cast<float>(radius * std::sin(p.tangent + pi_half));
    // 根据距离和曲率更新tangent（方向相关的值）
    p.tangent += dist * _curvature;
    // 根据更新后的tangent和半径再次更新位置的x坐标
    p.location.x -= static_cast<float>(radius * std::cos(p.tangent + pi_half));
    // 根据更新后的tangent和半径再次更新位置的y坐标
    p.location.y -= static_cast<float>(radius * std::sin(p.tangent + pi_half));
    // 返回计算后的DirectedPoint对象
    return p;
}

// 函数：用于将点绕某个角度旋转的辅助函数
// angle: 旋转角度
// x, y: 点的坐标
geom::Vector2D RotatebyAngle(double angle, double x, double y) {
    // 计算旋转角度的cos值
    const double cos_a = std::cos(angle);
    // 计算旋转角度的sin值
    const double sin_a = std::sin(angle);
    // 返回旋转后的向量
    return geom::Vector2D(
    static_cast<float>(x * cos_a - y * sin_a),
    static_cast<float>(y * cos_a + x * sin_a));
}

// 函数：GeometrySpiral类的成员函数，根据距离获取位置点
// dist: 距离
DirectedPoint GeometrySpiral::PosFromDist(double dist) const {
    // 将距离限制在0.0到_length之间
    dist = geom::Math::Clamp(dist, 0.0, _length);
    // 调试断言，确保_length大于0.0
    DEBUG_ASSERT(_length > 0.0);
    // 创建一个DirectedPoint对象，初始位置为_start_position，方向为_heading
    DirectedPoint p(_start_position, _heading);

    // 获取螺旋线终点的曲率
    const double curve_end = (_curve_end);
    // 获取螺旋线起点的曲率
    const double curve_start = (_curve_start);
    // 计算曲率的变化率
    const double curve_dot = (curve_end - curve_start) / (_length);
    // 计算起始参数s_o
    const double s_o = curve_start / curve_dot;
    // 计算当前参数s
    double s = s_o + dist;

    double x;
    double y;
    double t;
    // 调用odrSpiral函数，可能是根据参数计算螺旋线上的点坐标和切线方向
    odrSpiral(s, curve_dot, &x, &y, &t);

    double x_o;
    double y_o;
    double t_o;
    // 再次调用odrSpiral函数，计算起始参数对应的点坐标和切线方向
    odrSpiral(s_o, curve_dot, &x_o, &y_o, &t_o);

    // 计算相对坐标x
    x = x - x_o;
    // 计算相对坐标y
    y = y - y_o;
    // 计算相对切线方向t
    t = t - t_o;

    // 调用RotatebyAngle函数旋转点坐标
    geom::Vector2D pos = RotatebyAngle(_heading - t_o, x, y);
    // 更新位置的x坐标
    p.location.x += pos.x;
    // 更新位置的y坐标
    p.location.y += pos.y;
    // 更新tangent（方向相关的值）
    p.tangent = _heading + t;

    // 返回计算后的DirectedPoint对象
    return p;
}

// 函数：GeometrySpiral类的成员函数，计算到给定位置的距离（未完全实现）
// location: 给定的位置
std::pair<float, float> GeometrySpiral::DistanceTo(const geom::Location &location) const {
    // 注释表明不是解析解，采用离散化并找到最近点的方法（目前未实现）
    // 直接返回一个简单的差值作为临时结果（可能不准确）
    return {location.x - _start_position.x, location.y - _start_position.y};
}

// 函数：GeometryPoly3类的成员函数，根据距离获取位置点
// dist: 距离
DirectedPoint GeometryPoly3::PosFromDist(double dist) const {
    // 使用rtree获取最近邻元素
    auto result = _rtree.GetNearestNeighbours(
        Rtree::BPoint(static_cast<float>(dist))).front();

    // 获取最近邻元素中的两个值的引用
    auto &val1 = result.second.first;
    auto &val2 = result.second.second;

    // 计算插值比例
    double rate = (val2.s - dist) / (val2.s - val1.s);
    // 根据插值比例计算u值
    double u = rate * val1.u + (1.0 - rate) * val2.u;
    // 根据插值比例计算v值
    double v = rate * val1.v + (1.0 - rate) * val2.v;
    // 根据插值比例计算tangent（可能是方向相关的值，这里计算方式存疑）
    double tangent = atan((rate * val1.t + (1.0 - rate) * val2.t)); //?

    // 调用RotatebyAngle函数旋转点坐标
    geom::Vector2D pos = RotatebyAngle(_heading, u, v);
    // 创建一个DirectedPoint对象，初始位置为_start_position，方向为_heading + tangent
    DirectedPoint p(_start_position, _heading + tangent);
    // 更新位置的x坐标
    p.location.x += pos.x;
    // 更新位置的y坐标
    p.location.y += pos.y;
    // 返回计算后的DirectedPoint对象
    return p;
}

// 函数：GeometryPoly3类的成员函数，计算到给定位置的距离（未完全实现）
// p: 给定的位置（这里虽然有参数名，但未使用）
std::pair<float, float> GeometryPoly3::DistanceTo(const geom::Location & /*p*/) const {
    // 注释表明没有解析表达式（可能需要牛顿 - 拉夫逊法或者点搜索，未实现）
    // 直接返回起始位置坐标作为临时结果（可能不准确）
    return {_start_position.x, _start_position.y};
}

// 函数：GeometryPoly3类的成员函数，预计算样条曲线
void GeometryPoly3::PreComputeSpline() {
    // 定义常量，大概是区间大小（单位：米）
    constexpr double interval_size = 0.3;
    // 定义变量，表示u值的区间间隔，初始化为interval_size
    const double delta_u = interval_size; // interval between values of u
    double current_s = 0;
    double current_u = 0;
    double last_u = 0;
    // 计算初始的v值
    double last_v = _poly.Evaluate(current_u);
    double last_s = 0;
    // 创建一个RtreeValue对象，存储上次的值
    RtreeValue last_val{last_u, last_v, last_s, _poly.Tangent(current_u)};
    // 循环，直到current_s大于_length加上delta_u
    while (current_s < _length + delta_u) {
        // 更新u值
        current_u += delta_u;
        // 计算当前的v值
        double current_v = _poly.Evaluate(current_u);
        // 计算u值的差值
        double du = current_u - last_u;
        // 计算v值的差值
        double dv = current_v - last_v;
        // 计算s值的变化量
        double ds = sqrt(du * du + dv * dv);
        // 更新s值
        current_s += ds;
        // 计算当前的切线值
        double current_t = _poly.Tangent(current_u);
        // 创建一个当前的RtreeValue对象
        RtreeValue current_val{current_u, current_v, current_s, current_t};

        // 创建两个Rtree的点对象
        Rtree::BPoint p1(static_cast<float>(last_s));
        Rtree::BPoint p2(static_cast<float>(current_s));
        // 在rtree中插入元素
        _rtree.InsertElement(Rtree::BSegment(p1, p2), last_val, current_val);

        // 更新上次的值
        last_u = current_u;
        last_v = current_v;
        last_s = current_s;
        last_val = current_val;

    }
}

// 函数：GeometryParamPoly3类的成员函数，根据距离获取位置点
// dist: 距离
DirectedPoint GeometryParamPoly3::PosFromDist(double dist) const {
    // 使用rtree获取最近邻元素
    auto result = _rtree.GetNearestNeighbours(
        Rtree::BPoint(static_cast<float>(dist))).front();

    // 获取最近邻元素中的两个值的引用
    auto &val1 = result.second.first;
    auto &val2 = result.second.second;
    // 计算插值比例
    double rate = (val2.s - dist) / (val2.s - val1.s);
    // 根据插值比例计算u值
    double u = rate * val1.u + (1.0 - rate) * val2.u;
    // 根据插值比例计算v值
    double v = rate * val1.v + (1.0 - rate) * val2.v;
    // 根据插值比例计算t_u值
    double t_u = (rate * val1.t_u + (1.0 - rate) * val2.t_u);
    // 根据插值比例计算t_v值
    double t_v = (rate * val1.t_v + (1.0 - rate) * val2.t_v);
    // 根据t_u和t_v计算tangent（可能是方向相关的值）
    double tangent = atan2(t_v, t_u); //?

    // 调用RotatebyAngle函数旋转点坐标
    geom::Vector2D pos = RotatebyAngle(_heading, u, v);
    // 创建一个DirectedPoint对象，初始位置为_start_position，方向为_heading + tangent
    DirectedPoint p(_start_position, _heading + tangent);
    // 更新位置的x坐标
    p.location.x += pos.x;
    // 更新位置的y坐标
    p.location.y += pos.y;
    // 返回计算后的DirectedPoint对象
    return p;
}

// 函数：GeometryParamPoly3类的成员函数，计算到给定位置的距离（未完全实现）
// （这里虽然有参数，但未使用）
std::pair<float, float> GeometryParamPoly3::DistanceTo(const geom::Location &) const {
    // 注释表明没有解析表达式（可能需要牛顿 - 拉夫逊法或者点搜索，未实现）
    // 直接返回起始位置坐标作为临时结果（可能不准确）
    return {_start_position.x, _start_position.y};
}

// 函数：GeometryParamPoly3类的成员函数，预计算样条曲线
void GeometryParamPoly3::PreComputeSpline() {
    // 定义常量，大概是区间大小（单位：米）
    constexpr double interval_size = 0.5;
    // 根据_length和interval_size计算区间数量，至少为5
    size_t number_intervals =
        std::max(static_cast<size_t>(_length / interval_size), size_t(5));
    double delta_p = 1.0 / number_intervals;
    if (_arcLength) {
        // 如果_arcLength为真，根据_length调整delta_p
        delta_p *= _length;
    }
    double param_p = 0;
    double current_s = 0;
    double last_u = _polyU.Evaluate(param_p);
    double last_v = _polyV.Evaluate(param_p);
    double last_s = 0;
    // 创建一个RtreeValue对象，存储上次的值
    RtreeValue last_val{
        last_u,
        last_v,
        last_s,
        _polyU.Tangent(param_p),
        _polyV.Tangent(param_p) };
    // 循环number_intervals次
    for(size_t i = 0; i < number_intervals; ++i) {
        // 更新param_p
        param_p += delta_p;
        // 计算当前的u值
        double current_u = _polyU.Evaluate(param_p);
        // 计算当前的v值
        double current_v = _polyV.Evaluate(param_p);
        // 计算u值的差值
        double du = current_u - last_u;
        // 计算v值的差值
        double dv = current_v - last_v;
        // 计算s值的变化量
        double ds = sqrt(du * du + dv * dv);
        // 更新s值
        current_s += ds;
        // 计算当前的u方向切线值
        double current_t_u = _polyU.Tangent(param_p);
        // 计算当前的v方向切线值
        double current_t_v = _polyV.Tangent(param_p);
        // 创建一个当前的RtreeValue对象
        RtreeValue current_val{
            current_u,
            current_v,
            current_s,
            current_t_u,
            current_t_v };

       // 创建一个Rtree中的BPoint类型对象p1，将last_s转换为float类型后作为构造函数的参数
      // BPoint可能是Rtree中表示点的数据结构，这里是使用last_s的值来初始化这个点
       Rtree::BPoint p1(static_cast<float>(last_s));
      // 创建一个Rtree中的BPoint类型对象p2，将current_s转换为float类型后作为构造函数的参数
      // 同样，这里是使用current_s的值来初始化这个点
       Rtree::BPoint p2(static_cast<float>(current_s));
      // 调用_rtree对象的InsertElement函数，将由p1和p2组成的线段（BSegment）以及相关的值last_val和current_val插入到_rtree中
     // 这里的_rtree可能是某种数据结构（例如空间索引结构），用于存储和管理这些元素
      _rtree.InsertElement(Rtree::BSegment(p1, p2), last_val, current_val);

     // 将当前的u值赋给last_u，用于记录上一次的u值，可能是为了后续的计算或者数据更新
      last_u = current_u;
     // 将当前的v值赋给last_v，用于记录上一次的v值
      last_v = current_v;
    // 将当前的s值赋给last_s，用于记录上一次的s值
      last_s = current_s;
    // 将当前的val值赋给last_val，用于记录上一次的val值，这里的val可能是包含多个属性值的数据结构
      last_val = current_val;

   // 如果当前的s值大于_length（这里_length可能是预先定义的某个长度限制或者阈值）
      if (current_s > _length) {
    // 则跳出当前的循环
        break;
       }
    }
  }
} // namespace element
} // namespace road
} // namespace carla