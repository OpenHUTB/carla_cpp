// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "GraphGenerator.h"

#include <vector>

// 定义一个名为MapGen的命名空间，以下所有的函数、类型定义等都在这个命名空间内
namespace MapGen {

  // 使用Graph作为DoublyConnectedEdgeList的别名，方便后续使用
  using Graph = DoublyConnectedEdgeList;

  // 定义一个常量表达式，用于表示一个边距值，这里设置为6
  constexpr static int32 MARGIN = 6;

  // ===========================================================================
  // -- Static local methods ---------------------------------------------------
  // ===========================================================================

  // 定义一个静态函数signOf，它接受一个int32类型的参数val
  // 根据val的值返回其符号，若val大于0则返回1，若val小于0则返回-1，若val等于0则返回0
  static int32 signOf(int32 val) {
    return (0 < val) - (val < 0);
  }

  // 定义一个静态函数getSourcePosition，它接受一个Graph::HalfEdge类型的参数edge
  // 返回该边的源节点的位置信息，通过调用Graph::GetSource函数获取源节点，再获取其位置
  static const Graph::Position &getSourcePosition(const Graph::HalfEdge &edge) {
    return Graph::GetSource(edge).GetPosition();
  }

  // 定义一个静态函数getTargetPosition，它接受一个Graph::HalfEdge类型的参数edge
  // 返回该边的目标节点的位置信息，通过调用Graph::GetTarget函数获取目标节点，再获取其位置
  static const Graph::Position &getTargetPosition(const Graph::HalfEdge &edge) {
    return Graph::GetTarget(edge).GetPosition();
  }

  // 定义一个静态函数getDirection，它接受一个Graph::HalfEdge类型的参数edge
  // 通过目标节点位置减去源节点位置，计算并返回该边的方向向量
  static Graph::Position getDirection(const Graph::HalfEdge &edge) {
    return getTargetPosition(edge) - getSourcePosition(edge);
  }

  // 定义一个静态函数getRandomOpposingEdges，它接受一个Graph::Face类型的参数face和一个FRandomStream类型的参数random
  // 该函数的目的是从给定的面（face）中随机获取一对相对的边
  static std::pair<Graph::HalfEdge *, Graph::HalfEdge *> getRandomOpposingEdges(
      Graph::Face &face,
      FRandomStream &random) {
    // 创建一个向量用于存储面中的所有边
    std::vector<Graph::HalfEdge *> edges;
    // 获取面的第一条半边（half-edge），这是遍历面的边的起始点
    edges.reserve(4u);
    // 将第一条半边赋值给临时指针edge，用于后续遍历
    auto &firstEdge = Graph::GetHalfEdge(face);
    // 开始遍历面的所有边，直到回到起始边
    auto *edge = &firstEdge;
    do {
      // 将当前边添加到边向量中
      edges.emplace_back(edge);
      // 获取下一条在面中的半边，继续遍历
      edge = &Graph::GetNextInFace(*edge);
    } while (edge != &firstEdge);
    // 检查边向量的大小是否为4u，如果不是则可能存在错误
    check(edges.size() == 4u);
    // 在0到边向量大小减1的范围内随机生成一个索引值
    auto randomIndex = random.RandRange(0, edges.size() - 1);
    // 返回随机选择的一对相对边，其中一条是随机索引对应的边，另一条是相隔两条边的相对边
    return {edges[randomIndex], edges[(randomIndex + 2u) % edges.size()]};
  }

  // 定义一个静态函数splitFace，它接受一个Graph类型的参数graph、一个Graph::Face类型的参数face和一个FRandomStream类型的参数random
  // 该函数的目的是将给定的面（face）进行分割
  static Graph::Face *splitFace(Graph &graph, Graph::Face &face, FRandomStream &random) {
    // 获取面中的一对随机相对边
    auto edgePair = getRandomOpposingEdges(face, random);
    // 计算第一条随机边的方向向量
    auto dir = getDirection(*edgePair.first);
    // 假设两条随机边是矩形相对的面，计算第二条随机边的方向向量
    auto otherDir = getDirection(*edgePair.second);
    // 检查两条边的方向向量在x和y方向上是否互为相反数，以此验证是否是相对边的假设成立
    check((dir.x == -1 * otherDir.x) && (dir.y == -1 * otherDir.y));
    // 如果矩形的边长小于一定的阈值，则不进行分割
    if ((std::abs(dir.x) < 2*MARGIN+1) && (std::abs(dir.y) < 2*MARGIN+1))
      return nullptr;
    // 沿着边获取一个随机点
    auto randX = (dir.x != 0 ? signOf(dir.x) * random.RandRange(MARGIN, std::abs(dir.x) - MARGIN) : 0);
    // 根据第一条随机边的方向向量，在其边上随机生成一个y坐标值
    auto randY = (dir.y != 0 ? signOf(dir.y) * random.RandRange(MARGIN, std::abs(dir.y) - MARGIN) : 0);
    // 计算在第一条随机边的源节点位置基础上，根据随机生成的坐标偏移得到的新位置
    auto position0 = getSourcePosition(*edgePair.first) + Graph::Position{randX, randY};
    // 计算在第二条随机边的目标节点位置基础上，根据随机生成的坐标偏移得到的新位置
    auto position1 = getTargetPosition(*edgePair.second) + Graph::Position{randX, randY};
    // 拆分这些边并连接起来
    Graph::Node &node0 = graph.SplitEdge(position0, *edgePair.first);
    // 在图中根据新位置分割第二条随机边，并获取分割后产生的新节点node1
    Graph::Node &node1 = graph.SplitEdge(position1, *edgePair.second);
    // 连接两个新节点，并返回新生成的面的指针
    return &graph.ConnectNodes(node0, node1);
  }

  // 定义一个静态函数randomize，它接受一个Graph类型的参数graph和一个int32类型的参数seed
  // 该函数的目的是对给定的图（graph）进行随机化操作
  static void randomize(Graph &graph, const int32 seed)
  {
    // 检查图中的节点数量是否为4u，如果不是则可能存在错误
    check(graph.CountNodes() == 4u);
    // 检查图中的半边数量是否为8u，如果不是则可能存在错误
    check(graph.CountHalfEdges() == 8u);
    // 检查图中的面数量是否为2u，如果不是则可能存在错误
    check(graph.CountFaces() == 2u);
    // 创建一个随机数生成流，使用传入的seed作为种子
    FRandomStream random(seed);
    /// @todo 我们跳过第一个面，因为它是环绕面。
    /// 但情况并非总是如此，如果以不同的方式生成图，那可能就会是
    /// 另一个（面需要被处理）了。
    // 获取图中除了第一个面之外的第一个面的指针，这里假设第一个面是周围的面，可能需要根据实际情况调整
    Graph::Face *face = &*(++graph.GetFaces().begin());
    do {
      face = splitFace(graph, *face, random);
#ifdef CARLA_ROAD_GENERATOR_EXTRA_LOG
      graph.PrintToLog();
#endif // CARLA_ROAD_GENERATOR_EXTRA_LOG
    } while (face != nullptr);
  }

 // =============================================================================
 // -- GraphGenerator -----------------------------------------------------------
 // =============================================================================

  // 定义GraphGenerator类的Generate函数，它接受三个参数：SizeX（无符号32位整数）、SizeY（无符号32位整数）和Seed（int32类型）
  // 该函数的目的是生成一个特定的图结构（DoublyConnectedEdgeList类型）
  TUniquePtr<DoublyConnectedEdgeList> GraphGenerator::Generate(
      const uint32 SizeX,
      const uint32 SizeY,
      const int32 Seed)
  {
    // 使用DoublyConnectedEdgeList中的Position类型定义一个别名Position，方便后续使用
    using Position = typename DoublyConnectedEdgeList::Position;
    // 创建一个包含4个Position类型元素的数组box，用于表示一个矩形的四个顶点位置
    std::array<Position, 4u> box;
    box[0u] = Position(0, 0);
    box[1u] = Position(0, SizeY);
    box[2u] = Position(SizeX, SizeY);
    box[3u] = Position(SizeX, 0);
    // 创建一个唯一指针指向新生成的DoublyConnectedEdgeList对象，传入矩形的顶点位置数组作为初始化参数
    auto Dcel = MakeUnique<DoublyConnectedEdgeList>(box);
    // 对生成的图结构进行随机化操作，传入随机数种子
    randomize(*Dcel, Seed);
    // 返回生成并随机化后的图结构的唯一指针
    return Dcel;
  }

} // namespace MapGen
