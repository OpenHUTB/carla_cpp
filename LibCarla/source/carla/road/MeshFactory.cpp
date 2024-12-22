// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/road/MeshFactory.h>

#include <vector>

#include <carla/geom/Vector3D.h>
#include <carla/geom/Rtree.h>
#include <carla/road/element/LaneMarking.h>
#include <carla/road/element/RoadInfoMarkRecord.h>
#include <carla/road/Map.h>
#include <carla/road/Deformation.h>

namespace carla {
namespace geom {

  // MeshFactory构造函数，接受一个参数用于初始化路面相关参数
  MeshFactory::MeshFactory(rpc::OpendriveGenerationParameters params) {
    road_param.resolution = static_cast<float>(params.vertex_distance); // 设置顶点距离
    road_param.max_road_len = static_cast<float>(params.max_road_length); // 设置最大道路长度
    road_param.extra_lane_width = static_cast<float>(params.additional_width); // 设置额外车道宽度
    road_param.wall_height = static_cast<float>(params.wall_height); // 设置墙体高度
    road_param.vertex_width_resolution = static_cast<float>(params.vertex_width_resolution); // 设置顶点宽度分辨率
  }

  // 为避免浮点精度错误，使用此epsilon值将路径点从车道边缘移开
  static constexpr double EPSILON = 10.0 * std::numeric_limits<double>::epsilon();
  static constexpr double MESH_EPSILON = 50.0 * std::numeric_limits<double>::epsilon();

  // 根据路面生成网格
  std::unique_ptr<Mesh> MeshFactory::Generate(const road::Road &road) const {
    Mesh out_mesh; // 创建输出网格
    for (auto &&lane_section : road.GetLaneSections()) { // 遍历所有车道段
      out_mesh += *Generate(lane_section); // 生成每个车道段的网格并添加到输出网格中
    }
    return std::make_unique<Mesh>(out_mesh); // 返回生成的网格
  }

  // 根据车道段生成网格
  std::unique_ptr<Mesh> MeshFactory::Generate(const road::LaneSection &lane_section) const {
    Mesh out_mesh; // 创建输出网格
    for (auto &&lane_pair : lane_section.GetLanes()) { // 遍历车道段中的所有车道
      out_mesh += *Generate(lane_pair.second); // 生成每条车道的网格并添加到输出网格中
    }
    return std::make_unique<Mesh>(out_mesh); // 返回生成的网格
  }

  // 根据车道生成网格，默认起始点和结束点
  std::unique_ptr<Mesh> MeshFactory::Generate(const road::Lane &lane) const {
    const double s_start = lane.GetDistance() + EPSILON; // 计算起始点
    const double s_end = lane.GetDistance() + lane.GetLength() - EPSILON; // 计算结束点
    return Generate(lane, s_start, s_end); // 调用重载函数生成网格
  }

  // 根据车道生成细分网格，默认起始点和结束点
  std::unique_ptr<Mesh> MeshFactory::GenerateTesselated(const road::Lane& lane) const {
    const double s_start = lane.GetDistance() + EPSILON; // 计算起始点
    const double s_end = lane.GetDistance() + lane.GetLength() - EPSILON; // 计算结束点
    return GenerateTesselated(lane, s_start, s_end); // 调用重载函数生成细分网格
  }

  // 根据车道和给定的起始、结束位置生成网格
  std::unique_ptr<Mesh> MeshFactory::Generate(
      const road::Lane& lane, const double s_start, const double s_end) const {
      RELEASE_ASSERT(road_param.resolution > 0.0); // 确保分辨率大于0
      DEBUG_ASSERT(s_start >= 0.0); // 确保起始点合法
      DEBUG_ASSERT(s_end <= lane.GetDistance() + lane.GetLength()); // 确保结束点合法
      DEBUG_ASSERT(s_end >= EPSILON); // 确保结束点大于Epsilon
      DEBUG_ASSERT(s_start < s_end); // 确保起始点小于结束点

      // ID为0的车道在OpenDRIVE中没有物理表示
      Mesh out_mesh;
      if (lane.GetId() == 0) {
          return std::make_unique<Mesh>(out_mesh); // 返回空网格
      }

      double s_current = s_start; // 当前s值初始化为起始点

      std::vector<geom::Vector3D> vertices; // 存储顶点的向量
      if (lane.IsStraight()) { // 如果车道是直的
        // 网格优化：如果车道是直的，只需在开始和结束处添加顶点
          const auto edges = lane.GetCornerPositions(s_current, road_param.extra_lane_width); // 获取当前车道边缘位置
          vertices.push_back(edges.first); // 添加左边缘顶点
          vertices.push_back(edges.second); // 添加右边缘顶点
      }
      else {
          // 遍历车道的's'并根据宽度存储顶点
          do {
              // 获取当前路径点的车道边缘位置
              const auto edges = lane.GetCornerPositions(s_current, road_param.extra_lane_width);
              vertices.push_back(edges.first); // 添加左边缘顶点
              vertices.push_back(edges.second); // 添加右边缘顶点

              // 更新当前路径点的"s"
              s_current += road_param.resolution; // 增加当前s值
          } while (s_current < s_end); // 继续直到达到结束点
      }

      // 确保网格是连续的，并且车道之间没有间隙，
      // 在车道的末尾添加几何形状
      if (s_end - (s_current - road_param.resolution) > EPSILON) {
          const auto edges = lane.GetCornerPositions(s_end - MESH_EPSILON, road_param.extra_lane_width); // 获取结束点的车道边缘位置
          vertices.push_back(edges.first); // 添加左边缘顶点
          vertices.push_back(edges.second); // 添加右边缘顶点
      }

      // 添加材质，创建三角形带并结束材质
      out_mesh.AddMaterial(
          lane.GetType() == road::Lane::LaneType::Sidewalk ? "sidewalk" : "road"); // 根据车道类型选择材质
      out_mesh.AddTriangleStrip(vertices); // 添加三角形带
      out_mesh.EndMaterial(); // 结束材质
      return std::make_unique<Mesh>(out_mesh); // 返回生成的网格
  }

  std::unique_ptr<Mesh> MeshFactory::GenerateTesselated(
    const road::Lane& lane, const double s_start, const double s_end) const { // 生成细分网格
    RELEASE_ASSERT(road_param.resolution > 0.0); // 确保分辨率大于零
    DEBUG_ASSERT(s_start >= 0.0); // 确保起始s值非负
    DEBUG_ASSERT(s_end <= lane.GetDistance() + lane.GetLength()); // 确保结束s值在有效范围内
    DEBUG_ASSERT(s_end >= EPSILON); // 确保结束s值大于最小值
    DEBUG_ASSERT(s_start < s_end); // 确保起始s值小于结束s值
    // lane_id为0的车道在OpenDRIVE中没有物理表示
    Mesh out_mesh; // 创建网格对象
    if (lane.GetId() == 0) { // 检查车道ID
      return std::make_unique<Mesh>(out_mesh); // 如果ID为0，返回空网格
    }
    double s_current = s_start; // 初始化当前s值

    std::vector<geom::Vector3D> vertices; // 存储顶点的向量
    // 确保宽度方向上的最小顶点数为两个
    const int vertices_in_width = road_param.vertex_width_resolution >= 2 ? road_param.vertex_width_resolution : 2; // 设置宽度方向上的顶点数
    const int segments_number = vertices_in_width - 1; // 计算段数

    std::vector<geom::Vector2D> uvs; // 存储UV坐标
    int uvx = 0; // UV的X坐标
    int uvy = 0; // UV的Y坐标
    // 遍历车道的's'并根据其宽度存储顶点
    do {
      // 获取当前车道在当前位置的边缘位置
      std::pair<geom::Vector3D, geom::Vector3D> edges = lane.GetCornerPositions(s_current, road_param.extra_lane_width); // 获取边缘位置
      const geom::Vector3D segments_size = (edges.second - edges.first) / segments_number; // 计算每个段的大小
      geom::Vector3D current_vertex = edges.first; // 当前顶点初始化为左边缘
      uvx = 0; // 重置UV的X坐标
      for (int i = 0; i < vertices_in_width; ++i) { // 遍历宽度方向的顶点
        uvs.push_back(geom::Vector2D(uvx, uvy)); // 添加UV坐标
        vertices.push_back(current_vertex); // 添加当前顶点
        current_vertex = current_vertex + segments_size; // 更新当前顶点位置
        uvx++; // 更新UV的X坐标
      }
      uvy++; // 更新UV的Y坐标
      // 更新当前waypoint的"s"
      s_current += road_param.resolution; // 增加当前s值
    } while (s_current < s_end); // 当当前s值小于结束s值时继续

    // 确保网格是连续的，没有道路之间的间隙，
    // 在车道末尾添加几何体

    if (s_end - (s_current - road_param.resolution) > EPSILON) { // 检查是否需要在末尾添加几何体
      std::pair<carla::geom::Vector3D, carla::geom::Vector3D> edges =
        lane.GetCornerPositions(s_end - MESH_EPSILON, road_param.extra_lane_width); // 获取结束位置的边缘
      const geom::Vector3D segments_size = (edges.second - edges.first) / segments_number; // 计算段的大小
      geom::Vector3D current_vertex = edges.first; // 当前顶点初始化为左边缘
      uvx = 0; // 重置UV的X坐标
      for (int i = 0; i < vertices_in_width; ++i) // 遍历宽度方向的顶点
      {
        uvs.push_back(geom::Vector2D(uvx, uvy)); // 添加UV坐标
        vertices.push_back(current_vertex); // 添加当前顶点
        current_vertex = current_vertex + segments_size; // 更新当前顶点位置
        uvx++; // 更新UV的X坐标
      }
    }
    out_mesh.AddVertices(vertices); // 添加顶点到网格
    out_mesh.AddUVs(uvs); // 添加UV坐标到网格

    // 添加材质，创建索引并结束材质
    out_mesh.AddMaterial(
      lane.GetType() == road::Lane::LaneType::Sidewalk ? "sidewalk" : "road"); // 根据车道类型选择材质

    const size_t number_of_rows = (vertices.size() / vertices_in_width); // 计算行数

    for (size_t i = 0; i < (number_of_rows - 1); ++i) { // 遍历行
      for (size_t j = 0; j < vertices_in_width - 1; ++j) { // 遍历列
        out_mesh.AddIndex(   j       + (   i       * vertices_in_width ) + 1); // 添加索引
        out_mesh.AddIndex( ( j + 1 ) + (   i       * vertices_in_width ) + 1); // 添加索引
        out_mesh.AddIndex(   j       + ( ( i + 1 ) * vertices_in_width ) + 1); // 添加索引

        out_mesh.AddIndex( ( j + 1 ) + (   i       * vertices_in_width ) + 1); // 添加索引
        out_mesh.AddIndex( ( j + 1 ) + ( ( i + 1 ) * vertices_in_width ) + 1); // 添加索引
        out_mesh.AddIndex(   j       + ( ( i + 1 ) * vertices_in_width ) + 1); // 添加索引
      }
    }
    out_mesh.EndMaterial(); // 结束材质
    return std::make_unique<Mesh>(out_mesh); // 返回生成的网格
}

void MeshFactory::GenerateLaneSectionOrdered(
    const road::LaneSection &lane_section,
    std::map<carla::road::Lane::LaneType , std::vector<std::unique_ptr<Mesh>>>& result) const {

    const int vertices_in_width = road_param.vertex_width_resolution >= 2 ? road_param.vertex_width_resolution : 2; // 确定每个车道宽度的顶点分辨率，至少为2
    std::vector<size_t> redirections; // 用于存储已处理的车道ID
    for (auto &&lane_pair : lane_section.GetLanes()) { // 遍历车道段中的所有车道
      auto it = std::find(redirections.begin(), redirections.end(), lane_pair.first); // 查找当前车道ID是否已存在
      if ( it == redirections.end() ) { // 如果车道ID不存在，则添加
        redirections.push_back(lane_pair.first); // 将新的车道ID添加到列表中
        it = std::find(redirections.begin(), redirections.end(), lane_pair.first); // 重新查找当前车道ID的位置
      }
      size_t PosToAdd = it - redirections.begin(); // 计算当前车道ID在列表中的位置

      Mesh out_mesh; // 创建一个新的网格对象
      switch(lane_pair.second.GetType()) // 根据车道类型生成不同的网格
      {
        case road::Lane::LaneType::Driving: // 驾驶车道
        case road::Lane::LaneType::Parking: // 停车车道
        case road::Lane::LaneType::Bidirectional: // 双向车道
        {
          out_mesh += *GenerateTesselated(lane_pair.second); // 生成细分网格并添加到输出网格
          break; // 结束当前case
        }
        case road::Lane::LaneType::Shoulder: // 应急车道
        case road::Lane::LaneType::Sidewalk: // 人行道
        case road::Lane::LaneType::Biking: // 自行车道
        {
          out_mesh += *GenerateSidewalk(lane_pair.second); // 生成人行道网格并添加到输出网格
          break; // 结束当前case
        }
        default: // 其他类型的车道
        {
          out_mesh += *GenerateTesselated(lane_pair.second); // 默认生成细分网格并添加到输出网格
          break; // 结束当前case
        }
      }

      if( result[lane_pair.second.GetType()].size() <= PosToAdd ){ // 检查当前车道类型的网格数量是否小于等于位置索引
        result[lane_pair.second.GetType()].push_back(std::make_unique<Mesh>(out_mesh)); // 如果满足条件，则添加新的网格到结果中
      } else { // 如果已存在网格
        uint32_t verticesinwidth  = SelectVerticesInWidth(vertices_in_width, lane_pair.second.GetType()); // 选择适当的宽度顶点数量
        (result[lane_pair.second.GetType()][PosToAdd])->ConcatMesh(out_mesh, verticesinwidth); // 将新生成的网格与现有网格合并
      }
    }
 }

std::unique_ptr<Mesh> MeshFactory::GenerateSidewalk(const road::LaneSection &lane_section) const{ // 定义生成侧步的方法，接受车道段作为参数
    Mesh out_mesh; // 创建一个输出网格
    for (auto &&lane_pair : lane_section.GetLanes()) { // 遍历车道段中的所有车道
        const double s_start = lane_pair.second.GetDistance() + EPSILON; // 计算开始的s参数
        const double s_end = lane_pair.second.GetDistance() + lane_pair.second.GetLength() - EPSILON; // 计算结束的s参数
        out_mesh += *GenerateSidewalk(lane_pair.second, s_start, s_end); // 生成车道的侧步网格并添加到输出网格
    }
    return std::make_unique<Mesh>(out_mesh); // 返回创建的网格
}
std::unique_ptr<Mesh> MeshFactory::GenerateSidewalk(const road::Lane &lane) const{ // 定义生成侧步的方法，接受车道作为参数
    const double s_start = lane.GetDistance() + EPSILON; // 计算开始的s参数
    const double s_end = lane.GetDistance() + lane.GetLength() - EPSILON; // 计算结束的s参数
    return GenerateSidewalk(lane, s_start, s_end); // 调用重载方法生成侧步网格
}

std::unique_ptr<Mesh> MeshFactory::GenerateSidewalk( // 重载方法，接受车道及其起始和结束s参数
    const road::Lane &lane, const double s_start,
    const double s_end ) const {

    RELEASE_ASSERT(road_param.resolution > 0.0); // 确保道路参数的分辨率大于零
    DEBUG_ASSERT(s_start >= 0.0); // 确保开始的s参数非负
    DEBUG_ASSERT(s_end <= lane.GetDistance() + lane.GetLength()); // 确保结束的s参数不超过车道长度
    DEBUG_ASSERT(s_end >= EPSILON); // 确保结束的s参数大于或等于一个小的正值
    DEBUG_ASSERT(s_start < s_end); // 确保开始的s参数小于结束的s参数
    // lane_id为0的车道在OpenDRIVE中没有物理表示
    Mesh out_mesh; // 创建一个输出网格
    if (lane.GetId() == 0) { // 如果车道ID为0
        return std::make_unique<Mesh>(out_mesh); // 返回空网格
    }
    double s_current = s_start; // 初始化当前s为起始s值

    std::vector<geom::Vector3D> vertices; // 存储顶点的向量
    // 确保宽度上至少有两个顶点
    const int vertices_in_width = 6; // 每行中的顶点数量
    const int segments_number = vertices_in_width - 1; // 计算段落数量
    std::vector<geom::Vector2D> uvs; // 存储纹理坐标的向量
    int uvy = 0; // 纹理坐标y轴索引

    // 遍历车道的's'值，根据其宽度存储顶点
    do {
      // 获取当前路点处车道边缘的位置
      std::pair<geom::Vector3D, geom::Vector3D> edges =
        lane.GetCornerPositions(s_current, road_param.extra_lane_width);

      geom::Vector3D low_vertex_first = edges.first - geom::Vector3D(0,0,1); // 计算第一个低顶点
      geom::Vector3D low_vertex_second = edges.second - geom::Vector3D(0,0,1); // 计算第二个低顶点
      vertices.push_back(low_vertex_first); // 添加第一个低顶点到顶点列表
      uvs.push_back(geom::Vector2D(0, uvy)); // 添加相应的纹理坐标

      vertices.push_back(edges.first); // 添加第一个边缘顶点
      uvs.push_back(geom::Vector2D(1, uvy)); // 添加相应的纹理坐标

      vertices.push_back(edges.first); // 重复添加第一个边缘顶点
      uvs.push_back(geom::Vector2D(1, uvy)); // 添加相应的纹理坐标

      vertices.push_back(edges.second); // 添加第二个边缘顶点
      uvs.push_back(geom::Vector2D(2, uvy)); // 添加相应的纹理坐标

      vertices.push_back(edges.second); // 重复添加第二个边缘顶点
      uvs.push_back(geom::Vector2D(2, uvy)); // 添加相应的纹理坐标

      vertices.push_back(low_vertex_second); // 添加第二个低顶点到顶点列表
      uvs.push_back(geom::Vector2D(3, uvy)); // 添加相应的纹理坐标

      // 更新当前路点的"s"值
      s_current += road_param.resolution; // 增加当前的"s"值
      uvy++; // 增加y轴纹理坐标索引
    } while (s_current < s_end); // 当当前s小于结束s值时继续循环

    // 确保网格是连续的，并且道路之间没有缝隙，
    // 在车道的最末尾添加几何体

    if (s_end - (s_current - road_param.resolution) > EPSILON) { // 检查是否需要额外的顶点
      std::pair<carla::geom::Vector3D, carla::geom::Vector3D> edges =
        lane.GetCornerPositions(s_end - MESH_EPSILON, road_param.extra_lane_width);

      geom::Vector3D low_vertex_first = edges.first - geom::Vector3D(0,0,1); // 计算第一个低顶点
      geom::Vector3D low_vertex_second = edges.second - geom::Vector3D(0,0,1); // 计算第二个低顶点

      vertices.push_back(low_vertex_first); // 添加第一个低顶点到顶点列表
      uvs.push_back(geom::Vector2D(0, uvy)); // 添加相应的纹理坐标

      vertices.push_back(edges.first); // 添加第一个边缘顶点
      uvs.push_back(geom::Vector2D(1, uvy)); // 添加相应的纹理坐标

      vertices.push_back(edges.first); // 重复添加第一个边缘顶点
      uvs.push_back(geom::Vector2D(1, uvy)); // 添加相应的纹理坐标

      vertices.push_back(edges.second); // 添加第二个边缘顶点
      uvs.push_back(geom::Vector2D(2, uvy)); // 添加相应的纹理坐标

      vertices.push_back(edges.second); // 重复添加第二个边缘顶点
      uvs.push_back(geom::Vector2D(2, uvy)); // 添加相应的纹理坐标

      vertices.push_back(low_vertex_second); // 添加第二个低顶点到顶点列表
      uvs.push_back(geom::Vector2D(3, uvy)); // 添加相应的纹理坐标

    }

    out_mesh.AddVertices(vertices); // 将所有顶点添加到输出网格
    out_mesh.AddUVs(uvs); // 将所有纹理坐标添加到输出网格
    // 添加渐变材料，创建条带并结束材料
    out_mesh.AddMaterial(
      lane.GetType() == road::Lane::LaneType::Sidewalk ? "sidewalk" : "road"); // 根据车道类型添加材料

    const int number_of_rows = (vertices.size() / vertices_in_width); // 计算行数

    for (size_t i = 0; i < (number_of_rows - 1); ++i) { // 遍历每一行
      for (size_t j = 0; j < vertices_in_width - 1; ++j) { // 遍历每一行的每个顶点

        if(j == 1 || j == 3){ // 跳过特定的顶点索引
          continue; // 继续下一个循环
        }

        out_mesh.AddIndex(   j       + (   i       * vertices_in_width ) + 1); // 添加三角形索引
        out_mesh.AddIndex( ( j + 1 ) + (   i       * vertices_in_width ) + 1); // 添加三角形索引
        out_mesh.AddIndex(   j       + ( ( i + 1 ) * vertices_in_width ) + 1); // 添加三角形索引

        out_mesh.AddIndex( ( j + 1 ) + (   i       * vertices_in_width ) + 1); // 添加三角形索引
        out_mesh.AddIndex( ( j + 1 ) + ( ( i + 1 ) * vertices_in_width ) + 1); // 添加三角形索引
        out_mesh.AddIndex(   j       + ( ( i + 1 ) * vertices_in_width ) + 1); // 添加三角形索引

      }
    }
    out_mesh.EndMaterial(); // 结束材料定义
    return std::make_unique<Mesh>(out_mesh); // 返回创建的网格对象
}
std::unique_ptr<Mesh> MeshFactory::GenerateWalls(const road::LaneSection &lane_section) const {
    Mesh out_mesh; // 创建一个输出网格

    // 确定最小和最大车道的ID
    const auto min_lane = lane_section.GetLanes().begin()->first == 0 ?
        1 : lane_section.GetLanes().begin()->first; // 如果最小车道ID为0，则设置为1
    const auto max_lane = lane_section.GetLanes().rbegin()->first == 0 ?
        -1 : lane_section.GetLanes().rbegin()->first; // 如果最大车道ID为0，则设置为-1

    // 遍历所有车道
    for (auto &&lane_pair : lane_section.GetLanes()) {
        const auto &lane = lane_pair.second; // 获取当前车道
        const double s_start = lane.GetDistance() + EPSILON; // 车道起始位置
        const double s_end = lane.GetDistance() + lane.GetLength() - EPSILON; // 车道结束位置
        if (lane.GetId() == max_lane) { // 如果是最大车道
            out_mesh += *GenerateLeftWall(lane, s_start, s_end); // 生成左墙并添加到输出网格
        }
        if (lane.GetId() == min_lane) { // 如果是最小车道
            out_mesh += *GenerateRightWall(lane, s_start, s_end); // 生成右墙并添加到输出网格
        }
    }
    return std::make_unique<Mesh>(out_mesh); // 返回包含生成墙体的网格
}

std::unique_ptr<Mesh> MeshFactory::GenerateRightWall(
      const road::Lane &lane, const double s_start, const double s_end) const {
    RELEASE_ASSERT(road_param.resolution > 0.0); // 确保分辨率大于0
    DEBUG_ASSERT(s_start >= 0.0); // 确保起始位置合法
    DEBUG_ASSERT(s_end <= lane.GetDistance() + lane.GetLength()); // 确保结束位置合法
    DEBUG_ASSERT(s_end >= EPSILON); // 确保结束位置大于一个小值
    DEBUG_ASSERT(s_start < s_end); // 确保起始位置小于结束位置
    // ID为0的车道在OpenDRIVE中没有物理表示
    Mesh out_mesh; // 创建输出网格
    if (lane.GetId() == 0) { // 如果车道ID为0
        return std::make_unique<Mesh>(out_mesh); // 返回空网格
    }
    double s_current = s_start; // 当前s值初始化为起始位置
    const geom::Vector3D height_vector = geom::Vector3D(0.f, 0.f, road_param.wall_height); // 墙体高度向量

    std::vector<geom::Vector3D> r_vertices; // 存储右墙顶点的容器
    if (lane.IsStraight()) { // 如果车道是直的
        // 网格优化：如果车道是直的，只需在起始和结束位置添加顶点
        const auto edges = lane.GetCornerPositions(s_current, road_param.extra_lane_width); // 获取边缘位置
        r_vertices.push_back(edges.first + height_vector); // 添加上边缘顶点
        r_vertices.push_back(edges.first); // 添加下边缘顶点
    } else {
        // 遍历车道的's'并根据宽度存储顶点
        do {
            // 获取当前s位置的车道边缘位置
            const auto edges = lane.GetCornerPositions(s_current, road_param.extra_lane_width);
            r_vertices.push_back(edges.first + height_vector); // 添加上边缘顶点
            r_vertices.push_back(edges.first); // 添加下边缘顶点

            // 更新当前的s值
            s_current += road_param.resolution; // 将s值增加分辨率
        } while(s_current < s_end); // 当当前s值小于结束s值时继续
    }

    // 确保网格是连续的，且车道之间没有缝隙，
    // 在车道末尾添加几何体
    if (s_end - (s_current - road_param.resolution) > EPSILON) {
        const auto edges = lane.GetCornerPositions(s_end - MESH_EPSILON, road_param.extra_lane_width); // 获取结束位置的边缘
        r_vertices.push_back(edges.first + height_vector); // 添加上边缘顶点
        r_vertices.push_back(edges.first); // 添加下边缘顶点
    }

    // 添加合适的材质，创建三角形带，并结束材质定义
    out_mesh.AddMaterial(
        lane.GetType() == road::Lane::LaneType::Sidewalk ? "sidewalk" : "road"); // 根据车道类型添加材质
    out_mesh.AddTriangleStrip(r_vertices); // 添加三角带
    out_mesh.EndMaterial(); // 结束材质定义
    return std::make_unique<Mesh>(out_mesh); // 返回包含右墙的网格
}

std::unique_ptr<Mesh> MeshFactory::GenerateLeftWall(
      const road::Lane &lane, const double s_start, const double s_end) const {
    RELEASE_ASSERT(road_param.resolution > 0.0); // 确保分辨率大于0
    DEBUG_ASSERT(s_start >= 0.0); // 确保起始位置合法
    DEBUG_ASSERT(s_end <= lane.GetDistance() + lane.GetLength()); // 确保结束位置合法
    DEBUG_ASSERT(s_end >= EPSILON); // 确保结束位置大于一个小值
    DEBUG_ASSERT(s_start < s_end); // 确保起始位置小于结束位置
    // ID为0的车道在OpenDRIVE中没有物理表示
    Mesh out_mesh; // 创建输出网格
    if (lane.GetId() == 0) { // 如果车道ID为0
        return std::make_unique<Mesh>(out_mesh); // 返回空网格
    }

    double s_current = s_start; // 初始化当前的s值为起始位置
    const geom::Vector3D height_vector = geom::Vector3D(0.f, 0.f, road_param.wall_height); // 创建高度向量，表示墙体的高度

    std::vector<geom::Vector3D> l_vertices; // 存储左墙顶点的容器
    if (lane.IsStraight()) { // 如果车道是直的
      // 网格优化：如果车道是直的，只需在起始和结束位置添加顶点
      const auto edges = lane.GetCornerPositions(s_current, road_param.extra_lane_width); // 获取边缘位置
      l_vertices.push_back(edges.second); // 添加下边缘顶点
      l_vertices.push_back(edges.second + height_vector); // 添加上边缘顶点
    } else {
      // 遍历车道的's'并根据宽度存储顶点
      do {
        // 获取当前s位置的车道边缘位置
        const auto edges = lane.GetCornerPositions(s_current, road_param.extra_lane_width);
        l_vertices.push_back(edges.second); // 添加下边缘顶点
        l_vertices.push_back(edges.second + height_vector); // 添加上边缘顶点

        // 更新当前的s值
        s_current += road_param.resolution; // 将s值增加分辨率
      } while(s_current < s_end); // 当当前s值小于结束s值时继续
    }

    // 确保网格是连续的，且车道之间没有缝隙，
    // 在车道末尾添加几何体
    if (s_end - (s_current - road_param.resolution) > EPSILON) { // 如果s_end与当前s的差距大于小值
      const auto edges = lane.GetCornerPositions(s_end - MESH_EPSILON, road_param.extra_lane_width); // 获取结束位置的边缘
      l_vertices.push_back(edges.second); // 添加下边缘顶点
      l_vertices.push_back(edges.second + height_vector); // 添加上边缘顶点
    }

    // 添加合适的材质，创建三角形带，并结束材质定义
    out_mesh.AddMaterial(
        lane.GetType() == road::Lane::LaneType::Sidewalk ? "sidewalk" : "road"); // 根据车道类型添加材质
    out_mesh.AddTriangleStrip(l_vertices); // 添加三角带
    out_mesh.EndMaterial(); // 结束材质定义
    return std::make_unique<Mesh>(out_mesh); // 返回包含左墙的网格
  }

  std::vector<std::unique_ptr<Mesh>> MeshFactory::GenerateWithMaxLen(
      const road::Road &road) const { // 根据道路生成最大长度的网格
    std::vector<std::unique_ptr<Mesh>> mesh_uptr_list; // 存储生成的网格指针的列表
    for (auto &&lane_section : road.GetLaneSections()) { // 遍历道路的每个车道段
      auto section_uptr_list = GenerateWithMaxLen(lane_section); // 生成当前车道段的网格
      mesh_uptr_list.insert(
          mesh_uptr_list.end(),
          std::make_move_iterator(section_uptr_list.begin()), // 移动当前车道段网格到总列表
          std::make_move_iterator(section_uptr_list.end())); // 移动结束迭代器
    }
    return mesh_uptr_list; // 返回所有生成的网格列表
  }


std::vector<std::unique_ptr<Mesh>> MeshFactory::GenerateWithMaxLen(
      const road::LaneSection &lane_section) const {  // 定义一个成员函数，生成具有最大长度的Mesh
    std::vector<std::unique_ptr<Mesh>> mesh_uptr_list;  // 创建一个存储唯一指针的向量，用于保存Mesh
    if (lane_section.GetLength() < road_param.max_road_len) {  // 如果车道段的长度小于最大路段长度
      mesh_uptr_list.emplace_back(Generate(lane_section));  // 直接生成Mesh并加入到列表中
    } else {  // 否则（车道段较长）
      double s_current = lane_section.GetDistance() + EPSILON;  // 当前距离设置为车道段起点加一个小值
      const double s_end = lane_section.GetDistance() + lane_section.GetLength() - EPSILON;  // 终点距离计算
      while(s_current + road_param.max_road_len < s_end) {  // 当当前距离加上最大路段长度小于终点时
        const auto s_until = s_current + road_param.max_road_len;  // 计算本次生成的终点
        Mesh lane_section_mesh;  // 创建一个Mesh对象用于存储车道段的Mesh
        for (auto &&lane_pair : lane_section.GetLanes()) {  // 遍历车道段中的所有车道
          lane_section_mesh += *Generate(lane_pair.second, s_current, s_until);  // 生成车道的Mesh并累加
        }
        mesh_uptr_list.emplace_back(std::make_unique<Mesh>(lane_section_mesh));  // 将生成的Mesh加入到列表中
        s_current = s_until;  // 更新当前距离为本次生成的终点
      }
      if (s_end - s_current > EPSILON) {  // 如果还有剩余未处理的距离
        Mesh lane_section_mesh;  // 创建新的Mesh对象
        for (auto &&lane_pair : lane_section.GetLanes()) {  // 遍历车道段中的所有车道
          lane_section_mesh += *Generate(lane_pair.second, s_current, s_end);  // 生成剩余部分的Mesh并累加
        }
        mesh_uptr_list.emplace_back(std::make_unique<Mesh>(lane_section_mesh));  // 将最后生成的Mesh加入到列表中
      }
    }
    return mesh_uptr_list;  // 返回生成的Mesh列表
  }

std::map<road::Lane::LaneType , std::vector<std::unique_ptr<Mesh>>> MeshFactory::GenerateOrderedWithMaxLen(
      const road::Road &road) const {  // 定义一个成员函数，根据车道类型生成有序的Mesh列表
    std::map<road::Lane::LaneType , std::vector<std::unique_ptr<Mesh>>> mesh_uptr_list;  // 创建一个映射，用于存储不同类型车道的Mesh列表
    for (auto &&lane_section : road.GetLaneSections()) {  // 遍历道路中的每个车道段
      std::map<road::Lane::LaneType , std::vector<std::unique_ptr<Mesh>>> section_uptr_list = GenerateOrderedWithMaxLen(lane_section);  // 递归调用生成车道段的Mesh列表
      mesh_uptr_list.insert(  // 将生成的车道段Mesh列表插入到总列表中
        std::make_move_iterator(section_uptr_list.begin()),  // 移动迭代器，避免拷贝
        std::make_move_iterator(section_uptr_list.end()));  // 移动到末尾
    }
    return mesh_uptr_list;  // 返回按类型排序的Mesh列表
  }

std::map<road::Lane::LaneType, std::vector<std::unique_ptr<Mesh>>> MeshFactory::GenerateOrderedWithMaxLen(
    const road::LaneSection& lane_section) const { // 定义函数，生成具有最大长度的有序Mesh
    const int vertices_in_width = road_param.vertex_width_resolution >= 2 ? road_param.vertex_width_resolution : 2; // 确定宽度方向上的顶点数量
    std::map<road::Lane::LaneType, std::vector<std::unique_ptr<Mesh>>> mesh_uptr_list; // 创建一个映射，用于存储不同车道类型的Mesh指针

    if (lane_section.GetLength() < road_param.max_road_len) { // 如果车道段长度小于最大道路长度
        GenerateLaneSectionOrdered(lane_section, mesh_uptr_list); // 生成有序车道段Mesh
    }
    else {
        double s_current = lane_section.GetDistance() + EPSILON; // 当前的s值，初始为车道段的起始距离加上一个非常小的值
        const double s_end = lane_section.GetDistance() + lane_section.GetLength() - EPSILON; // 结束的s值
        std::vector<size_t> redirections; // 用于存储车道索引的重定向

        while (s_current + road_param.max_road_len < s_end) { // 当当前s值加上最大道路长度小于结束的s值时
            const auto s_until = s_current + road_param.max_road_len; // 计算当前片段的结束s值

            for (auto&& lane_pair : lane_section.GetLanes()) { // 遍历车道段中的每个车道
                Mesh lane_section_mesh; // 创建一个Mesh对象用于存储当前车道段的Mesh

                switch (lane_pair.second.GetType()) // 根据车道类型生成Mesh
                {
                case road::Lane::LaneType::Driving: // 驾驶车道
                case road::Lane::LaneType::Parking: // 停车车道
                case road::Lane::LaneType::Bidirectional: // 双向车道
                {
                    lane_section_mesh += *GenerateTesselated(lane_pair.second, s_current, s_until); // 生成细分网格并累加到mesh中
                    break;
                }
                case road::Lane::LaneType::Shoulder: // 路肩
                case road::Lane::LaneType::Sidewalk: // 人行道
                case road::Lane::LaneType::Biking: // 自行车道
                {
                    lane_section_mesh += *GenerateSidewalk(lane_pair.second, s_current, s_until); // 生成人行道Mesh并累加
                    break;
                }
                default: // 默认情况
                {
                    lane_section_mesh += *GenerateTesselated(lane_pair.second, s_current, s_until); // 生成细分网格并累加
                    break;
                }
                }
                auto it = std::find(redirections.begin(), redirections.end(), lane_pair.first); // 查找当前车道的索引
                if (it == redirections.end()) { // 如果未找到
                    redirections.push_back(lane_pair.first); // 将车道索引添加到重定向列表中
                    it = std::find(redirections.begin(), redirections.end(), lane_pair.first); // 再次查找以获取迭代器
                }

                size_t PosToAdd = it - redirections.begin(); // 计算要添加的位置
                if (mesh_uptr_list[lane_pair.second.GetType()].size() <= PosToAdd) { // 如果该类型的Mesh数量不足
                    mesh_uptr_list[lane_pair.second.GetType()].push_back(std::make_unique<Mesh>(lane_section_mesh)); // 新建一个Mesh并添加
                }
                else {
                    uint32_t verticesinwidth = SelectVerticesInWidth(vertices_in_width, lane_pair.second.GetType()); // 根据车道类型选择宽度方向的顶点数
                    (mesh_uptr_list[lane_pair.second.GetType()][PosToAdd])->ConcatMesh(lane_section_mesh, verticesinwidth); // 拼接当前Mesh到已有Mesh中
                }
            }
            s_current = s_until; // 更新当前s值
        }
        if (s_end - s_current > EPSILON) { // 如果还有剩余的段落
            for (auto&& lane_pair : lane_section.GetLanes()) { // 遍历车道段中的每个车道
                Mesh lane_section_mesh; // 创建一个Mesh对象

                switch (lane_pair.second.GetType()) // 根据车道类型生成Mesh
                {
                case road::Lane::LaneType::Driving: // 驾驶车道
                case road::Lane::LaneType::Parking: // 停车车道
                case road::Lane::LaneType::Bidirectional: // 双向车道
                {
                    lane_section_mesh += *GenerateTesselated(lane_pair.second, s_current, s_end); // 生成细分网格并累加
                    break;
                }
                case road::Lane::LaneType::Shoulder: // 路肩
                case road::Lane::LaneType::Sidewalk: // 人行道
                case road::Lane::LaneType::Biking: // 自行车道
                {
                    lane_section_mesh += *GenerateSidewalk(lane_pair.second, s_current, s_end); // 生成人行道Mesh并累加
                    break;
                }
                default: // 默认情况
                {
                    lane_section_mesh += *GenerateTesselated(lane_pair.second, s_current, s_end); // 生成细分网格并累加
                    break;
                }
                }

                auto it = std::find(redirections.begin(), redirections.end(), lane_pair.first); // 查找当前车道的索引
                if (it == redirections.end()) { // 如果未找到该车道索引
                    redirections.push_back(lane_pair.first); // 将车道索引添加到重定向列表中
                    it = std::find(redirections.begin(), redirections.end(), lane_pair.first); // 再次查找以获取迭代器
                }

                size_t PosToAdd = it - redirections.begin(); // 计算要添加的位置

                if (mesh_uptr_list[lane_pair.second.GetType()].size() <= PosToAdd) { // 如果该类型的Mesh数量不足
                    mesh_uptr_list[lane_pair.second.GetType()].push_back(std::make_unique<Mesh>(lane_section_mesh)); // 新建一个Mesh并添加到列表中
                }
                else {
                    *(mesh_uptr_list[lane_pair.second.GetType()][PosToAdd]) += lane_section_mesh; // 将当前Mesh累加到已有Mesh中
                }
            }
        }
    }
    return mesh_uptr_list;
}

  std::vector<std::unique_ptr<Mesh>> MeshFactory::GenerateWallsWithMaxLen(
      const road::Road &road) const { // 根据给定的道路生成最大长度的墙体
    std::vector<std::unique_ptr<Mesh>> mesh_uptr_list; // 存储生成的Mesh指针列表
    for (auto &&lane_section : road.GetLaneSections()) { // 遍历每个车道段
      auto section_uptr_list = GenerateWallsWithMaxLen(lane_section); // 生成该车道段的墙体
      mesh_uptr_list.insert(
          mesh_uptr_list.end(),
          std::make_move_iterator(section_uptr_list.begin()), // 将生成的Mesh移动到主列表中
          std::make_move_iterator(section_uptr_list.end()));
    }
    return mesh_uptr_list; // 返回生成的Mesh列表
  }

  std::vector<std::unique_ptr<Mesh>> MeshFactory::GenerateWallsWithMaxLen(
      const road::LaneSection &lane_section) const { // 根据给定的车道段生成最大长度的墙体
    std::vector<std::unique_ptr<Mesh>> mesh_uptr_list; // 存储生成的Mesh指针列表

    const auto min_lane = lane_section.GetLanes().begin()->first == 0 ? // 获取最小车道ID，如果是0则设为1
        1 : lane_section.GetLanes().begin()->first;
    const auto max_lane = lane_section.GetLanes().rbegin()->first == 0 ? // 获取最大车道ID，如果是0则设为-1
        -1 : lane_section.GetLanes().rbegin()->first;

    if (lane_section.GetLength() < road_param.max_road_len) { // 如果车道段长度小于最大道路长度
      mesh_uptr_list.emplace_back(GenerateWalls(lane_section)); // 直接生成墙体并添加到列表
    } else { // 如果车道段长度大于等于最大道路长度
      double s_current = lane_section.GetDistance() + EPSILON; // 当前距离加上一个小值（避免浮点误差）
      const double s_end = lane_section.GetDistance() + lane_section.GetLength() - EPSILON; // 结束距离
      while(s_current + road_param.max_road_len < s_end) { // 当当前距离加上最大长度小于结束距离时
        const auto s_until = s_current + road_param.max_road_len; // 计算下一个段的结束位置
        Mesh lane_section_mesh; // 创建一个Mesh对象用于存储当前段的墙体
        for (auto &&lane_pair : lane_section.GetLanes()) { // 遍历车道段中的每个车道
          const auto &lane = lane_pair.second; // 获取当前车道
          if (lane.GetId() == max_lane) { // 如果是最大车道
            lane_section_mesh += *GenerateLeftWall(lane, s_current, s_until); // 生成左侧墙体并累加
          }
          if (lane.GetId() == min_lane) { // 如果是最小车道
            lane_section_mesh += *GenerateRightWall(lane, s_current, s_until); // 生成右侧墙体并累加
          }
        }
        mesh_uptr_list.emplace_back(std::make_unique<Mesh>(lane_section_mesh)); // 将生成的Mesh添加到列表
        s_current = s_until; // 更新当前距离
      }
      if (s_end - s_current > EPSILON) { // 如果结束距离与当前距离之间的差值大于一个小值
        Mesh lane_section_mesh; // 创建一个Mesh对象用于存储最后一段的墙体
        for (auto &&lane_pair : lane_section.GetLanes()) { // 遍历车道段中的每个车道
          const auto &lane = lane_pair.second; // 获取当前车道
          if (lane.GetId() == max_lane) { // 如果是最大车道
            lane_section_mesh += *GenerateLeftWall(lane, s_current, s_end); // 生成左侧墙体并累加
          }
          if (lane.GetId() == min_lane) { // 如果是最小车道
            lane_section_mesh += *GenerateRightWall(lane, s_current, s_end); // 生成右侧墙体并累加
          }
        }
        mesh_uptr_list.emplace_back(std::make_unique<Mesh>(lane_section_mesh)); // 将生成的Mesh添加到列表
      }
    }
    return mesh_uptr_list; // 返回生成的Mesh列表
  }

std::vector<std::unique_ptr<Mesh>> MeshFactory::GenerateAllWithMaxLen(
      const road::Road &road) const { // 定义一个函数，生成具有最大长度的所有网格
    std::vector<std::unique_ptr<Mesh>> mesh_uptr_list; // 创建一个唯一指针的网格列表

    // 获取道路网格
    auto roads = GenerateWithMaxLen(road); // 生成最大长度的道路网格
    mesh_uptr_list.insert(
        mesh_uptr_list.end(),
        std::make_move_iterator(roads.begin()), // 移动道路网格到列表中
        std::make_move_iterator(roads.end())); // 移动道路网格到列表中

    //如果不是交叉口则获取墙体网格
    if (!road.IsJunction()) { // 如果不是交叉口
      auto walls = GenerateWallsWithMaxLen(road); // 生成最大长度的墙体网格

      if (roads.size() == walls.size()) { // 如果道路和墙体的数量相同
        for (size_t i = 0; i < walls.size(); ++i) { // 遍历墙体网格
          *mesh_uptr_list[i] += *walls[i]; // 将墙体网格合并到对应的道路网格中
        }
      } else { // 如果数量不同
        mesh_uptr_list.insert(
            mesh_uptr_list.end(),
            std::make_move_iterator(walls.begin()), // 移动墙体网格到列表中
            std::make_move_iterator(walls.end())); // 移动墙体网格到列表中
      }
    }

    return mesh_uptr_list; // 返回生成的网格列表
  }

  void MeshFactory::GenerateAllOrderedWithMaxLen(
      const road::Road &road,
      std::map<road::Lane::LaneType , std::vector<std::unique_ptr<Mesh>>>& roads
      ) const { // 定义一个函数，按顺序生成具有最大长度的所有网格

    // Get road meshes 获取道路网格
    std::map<road::Lane::LaneType , std::vector<std::unique_ptr<Mesh>>> result = GenerateOrderedWithMaxLen(road); // 生成有序的最大长度道路网格
    for (auto &pair_map : result) { // 遍历结果中的每一对
      std::vector<std::unique_ptr<Mesh>>& origin = roads[pair_map.first]; // 获取原始网格列表
      std::vector<std::unique_ptr<Mesh>>& source = pair_map.second; // 获取源网格列表
      std::move(source.begin(), source.end(), std::back_inserter(origin)); // 将源网格移动到原始网格列表中
    }
  }

  void MeshFactory::GenerateLaneMarkForRoad(
    const road::Road& road,
    std::vector<std::unique_ptr<Mesh>>& inout,
    std::vector<std::string>& outinfo ) const // 定义一个函数，为道路生成车道标记
  {
    for (auto&& lane_section : road.GetLaneSections()) { // 遍历道路的车道段
      for (auto&& lane : lane_section.GetLanes()) { // 遍历车道
        if (lane.first != 0) { // 如果车道索引不为0
          switch(lane.second.GetType()) // 根据车道类型选择操作
          {
            case road::Lane::LaneType::Driving: // 驾驶车道
            case road::Lane::LaneType::Parking: // 停车车道
            case road::Lane::LaneType::Bidirectional: // 双向车道
            {
              GenerateLaneMarksForNotCenterLine(lane_section, lane.second, inout, outinfo); // 为非中心线生成车道标记
              outinfo.push_back("white"); // 添加白色到输出信息
              break; // 退出switch
            }
          }
        } else { // 如果车道索引为0
          if(lane.second.GetType() == road::Lane::LaneType::None ){ // 如果车道类型为无
            GenerateLaneMarksForCenterLine(road, lane_section, lane.second, inout, outinfo); // 为中心线生成车道标记
            outinfo.push_back("yellow"); // 添加黄色到输出信息
          }
        }
      }
    }
  }

void MeshFactory::GenerateLaneMarksForNotCenterLine(
    const road::LaneSection& lane_section, // 道路车道段
    const road::Lane& lane, // 道路车道
    std::vector<std::unique_ptr<Mesh>>& inout, // 输入输出网格
    std::vector<std::string>& outinfo ) const { // 输出信息
    Mesh out_mesh; // 输出网格
    const double s_start = lane_section.GetDistance(); // 起始距离
    const double s_end = lane_section.GetDistance() + lane_section.GetLength(); // 结束距离
    double s_current = s_start; // 当前距离
    std::vector<geom::Vector3D> vertices; // 顶点向量
    std::vector<size_t> indices; // 索引向量

    do {
      // 获取车道信息
      const carla::road::element::RoadInfoMarkRecord* road_info_mark = lane.GetInfo<carla::road::element::RoadInfoMarkRecord>(s_current); // 获取当前距离的标记信息
      if (road_info_mark != nullptr) { // 如果标记信息存在
        carla::road::element::LaneMarking lane_mark_info(*road_info_mark); // 创建车道标记信息对象

        switch (lane_mark_info.type) { // 根据标记类型进行处理
          case carla::road::element::LaneMarking::Type::Solid: { // 实线的情况
            size_t currentIndex = out_mesh.GetVertices().size() + 1; // 当前索引

            std::pair<geom::Vector3D, geom::Vector3D> edges = 
              ComputeEdgesForLanemark(lane_section, lane, s_current, lane_mark_info.width); // 计算边缘

            out_mesh.AddVertex(edges.first); // 添加第一个顶点
            out_mesh.AddVertex(edges.second); // 添加第二个顶点

            out_mesh.AddIndex(currentIndex); // 添加索引
            out_mesh.AddIndex(currentIndex + 1); // 添加索引
            out_mesh.AddIndex(currentIndex + 2); // 添加索引

            out_mesh.AddIndex(currentIndex + 1); // 添加索引
            out_mesh.AddIndex(currentIndex + 3); // 添加索引
            out_mesh.AddIndex(currentIndex + 2); // 添加索引

            s_current += road_param.resolution; // 更新当前距离
            break;
          }
          case carla::road::element::LaneMarking::Type::Broken: { // 虚线的情况
            size_t currentIndex = out_mesh.GetVertices().size() + 1; // 当前索引

            std::pair<geom::Vector3D, geom::Vector3D> edges = 
              ComputeEdgesForLanemark(lane_section, lane, s_current, lane_mark_info.width); // 计算边缘

            out_mesh.AddVertex(edges.first); // 添加第一个顶点
            out_mesh.AddVertex(edges.second); // 添加第二个顶点

            s_current += road_param.resolution * 3; // 更新当前距离
            if (s_current > s_end) // 如果超出结束距离
            {
              s_current = s_end; // 将当前距离设置为结束距离
            }

            edges = ComputeEdgesForLanemark(lane_section, lane, s_current, lane_mark_info.width); // 计算新的边缘

            out_mesh.AddVertex(edges.first); // 添加第一个顶点
            out_mesh.AddVertex(edges.second); // 添加第二个顶点
            
            out_mesh.AddIndex(currentIndex); // 添加索引
            out_mesh.AddIndex(currentIndex + 1); // 添加索引
            out_mesh.AddIndex(currentIndex + 2); // 添加索引

            out_mesh.AddIndex(currentIndex + 1); // 添加索引
            out_mesh.AddIndex(currentIndex + 3); // 添加索引
            out_mesh.AddIndex(currentIndex + 2); // 添加索引

            s_current += road_param.resolution * 3; // 更新当前距离

            break;
          }
          case carla::road::element::LaneMarking::Type::SolidSolid: { // 实线与实线
            s_current += road_param.resolution; // 更新当前距离
            break;
          }
          case carla::road::element::LaneMarking::Type::SolidBroken: { // 实线与虚线
            s_current += road_param.resolution; // 更新当前距离
            break;
          }
          case carla::road::element::LaneMarking::Type::BrokenSolid: { // 虚线与实线
            s_current += road_param.resolution; // 更新当前距离
            break;
          }
          case carla::road::element::LaneMarking::Type::BrokenBroken: { // 虚线与虚线
            s_current += road_param.resolution; // 更新当前距离
            break;
          }
          case carla::road::element::LaneMarking::Type::BottsDots: { // 点状标记
            s_current += road_param.resolution; // 更新当前距离
            break;
          }
          case carla::road::element::LaneMarking::Type::Grass: { // 草坪标记
            s_current += road_param.resolution; // 更新当前距离
            break;
          }
          case carla::road::element::LaneMarking::Type::Curb: { // 路缘标记
            s_current += road_param.resolution; // 更新当前距离
            break;
          }
          case carla::road::element::LaneMarking::Type::Other: { // 其他标记
            s_current += road_param.resolution; // 更新当前距离
            break;
          }
          default: { // 默认情况
            s_current += road_param.resolution; // 更新当前距离
            break;
          }
        }
      }
     } while (s_current < s_end); // 当当前距离小于结束距离时继续循环

    if (out_mesh.IsValid()) { // 如果输出网格有效
      const carla::road::element::RoadInfoMarkRecord* road_info_mark = lane.GetInfo<carla::road::element::RoadInfoMarkRecord>(s_current); // 获取当前距离的标记信息
      if (road_info_mark != nullptr) { // 如果标记信息存在
        carla::road::element::LaneMarking lane_mark_info(*road_info_mark); // 创建车道标记信息对象
        
        std::pair<geom::Vector3D, geom::Vector3D> edges = 
              ComputeEdgesForLanemark(lane_section, lane, s_end, lane_mark_info.width); // 计算结束距离的边缘

        out_mesh.AddVertex(edges.first); // 添加第一个顶点
        out_mesh.AddVertex(edges.second); // 添加第二个顶点
      }
      inout.push_back(std::make_unique<Mesh>(out_mesh)); // 将输出网格添加到输入输出中
    }
  }


  void MeshFactory::GenerateLaneMarksForCenterLine(
    const road::Road& road,
    const road::LaneSection& lane_section,
    const road::Lane& lane,
    std::vector<std::unique_ptr<Mesh>>& inout,
    std::vector<std::string>& outinfo ) const
  {
    Mesh out_mesh; // 初始化输出网格
    const double s_start = lane_section.GetDistance(); // 获取车道段的起始距离
    const double s_end = lane_section.GetDistance() + lane_section.GetLength(); // 获取车道段的结束距离
    double s_current = s_start; // 当前距离从起始距离开始
    std::vector<geom::Vector3D> vertices; // 顶点容器
    std::vector<size_t> indices; // 索引容器

    do {
      // 获取车道信息
      const carla::road::element::RoadInfoMarkRecord* road_info_mark = lane.GetInfo<carla::road::element::RoadInfoMarkRecord>(s_current);
      if (road_info_mark != nullptr) { // 如果标记信息不为空
        carla::road::element::LaneMarking lane_mark_info(*road_info_mark); // 创建车道标记信息

        switch (lane_mark_info.type) { // 根据标记类型进行处理
          case carla::road::element::LaneMarking::Type::Solid: { // 实线情况
            size_t currentIndex = out_mesh.GetVertices().size() + 1; // 当前顶点索引

            carla::road::element::DirectedPoint rightpoint = road.GetDirectedPointIn(s_current); // 获取右侧点
            carla::road::element::DirectedPoint leftpoint = rightpoint; // 左侧点初始化为右侧点

            rightpoint.ApplyLateralOffset(lane_mark_info.width * 0.5); // 应用右侧偏移
            leftpoint.ApplyLateralOffset(lane_mark_info.width * -0.5); // 应用左侧偏移

            // Unreal的Y轴处理
            rightpoint.location.y *= -1; // 反转右侧点的Y坐标
            leftpoint.location.y *= -1; // 反转左侧点的Y坐标

            out_mesh.AddVertex(rightpoint.location); // 添加右侧点到网格
            out_mesh.AddVertex(leftpoint.location); // 添加左侧点到网格

            out_mesh.AddIndex(currentIndex); // 添加索引
            out_mesh.AddIndex(currentIndex + 1); // 添加索引
            out_mesh.AddIndex(currentIndex + 2); // 添加索引

            out_mesh.AddIndex(currentIndex + 1); // 添加索引
            out_mesh.AddIndex(currentIndex + 3); // 添加索引
            out_mesh.AddIndex(currentIndex + 2); // 添加索引

            s_current += road_param.resolution; // 更新当前距离
            break;
          }
          case carla::road::element::LaneMarking::Type::Broken: { // 虚线情况
            size_t currentIndex = out_mesh.GetVertices().size() + 1; // 当前顶点索引

            std::pair<geom::Vector3D, geom::Vector3D> edges = 
              ComputeEdgesForLanemark(lane_section, lane, s_current, lane_mark_info.width); // 计算边缘
            
            out_mesh.AddVertex(edges.first); // 添加边缘第一个点到网格
            out_mesh.AddVertex(edges.second); // 添加边缘第二个点到网格

            s_current += road_param.resolution * 3; // 更新当前距离
            if (s_current > s_end) { // 如果当前距离超过结束距离
              s_current = s_end; // 修正当前距离为结束距离
            }

            edges = ComputeEdgesForLanemark(lane_section, lane, s_current, lane_mark_info.width); // 计算下一个边缘

            out_mesh.AddVertex(edges.first); // 添加下一个边缘第一个点到网格
            out_mesh.AddVertex(edges.second); // 添加下一个边缘第二个点到网格

            out_mesh.AddIndex(currentIndex); // 添加索引
            out_mesh.AddIndex(currentIndex + 1); // 添加索引
            out_mesh.AddIndex(currentIndex + 2); // 添加索引

            out_mesh.AddIndex(currentIndex + 1); // 添加索引
            out_mesh.AddIndex(currentIndex + 3); // 添加索引
            out_mesh.AddIndex(currentIndex + 2); // 添加索引

            s_current += road_param.resolution * 3; // 更新当前距离

            break;
          }
          case carla::road::element::LaneMarking::Type::SolidSolid: {  // 实线-实线标记
            s_current += road_param.resolution;   // 当前距离增加分辨率
            break;
          }
          case carla::road::element::LaneMarking::Type::SolidBroken: {  // 实线-实线标记
            s_current += road_param.resolution;  // 当前距离增加分辨率
            break;
          }
          case carla::road::element::LaneMarking::Type::BrokenSolid: {  //虚线-实线标记
            s_current += road_param.resolution;  // 当前距离增加分辨率
            break; 
          }
          case carla::road::element::LaneMarking::Type::BrokenBroken: {  // 虚线-虚线标记
            s_current += road_param.resolution;  // 当前距离增加分辨率
            break;
          }
          case carla::road::element::LaneMarking::Type::BottsDots: {   // Botts点标记
            s_current += road_param.resolution;  // 当前距离增加分辨率
            break;
          }
          case carla::road::element::LaneMarking::Type::Grass: {  // 草地标记
            s_current += road_param.resolution;  // 当前距离增加分辨率
            break;
          }
          case carla::road::element::LaneMarking::Type::Curb: {   // 路缘标记
            s_current += road_param.resolution;  // 当前距离增加分辨率
            break;
          }
          case carla::road::element::LaneMarking::Type::Other: {  // 其他类型标记
            s_current += road_param.resolution;  // 当前距离增加分辨率
            break;
          }
          default: {
            s_current += road_param.resolution;  // 当前距离增加分辨率
            break;
          }
        }
      }
    } while (s_current < s_end);   // 循环直到当前距离小于结束距离

    if (out_mesh.IsValid()) {  // 如果输出网格有效
      const carla::road::element::RoadInfoMarkRecord* road_info_mark = lane.GetInfo<carla::road::element::RoadInfoMarkRecord>(s_current);  // 获取道路信息标记记录
      if (road_info_mark != nullptr)  // 如果道路信息标记不为空
      {
        carla::road::element::LaneMarking lane_mark_info(*road_info_mark);  // 创建车道标记信息
        carla::road::element::DirectedPoint rightpoint = road.GetDirectedPointIn(s_current);   // 获取当前距离的右侧点
        carla::road::element::DirectedPoint leftpoint = rightpoint;  // 左侧点初始化为右侧点

        rightpoint.ApplyLateralOffset(lane_mark_info.width * 0.5f);   // 右侧点应用横向偏移
        leftpoint.ApplyLateralOffset(lane_mark_info.width * -0.5f);  // 左侧点应用横向偏移

        // Unreal的Y轴处理
        rightpoint.location.y *= -1;  // 反转右侧点的Y轴
        leftpoint.location.y *= -1;  // 反转左侧点的Y轴

        out_mesh.AddVertex(rightpoint.location);  // 向输出网格添加右侧点
        out_mesh.AddVertex(leftpoint.location);  // 向输出网格添加左侧点

      }
      inout.push_back(std::make_unique<Mesh>(out_mesh));  // 将输出网格添加到输出列表
    }
  }

  struct VertexWeight {   // 顶点权重结构
    Mesh::vertex_type* vertex;  // 顶点指针
    double weight;  // 权重值
  };
  struct VertexNeighbors {  // 顶点邻居结构
    Mesh::vertex_type* vertex;  // 顶点指针
    std::vector<VertexWeight> neighbors;  // 邻居顶点权重列表
  };
  struct VertexInfo {   // 顶点信息结构
    Mesh::vertex_type * vertex;  // 顶点指针
    size_t lane_mesh_idx;  // 车道网格索引
    bool is_static;  // 是否为静态顶点
  };

  // 计算邻居顶点权重的辅助函数
  static VertexWeight ComputeVertexWeight(
      const MeshFactory::RoadParameters &road_param,  // 道路参数
      const VertexInfo &vertex_info,  // 顶点信息
      const VertexInfo &neighbor_info) {  // 邻居信息
    const float distance3D = geom::Math::Distance(*vertex_info.vertex, *neighbor_info.vertex);  // 计算三维距离
    // 忽略超过一定距离的顶点
    if(distance3D > road_param.max_weight_distance) {
      return {neighbor_info.vertex, 0};  // 超过最大权重距离，返回权重为0
    }
    if(abs(distance3D) < EPSILON) {  // 如果距离小于极小值
      return {neighbor_info.vertex, 0};  // 返回权重为0
    }
    float weight = geom::Math::Clamp<float>(1.0f / distance3D, 0.0f, 100000.0f);   // 计算权重并限制范围

    // 对于同一车道的顶点增加额外权重
    if(vertex_info.lane_mesh_idx == neighbor_info.lane_mesh_idx) {
      weight *= road_param.same_lane_weight_multiplier;  // 乘以同车道权重因子
      // 对于固定顶点进一步增加权重
      if(neighbor_info.is_static) {
        weight *= road_param.lane_ends_multiplier;  / 乘以车道结束权重因子
      }
    }
    return {neighbor_info.vertex, weight};  // 返回邻居顶点和计算的权重
  }
  // 辅助函数，计算顶点的邻域及其权重
  std::vector<VertexNeighbors> GetVertexNeighborhoodAndWeights(
      const MeshFactory::RoadParameters &road_param,  // 道路参数
      std::vector<std::unique_ptr<Mesh>> &lane_meshes) {  // 车道网格
    // 构建R树以进行邻域查询
    using Rtree = geom::PointCloudRtree<VertexInfo>;  // R树类型
    using Point = Rtree::BPoint;  // 点类型
    Rtree rtree;  // 创建R树实例
    for (size_t lane_mesh_idx = 0; lane_mesh_idx < lane_meshes.size(); ++lane_mesh_idx) {  // 遍历每个车道网格
      auto& mesh = lane_meshes[lane_mesh_idx];   // 获取当前网格
      for(size_t i = 0; i < mesh->GetVerticesNum(); ++i) {  // 遍历每个顶点
        auto& vertex = mesh->GetVertices()[i];  // 获取当前顶点
        Point point(vertex.x, vertex.y, vertex.z);  // 创建点对象
        if (i < 2 || i >= mesh->GetVerticesNum() - 2) {  // 判断顶点是否为边界顶点
          rtree.InsertElement({point, {&vertex, lane_mesh_idx, true}});  // 插入边界顶点到R树
        } else {
          rtree.InsertElement({point, {&vertex, lane_mesh_idx, false}});  // 插入非边界顶点到R树
        }
      }
    }

  // 查找每个顶点的邻居并计算它们的权重
  std::vector<VertexNeighbors> vertices_neighborhoods;  // 顶点邻域集合
  for (size_t lane_mesh_idx = 0; lane_mesh_idx < lane_meshes.size(); ++lane_mesh_idx) {  // 遍历每个车道网格
    auto& mesh = lane_meshes[lane_mesh_idx];  // 获取当前网格
    for(size_t i = 0; i < mesh->GetVerticesNum(); ++i) {  // 遍历每个顶点
      if (i > 2 && i < mesh->GetVerticesNum() - 2) {  // 排除边界顶点
        auto& vertex = mesh->GetVertices()[i];  // 获取当前顶点
        Point point(vertex.x, vertex.y, vertex.z);  // 创建点对象
        auto closest_vertices = rtree.GetNearestNeighbours(point, 20);  // 查询最近的20个顶点
        VertexNeighbors vertex_neighborhood;  // 创建顶点邻域对象
        vertex_neighborhood.vertex = &vertex;  // 设置当前顶点
        for(auto& close_vertex : closest_vertices) {  // 遍历最近的顶点
          auto &vertex_info = close_vertex.second;  // 获取顶点信息
          if(&vertex == vertex_info.vertex) {  // 如果是自身，跳过
            continue;
          }
          auto vertex_weight = ComputeVertexWeight(  // 计算顶点权重
              road_param, {&vertex, lane_mesh_idx, false}, vertex_info);
          if(vertex_weight.weight > 0)  // 如果权重大于0
            vertex_neighborhood.neighbors.push_back(vertex_weight);  // 添加到邻域中
        }
        vertices_neighborhoods.push_back(vertex_neighborhood);  // 将邻域添加到结果中
      }
    }
  }
  return vertices_neighborhoods;  // 返回所有顶点的邻域
}

std::unique_ptr<Mesh> MeshFactory::MergeAndSmooth(std::vector<std::unique_ptr<Mesh>> &lane_meshes) const {
    geom::Mesh out_mesh;  // 创建一个输出网格对象

    auto vertices_neighborhoods = GetVertexNeighborhoodAndWeights(road_param, lane_meshes);  // 获取顶点邻域和权重

    // 拉普拉斯函数
    auto Laplacian = [&](const Mesh::vertex_type* vertex, const std::vector<VertexWeight> &neighbors) -> double {
      double sum = 0;  // 初始化总和
      double sum_weight = 0;  // 初始化权重总和
      for(auto &element : neighbors) {  // 遍历邻居
        sum += (element.vertex->z - vertex->z)*element.weight;  // 计算高度差乘以权重的累加
        sum_weight += element.weight;  // 累加权重
      }
      if(sum_weight > 0)  // 如果权重大于0
        return sum / sum_weight;  // 返回平均值
      else
        return 0;  // 否则返回0
    };
    
    // 运行迭代算法
    double lambda = 0.5;  // 设置拉普拉斯平滑的参数
    int iterations = 100;  // 设置迭代次数
    for(int iter = 0; iter < iterations; ++iter) {  // 进行迭代
      for (auto& vertex_neighborhood : vertices_neighborhoods) {  // 遍历每个顶点的邻域
        auto * vertex = vertex_neighborhood.vertex;  // 获取当前顶点
        vertex->z += static_cast<float>(lambda*Laplacian(vertex, vertex_neighborhood.neighbors));  // 更新顶点高度
      }
    }

    for(auto &mesh : lane_meshes) {  // 遍历所有车道网格
      out_mesh += *mesh;  // 将每个网格添加到输出网格中
    }

    return std::make_unique<Mesh>(out_mesh);  // 返回新的网格对象
}

uint32_t MeshFactory::SelectVerticesInWidth(uint32_t default_num_vertices, road::Lane::LaneType type) {
    switch(type) {  // 根据车道类型选择顶点数量
      case road::Lane::LaneType::Driving:  // 驾驶车道
      case road::Lane::LaneType::Parking:  // 停车车道
      case road::Lane::LaneType::Bidirectional:  // 双向车道
      {
        return default_num_vertices;  // 返回默认顶点数量
      }
      case road::Lane::LaneType::Shoulder:  // 应急车道
      case road::Lane::LaneType::Sidewalk:  // 人行道
      case road::Lane::LaneType::Biking:  // 自行车道
      {
        return 6;  // 返回固定的6个顶点
      }
      default:  // 其他类型
      {
        return 2;  // 返回2个顶点
      }
    }
}

std::pair<geom::Vector3D, geom::Vector3D> MeshFactory::ComputeEdgesForLanemark(
    const road::LaneSection& lane_section,  // 车道段
    const road::Lane& lane,  // 车道
    const double s_current,  // 当前参数
    const double lanemark_width) const {  // 车道标记宽度
    std::pair<geom::Vector3D, geom::Vector3D> edges =
      lane.GetCornerPositions(s_current, road_param.extra_lane_width);  // 获取车道边缘位置

    geom::Vector3D director;  // 方向向量
    if (edges.first != edges.second) {  // 如果两个边缘位置不同
      director = edges.second - edges.first;  // 计算方向向量
      director /= director.Length();  // 标准化方向向量
    } else {  // 如果边缘位置相同
      const std::map<road::LaneId, road::Lane> & lanes = lane_section.GetLanes();  // 获取车道映射
      for (const auto& lane_pair : lanes) {  // 遍历每个车道
        std::pair<geom::Vector3D, geom::Vector3D> another_edge =
          lane_pair.second.GetCornerPositions(s_current, road_param.extra_lane_width);  // 获取另一个车道的边缘位置
        if (another_edge.first != another_edge.second) {  // 如果边缘位置不同
          director = another_edge.second - another_edge.first;  // 计算方向向量
          director /= director.Length();  // 标准化方向向量
          break;  // 退出循环
        }
      }
    }
    geom::Vector3D endmarking = edges.first + director * lanemark_width;  // 计算车道标记的结束位置
    return std::make_pair(edges.first, endmarking);  // 返回边缘位置对
  }

} // namespace geom
} // namespace carla
