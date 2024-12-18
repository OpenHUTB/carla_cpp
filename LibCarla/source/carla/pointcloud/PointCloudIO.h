// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/FileSystem.h"

#include <fstream>
#include <iterator>
#include <iomanip>

namespace carla {// 定义命名空间carla，用于组织相关的代码和数据
namespace pointcloud {// 定义命名空间pointcloud，进一步组织特定于点云处理的代码
// 定义PointCloudIO类，用于处理点云数据的输入输出
  class PointCloudIO {

  public:
  // 模板函数Dump，用于将点云数据写入到输出流中，PointIt是点迭代器类型，用于遍历点云数据，out是输出流对象，begin和end分别是点云数据的起始和结束迭代器
    template <typename PointIt>
    static void Dump(std::ostream &out, PointIt begin, PointIt end) {
     // 写入PLY文件的头部信息
      WriteHeader(out, begin, end);
      // 遍历点云数据，将每个点的信息写入到输出流中
      for (; begin != end; ++begin) {
        begin->WriteDetection(out);// 假设每个点对象都有WriteDetection方法，用于写入点信息 
        out << '\n';
      }
    }

    template <typename PointIt>
    static std::string SaveToDisk(std::string path, PointIt begin, PointIt end) {
      // 验证文件路径是否以".ply"结尾，确保文件类型为PLY 
      FileSystem::ValidateFilePath(path, ".ply");
      // 创建输出文件流对象，并打开文件
      std::ofstream out(path);
      // 调用Dump函数，将点云数据写入到文件中
      Dump(out, begin, end);
       // 返回文件路径
      return path;
    }

  private:
    template <typename PointIt> static void WriteHeader(std::ostream &out, PointIt begin, PointIt end) {
      // 断言确保点云数据的数量非负
      DEBUG_ASSERT(std::distance(begin, end) >= 0);
      // 写入PLY文件的基本头部信息
      out << "ply\n"
           "format ascii 1.0\n"
           // 写入元素(vertex)的数量，即点云中的点数
           "element vertex " << std::to_string(static_cast<size_t>(std::distance(begin, end))) << "\n";
      // 假设每个点对象都有WritePlyHeaderInfo方法，用于写入特定的头部信息      
      begin->WritePlyHeaderInfo(out);
      // 写入PLY文件头部的结束标志
      out << "\nend_header\n";
      // 设置输出流的格式，固定小数点后4位 
      out << std::fixed << std::setprecision(4u);
    }
  };

} // namespace pointcloud
} // namespace carla
