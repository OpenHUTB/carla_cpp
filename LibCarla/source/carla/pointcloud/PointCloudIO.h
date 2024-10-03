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
      FileSystem::ValidateFilePath(path, ".ply");
      std::ofstream out(path);
      Dump(out, begin, end);
      return path;
    }

  private:
    template <typename PointIt> static void WriteHeader(std::ostream &out, PointIt begin, PointIt end) {
      DEBUG_ASSERT(std::distance(begin, end) >= 0);
      out << "ply\n"
           "format ascii 1.0\n"
           "element vertex " << std::to_string(static_cast<size_t>(std::distance(begin, end))) << "\n";
      begin->WritePlyHeaderInfo(out);
      out << "\nend_header\n";
      out << std::fixed << std::setprecision(4u);
    }
  };

} // namespace pointcloud
} // namespace carla
