// 版权所有 （c） 巴塞罗那自治大学 （UAB） 的 2023 计算机视觉中心 （CVC）。
//
// 本作品根据 MIT 许可证的条款进行许可。
// 有关副本，请参阅 <https://opensource.org/licenses/MIT>。

#include "Carla.h"
#include "GraphParser.h"
#include "DoublyConnectedEdgeList.h"
#include <type_traits>
#include <unordered_set>

// 定义命名空间MapGen，以下代码中的类型、函数、类等都在这个命名空间范围内
namespace MapGen {

  // 使用DoublyConnectedEdgeList类型定义一个别名Graph，方便后续代码使用该类型时书写更简洁
  using Graph = DoublyConnectedEdgeList;

  // ===========================================================================
  // -- Local static methods ---------------------------------------------------
  // ===========================================================================

  // 静态函数，根据给定的角度（以弧度为单位）确定其所在的象限（将圆周划分为四个象限，每个象限角度范围是PI/2）
  // 通过将角度除以HALF_PI（假设HALF_PI已定义为PI/2）并四舍五入取整来得到象限编号（0 - 3）
  static int getQuadrant(float angle) {
    return static_cast<int>(std::round(angle/HALF_PI));
  }

  // 静态函数，计算两个角度之间的旋转角度（假设输入的角度大致相隔PI/2）
  // 先获取两个角度中较小和较大角度对应的象限编号，然后根据象限关系计算旋转角度，旋转角度是PI/2的整数倍
  static float getRotation(float angle0, float angle1) {
    const int min = getQuadrant(std::min(angle0, angle1));
    const int max = getQuadrant(std::max(angle0, angle1));
    return HALF_PI * std::min(min, min * max);
  }

  // 静态函数，计算三个角度之间的旋转角度（假设输入的角度大致相隔PI/2）
  // 通过获取三个角度对应的象限编号之和，利用switch语句根据不同的和值情况返回相应的旋转角度（PI/2、0、-PI/2、PI等）
  // 如果出现意外的象限组合情况，会输出错误日志并返回0.0
  static float getRotation(float angle0, float angle1, float angle2) {
    /// @todo There has to be a better way.
    switch (getQuadrant(angle0) + getQuadrant(angle1) + getQuadrant(angle2)) {
      case 0:
        return HALF_PI;
      case 1:
        return 0.0;
      case 2:
        return -1.0 * HALF_PI;
      case 3:
        return PI;
      default:
        UE_LOG(LogCarla, Error, TEXT("Wrong quadrants"));
        return 0.0;
    }
  }

  // 静态函数，用于修正图的数据（对输入的图结构Graph进行一些属性的设置和调整）
  // 遍历图中的每个节点，计算每个节点相关的角度信息，根据节点连接的边的数量设置节点的旋转角度、交点类型等属性
  static void fixGraphData(Graph &graph) {
    // 遍历图中的每个节点
    for (auto &node : graph.GetNodes()) {
      std::vector<float> angles;
      angles.reserve(4u);
      // 获取该节点的第一条出边（半边缘）
      auto &firstEdge = Graph::GetLeavingHalfEdge(node);
      auto *edge = &firstEdge;
      // 循环遍历该节点的所有出边（半边缘），形成一个环的遍历
      do {
        // 获取并设置当前边的角度属性
        edge->Angle = Graph::GetAngle(*edge);
        angles.emplace_back(edge->Angle);
        // 获取下一条在该节点的边（半边缘）
        edge = &Graph::GetNextInNode(*edge);
      } while (edge!= &firstEdge);
      // 确保角度列表不为空（理论上正常情况下不会为空，这里是一种检查机制）
      check(!angles.empty());
      node.EdgeCount = angles.size();
      node.bIsIntersection = true;
      // 根据节点连接的边的数量（2、3、4或其他情况）来设置节点的旋转角度和交点类型
      switch (node.EdgeCount) {
        case 2:
          node.Rotation = getRotation(angles[0u], angles[1u]);
          node.IntersectionType = EIntersectionType::Turn90Deg;
          break;
        case 3:
          node.Rotation = getRotation(angles[0u], angles[1u], angles[2u]);
          node.IntersectionType = EIntersectionType::TIntersection;
          break;
        case 4:
        default:
          node.Rotation = 0.0;
          node.IntersectionType = EIntersectionType::XIntersection;
          break;
      }
      node.Rots.swap(angles);
    }
  }

  // ===========================================================================
  // -- RoadSegmentBuilder -----------------------------------------------------
  // ===========================================================================

  // RoadSegmentBuilder类，用于构建道路路段相关的描述信息
  class RoadSegmentBuilder {
  public:
    // 存储道路路段描述的智能指针向量，每个元素指向一个RoadSegmentDescription类型的对象
    std::vector<TUniquePtr<RoadSegmentDescription>> Segments;

    // 构造函数，接受一个图结构（Graph类型）作为参数，用于初始化构建器关联的图
    explicit RoadSegmentBuilder(const Graph &graph) : _graph(graph) {}

    // 向构建器中添加一个半边缘（HalfEdge），根据一些条件决定是否插入，并处理道路路段的构建逻辑
    void Add(Graph::HalfEdge &edge) {
      if (!insert(edge))
        return;
      // 如果该半边缘的源节点是一个交点，则创建一个新的道路路段描述对象，并标记当前不是处理初始状态了
      if (Graph::GetSource(edge).bIsIntersection) {
        Segments.emplace_back(MakeUnique<RoadSegmentDescription>());
        _handlingInitial = false;
      }
      // 如果当前处于处理初始状态，则将该半边缘指针添加到初始边的向量中
      if (_handlingInitial) {
        _initial.emplace_back(&edge);
      } else {
        // 否则将该半边缘添加到当前正在构建的道路路段描述对象中
        Segments.back()->Add(edge);
      }
    }

    // 关闭当前正在构建的道路路段，将初始边添加到当前路段描述对象中，并重置处理初始状态的标记为true
    void Close() {
      for (auto edge : _initial) {
        Segments.back()->Add(*edge);
      }
      _handlingInitial = true;
    }

  private:
    // 私有函数，用于插入半边缘及其对应的配对半边缘到已访问边的集合中
    // 如果这两条边都还没有被访问过，则插入成功并返回true，否则返回false
    bool insert(Graph::HalfEdge &edge) {
      return _visitedEdges.insert(&edge).second &&
             _visitedEdges.insert(&Graph::GetPair(edge)).second;
    }

    // 存储构建器关联的图结构，不可修改（通过const修饰）
    const Graph &_graph;
    // 用于记录已经访问过的半边缘的集合，避免重复处理边
    std::unordered_set<const Graph::HalfEdge *> _visitedEdges;
    // 标记是否处于处理初始状态，初始化为true
    bool _handlingInitial = true;
    // 存储初始的半边缘指针的向量，用于在关闭道路路段时添加到路段描述中
    std::vector<const Graph::HalfEdge *> _initial;
  };

  // ===========================================================================
  // -- GraphParser ------------------------------------------------------------
  // ===========================================================================

  // GraphParser类的构造函数，用于解析图结构并构建相关的城市区域和道路路段描述信息
  GraphParser::GraphParser(DoublyConnectedEdgeList &graph) {
    // 进行一些前置条件检查，确保图中的节点数量、半边缘数量、面的数量满足一定要求
    check(graph.CountNodes() >= 4u);
    check(graph.CountHalfEdges() >= 8u);
    check(graph.CountFaces() >= 2u);

    // 调用fixGraphData函数修正图的数据，设置节点相关属性
    fixGraphData(graph);

    // 为存储城市区域描述的向量预先分配空间，数量是图的面数量减1（可能排除了某个特殊的面，比如无界的面）
    CityAreas.reserve(graph.CountFaces() - 1);

    // 创建RoadSegmentBuilder对象，传入图结构，用于后续构建道路路段描述
    RoadSegmentBuilder rsb(graph);

    // 获取图的面的列表
    auto faceList = graph.GetFaces();
    auto it = faceList.begin();
    ++it; // 忽略第一个面（可能是无界的面，具体取决于图的定义和使用场景）
    // 遍历除第一个面之外的每个面
    for (; it!= faceList.end(); ++it) {
      // 创建一个新的城市区域描述对象，并添加到城市区域描述向量中
      CityAreas.emplace_back(MakeUnique<CityAreaDescription>(*it));
      CityAreaDescription &cityArea = *CityAreas.back();
      // 获取该面的第一条半边缘
      auto &firstEdge = Graph::GetHalfEdge(*it);
      // 遍历该面的所有半边缘（形成一个环的遍历）
      for (auto *edge = &Graph::GetNextInFace(firstEdge);
           edge!= &firstEdge;
           edge = &Graph::GetNextInFace(*edge)) {
        // 将半边缘的源节点添加到城市区域描述对象中
        cityArea.Add(Graph::GetSource(*edge));
        // 将半边缘添加到RoadSegmentBuilder中，用于构建道路路段描述
        rsb.Add(*edge);
      }
      // 关闭当前正在构建的道路路段
      rsb.Close();
    }

    // 将RoadSegmentBuilder中构建好的道路路段描述向量交换到GraphParser类自身的RoadSegments成员变量中
    RoadSegments.swap(rsb.Segments);
  }

} // namespace MapGen
