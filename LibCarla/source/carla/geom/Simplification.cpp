// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// 简化
// 
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/geom/Simplification.h"
#include "simplify/Simplify.h"

namespace carla {
namespace geom {
// 简化函数，对给定的网格进行简化
  void Simplification::Simplificate(const std::unique_ptr<geom::Mesh>& pmesh){
  	// 创建一个简化对象
    Simplify::SimplificationObject Simplification;
    // 将输入网格的顶点转换为简化对象的顶点格式，并添加到简化对象中
    for (carla::geom::Vector3D& current_vertex : pmesh->GetVertices()) {
      Simplify::Vertex v;
      v.p.x = current_vertex.x;
      v.p.y = current_vertex.y;
      v.p.z = current_vertex.z;
      Simplification.vertices.push_back(v);
    }
    // 将输入网格的索引转换为简化对象的三角形格式，并添加到简化对象中
    for (size_t i = 0; i < pmesh->GetIndexes().size() - 2; i += 3) {
      Simplify::Triangle t;
      t.material = 0;
      auto indices = pmesh->GetIndexes();
      t.v[0] = (indices[i]) - 1;
      t.v[1] = (indices[i + 1]) - 1;
      t.v[2] = (indices[i + 2]) - 1;
      Simplification.triangles.push_back(t);
    }

    // 减少到多边形的 X%
    float target_size = Simplification.triangles.size();
    // 执行简化操作，将三角形数量减少到目标大小
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
    for (size_t i = 0; i < Simplification.triangles.size(); ++i) {
      pmesh->GetIndexes().push_back((Simplification.triangles[i].v[0]) + 1);
      pmesh->GetIndexes().push_back((Simplification.triangles[i].v[1]) + 1);
      pmesh->GetIndexes().push_back((Simplification.triangles[i].v[2]) + 1);
    }
  }

} // namespace geom
} // namespace carla
