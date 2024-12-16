// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>
#include <vector>

#include <carla/geom/Mesh.h>  // 引入Mesh类的定义
#include <carla/road/Road.h>  // 引入Road类的定义
#include <carla/road/LaneSection.h>  // 引入LaneSection类的定义
#include <carla/road/Lane.h>  // 引入Lane类的定义
#include <carla/rpc/OpendriveGenerationParameters.h>  // 引入Opendrive生成参数类的定义

namespace carla {
namespace geom {

  /// Mesh辅助生成器
  class MeshFactory {
  public:

    // 构造函数，初始化生成参数
    MeshFactory(rpc::OpendriveGenerationParameters params =
        rpc::OpendriveGenerationParameters());

    // =========================================================================
    // -- 地图相关 -------------------------------------------------------------
    // =========================================================================

    // -- 基本 --

    /// 生成定义一条道路的网格
    std::unique_ptr<Mesh> Generate(const road::Road &road) const;

    /// 生成定义一段车道的网格
    std::unique_ptr<Mesh> Generate(const road::LaneSection &lane_section) const;

    /// 从给定的s起始和结束生成车道的网格
    std::unique_ptr<Mesh> Generate(
        const road::Lane &lane, const double s_start, const double s_end) const;

    /// 用更高的细分生成从给定的s起始和结束的车道网格
    std::unique_ptr<Mesh> GenerateTesselated(
      const road::Lane& lane, const double s_start, const double s_end) const;

    /// 生成定义整个车道的网格
    std::unique_ptr<Mesh> Generate(const road::Lane &lane) const;

    /// 生成定义整个车道的网格，使用更高的细分
    std::unique_ptr<Mesh> GenerateTesselated(const road::Lane& lane) const;

    /// 生成按顺序排列的车道段网格
    void GenerateLaneSectionOrdered(const road::LaneSection &lane_section,
        std::map<carla::road::Lane::LaneType , std::vector<std::unique_ptr<Mesh>>>& result ) const;

    /// 生成车道段的人行道网格
    std::unique_ptr<Mesh> GenerateSidewalk(const road::LaneSection &lane_section) const;

    /// 生成车道的人行道网格
    std::unique_ptr<Mesh> GenerateSidewalk(const road::Lane &lane) const;

    /// 从给定的s起始和结束生成车道的人行道网格
    std::unique_ptr<Mesh> GenerateSidewalk(const road::Lane &lane, const double s_start, const double s_end) const;

    // -- 墙 --

    /// 生成一个网格，表示道路角落的墙，以避免车辆掉下去
    std::unique_ptr<Mesh> GenerateWalls(const road::LaneSection &lane_section) const;

    /// 在车道的右侧生成一个墙状网格
    std::unique_ptr<Mesh> GenerateRightWall(
        const road::Lane &lane, const double s_start, const double s_end) const;

    /// 在车道的左侧生成一个墙状网格
    std::unique_ptr<Mesh> GenerateLeftWall(
        const road::Lane &lane, const double s_start, const double s_end) const;

    // -- 分块 --

    /// 生成一个定义道路的网格列表，限制最大长度
    std::vector<std::unique_ptr<Mesh>> GenerateWithMaxLen(
        const road::Road &road) const;

    /// 生成一个定义车道段的网格列表，限制最大长度
    std::vector<std::unique_ptr<Mesh>> GenerateWithMaxLen(
        const road::LaneSection &lane_section) const;

    /// 生成一个定义道路的按顺序排列的网格列表，限制最大长度
    std::map<carla::road::Lane::LaneType , std::vector<std::unique_ptr<Mesh>>> GenerateOrderedWithMaxLen(
        const road::Road &road) const;

    /// 生成一个定义车道段的按顺序排列的网格列表，限制最大长度
    std::map<carla::road::Lane::LaneType , std::vector<std::unique_ptr<Mesh>>> GenerateOrderedWithMaxLen(
        const road::LaneSection &lane_section) const;

    /// 生成一个定义道路安全墙的网格列表，限制最大长度
    std::vector<std::unique_ptr<Mesh>> GenerateWallsWithMaxLen(
        const road::Road &road) const;

    /// 生成一个定义车道段安全墙的网格列表，限制最大长度
    std::vector<std::unique_ptr<Mesh>> GenerateWallsWithMaxLen(
        const road::LaneSection &lane_section) const;

    // -- Util --

    /// 生成带有所有模拟所需特性的分块道路
    std::vector<std::unique_ptr<Mesh>> GenerateAllWithMaxLen(
        const road::Road &road) const;  // 根据给定的道路生成分块网格


    /// 生成按顺序排列的道路网格，限制最大长度
    void GenerateAllOrderedWithMaxLen(const road::Road &road,
         std::map<road::Lane::LaneType , std::vector<std::unique_ptr<Mesh>>>& roads) const;  // 生成有序的网格列表

    /// 合并并平滑多个车道网格
    std::unique_ptr<Mesh> MergeAndSmooth(std::vector<std::unique_ptr<Mesh>> &lane_meshes) const;  // 合并并平滑车道网格

    // -- LaneMarks --
    
    /// 为道路生成车道标记
    void GenerateLaneMarkForRoad(const road::Road& road,
      std::vector<std::unique_ptr<Mesh>>& inout,
      std::vector<std::string>& outinfo ) const;  // 生成道路的车道标记

    /// 为非中心线（即ID不为0的所有线）生成车道标记
    void GenerateLaneMarksForNotCenterLine(
      const road::LaneSection& lane_section,
      const road::Lane& lane,
      std::vector<std::unique_ptr<Mesh>>& inout,
      std::vector<std::string>& outinfo ) const;  // 生成非中心线的车道标记

    /// 仅为中心线（ID为0的线）生成车道标记
    void GenerateLaneMarksForCenterLine(
      const road::Road& road,
      const road::LaneSection& lane_section,
      const road::Lane& lane,
      std::vector<std::unique_ptr<Mesh>>& inout,
      std::vector<std::string>& outinfo ) const;  // 生成中心线的车道标记
      
    // =========================================================================
    // -- Generation parameters ------------------------------------------------
    // =========================================================================

    /// 道路生成参数
    struct RoadParameters {
      float resolution                  =  2.0f;  // 分辨率
      float max_road_len                = 50.0f;  // 最大道路长度
      float extra_lane_width            =  1.0f;  // 额外车道宽度
      float wall_height                 =  0.6f;  // 墙的高度
      float vertex_width_resolution     =  4.0f;  // 顶点宽度分辨率
      // 道路网格平滑度：
      float max_weight_distance         =  5.0f;  // 最大权重距离
      float same_lane_weight_multiplier =  2.0f;  // 同一车道的权重乘数
      float lane_ends_multiplier        =  2.0f;  // 车道末端的乘数
    };

    RoadParameters road_param;  // 道路参数实例


    // =========================================================================
    // -- Helper functions ------------------------------------------------
    // =========================================================================

    /// 在给定车道类型下选择的宽度内的顶点数量
    static uint32_t SelectVerticesInWidth(uint32_t default_num_vertices, road::Lane::LaneType type);  // 根据车道类型选择顶点数量
  private:

    /// 计算车道标记的当前s位置两侧的点
    std::pair<geom::Vector3D, geom::Vector3D> ComputeEdgesForLanemark(
      const road::LaneSection& lane_section,
      const road::Lane& lane,
      const double s_current,
      const double lanemark_width) const;  // 计算车道标记的边缘点


  };

} // namespace geom
} // namespace carla
