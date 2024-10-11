// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/opendrive/parser/GeoReferenceParser.h" // 引入 GeoReferenceParser 头文件

#include "carla/Logging.h" // 引入日志功能
#include "carla/StringUtil.h" // 引入字符串工具
#include "carla/geom/GeoLocation.h" // 引入地理位置类
#include "carla/road/MapBuilder.h" // 引入地图构建器

#include <pugixml/pugixml.hpp> // 引入 XML 解析库

#include <limits> // 引入数值限制
#include <string> // 引入字符串处理
#include <vector> // 引入向量容器

namespace carla { // 定义 carla 命名空间
namespace opendrive { // 定义 opendrive 命名空间
namespace parser { // 定义 parser 命名空间

  static double ParseDouble(const std::string &string_value) { // 静态函数，将字符串转换为双精度浮点数
    return std::stod(string_value); // 使用 std::stod 转换
  }

  static geom::GeoLocation ParseGeoReference(const std::string &geo_reference_string) { // 静态函数，解析地理参考字符串
    geom::GeoLocation result{ // 创建 GeoLocation 对象，初始化为 NaN
        std::numeric_limits<double>::quiet_NaN(), // 纬度初始为 NaN
        std::numeric_limits<double>::quiet_NaN(), // 经度初始为 NaN
        0.0}; // 高度初始为 0.0

    std::vector<std::string> geo_ref_splitted; // 创建字符串向量以存储拆分结果
    StringUtil::Split(geo_ref_splitted, geo_reference_string, " "); // 按空格拆分 geo_reference_string

    for (auto element: geo_ref_splitted) { // 遍历拆分后的每个元素
      std::vector<std::string> geo_ref_key_value; // 创建键值对向量
      StringUtil::Split(geo_ref_key_value, element, "="); // 按等号拆分元素
      if (geo_ref_key_value.size() != 2u) { // 如果不是两个部分，则跳过
        continue; // 继续下一个循环
      }

      if (geo_ref_key_value[0] == "+lat_0") { // 如果键是 +lat_0
        result.latitude = ParseDouble(geo_ref_key_value[1]); // 解析并设置纬度
      } else if (geo_ref_key_value[0] == "+lon_0") { // 如果键是 +lon_0
        result.longitude = ParseDouble(geo_ref_key_value[1]); // 解析并设置经度
      }
    }

    if (std::isnan(result.latitude) || std::isnan(result.longitude)) { // 如果纬度或经度仍为 NaN
      log_warning("cannot parse georeference: '" + geo_reference_string + "'. Using default values."); // 记录警告日志
      result.latitude = 42.0; // 使用默认纬度
      result.longitude = 2.0; // 使用默认经度
    }

    log_debug("map geo reference: latitude ", result.latitude, ", longitude ", result.longitude); // 记录调试信息

    return result; // 返回解析的 GeoLocation 对象
  }

  void GeoReferenceParser::Parse( // GeoReferenceParser 的 Parse 函数
      const pugi::xml_document &xml, // 输入的 XML 文档
      carla::road::MapBuilder &map_builder) { // 地图构建器的引用
    map_builder.SetGeoReference(ParseGeoReference( // 设置地图的地理参考
        xml.child("OpenDRIVE").child("header").child_value("geoReference"))); // 从 XML 中提取 geoReference 字段
  }

} // parser
} // opendrive
} // carla
