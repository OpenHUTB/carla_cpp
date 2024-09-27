// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <memory>  // 引入内存管理的头文件
#include <vector>  // 引入动态数组的头文件

#include <carla/geom/Mesh.h>  // 引入Mesh类的定义
#include <carla/road/Road.h>  // 引入Road类的定义
#include <carla/road/LaneSection.h>  // 引入LaneSection类的定义
#include <carla/road/Lane.h>  // 引入Lane类的定义
#include <carla/rpc/OpendriveGenerationParameters.h>  // 引入Opendrive生成参数的定义

namespace carla {  // 定义carla命名空间
namespace geom {  // 定义geom命名空间
namespace deformation {  // 定义deformation命名空间

  // 内联函数，用于计算给定坐标在变形中的z轴位置
  inline float GetZPosInDeformation(float posx, float posy) {
    // 振幅
    const float A1 = 0.6f;  // 第一振幅
    const float A2 = 1.1f;  // 第二振幅
    // 相位
    const float F1 = 1000.0;  // 第一相位
    const float F2 = -1500.0;  // 第二相位
    // 修改因子
    const float Kx1 = 0.035f;  // x方向的修改因子1
    const float Kx2 = 0.02f;  // x方向的修改因子2

    const float Ky1 = -0.08f;  // y方向的修改因子1
    const float Ky2 = 0.05f;  // y方向的修改因子2

    // 计算并返回变形后的z轴位置
    return A1 * sin((Kx1 * posx + Ky1 * posy + F1)) +
           A2 * sin((Kx2 * posx + Ky2 * posy + F2));
  }

  // 内联函数，用于计算给定坐标的隆起变形
  inline float GetBumpDeformation(float posx, float posy) {
    const float A3 = 0.10f;  // 隆起的振幅
    float bumpsoffset = 0;  // 隆起偏移量，初始化为0

    float constraintX = 17.0f;  // x方向的约束
    float constraintY = 12.0f;  // y方向的约束

    // 计算与约束相关的隆起坐标
    float BumpX = std::ceil(posx / constraintX);  // 向上取整计算隆起x坐标
    float BumpY = std::floor(posy / constraintY);  // 向下取整计算隆起y坐标

    // 将隆起坐标乘以约束，得到实际隆起坐标
    BumpX *= constraintX;  
    BumpY *= constraintY;  

    // 计算到隆起中心的距离
    float DistanceToBumpOrigin = sqrt(pow(BumpX - posx, 2) + pow(BumpY - posy, 2));
    float MaxDistance = 2.0;  // 最大距离限制

    // 如果距离小于等于最大距离，则计算隆起偏移量
    if (DistanceToBumpOrigin <= MaxDistance) {
      bumpsoffset = sin(DistanceToBumpOrigin);  // 使用距离计算隆起偏移量
    }

    // 返回最终的隆起变形值
    return A3 * bumpsoffset;  
  }

} // namespace deformation
} // namespace geom
} // namespace carla