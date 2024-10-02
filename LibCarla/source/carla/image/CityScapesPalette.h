// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstdint>

namespace carla {
namespace image {
namespace detail {

    static constexpr
#if __cplusplus >= 201703L  // 使用C++17标准（如果可用）定义内联函数。如果不是C++17，则忽略内联。
    inline 
#endif
    // 如果修改此颜色映射表，请更新相关文档。
    uint8_t CITYSCAPES_PALETTE_MAP[][3u] = { ///定义一个二维数组 CITYSCAPES_PALETTE_MAP，用于存储城市景观数据集的RGB颜色映射。
        {  0u,   0u,   0u},   // 黑色（RGB：0, 0, 0），用于未标记的像素。
        // 城市景观颜色映射
        {128u,  64u, 128u},   // 深紫色（RGB：128, 64, 128），用于道路。
        {244u,  35u, 232u},   // 亮紫色（RGB：244, 35, 232），用于人行道。
        { 70u,  70u,  70u},   // 中灰色（RGB：70, 70, 70），用于建筑物。
        {102u, 102u, 156u},   // 深灰色（RGB：102, 102, 156），用于墙壁。
        {190u, 153u, 153u},   // 浅棕色（RGB：190, 153, 153），用于栅栏。
        {153u, 153u, 153u},   // 中灰色（RGB：153, 153, 153），用于电线杆等柱子。
        {250u, 170u,  30u},   // 橙色（RGB：250, 170, 30），用于交通灯。
        {220u, 220u,   0u},   // 鲜黄色（RGB：220, 220, 0），用于交通标志。
        {107u, 142u,  35u},   // 深绿色（RGB：107, 142, 35），用于植被。
        {152u, 251u, 152u},   // 浅绿色（RGB：152, 251, 152），用于地形。
        { 70u, 130u, 180u},   // 浅蓝色（RGB：70, 130, 180），用于天空。
        {220u,  20u,  60u},   // 深红色（RGB：220, 20, 60），用于行人。
        {255u,   0u,   0u},   // 鲜红色（RGB：255, 0, 0），用于骑行者。
        {  0u,   0u, 142u},   // 深蓝色（RGB：0, 0, 142），用于汽车。
        {  0u,   0u,  70u},   // 深蓝色（RGB：0, 0, 70），用于卡车。
        {  0u,  60u, 100u},   // 青色（RGB：0, 60, 100），用于公共汽车。
        {  0u,  80u, 100u},   // 青色（RGB：0, 80, 100），用于火车。
        {  0u,   0u, 230u},   // 亮蓝色（RGB：0, 0, 230），用于摩托车。
        {119u,  11u,  32u},   // 深棕色（RGB：119, 11, 32），用于自行车。
        //自定义颜色映射
        {110u, 190u, 160u},   // 浅绿色（RGB：110, 190, 160），用于静态对象。
        {170u, 120u,  50u},   // 棕色（RGB：170, 120, 50），用于动态对象。
        { 55u,  90u,  80u},   // 深灰色（RGB：55, 90, 80），用于其他对象。
        { 45u,  60u, 150u},   // 深青色（RGB：45, 60, 150），用于水域。
        {157u, 234u,  50u},   // 鲜绿色（RGB：157, 234, 50），用于道路标线。
        { 81u,   0u,  81u},   // 深紫色（RGB：81, 0, 81），用于地面。
        {150u, 100u, 100u},   // 棕色（RGB：150, 100, 100），用于桥梁。
        {230u, 150u, 140u},   // 浅棕色（RGB：230, 150, 140），用于铁路轨道。
        {180u, 165u, 180u}    // 浅紫色（RGB：180, 165, 180），用于护栏。
      };

} // namespace detail

  class CityScapesPalette { // 定义 CityScapesPalette 类，用于获取城市景观数据集中不同标签的RGB颜色值。
  public:

    static constexpr auto GetNumberOfTags() {   /// 返回颜色映射表中的标签数量。
      return sizeof(detail::CITYSCAPES_PALETTE_MAP) /
          sizeof(*detail::CITYSCAPES_PALETTE_MAP);
    }

    /// 根据标签返回对应的RGB颜色值。
    ///
    ///@警告 如果标签大于颜色映射表的大小，会发生溢出。
    static constexpr auto GetColor(uint8_t tag) {
      return detail::CITYSCAPES_PALETTE_MAP[tag % GetNumberOfTags()];
    }
  };

} // namespace image
} // namespace carla // 结束命名空间 image 和 carla。
