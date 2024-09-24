// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GraphTypes.h"
#include "Position.h"
#include "Util/ListView.h"

#include <array>
#include <list>

namespace MapGen {

  /// 简单的双连通边链表结构。它只允许添加元素，不允许删除元素。
  class CARLA_API DoublyConnectedEdgeList : private NonCopyable
  {
    // =========================================================================
    // -- DCEL 类型 -----------------------------------------------------------
    // =========================================================================

  public:

    using Position = MapGen::Position<int32>;

    struct Node;
    struct HalfEdge;
    struct Face;

    struct Node : public GraphNode
    {
      friend DoublyConnectedEdgeList;

      Node(const Position &Pos) : Position(Pos) {}

      Node &operator=(const Node &) = delete;

      const DoublyConnectedEdgeList::Position &GetPosition() const
      {
        return Position;
      }

    private:
      DoublyConnectedEdgeList::Position Position;
      HalfEdge *LeavingHalfEdge = nullptr;
    };

    struct HalfEdge : public GraphHalfEdge
    {
      friend DoublyConnectedEdgeList;

      HalfEdge &operator=(const HalfEdge &) = delete;

    private:
      Node *Source = nullptr;
      Node *Target = nullptr;
      HalfEdge *Next = nullptr;
      HalfEdge *Pair = nullptr;
      Face *Face = nullptr;
    };

    struct Face : public GraphFace
    {
      friend DoublyConnectedEdgeList;

      Face &operator=(const Face &) = delete;

    private:
      HalfEdge *HalfEdge = nullptr;
    };

    using NodeContainer = std::list<Node>;
    using NodeIterator = typename NodeContainer::iterator;
    using ConstNodeIterator = typename NodeContainer::const_iterator;

    using HalfEdgeContainer = std::list<HalfEdge>;
    using HalfEdgeIterator = typename HalfEdgeContainer::iterator;
    using ConstHalfEdgeIterator = typename HalfEdgeContainer::const_iterator;

    using FaceContainer = std::list<Face>;
    using FaceIterator = typename FaceContainer::iterator;
    using ConstFaceIterator = typename FaceContainer::const_iterator;

    // =========================================================================
    // -- 构造函数和析构函数 ----------------------------------------------------
    // =========================================================================

  public:

    /// 创建一个有2个节点、2个边和1个面的双连通边链表DoublyConnectedEdgeList。
    explicit DoublyConnectedEdgeList(const Position &Position0, const Position &Position1);

    /// 创建一个由N个节点组成双连通链表DoublyConnectedEdgeList环。
    template <size_t N>
    explicit DoublyConnectedEdgeList(const std::array<Position, N> &Cycle)
      : DoublyConnectedEdgeList(Cycle[0u], Cycle[1u])
    {
      static_assert(N > 2u, "Not enough nodes to make a cycle!");
      for (auto i = 2u; i < Cycle.size(); ++i) {
        AddNode(Cycle[i], Nodes.back());
      }
      ConnectNodes(Nodes.front(), Nodes.back());
    }

    ~DoublyConnectedEdgeList();

    // =========================================================================
    /// @name 向图中添加元素-----------------------------------------------------
    // =========================================================================
    /// {
  public:

    /// Add a node at @a NodePosition and attach it to @a OtherNode.
    ///
    /// 时间复杂度为 O(n*log(n))，其中 n 是离开节点 @a OtherNode 的边数。
    ///
    /// @return 新生成的节点。
    Node &AddNode(const Position &NodePosition, Node &OtherNode);

    /// 在 @a 位置分割 @a HalfEdge （和它的配对）
    ///
    /// 时间复杂度为 O(n*log(n))，其中 n 是离开 @a HalfEdge 源的边数
    ///
    /// @return 新生成的节点。
    Node &SplitEdge(const Position &Position, HalfEdge &HalfEdge);

    /// 用一对边连接两个节点。
    ///
    /// 假设两个节点由同一面连接。
    ///
    /// 时间复杂度为 O(n0*log(n0) + n1*log(n1) + nf)，
    /// 其中 n0 和 n1 分别是离开节点 @a Node0 和节点 @a Node1 的边数。
    /// 并且 nf 是包含两个节点的面的边数。
    ///
    /// @return 新生成的面。
    Face &ConnectNodes(Node &Node0, Node &Node1);

    /// @}
    // =========================================================================
    /// @name 统计图元素的数目 --------------------------------------------------
    // =========================================================================
    /// @{
  public:

    size_t CountNodes() const
    {
      return Nodes.size();
    }

    size_t CountHalfEdges() const
    {
      return HalfEdges.size();
    }

    size_t CountFaces() const
    {
      return Faces.size();
    }

    /// @}
    // =========================================================================
    /// @name 访问图的元素 ------------------------------------------------------
    // =========================================================================
    /// @{
  public:

    ListView<NodeIterator> GetNodes()
    {
      return ListView<NodeIterator>(Nodes);
    }

    ListView<ConstNodeIterator> GetNodes() const
    {
      return ListView<ConstNodeIterator>(Nodes);
    }

    ListView<HalfEdgeIterator> GetHalfEdges()
    {
      return ListView<HalfEdgeIterator>(HalfEdges);
    }

    ListView<ConstHalfEdgeIterator> GetHalfEdges() const
    {
      return ListView<ConstHalfEdgeIterator>(HalfEdges);
    }

    ListView<FaceIterator> GetFaces()
    {
      return ListView<FaceIterator>(Faces);
    }

    ListView<ConstFaceIterator> GetFaces() const
    {
      return ListView<ConstFaceIterator>(Faces);
    }

    /// @}
    // =========================================================================
    /// @name 访问图指针 --------------------------------------------------------
    // =========================================================================
    /// @{
  public:

    // -- 主要指针 --------------------------------------------------------------

    static Node &GetSource(HalfEdge &halfEdge)
    {
      check(halfEdge.Source != nullptr);
      return *halfEdge.Source;
    }

    static const Node &GetSource(const HalfEdge &halfEdge)
    {
      check(halfEdge.Source != nullptr);
      return *halfEdge.Source;
    }

    static Node &GetTarget(HalfEdge &halfEdge)
    {
      check(halfEdge.Target != nullptr);
      return *halfEdge.Target;
    }

    static const Node &GetTarget(const HalfEdge &halfEdge)
    {
      check(halfEdge.Target != nullptr);
      return *halfEdge.Target;
    }

    static HalfEdge &GetPair(HalfEdge &halfEdge)
    {
      check(halfEdge.Pair != nullptr);
      return *halfEdge.Pair;
    }

    static const HalfEdge &GetPair(const HalfEdge &halfEdge)
    {
      check(halfEdge.Pair != nullptr);
      return *halfEdge.Pair;
    }

    static Face &GetFace(HalfEdge &halfEdge)
    {
      check(halfEdge.Face != nullptr);
      return *halfEdge.Face;
    }

    static const Face &GetFace(const HalfEdge &halfEdge)
    {
      check(halfEdge.Face != nullptr);
      return *halfEdge.Face;
    }

    static HalfEdge &GetLeavingHalfEdge(Node &node)
    {
      check(node.LeavingHalfEdge != nullptr);
      return *node.LeavingHalfEdge;
    }

    static const HalfEdge &GetLeavingHalfEdge(const Node &node)
    {
      check(node.LeavingHalfEdge != nullptr);
      return *node.LeavingHalfEdge;
    }

    static HalfEdge &GetHalfEdge(Face &face)
    {
      check(face.HalfEdge != nullptr);
      return *face.HalfEdge;
    }

    static const HalfEdge &GetHalfEdge(const Face &face)
    {
      check(face.HalfEdge != nullptr);
      return *face.HalfEdge;
    }

    // -- 二级指针 ------------------------------------------------------------

    static HalfEdge &GetNextInFace(HalfEdge &halfEdge)
    {
      check(halfEdge.Next != nullptr);
      return *halfEdge.Next;
    }

    static const HalfEdge &GetNextInFace(const HalfEdge &halfEdge)
    {
      check(halfEdge.Next != nullptr);
      return *halfEdge.Next;
    }

    static HalfEdge &GetNextInNode(HalfEdge &halfEdge)
    {
      return GetNextInFace(GetPair(halfEdge));
    }

    static const HalfEdge &GetNextInNode(const HalfEdge &halfEdge)
    {
      return GetNextInFace(GetPair(halfEdge));
    }

    /// @}
    // =========================================================================
    /// @name 其他成员函数 ------------------------------------------------------
    // =========================================================================
    /// @{
  public:

    /// 返回 half-edge 的角度，范围为 [-pi, pi]
    static float GetAngle(const HalfEdge &halfEdge);

#ifdef CARLA_ROAD_GENERATOR_EXTRA_LOG
    void PrintToLog() const;
 #endif // CARLA_ROAD_GENERATOR_EXTRA_LOG

    /// @}
    // =========================================================================
    // -- 私有成员 --------------------------------------------------------------
    // =========================================================================

  private:

    NodeContainer Nodes;

    HalfEdgeContainer HalfEdges;

    FaceContainer Faces;
  };

} // namespace MapGen
