// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/geom/Mesh.h>

#include <string>
#include <sstream>
#include <ios>
#include <iostream>
#include <fstream>

#include <carla/geom/Math.h>

namespace carla {
namespace geom {

  bool Mesh::IsValid() const {
    // 至少应为某个顶点
    if (_vertices.empty()) {
      std::cout << "Mesh validation error: there are no vertices in the mesh." << std::endl;
      return false;
    }

    // 如果有 indices，数量必须是3的倍数
    if (!_indexes.empty() && _indexes.size() % 3 != 0) {
      std::cout << "Mesh validation error: the index amount must be multiple of 3." << std::endl;
      return false;
    }

    if (!_materials.empty() && _materials.back().index_end == 0) {
      std::cout << "Mesh validation error: last material was not closed." << std::endl;
      return false;
    }

    return true;
  }

  // 添加 三角形条带(triangle strip)
  // 三角形条带是一条，每增加一个点增加一个三角形
  // 参考：https://blog.csdn.net/tomorrow_opal/article/details/70140965
  // e.g:
  // 1   3   5   7
  // #---#---#---#
  // | / | / | / |
  // #---#---#---#
  // 2   4   6   8
  // Mesh类中的AddTriangleStrip函数，作用是向Mesh对象添加三角形条带（Triangle Strip）数据
// 参数vertices是一个包含Mesh顶点类型（Mesh::vertex_type）的向量引用，代表了构成三角形条带的顶点集合
void Mesh::AddTriangleStrip(const std::vector<Mesh::vertex_type> &vertices) {
    // 如果传入的顶点向量大小为0，说明没有顶点数据可用于构建三角形条带，直接返回，不做后续操作
    if (vertices.size() == 0) {
      return;
    }
    // 使用DEBUG_ASSERT进行调试断言，确保传入的顶点数量至少为3个
    // 因为三角形条带至少需要3个顶点才能构成最基本的图形结构，否则不符合逻辑
    DEBUG_ASSERT(vertices.size() >= 3);

    // 获取当前Mesh对象已有的顶点数量，然后在此基础上加2，用于后续构建三角形条带索引时的起始位置计算
    // 这个起始位置的计算方式是基于Mesh对象内部存储顶点和索引的机制来确定的
    size_t i = GetVerticesNum() + 2;

    // 调用AddVertices函数（应该是Mesh类内部的另一个函数）将传入的顶点数据添加到Mesh对象中
    // 这样Mesh对象就包含了构建三角形条带所需的顶点信息
    AddVertices(vertices);

    // 初始化一个布尔变量，表示索引的添加顺序是否为顺时针方向，初始化为true，即先按顺时针方向添加索引
    bool index_clockwise = true;

    // 开始循环构建三角形条带的索引，只要当前索引位置（i）小于Mesh对象的总顶点数量，就继续循环
    while (i < GetVerticesNum()) {
        // 每次循环切换索引的添加顺序，即本次循环如果是顺时针，下次就是逆时针，反之亦然
        // 这样交替的顺序可以正确地构建出连续的三角形条带
        index_clockwise =!index_clockwise;
        if (index_clockwise) {
            // 如果当前是顺时针方向，按照顺时针顺序添加索引
            // 先添加当前顶点的下一个顶点索引（i + 1）
            AddIndex(i + 1);
            // 再添加当前顶点索引（i）
            AddIndex(i);
            // 最后添加当前顶点的上一个顶点索引（i - 1）
            AddIndex(i - 1);
        } else {
            // 如果当前是逆时针方向，按照逆时针顺序添加索引
            // 先添加当前顶点的上一个顶点索引（i - 1）
            AddIndex(i - 1);
            // 再添加当前顶点索引（i）
            AddIndex(i);
            // 最后添加当前顶点的下一个顶点索引（i + 1）
            AddIndex(i + 1);
        }
        // 将索引位置向后移动一位，准备构建下一组三角形的索引
        ++i;
    }
}

  // 添加 三角形扇(triangle fan)
  // 三角形扇是第一个点是中心，其他的点都围绕着它，第2个点和最后一个点是一样的就可以围成一个圈了
  // 例如：
  // 2   1   6
  // #---#---#
  // | / | \ |
  // #---#---#
  // 3   4   5
  void Mesh::AddTriangleFan(const std::vector<Mesh::vertex_type> &vertices) {
    DEBUG_ASSERT(vertices.size() >= 3);
    const size_t initial_index = GetVerticesNum() + 1;
    size_t i = GetVerticesNum() + 2;
    AddVertices(vertices);
    while (i < GetVerticesNum()) {
      AddIndex(initial_index);
      AddIndex(i);
      AddIndex(i + 1);
      ++i;
    }
  }

  void Mesh::AddVertex(vertex_type vertex) {
    _vertices.push_back(vertex);
  }

  void Mesh::AddVertices(const std::vector<Mesh::vertex_type> &vertices) {
    std::copy(vertices.begin(), vertices.end(), std::back_inserter(_vertices));
  }

  void Mesh::AddNormal(normal_type normal) {
    _normals.push_back(normal);
  }

  void Mesh::AddIndex(index_type index) {
    _indexes.push_back(index);
  }

  void Mesh::AddUV(uv_type uv) {
    _uvs.push_back(uv);
  }

  void Mesh::AddUVs(const std::vector<uv_type> & uv) {
    std::copy(uv.begin(), uv.end(), std::back_inserter(_uvs));
  }

  void Mesh::AddMaterial(const std::string &material_name) {
    const size_t open_index = _indexes.size();
    if (!_materials.empty()) {
      if (_materials.back().index_end == 0) {
        // @todo: 将此注释更改为调试警告
        // std::cout << "last material was not closed, closing it..." << std::endl;
        EndMaterial();
      }
    }
    if (open_index % 3 != 0) {
      std::cout << "open_index % 3 != 0" << std::endl;
      return;
    }
    _materials.emplace_back(material_name, open_index, 0);
  }

  void Mesh::EndMaterial() {
    const size_t close_index = _indexes.size();
    if (_materials.empty() ||
        _materials.back().index_start == close_index ||
        _materials.back().index_end != 0) {
      // @todo: 将此注释更改为调试警告
      // std::cout << "WARNING: Bad end of material. Material not started." << std::endl;
      return;
    }
    if (_indexes.empty() || close_index % 3 != 0) {
      // @todo: 将此注释更改为调试警告
      // std::cout << "WARNING: Bad end of material. Face not started/ended." << std::endl;
      return;
    }
    _materials.back().index_end = close_index;
  }

  std::string Mesh::GenerateOBJ() const {
    if (!IsValid()) {
      return "";
    }
    std::stringstream out;
    out << std::fixed; // 避免使用科学计数法

    out << "# List of geometric vertices, with (x, y, z) coordinates." << std::endl;
    for (auto &v : _vertices) {
      out << "v " << v.x << " " << v.y << " " << v.z << std::endl;
    }

    if (!_uvs.empty()) {
      out << std::endl << "# List of texture coordinates, in (u, v) coordinates, these will vary between 0 and 1." << std::endl;
      for (auto &vt : _uvs) {
        out << "vt " << vt.x << " " << vt.y << std::endl;
      }
    }

    if (!_normals.empty()) {
      out << std::endl << "# List of vertex normals in (x, y, z) form; normals might not be unit vectors." << std::endl;
      for (auto &vn : _normals) {
        out << "vn " << vn.x << " " << vn.y << " " << vn.z << std::endl;
      }
    }

    if (!_indexes.empty()) {
      out << std::endl << "# Polygonal face element." << std::endl;
      auto it_m = _materials.begin();
      auto it = _indexes.begin();
      size_t index_counter = 0u;
      while (it != _indexes.end()) {
        // While exist materials
        if (it_m != _materials.end()) {
          // 如果当前材料在此索引处结束
          if (it_m->index_end == index_counter) {
            ++it_m;
          }
          // 如果当前材料从该索引开始
          if (it_m->index_start == index_counter) {
            out << "\nusemtl " << it_m->name << std::endl;
          }
        }

        // 使用 3 个连续的索引添加实际表面
        out << "f " << *it; ++it;
        out << " " << *it; ++it;
        out << " " << *it << std::endl; ++it;

        index_counter += 3;
      }
    }

    return out.str();
  }

  std::string Mesh::GenerateOBJForRecast() const {
    if (!IsValid()) {
      return "";
    }
    std::stringstream out;
    out << std::fixed; // 避免使用科学计数法

    out << "# List of geometric vertices, with (x, y, z) coordinates." << std::endl;
    for (auto &v : _vertices) {
      // 为 Recast 库切换“y”和“z”
      out << "v " << v.x << " " << v.z << " " << v.y << std::endl;
    }

    if (!_indexes.empty()) {
      out << std::endl << "# Polygonal face element." << std::endl;
      auto it_m = _materials.begin();
      auto it = _indexes.begin();
      size_t index_counter = 0u;
      while (it != _indexes.end()) {
        // While exist materials
        if (it_m != _materials.end()) {
          // 如果当前材料在此索引处结束
          if (it_m->index_end == index_counter) {
            ++it_m;
          }
          // 如果当前材料从该索引开始
          if (it_m->index_start == index_counter) {
            out << "\nusemtl " << it_m->name << std::endl;
          }
        }
        // 使用 3 个连续的索引添加实际面由于空间已经改变，因此将面构建方向更改为顺时针。
        out << "f " << *it; ++it;
        const auto i_2 = *it; ++it;
        const auto i_3 = *it; ++it;
        out << " " << i_3 << " " << i_2 << std::endl;
        index_counter += 3;
      }
    }

    return out.str();
  }

  std::string Mesh::GeneratePLY() const {
    if (!IsValid()) {
      return "Invalid Mesh";
    }
    // 生成头
    std::stringstream out;
    return out.str();
  }

  const std::vector<Mesh::vertex_type> &Mesh::GetVertices() const {
    return _vertices;
  }

  std::vector<Mesh::vertex_type> &Mesh::GetVertices() {
    return _vertices;
  }

  size_t Mesh::GetVerticesNum() const {
    return _vertices.size();
  }

  const std::vector<Mesh::normal_type> &Mesh::GetNormals() const {
    return _normals;
  }

  const std::vector<Mesh::index_type> &Mesh::GetIndexes() const {
    return _indexes;
  }

  std::vector<Mesh::index_type>& Mesh::GetIndexes() {
    return _indexes;
  }
  size_t Mesh::GetIndexesNum() const {
    return _indexes.size();
  }

  const std::vector<Mesh::uv_type> &Mesh::GetUVs() const {
    return _uvs;
  }

  const std::vector<Mesh::material_type> &Mesh::GetMaterials() const {
    return _materials;
  }

  size_t Mesh::GetLastVertexIndex() const {
    return _vertices.size();
  }

  Mesh& Mesh::ConcatMesh(const Mesh& rhs, int num_vertices_to_link) {

    if (!rhs.IsValid()){
      return *this += rhs;
    }
    const size_t v_num = GetVerticesNum();
    const size_t i_num = GetIndexesNum();

    _vertices.insert(
      _vertices.end(),
      rhs.GetVertices().begin(),
      rhs.GetVertices().end());

    _normals.insert(
      _normals.end(),
      rhs.GetNormals().begin(),
      rhs.GetNormals().end());

    const size_t vertex_to_start_concating = v_num - num_vertices_to_link;
    for( size_t i = 1; i < num_vertices_to_link; ++i ) {
      _indexes.push_back( vertex_to_start_concating + i );
      _indexes.push_back( vertex_to_start_concating + i  + 1 );
      _indexes.push_back( v_num + i );

      _indexes.push_back( vertex_to_start_concating + i + 1);
      _indexes.push_back( v_num + i + 1);
      _indexes.push_back( v_num + i);
    }

    std::transform(
      rhs.GetIndexes().begin(),
      rhs.GetIndexes().end(),
      std::back_inserter(_indexes),
      [=](size_t index) {return index + v_num; });

    _uvs.insert(
      _uvs.end(),
      rhs.GetUVs().begin(),
      rhs.GetUVs().end());

    std::transform(
      rhs.GetMaterials().begin(),
      rhs.GetMaterials().end(),
      std::back_inserter(_materials),
      [=](MeshMaterial mat) {
        mat.index_start += i_num;
        mat.index_end += i_num;
        return mat;
      });

    return *this;
  }

  Mesh &Mesh::operator+=(const Mesh &rhs) {
    const size_t v_num = GetVerticesNum();
    const size_t i_num = GetIndexesNum();

    _vertices.insert(
        _vertices.end(),
        rhs.GetVertices().begin(),
        rhs.GetVertices().end());

    _normals.insert(
        _normals.end(),
        rhs.GetNormals().begin(),
        rhs.GetNormals().end());

    std::transform(
        rhs.GetIndexes().begin(),
        rhs.GetIndexes().end(),
        std::back_inserter(_indexes),
        [=](size_t index) {return index + v_num;});

    _uvs.insert(
        _uvs.end(),
        rhs.GetUVs().begin(),
        rhs.GetUVs().end());

    std::transform(
        rhs.GetMaterials().begin(),
        rhs.GetMaterials().end(),
        std::back_inserter(_materials),
        [=](MeshMaterial mat) {
          mat.index_start += i_num;
          mat.index_end += i_num;
          return mat;
        });

    return *this;
  }

  Mesh operator+(const Mesh &lhs, const Mesh &rhs) {
    Mesh m = lhs;
    return m += rhs;
  }

} // namespace geom
} // namespace carla
