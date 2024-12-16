// Copyright (c) 2020 Robotics and Perception Group (GPR)
// University of Zurich and ETH Zurich
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 防止头文件被多次包含

#include "carla/Debug.h" // 包含调试相关的头文件
#include "carla/sensor/data/Array.h" // 包含数组数据结构的头文件
#include "carla/sensor/data/DVSEvent.h" // 包含DVS事件的数据结构
#include "carla/sensor/data/Color.h" // 包含颜色相关的数据结构
#include "carla/sensor/s11n/DVSEventArraySerializer.h" // 包含序列化DVS事件数组的头文件

namespace carla {
namespace sensor {
namespace data {

  /// DVS事件的数组，采用HxW图像结构
  class DVSEventArray : public Array<DVSEvent> {
    using Super = Array<DVSEvent>; // 使用父类Array<DVSEvent>的别名
  protected:

    using Serializer = s11n::DVSEventArraySerializer; // 序列化器的别名

    friend Serializer; // 声明Serializer为友元类，可以访问私有成员

    explicit DVSEventArray(RawData &&data) // 构造函数，接受原始数据
      : Super(Serializer::header_offset, std::move(data)) { // 调用父类构造函数
    }

  private:

    const auto &GetHeader() const { // 获取头部信息
      return Serializer::DeserializeHeader(Super::GetRawData()); // 反序列化头部
    }
  public:

    using event_type = DVSEvent; // 将事件类型定义为DVSEvent

    /// 获取图像的宽度（以像素为单位）
    auto GetWidth() const { 
      return GetHeader().width; // 返回头部中的宽度
    }

    /// 获取图像的高度（以像素为单位）
    auto GetHeight() const { 
      return GetHeader().height; // 返回头部中的高度
    }

    /// 获取图像的水平视野角度（以度为单位）
    auto GetFOVAngle() const { 
      return GetHeader().fov_angle; // 返回头部中的视野角度
    }

    /// 获取事件"帧"图像用于可视化
    std::vector<Color> ToImage() const { 
      std::vector<Color> img(GetHeight() * GetWidth()); // 创建图像向量
      for (const auto &event : *this) { // 遍历所有事件
        size_t index = (GetWidth() * event.y) + event.x; // 计算图像索引
        if (event.pol == true) { // 如果极性为正
          img[index].b = 255u; // 将蓝色通道设为255
        } else { // 否则
          img[index].r = 255u; // 将红色通道设为255
        }
      }
      return img; // 返回生成的图像
    }

    /// 获取事件的纯向量格式数组
    std::vector<std::vector<std::int64_t>> ToArray() const { 
      std::vector<std::vector<std::int64_t>> array; // 创建二维数组
      for (const auto &event : *this) { // 遍历所有事件
        array.push_back({static_cast<std::int64_t>(event.x), static_cast<std::int64_t>(event.y), static_cast<std::int64_t>(event.t), (2*static_cast<std::int64_t>(event.pol)) - 1}); // 添加事件的x, y, t和极性信息
      }
      return array; // 返回事件数组
    }

    /// 获取所有事件的x坐标，便于使用
    std::vector<std::uint16_t> ToArrayX() const { 
      std::vector<std::uint16_t> array; // 创建x坐标数组
      for (const auto &event : *this) { // 遍历所有事件
        array.push_back(event.x); // 添加x坐标
      }
      return array; // 返回x坐标数组
    }

    /// 获取所有事件的y坐标，便于使用
    std::vector<std::uint16_t> ToArrayY() const { 
      std::vector<std::uint16_t> array; // 创建y坐标数组
      for (const auto &event : *this) { // 遍历所有事件
        array.push_back(event.y); // 添加y坐标
      }
      return array; // 返回y坐标数组
    }

    /// 获取所有事件的时间戳，便于使用
    std::vector<std::int64_t> ToArrayT() const { 
      std::vector<std::int64_t> array; // 创建时间戳数组
      for (const auto &event : *this) { // 遍历所有事件
        array.push_back(event.t); // 添加时间戳
      }
      return array; // 返回时间戳数组
    }

    /// 获取所有事件的极性，便于使用
    std::vector<short> ToArrayPol() const { 
      std::vector<short> array; // 创建极性数组
      for (const auto &event : *this) { // 遍历所有事件
        array.push_back(2*static_cast<short>(event.pol) - 1); // 将极性转换为-1和1
      }
      return array; // 返回极性数组
    }

  };

} // namespace data
} // namespace sensor
} // namespace carla
