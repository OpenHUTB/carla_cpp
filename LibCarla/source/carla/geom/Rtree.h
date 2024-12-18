// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// R树的核心思想是聚合距离相近的节点并在树结构的上一层将其表示为这些节点的最小外接矩形，这个最小外接矩形就成为上一层的一个节点。
// R树的“R”代表“Rectangle（矩形）”。
// 参考：https://zh.wikipedia.org/wiki/R%E6%A0%91
// 
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once ///预处理器指令，用于确保头文件只被包含一次，以避免重复包含。

#include <vector>

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wshadow"
#endif
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/index/rtree.hpp>
#if defined(__clang__) // 恢复之前的编译器警告设置。
#  pragma clang diagnostic pop
#endif


namespace carla {
namespace geom {

  /// PointCloudRtree 类用于处理 3D 点云。
  ///将类型 T 的元素与 3D 点关联，用于快速 k-NN 搜索。
  /// 
  template <typename T, size_t Dimension = 3> // 定义模板类 PointCloudRtree，用于处理点云，T 是元素类型，默认维度为 3。
  class PointCloudRtree {
  public:

    typedef boost::geometry::model::point<float, Dimension, boost::geometry::cs::cartesian> BPoint; // 定义类型别名 BPoint，表示 Boost 几何库中的点。
    typedef std::pair<BPoint, T> TreeElement;  // 定义类型别名 TreeElement，表示 R-tree 中的元素，包含一个点和一个关联元素。

    void InsertElement(const BPoint &point, const T &element) {
      _rtree.insert(std::make_pair(point, element));
    } // 成员函数，将一个点和关联元素插入 R-tree。

    void InsertElement(const TreeElement &element) {
      _rtree.insert(element);
    } // 成员函数，将一个 TreeElement 插入 R-tree。

    void InsertElements(const std::vector<TreeElement> &elements) {
      _rtree.insert(elements.begin(), elements.end());
    } // 成员函数，批量插入多个 TreeElement 到 R-tree。

    /// 返回最近邻元素，可以应用用户定义的过滤器。
    ///  过滤器接收一个 TreeElement 值作为参数，并且需要
    /// 返回一个布尔值以接受或拒绝该值
    /// 例如：[&](Rtree::TreeElement const &element){如果 IsOk(element) 返回 true;
    /// 否则返回 false;}
    template <typename Filter>
    std::vector<TreeElement> GetNearestNeighboursWithFilter(const BPoint &point, Filter filter,
        size_t number_neighbours = 1) const {
      std::vector<TreeElement> query_result;
      auto nearest = boost::geometry::index::nearest(point, static_cast<unsigned int>(number_neighbours));
      auto satisfies = boost::geometry::index::satisfies(filter);
      // Using explicit operator&& to workaround Bullseye coverage issue
      // https://www.bullseye.com/help/trouble-logicalOverload.html.
      _rtree.query(operator&&(nearest, satisfies), std::back_inserter(query_result));
      return query_result;
    } // 成员函数模板，返回最近邻元素，可以应用用户定义的过滤器。

    std::vector<TreeElement> GetNearestNeighbours(const BPoint &point, size_t number_neighbours = 1) const {
      std::vector<TreeElement> query_result;
      _rtree.query(boost::geometry::index::nearest(point, static_cast<unsigned int>(number_neighbours)),
      std::back_inserter(query_result));
      return query_result;
    } // 成员函数，返回最近邻元素，不使用过滤器。

    size_t GetTreeSize() const {
      return _rtree.size();
    } // 成员函数，返回 R-tree 的大小。

  private:

    boost::geometry::index::rtree<TreeElement, boost::geometry::index::linear<16>> _rtree;
    // 私有成员变量，R-tree 数据结构实例。
  };

  /// SegmentCloudRtree 类用于处理 3D 线段云
  /// 将类型 T 的元素与线段的两个端点关联，用于快速 k-NN 搜索。
  /// 
  template <typename T, size_t Dimension = 3>
  class SegmentCloudRtree {
  public:// 定义模板类 SegmentCloudRtree，用于处理线段云，T 是元素类型，默认维度为 3。

    typedef boost::geometry::model::point<float, Dimension, boost::geometry::cs::cartesian> BPoint;
    typedef boost::geometry::model::segment<BPoint> BSegment;
    typedef std::pair<BSegment, std::pair<T, T>> TreeElement;

    void InsertElement(const BSegment &segment, const T &element_start, const T &element_end) {
      _rtree.insert(std::make_pair(segment, std::make_pair(element_start, element_end)));
    }// 成员函数，将一个线段和两个关联元素插入 R-tree。

    void InsertElement(const TreeElement &element) {
      _rtree.insert(element);
    }// 成员函数，将一个 TreeElement 插入 R-tree。

    void InsertElements(const std::vector<TreeElement> &elements) {
      _rtree.insert(elements.begin(), elements.end());
    }// 成员函数，批量插入多个 TreeElement 到 R-tree。

    /// 返回带有用户定义过滤器的最近邻元素。
    /// 过滤器接收一个 TreeElement 值作为参数，并且需要
    /// 返回一个布尔值以接受或拒绝该值
    /// 示例过滤器：[&](Rtree::TreeElement const &element){如果 IsOk(element) 返回 true；
    /// 否则返回 false;}
    template <typename Geometry, typename Filter>
    std::vector<TreeElement> GetNearestNeighboursWithFilter(
        const Geometry &geometry,
        Filter filter,
        size_t number_neighbours = 1) const {
      std::vector<TreeElement> query_result;
      _rtree.query(
          boost::geometry::index::nearest(geometry, static_cast<unsigned int>(number_neighbours)) &&
              boost::geometry::index::satisfies(filter),
          std::back_inserter(query_result));
      return query_result;
    } // 成员函数模板，返回最近邻元素，可以应用用户定义的过滤器。

    template<typename Geometry>
    std::vector<TreeElement> GetNearestNeighbours(const Geometry &geometry, size_t number_neighbours = 1) const {
      std::vector<TreeElement> query_result;
      _rtree.query(
          boost::geometry::index::nearest(geometry, static_cast<unsigned int>(number_neighbours)),
          std::back_inserter(query_result));
      return query_result;
    } // 成员函数模板，返回最近邻元素，不使用过滤器。

    /// 返回与指定几何形状相交的线段。
    /// 警告：Boost 库没有实现3D线段间的交集计算。
    template<typename Geometry>
    std::vector<TreeElement> GetIntersections(const Geometry &geometry) const {
      std::vector<TreeElement> query_result;
      _rtree.query(
          boost::geometry::index::intersects(geometry),
          std::back_inserter(query_result));
      return query_result;
    } // 成员函数模板，返回与指定几何形状相交的线段。

    size_t GetTreeSize() const {
      return _rtree.size();
    } // 成员函数，返回 R-tree 的大小。

  private:

    boost::geometry::index::rtree<TreeElement, boost::geometry::index::linear<16>> _rtree;
    // 私有成员变量，R-tree 数据结构实例。
  };

} // namespace geom
} // namespace carla
