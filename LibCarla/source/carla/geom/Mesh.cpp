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

// 判断网格是否有效，必须至少包含一个顶点，索引必须是3的倍数等
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

  // 如果有材料且最后一个材料未关闭，返回无效
  if (!_materials.empty() && _materials.back().index_end == 0) {
    std::cout << "Mesh validation error: last material was not closed." << std::endl;
    return false;
  }

  return true;
}

// 添加三角形条带（Triangle Strip）
// 三角形条带是每增加一个点就增加一个三角形
void Mesh::AddTriangleStrip(const std::vector<Mesh::vertex_type> &vertices) {
  if (vertices.size() == 0) {
    return;
  }
  DEBUG_ASSERT(vertices.size() >= 3);  // 确保至少有3个顶点
  size_t i = GetVerticesNum() + 2;  // 索引从当前顶点数加2开始
  AddVertices(vertices);  // 将顶点添加到网格中
  bool index_clockwise = true;  // 默认按照顺时针添加索引
  while (i < GetVerticesNum()) {
    index_clockwise = !index_clockwise;  // 反转顺时针方向
    if (index_clockwise) {
      AddIndex(i + 1);  // 按照顺时针顺序添加索引
      AddIndex(i);
      AddIndex(i - 1);
    } else {
      AddIndex(i - 1);  // 按照逆时针顺序添加索引
      AddIndex(i);
      AddIndex(i + 1);
    }
    ++i;
  }
}

// 添加三角形扇（Triangle Fan）
// 三角形扇是一个中心点和围绕它的多个点连接形成三角形
void Mesh::AddTriangleFan(const std::vector<Mesh::vertex_type> &vertices) {
  DEBUG_ASSERT(vertices.size() >= 3);  // 确保至少有3个顶点
  const size_t initial_index = GetVerticesNum() + 1;  // 获取初始的顶点索引
  size_t i = GetVerticesNum() + 2;
  AddVertices(vertices);  // 添加顶点
  while (i < GetVerticesNum()) {
    AddIndex(initial_index);  // 将第一个顶点作为中心
    AddIndex(i);
    AddIndex(i + 1);
    ++i;
  }
}

// 添加单个顶点到网格
void Mesh::AddVertex(vertex_type vertex) {
  _vertices.push_back(vertex);
}

// 添加多个顶点到网格
void Mesh::AddVertices(const std::vector<Mesh::vertex_type> &vertices) {
  std::copy(vertices.begin(), vertices.end(), std::back_inserter(_vertices));
}

// 添加法线到网格
void Mesh::AddNormal(normal_type normal) {
  _normals.push_back(normal);
}

// 添加索引到网格
void Mesh::AddIndex(index_type index) {
  _indexes.push_back(index);
}

// 添加UV坐标到网格
void Mesh::AddUV(uv_type uv) {
  _uvs.push_back(uv);
}

// 添加多个UV坐标到网格
void Mesh::AddUVs(const std::vector<uv_type> & uv) {
  std::copy(uv.begin(), uv.end(), std::back_inserter(_uvs));
}

// 添加材质到网格
void Mesh::AddMaterial(const std::string &material_name) {
  const size_t open_index = _indexes.size();  // 获取当前索引的位置
  if (!_materials.empty()) {
    // 如果上一个材质没有结束，关闭它
    if (_materials.back().index_end == 0) {
      EndMaterial();
    }
  }
  if (open_index % 3 != 0) {
    std::cout << "open_index % 3 != 0" << std::endl;
    return;
  }
  _materials.emplace_back(material_name, open_index, 0);  // 新建材质并添加
}

// 结束当前材质的定义
void Mesh::EndMaterial() {
  const size_t close_index = _indexes.size();
  if (_materials.empty() ||
      _materials.back().index_start == close_index ||
      _materials.back().index_end != 0) {
    return;  // 如果没有材质或材质没有开始，则返回
  }
  if (_indexes.empty() || close_index % 3 != 0) {
    return;  // 如果索引不合法，返回
  }
  _materials.back().index_end = close_index;  // 设置材质结束索引
}

// 生成OBJ格式的字符串，包含顶点、法线、UV、面和材质信息
std::string Mesh::GenerateOBJ() const {
  if (!IsValid()) {
    return "";  // 如果网格无效，返回空字符串
  }
  std::stringstream out;
  out << std::fixed;  // 避免使用科学计数法

  // 写入顶点数据
  out << "# List of geometric vertices, with (x, y, z) coordinates." << std::endl;
  for (auto &v : _vertices) {
    out << "v " << v.x << " " << v.y << " " << v.z << std::endl;
  }

  // 写入UV数据
  if (!_uvs.empty()) {
    out << std::endl << "# List of texture coordinates, in (u, v) coordinates." << std::endl;
    for (auto &vt : _uvs) {
      out << "vt " << vt.x << " " << vt.y << std::endl;
    }
  }

  // 写入法线数据
  if (!_normals.empty()) {
    out << std::endl << "# List of vertex normals in (x, y, z) form." << std::endl;
    for (auto &vn : _normals) {
      out << "vn " << vn.x << " " << vn.y << " " << vn.z << std::endl;
    }
  }

  // 写入面数据
  if (!_indexes.empty()) {
    out << std::endl << "# Polygonal face element." << std::endl;
    auto it_m = _materials.begin();
    auto it = _indexes.begin();
    size_t index_counter = 0u;
    while (it != _indexes.end()) {
      // 如果当前材质在此索引处结束
      if (it_m != _materials.end() && it_m->index_end == index_counter) {
        ++it_m;
      }
      // 如果当前材质从该索引开始
      if (it_m != _materials.end() && it_m->index_start == index_counter) {
        out << "\nusemtl " << it_m->name << std::endl;
      }

      // 使用3个连续的索引添加实际面
      out << "f " << *it; ++it;
      out << " " << *it; ++it;
      out << " " << *it << std::endl; ++it;

      index_counter += 3;
    }
  }

  return out.str();
}

// 为Recast生成OBJ文件，Recast要求y和z坐标互换
std::string Mesh::GenerateOBJForRecast() const {
  if (!IsValid()) {
    return "";
  }
  std::stringstream out;
  out << std::fixed;  // 避免使用科学计数法

  // 写入顶点数据，Recast要求y和z交换
  out << "# List of geometric vertices, with (x, y, z) coordinates." << std::endl;
  for (auto &v : _vertices) {
    out << "v " << v.x << " " << v.z << " " << v.y << std::endl;
  }

  // 写入面数据
  if (!_indexes.empty()) {
    out << std::endl << "# Polygonal face element." << std::endl;
    auto it_m = _materials.begin();
    auto it = _indexes.begin();
    size_t index_counter = 0u;
    while (it != _indexes.end()) {
      // 如果当前材质在此索引处结束
      if (it_m != _materials.end() && it_m->index_end == index_counter) {
        ++it_m;
      }
      // 如果当前材质从该索引开始
      if (it_m != _materials.end() && it_m->index_start == index_counter) {
        out << "\nusemtl " << it_m->name << std::endl;
      }

      // 添加面数据，注意顺时针顺序
      out << "f " << *it; ++it;
      const auto i_2 = *it; ++it;
      const auto i_3 = *it; ++it;
      out << " " << i_3 << " " << i_2 << std::endl;
      index_counter += 3;
    }
  }

  return out.str();
}

// 生成PLY格式的字符串（目前为空实现）
std::string Mesh::GeneratePLY() const {
  if (!IsValid()) {
    return "Invalid Mesh";
  }
  std::stringstream out;
  return out.str();
}

// 获取顶点数据
const std::vector<Mesh::vertex_type> &Mesh::GetVertices() const {
  return _vertices;
}

// 获取顶点数据（可修改）
std::vector<Mesh::vertex_type> &Mesh::GetVertices() {
  return _vertices;
}

// 获取顶点数量
size_t Mesh::GetVerticesNum() const {
  return _vertices.size();
}

// 获取法线数据
const std::vector<Mesh::normal_type> &Mesh::GetNormals() const {
  return _normals;
}

// 获取索引数据
const std::vector<Mesh::index_type> &Mesh::GetIndexes() const {
  return _indexes;
}

// 获取索引数据（可修改）
std::vector<Mesh::index_type>& Mesh::GetIndexes() {
  return _indexes;
}

// 获取索引数量
size_t Mesh::GetIndexesNum() const {
  return _indexes.size();
}

// 获取UV数据
const std::vector<Mesh::uv_type> &Mesh::GetUVs() const {
  return _uvs;
}

// 获取材质数据
const std::vector<Mesh::material_type> &Mesh::GetMaterials() const {
  return _materials;
}

// 获取最后一个顶点的索引
size_t Mesh::GetLastVertexIndex() const {
  return _vertices.size();
}

// 合并两个网格，返回合并后的网格
Mesh& Mesh::ConcatMesh(const Mesh& rhs, int num_vertices_to_link) {
  if (!rhs.IsValid()){
    return *this += rhs;
  }
  const size_t v_num = GetVerticesNum();
  const size_t i_num = GetIndexesNum();

  // 合并顶点、法线、索引、UV和材质
  _vertices.insert(
    _vertices.end(),
    rhs.GetVertices().begin(),
    rhs.GetVertices().end());
  _normals.insert(
    _normals.end(),
    rhs.GetNormals().begin(),
    rhs.GetNormals().end());

  const size_t vertex_to_start_concating = v_num - num_vertices_to_link;
  for (size_t i = 1; i < num_vertices_to_link; ++i) {
    _indexes.push_back(vertex_to_start_concating + i);
    _indexes.push_back(vertex_to_start_concating + i + 1);
    _indexes.push_back(v_num + i);
    _indexes.push_back(vertex_to_start_concating + i + 1);
    _indexes.push_back(v_num + i + 1);
    _indexes.push_back(v_num + i);
  }

  // 添加第二个网格的索引
  std::transform(
    rhs.GetIndexes().begin(),
    rhs.GetIndexes().end(),
    std::back_inserter(_indexes),
    [=](size_t index) {return index + v_num; });

  // 合并UV和材质数据
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

// 重载`+=`操作符，合并两个网格
Mesh &Mesh::operator+=(const Mesh &rhs) {
  const size_t v_num = GetVerticesNum();
  const size_t i_num = GetIndexesNum();

  // 合并顶点、法线、索引、UV和材质
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

// 重载`+`操作符，返回两个网格合并后的新网格
Mesh operator+(const Mesh &lhs, const Mesh &rhs) {
  Mesh m = lhs;
  return m += rhs;
}

} // namespace geom
} // namespace carla
