// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// 包含carla/geom/Location.h头文件
#include "carla/geom/Location.h"
// 包含carla/geom/Math.h头文件
#include "carla/geom/Math.h"
// 包含carla/geom/CubicPolynomial.h头文件
#include "carla/geom/CubicPolynomial.h"
// 包含carla/geom/Rtree.h头文件
#include "carla/geom/Rtree.h"

// 定义命名空间carla，在这个命名空间下包含road和其他相关的定义
namespace carla {
// 定义road命名空间，在carla命名空间下，用于与道路相关的定义
namespace road {
// 定义element命名空间，在road命名空间下，用于道路元素相关的定义
namespace element {

    // 定义一个枚举类型GeometryType，表示几何类型，是无符号整数类型
    enum class GeometryType : unsigned int {
        // 直线类型
        LINE,
        // 弧线类型
        ARC,
        // 螺旋线类型
        SPIRAL,
        // 三次多项式（可能表示曲线）类型
        POLY3,
        // 带参数的三次多项式（可能表示曲线）类型
        POLY3PARAM
    };

    // 定义一个结构体DirectedPoint，表示有方向的点
    struct DirectedPoint {
        // 默认构造函数，初始化location为(0, 0, 0)，tangent为0
        DirectedPoint()
            : location(0, 0, 0),
            tangent(0) {}
        // 构造函数，根据给定的geom::Location和tangent值初始化
        DirectedPoint(const geom::Location &l, double t)
            : location(l),
            tangent(t) {}
        // 构造函数，根据给定的坐标和tangent值初始化
        DirectedPoint(float x, float y, float z, double t)
            : location(x, y, z),
            tangent(t) {}

        // 点的坐标位置
        geom::Location location = {0.0f, 0.0f, 0.0f};
        // 切线方向（以弧度表示）
        double tangent = 0.0;   // [radians]
        // 俯仰角（以弧度表示）
        double pitch = 0.0;   // [radians]

        // 应用横向偏移量的函数，函数体未实现
        void ApplyLateralOffset(float lateral_offset);

        // 重载==运算符，用于比较两个DirectedPoint是否相等
        friend bool operator==(const DirectedPoint &lhs, const DirectedPoint &rhs) {
            return (lhs.location == rhs.location) && (lhs.tangent == rhs.tangent);
        }
    };

    // 定义一个抽象基类Geometry，表示几何形状
    class Geometry {
    public:
        // 获取几何类型的函数
        GeometryType GetType() const {
            return _type;
        }
        // 获取几何形状长度的函数
        double GetLength() const {
            return _length;
        }
        // 获取起始偏移量的函数
        double GetStartOffset() const {
            return _start_position_offset;
        }
        // 获取起始方向（以弧度表示）的函数
        double GetHeading() const {
            return _heading;
        }

        // 获取起始位置的引用
        const geom::Location &GetStartPosition() {
            return _start_position;
        }

        // 虚析构函数，用于多态删除对象时正确释放资源
        virtual ~Geometry() = default;

        // 纯虚函数，根据距离计算位置，需要在派生类中实现
        virtual DirectedPoint PosFromDist(double dist) const = 0;

        // 纯虚函数，计算到给定点的距离，返回一对浮点数，需要在派生类中实现
        virtual std::pair<float, float> DistanceTo(const geom::Location &p) const = 0;

    protected:
        // 受保护的构造函数，用于初始化几何形状的基本属性
        Geometry(
            GeometryType type,
            double start_offset,
            double length,
            double heading,
            const geom::Location &start_pos)
            : _type(type),
            _length(length),
            _start_position_offset(start_offset),
            _heading(heading),
            _start_position(start_pos) {}

    protected:
        GeometryType _type;             // 几何类型
        double _length;                 // 道路部分的长度（单位：米）

        double _start_position_offset;   // s - 偏移量（单位：米）
        double _heading;                // 起始方向（单位：弧度）

        geom::Location _start_position; // 起始位置（单位：米）
    };

    // 定义表示直线的几何形状类，继承自Geometry类
    class GeometryLine final : public Geometry {
    public:
        // 构造函数，用于初始化直线几何形状的属性
        GeometryLine(
            double start_offset,
            double length,
            double heading,
            const geom::Location &start_pos)
            : Geometry(GeometryType::LINE, start_offset, length, heading, start_pos) {}

        // 重写PosFromDist函数，根据距离计算直线上的位置
        DirectedPoint PosFromDist(double dist) const override;

        // 重写DistanceTo函数，计算到给定点的距离
        /// 返回一个包含以下内容的对（pair）：
        ///- @b first（第一个元素）：从形状起点到这条直线上最近点的距离。
        ///- @b second（第二个元素）：从这条直线上最近点到 p 点的欧几里得距离。
        /// @param p 用于计算距离的点
        std::pair<float, float> DistanceTo(const geom::Location &p) const override {
            return geom::Math::DistanceSegmentToPoint(
                p,
                _start_position,
                PosFromDist(_length).location);
        }

    };

    // 定义表示弧线的几何形状类，继承自Geometry类
    class GeometryArc final : public Geometry {
    public:
        // 构造函数，用于初始化弧线几何形状的属性，包括曲率
        GeometryArc(
            double start_offset,
            double length,
            double heading,
            const geom::Location &start_pos,
            double curv)
            : Geometry(GeometryType::ARC, start_offset, length, heading, start_pos),
            _curvature(curv) {}

        // 重写PosFromDist函数，根据距离计算弧线上的位置
        DirectedPoint PosFromDist(double dist) const override;

        // 重写DistanceTo函数，计算到给定点的距离
        /// 返回一个包含以下内容的数对：
       ///- @b first（第一个元素）：从形状起始点到该弧线最近点的距离。
       ///- @b second（第二个元素）：从该弧线最近点到 p 点的欧几里得距离。
       /// @param p 为用于计算距离的点。
        std::pair<float, float> DistanceTo(const geom::Location &p) const override {
            return geom::Math::DistanceArcToPoint(
                p,
                _start_position,
                static_cast<float>(_length),
                static_cast<float>(_heading),
                static_cast<float>(_curvature));
        }

        // 获取曲率的函数
        double GetCurvature() const {
            return _curvature;
        }

    private:
        // 弧线的曲率
        double _curvature;
    };

    // 定义表示螺旋线的几何形状类，继承自Geometry类
    class GeometrySpiral final : public Geometry {
    public:
        // 构造函数，用于初始化螺旋线几何形状的属性，包括曲线起始和结束的曲率
       // 构造函数名为GeometrySpiral，接受以下参数
       GeometrySpiral(
     // 起始偏移量，double类型
           double start_offset,
     // 螺旋线长度，double类型
           double length,
     // 起始方向（弧度表示），double类型
           double heading,
     // 起始位置，类型为const geom::Location &，表示不可修改的geom::Location类型的引用
           const geom::Location &start_pos,
     // 曲线起始曲率，double类型
           double curv_s,
     // 曲线结束曲率，double类型
           double curv_e)
     // 使用初始化列表初始化基类Geometry部分的属性
     // 将GeometryType::SPIRAL作为几何类型，以及传入的起始偏移量、长度、起始方向、起始位置等参数来初始化基类部分
           : Geometry(GeometryType::SPIRAL, start_offset, length, heading, start_pos),
     // 初始化本类中的_curve_start成员变量，将传入的curv_s赋值给它，表示曲线起始曲率
           _curve_start(curv_s),
     // 初始化本类中的_curve_end成员变量，将传入的curv_e赋值给它，表示曲线结束曲率
           _curve_end(curv_e) {}

        // 获取曲线起始曲率的函数
        double GetCurveStart() {
            return _curve_start;
        }

        // 获取曲线结束曲率的函数
        double GetCurveEnd() {
            return _curve_end;
        }

        // 重写PosFromDist函数，根据距离计算螺旋线上的位置
        DirectedPoint PosFromDist(double dist) const override;

        // 重写DistanceTo函数，计算到给定点的距离（函数体未完整实现）
        std::pair<float, float> DistanceTo(const geom::Location &) const override;

    private:
        // 曲线起始曲率
        double _curve_start;
        // 曲线结束曲率
        double _curve_end;
    };

    // 定义表示三次多项式曲线的几何形状类，继承自Geometry类
    class GeometryPoly3 final : public Geometry {
    public:
      // 构造函数，用于初始化三次多项式曲线几何形状的属性，包括多项式系数
     // 构造函数名为GeometryPoly3，接受以下参数
       GeometryPoly3(
        // 起始偏移量，double类型
           double start_offset,
        // 曲线长度，double类型
           double length,
        // 起始方向（以弧度表示），double类型
           double heading,
        // 起始位置，类型为const geom::Location &，表示不可修改的geom::Location类型的引用
           const geom::Location &start_pos,
          // 三次多项式的系数a，double类型
           double a,
          // 三次多项式的系数b，double类型
           double b,
          // 三次多项式的系数c，double类型
           double c,
         // 三次多项式的系数d，double类型
           double d)
         // 使用初始化列表初始化基类Geometry部分的属性
         // 将GeometryType::POLY3作为几何类型，以及传入的起始偏移量、长度、起始方向、起始位置等参数来初始化基类部分
           : Geometry(GeometryType::POLY3, start_offset, length, heading, start_pos),
         // 初始化本类中的_a成员变量，将传入的a赋值给它，表示三次多项式的系数a
           _a(a),
         // 初始化本类中的_b成员变量，将传入的b赋值给它，表示三次多项式的系数b
           _b(b),
         // 初始化本类中的_c成员变量，将传入的c赋值给它，表示三次多项式的系数c
          _c(c),
         // 初始化本类中的_d成员变量，将传入的d赋值给它，表示三次多项式的系数d
         _d(d) {
        // 调用_poly对象（类型为geom::CubicPolynomial）的Set函数
        // 传入三次多项式的系数a、b、c、d来设置多项式
        _poly.Set(a, b, c, d);
        // 调用PreComputeSpline函数，函数体未实现，可能用于预计算样条相关的操作
        PreComputeSpline();
    }
        // 获取系数a的函数
        double Geta() const {
            return _a;
        }
        // 获取系数b的函数
        double Getb() const {
            return _b;
        }
        // 获取系数c的函数
        double Getc() const {
            return _c;
        }
        // 获取系数d的函数
        double Getd() const {
            return _d;
        }

        // 重写PosFromDist函数，根据距离计算三次多项式曲线上的位置
        DirectedPoint PosFromDist(double dist) const override;

        // 重写DistanceTo函数，计算到给定点的距离（函数体未完整实现）
        std::pair<float, float> DistanceTo(const geom::Location &) const override;

    private:
        // 三次多项式对象
        geom::CubicPolynomial _poly;

        // 多项式系数
        double _a;
        double _b;
        double _c;
        double _d;

        // 定义用于R - tree的值结构体
        struct RtreeValue {
            double u = 0;
            double v = 0;
            double s = 0;
            double t = 0;
        };
        // 定义R - tree类型
        using Rtree = geom::SegmentCloudRtree<RtreeValue, 1>;
        // 定义R - tree元素类型
        using TreeElement = Rtree::TreeElement;
        // R - tree对象
        Rtree _rtree;
        // 预计算样条函数，函数体未实现
        void PreComputeSpline();
    };

    // 定义表示带参数的三次多项式曲线的几何形状类，继承自Geometry类
    class GeometryParamPoly3 final : public Geometry {
    public:
        // 构造函数，用于初始化带参数的三次多项式曲线几何形状的属性，包括多项式系数等
// 定义名为GeometryParamPoly3的函数（从代码结构看很可能是一个构造函数，用于初始化对应类的对象），它接收一系列参数来构建特定的几何对象相关信息
        GeometryParamPoly3(
            double start_offset,// 起始偏移量，可能表示几何形状在某个参考方向上的起始位置的偏移数值，具体含义取决于使用场景
            double length,// 几何形状的长度，比如可以是一段曲线、线段等几何元素沿着某个方向延伸的长度值
            double heading,// 朝向角度，通常用于表示几何形状的方向，例如在平面坐标系中与某个基准坐标轴所成的角度
            const geom::Location &start_pos,// 起始位置，通过geom::Location类型传入，包含了具体的坐标等位置信息，代表几何形状起始的地点
            double aU,// 多项式U（可能是用于描述几何形状在U方向上特征的多项式系数，具体取决于所在几何模型的定义）的三次项系数
            double bU,// 多项式U的二次项系数
            double cU,// 多项式U一次项系数
            double dU,// 多项式U的常数项系数
            double aV,// 多项式V（同理，可能用于描述几何形状在V方向上特征）的三次项系数
            double bV, // 多项式V的二次项系数
            double cV, // 多项式V的一次项系数
            double dV, // 多项式V的常数项系数
            bool arcLength)
// 一个布尔值，用于指示是否基于弧长进行相关计算（比如在涉及曲线的几何处理中，弧长相关设置会影响计算方式等）
            // 通过初始化列表初始化基类Geometry，调用基类的构造函数，传入几何类型（这里是GeometryType::POLY3PARAM表示是特定的多项式参数定义的几何类型）、起始偏移量、长度、朝向以及起始位置等参数
            : Geometry(GeometryType::POLY3PARAM, start_offset, length, heading, start_pos),
// 初始化成员变量_aU，将传入的参数aU的值赋给它，以下同理，分别初始化各个对应的成员变量
            _aU(aU),
            _bU(bU),
            _cU(cU),
            _dU(dU),
            _aV(aV),
            _bV(bV),
            _cV(cV),
            _dV(dV),
            _arcLength(arcLength) {
 // 使用传入的系数来设置_polyU这个多项式对象（应该是类内封装用于处理U方向相关几何计算的多项式表示），调用其Set方法传入相应系数
            _polyU.Set(aU, bU, cU, dU); 
// 同理，使用传入的系数设置_polyV这个多项式对象，用于V方向的相关几何计算
            _polyV.Set(aV, bV, cV, dV);
// 调用PreComputeSpline函数（应该是用于预先计算样条相关的数据或者进行一些初始化计算，为后续几何处理做准备，具体功能要看其函数实现）
            PreComputeSpline();
        }

        // 获取系数aU的函数
        double GetaU() const {
            return _aU;
        }
        // 获取系数bU的函数
        double GetbU() const {
            return _bU;
        }
        // 获取系数cU的函数
        double GetcU() const {
            return _cU;
        }
        // 获取系数dU的函数
        double GetdU() const {
            return _dU;
        }
        // 获取系数aV的函数
        double GetaV() const {
            return _aV;
        }
        // 获取系数bV的函数
        double GetbV() const {
            return _bV;
        }
        // 获取系数cV的函数
        double GetcV() const {
            return _cV;
        }
        // 获取系数dV的函数
        double GetdV() const {
            return _dV;
        }

        // 重写PosFromDist函数，根据距离计算带参数的三次多项式曲线上的位置
        DirectedPoint PosFromDist(double dist) const override;

        // 重写DistanceTo函数，计算到给定点的距离（函数体未完整实现）
        std::pair<float, float> DistanceTo(const geom::Location &) const override;

    private:
        // 用于U方向的三次多项式对象
        geom::CubicPolynomial _polyU;
        // 用于V方向的三次多项式对象
        geom::CubicPolynomial _polyV;
        // 多项式系数
        double _aU;
        double _bU;
        double _cU;
        double _dU;
        double _aV;
        double _bV;
        double _cV;
        double _dV;
        // 是否为弧长相关的标志
        bool _arcLength;

        // 定义用于R - tree的值结构体
        struct RtreeValue {
            double u = 0;
            double v = 0;
            double s = 0;
            double t_u = 0;
            double t_v = 0;
        };
        // 定义R - tree类型
        using Rtree = geom::SegmentCloudRtree<RtreeValue, 1>;
        // 定义R - tree元素类型
        using TreeElement = Rtree::TreeElement;
        // R - tree对象
        Rtree _rtree;
        // 预计算样条函数，函数体未实现
        void PreComputeSpline();
    };

} // namespace element
} // namespace road
} // namespace carla