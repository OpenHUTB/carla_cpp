// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// 简化
// 
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/geom/Simplification.h"
#include "simplify/Simplify.h"

// 定义在carla和geom命名空间下，这里应该是实现与几何图形简化相关的功能模块
namespace carla {
namespace geom {
// 简化函数，对给定的网格进行简化
  void Simplification::Simplificate(const std::unique_ptr<geom::Mesh>& pmesh){
  	// 创建一个简化对象
    Simplify::SimplificationObject Simplification;
    // 将输入网格的顶点转换为简化对象的顶点格式，并添加到简化对象中
    for (carla::geom::Vector3D& current_vertex : pmesh->GetVertices()) {
      Simplify::Vertex v;
      // 将当前顶点的 x 坐标赋值给简化对象顶点格式中的对应分量，完成数据的拷贝转换。
      v.p.x = current_vertex.x;
      v.p.y = current_vertex.y;
      v.p.z = current_vertex.z;
      Simplification.vertices.push_back(v);
    }
    // 将输入网格的索引转换为简化对象的三角形格式，并添加到简化对象中
    for (size_t i = 0; i < pmesh->GetIndexes().size() - 2; i += 3) {
      Simplify::Triangle t;
      t.material = 0;
      // 获取输入网格的索引列表，以便后续构建三角形的顶点索引关系。
      auto indices = pmesh->GetIndexes();
      // 根据索引确定三角形的第一个顶点在顶点列表中的位置，注意这里减 1 可能是因为索引的计数方式在不同数据结构中的差异（比如从 0 开始还是从 1 开始等）。
      t.v[0] = (indices[i]) - 1;
      t.v[1] = (indices[i + 1]) - 1;
      t.v[2] = (indices[i + 2]) - 1;
      Simplification.triangles.push_back(t);
    }

    // 减少到多边形的 X%
    float target_size = Simplification.triangles.size();
    // 执行简化操作，将三角形数量减少到目标大小
    // 执行简化操作，调用 Simplify::SimplificationObject 的 simplify_mesh 方法，传入根据简化比例计算出的目标三角形数量，
    // 这个方法内部应该实现了具体的网格简化算法，例如基于边折叠（Edge Collapse）等常见图形学简化算法，通过合理地去除一些顶点和边（在这个函数中体现为三角形数量的减少）来简化网格模型，
    // 同时尽量保持模型的外观特征等重要信息，类似于在图结构中通过一些规则合并节点、删除边来简化图的表示，同时保留关键拓扑信息。
    Simplification.simplify_mesh((target_size * simplification_percentage));
// 清空输入网格的顶点和索引列表
    pmesh->GetVertices().clear();
    pmesh->GetIndexes().clear();
// 将简化后的顶点转换回输入网格的顶点格式，并添加到输入网格中
    for (Simplify::Vertex& current_vertex : Simplification.vertices) {
      carla::geom::Vector3D v;
      v.x = current_vertex.p.x;
      v.y = current_vertex.p.y;
      v.z = current_vertex.p.z;
      pmesh->AddVertex(v);
    }
// 将简化后的索引转换回输入网格的索引格式，并添加到输入网格中
   // 以下循环将简化后 Simplify::SimplificationObject 中的索引数据（三角形的顶点索引关系）转换回输入网格（pmesh）的索引格式，并添加回输入网格中。
   // 同样是将经过简化后重新组织的三角形面片信息（通过索引体现）更新到原始的网格数据结构中，完成整个网格简化后的数据更新操作，使得输入网格对象 pmesh 最终保存的是简化后的模型数据。   
  
    for (size_t i = 0; i < Simplification.triangles.size(); ++i) {
      pmesh->GetIndexes().push_back((Simplification.triangles[i].v[0]) + 1);
      pmesh->GetIndexes().push_back((Simplification.triangles[i].v[1]) + 1);
      pmesh->GetIndexes().push_back((Simplification.triangles[i].v[2]) + 1);
    }
  }

} // namespace geom
} // namespace carla
