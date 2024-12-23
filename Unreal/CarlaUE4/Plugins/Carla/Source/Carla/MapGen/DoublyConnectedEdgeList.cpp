// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "DoublyConnectedEdgeList.h"

#include <cmath>
#include <map>

#ifdef CARLA_ROAD_GENERATOR_EXTRA_LOG
#include <sstream>
#endif // CARLA_ROAD_GENERATOR_EXTRA_LOG

namespace MapGen {

  // ===========================================================================
  // --局部静态方法 ---------------------------------------------------
  // ===========================================================================

#ifdef CARLA_ROAD_GENERATOR_EXTRA_LOG

  static void ResetIndices() {
    GraphNode::ResetIndex();
    GraphHalfEdge::ResetIndex();
    GraphFace::ResetIndex();
  }

#endif // CARLA_ROAD_GENERATOR_EXTRA_LOG

  /// 返回一对 {prev, next}，其中 prev/next 是围绕边的源节点逆时针方向的前一条/后一条边。即，边的位置介于 prev 和 next 之间
  ///
  /// 注意：始终返回从节点指向外的半边
  ///
  /// 时间复杂度为 O(n*log(n))，其中 n 是边的源节点的边数
  static std::pair<DoublyConnectedEdgeList::HalfEdge *, DoublyConnectedEdgeList::HalfEdge *>
  FindPositionInNode(DoublyConnectedEdgeList::HalfEdge &halfEdge)
  {
    using Dcel = DoublyConnectedEdgeList;
    // 从 [-π, π] 到 [0, 1]
    auto normalize = [](auto a) {
      constexpr float twoPi = 2.0 * 3.14159265359;
      a /= twoPi;
      while (a >= 1.0) a -= 1.0;
      while (a <  0.0) a += 1.0;
      return a;
    };
    auto angle = Dcel::GetAngle(halfEdge);
    std::map<decltype(angle), Dcel::HalfEdge *> edgeMap;
    //遍历源节点中的每一条半边
    auto &firstHalfEdge = Dcel::GetLeavingHalfEdge(Dcel::GetSource(halfEdge));
    auto *edge = &firstHalfEdge;
    do {
      if (edge != &halfEdge) {
        auto alpha = DoublyConnectedEdgeList::GetAngle(*edge);
        auto a = normalize(alpha - angle);
        edgeMap.insert({a, edge});
      }
      edge = &Dcel::GetNextInNode(*edge);
    } while (edge != &firstHalfEdge);
    check(!edgeMap.empty());
    return {edgeMap.rbegin()->second, edgeMap.begin()->second};
  }

  // ===========================================================================
  // -- 构造函数和析构函数 --------------------------------------------
  // ===========================================================================

  DoublyConnectedEdgeList::DoublyConnectedEdgeList(
      const Position &Position0,
      const Position &Position1) :
    Nodes(),
    HalfEdges(2u),
    Faces(1u)
  {
    Nodes.emplace_back(Position0);
    Nodes.emplace_back(Position1);

    Faces.front().HalfEdge = &HalfEdges.front();

    HalfEdges.front().Source = &Nodes.front();
    HalfEdges.front().Target = &Nodes.back();
    HalfEdges.front().Next = &HalfEdges.back();
    HalfEdges.front().Pair = &HalfEdges.back();
    HalfEdges.front().Face = &Faces.front();

    HalfEdges.back().Source = &Nodes.back();
    HalfEdges.back().Target = &Nodes.front();
    HalfEdges.back().Next = &HalfEdges.front();
    HalfEdges.back().Pair = &HalfEdges.front();
    HalfEdges.back().Face = &Faces.front();

    Nodes.front().LeavingHalfEdge = &HalfEdges.front();
    Nodes.back().LeavingHalfEdge = &HalfEdges.back();
  }

  DoublyConnectedEdgeList::~DoublyConnectedEdgeList()
  {
#ifdef CARLA_ROAD_GENERATOR_EXTRA_LOG
    ResetIndices();
#endif // CARLA_ROAD_GENERATOR_EXTRA_LOG
  }

  // ===========================================================================
  // -- 将元素添加到图中 -------------------------------------------
  // ===========================================================================

  DoublyConnectedEdgeList::Node &DoublyConnectedEdgeList::AddNode(
      const Position &NodePosition,
      Node &OtherNode)
  {
    Nodes.emplace_back(NodePosition);
    auto &newNode = Nodes.back();
    HalfEdges.emplace_back();
    auto &edge0 = HalfEdges.back();
    HalfEdges.emplace_back();
    auto &edge1 = HalfEdges.back();

    edge0.Source = &newNode;
    edge0.Target = &OtherNode;
    edge0.Pair = &edge1;

    edge1.Source = &OtherNode;
    edge1.Target = &newNode;
    edge1.Pair = &edge0;

    HalfEdge *prev;
    HalfEdge *next;
    std::tie(prev, next) = FindPositionInNode(edge1);

    edge0.Next = next;
    edge0.Face = next->Face;

    edge1.Next = &edge0;
    edge1.Face = next->Face;

    prev->Pair->Next = &edge1;

    newNode.LeavingHalfEdge = &edge0;
    return newNode;
  }

  DoublyConnectedEdgeList::Node &DoublyConnectedEdgeList::SplitEdge(
      const Position &Position,
      HalfEdge &edge)
  {
    HalfEdges.emplace_back();
    auto &edge0 = HalfEdges.back();
    HalfEdges.emplace_back();
    auto &edge1 = HalfEdges.back();

    Nodes.emplace_back(Position);
    auto &newNode = Nodes.back();

    auto &node0 = *edge.Source;

    //边缘的相反方向
    edge0.Source = &newNode;
    edge0.Target = &node0;
    edge0.Pair = &edge1;
    edge0.Face = edge.Pair->Face;

    // 与边缘方向相同
    edge1.Source = &node0;
    edge1.Target = &newNode;
    edge1.Pair = &edge0;
    edge1.Next = &edge;
    edge1.Face = edge.Face;

    // 修复与节点0的连接
    HalfEdge *prev;
    HalfEdge *next;
    std::tie(prev, next) = FindPositionInNode(edge);
    edge0.Next = next;
    prev->Pair->Next = &edge1;

    // 修复被拆分的那一对
    edge.Source = &newNode;
    edge.Pair->Target = &newNode;
    edge.Pair->Next = &edge0;

    // 修复节点的边
    node0.LeavingHalfEdge = &edge1;
    newNode.LeavingHalfEdge = &edge0;

    return newNode;
  }

  DoublyConnectedEdgeList::Face &DoublyConnectedEdgeList::ConnectNodes(
      Node &Node0,
      Node &Node1)
  {
    Faces.emplace_back();
    auto &newFace = Faces.back();
    HalfEdges.emplace_back();
    auto &edge0 = HalfEdges.back();
    HalfEdges.emplace_back();
    auto &edge1 = HalfEdges.back();

    edge0.Source = &Node0;
    edge0.Target = &Node1;
    edge0.Pair = &edge1;
    edge1.Source = &Node1;
    edge1.Target = &Node0;
    edge1.Pair = &edge0;

    //将边连接到节点0
    HalfEdge *prev0;
    HalfEdge *next0;
    std::tie(prev0, next0) = FindPositionInNode(edge0);
    edge1.Next = next0;
    prev0->Pair->Next = &edge0;

    //将边连接到节点1
    HalfEdge *prev1;
    HalfEdge *next1;
    std::tie(prev1, next1) = FindPositionInNode(edge1);
    edge0.Next = next1;
    prev1->Pair->Next = &edge1;

    //将面附加到新创建的边上
    auto &oldFace = *next1->Face;
    oldFace.HalfEdge = &edge0;
    newFace.HalfEdge = &edge1;

    // 遍历每个面的边并修正指针
    auto fixFace = [](Face &face) {
      auto &firstEdge = GetHalfEdge(face);
      auto *edge = &firstEdge;
      do {
        edge->Face = &face;
        edge = &GetNextInFace(*edge);
      } while (edge != &firstEdge);
    };
    fixFace(oldFace);
    fixFace(newFace);

    return newFace;
  }

  // ===========================================================================
  // --其他成员函数 -------------------------------------------------
  // ===========================================================================

  float DoublyConnectedEdgeList::GetAngle(const HalfEdge &halfEdge) {
    auto src = GetSource(halfEdge).GetPosition();
    auto trg = GetTarget(halfEdge).GetPosition();
    auto dir = trg - src; // @todo normalize?
    return std::atan2(static_cast<double>(dir.y), static_cast<double>(dir.x));
  }

#ifdef CARLA_ROAD_GENERATOR_EXTRA_LOG

  void DoublyConnectedEdgeList::PrintToLog() const
  {
    std::wstringstream sout;
    {
      sout << "iterate all nodes:       ";
      for (auto &node : GetNodes()) {
        auto p = node.GetPosition();
        sout << node << "{" << p.x << "," << p.y << "} ";
      }
      sout << "\n";
    }
    {
      sout << "iterate all faces:       ";
      for (auto &face : GetFaces()) {
        sout << face << " ";
      }
      sout << "\n";
    }
    {
      sout << "iterate all edges:       ";
      for (auto &edge : GetHalfEdges()) {
        auto &src = GetSource(edge);
        auto &trg = GetTarget(edge);
        auto &face = GetFace(edge);
        sout << edge << "{" << src << "->" << trg << "," << face << "} ";
      }
      sout << "\n";
    }
    {
      sout << "iterate nodes in face:   ";
      for (auto &face : GetFaces()) {
        sout << face << "{ ";
        auto &firstEdge = GetHalfEdge(face);
        const auto *edge = &firstEdge;
        do {
          sout << GetSource(*edge) << " ";
          edge = &GetNextInFace(*edge);
        } while (edge != &firstEdge);
        sout << "} ";
      }
      sout << "\n";
    }
    {
      sout << "iterate edges in node:   ";
      for (auto &node : GetNodes()) {
        sout << node << "{ ";
        auto &firstEdge = GetLeavingHalfEdge(node);
        const auto *edge = &firstEdge;
        do {
          sout << GetTarget(*edge) << " ";
          edge = &GetNextInNode(*edge);
        } while (edge != &firstEdge);
        sout << "} ";
      }
      sout << "\n";
    }
    UE_LOG(LogCarla, Log, TEXT("\n%s"), sout.str().c_str());
  }

#endif // CARLA_ROAD_GENERATOR_EXTRA_LOG

} // namespace MapGen
