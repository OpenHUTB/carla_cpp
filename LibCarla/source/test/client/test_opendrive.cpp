// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h" /// @brief 包含测试相关的头文件（假设用于单元测试或示例代码）。
#include "OpenDrive.h"/// @brief 包含OpenDrive相关的自定义头文件（可能是封装了OpenDrive解析的类）。
#include "Random.h"/// @brief 包含随机数生成相关的头文件。

#include <carla/StopWatch.h> /// @brief 包含CARLA的计时器类，用于性能测量。
#include <carla/ThreadPool.h>/// @brief 包含CARLA的线程池类，用于并行处理任务。
#include <carla/geom/Location.h>/// @brief 包含地理位置相关的类，如点、向量等。
#include <carla/geom/Math.h>/// @brief 包含几何数学运算相关的函数和类。
#include <carla/opendrive/OpenDriveParser.h>/// @brief 包含OpenDrive解析器类，用于解析OpenDrive格式的地图文件。
#include <carla/road/MapBuilder.h>/// @brief 包含CARLA的路网构建器类，用于构建路网。
#include <carla/road/element/RoadInfoElevation.h>/// @brief 包含道路高程信息相关的类。
#include <carla/road/element/RoadInfoGeometry.h>/// @brief 包含道路几何信息相关的类。
#include <carla/road/element/RoadInfoMarkRecord.h>/// @brief 包含道路标记记录信息相关的类
#include <carla/road/element/RoadInfoVisitor.h>/// @brief 包含道路信息访问者模式的基类，用于遍历路网元素。

#include <pugixml/pugixml.hpp>/// @brief 包含pugixml库的头文件，用于XML解析和生成。

#include <fstream>/// @brief 包含C++标准库的文件流类，用于文件读写。
#include <string>/// @brief 包含C++标准库的字符串类。

using namespace carla::road;/// 导入CARLA的路面相关命名空间，包括道路定义和元素。
using namespace carla::road::element;/// 导入CARLA的路面元素相关的命名空间，包括具体的道路元素定义。
using namespace carla::geom;/// 导入CARLA的几何相关命名空间，包括位置、方向和形状的定义。
using namespace carla::opendrive;/// 导入CARLA的OpenDRIVE数据模型相关的命名空间，用于处理道路网络数据。
using namespace util;/// 导入CARLA的实用工具命名空间，包含常用的功能函数和类。
/**
 * @brief 定义了OpenDrive XML文件的基本路径。
 */
const std::string BASE_PATH = LIBCARLA_TEST_CONTENT_FOLDER "/OpenDrive/";


static void test_road_elevation(const pugi::xml_document &xml, boost::optional<Map>& map) {
  pugi::xml_node open_drive_node = xml.child("OpenDRIVE");

  for (pugi::xml_node road_node : open_drive_node.children("road")) {
    RoadId road_id = road_node.attribute("id").as_uint();
    auto elevation_profile_nodes = road_node.children("elevationProfile");

    for (pugi::xml_node elevation_profile_node : elevation_profile_nodes) {
      auto total_elevations = 0;
      auto elevation_nodes = elevation_profile_node.children("elevation");
      auto total_elevation_parser = std::distance(elevation_nodes.begin(), elevation_nodes.end());

      for (pugi::xml_node elevation_node : elevation_nodes) {
        float s = elevation_node.attribute("s").as_float();
        const auto elevation = map->GetMap().GetRoad(road_id).GetInfo<RoadInfoElevation>(s);
        if (elevation != nullptr)
          ++total_elevations;
      }
      ASSERT_EQ(total_elevations, total_elevation_parser);
    }
  }
}


static void test_geometry(const pugi::xml_document &xml, boost::optional<Map>& map) {
  pugi::xml_node open_drive_node = xml.child("OpenDRIVE");

  for (pugi::xml_node road_node : open_drive_node.children("road")) {
    RoadId road_id = road_node.attribute("id").as_uint();

    for (pugi::xml_node plan_view_nodes : road_node.children("planView")) {
      auto geometries_parser = plan_view_nodes.children("geometry");
      auto total_geometries_parser = std::distance(geometries_parser.begin(), geometries_parser.end());
      auto total_geometries = 0;
      for (pugi::xml_node geometry_node : plan_view_nodes.children("geometry")){
        float s = geometry_node.attribute("s").as_float();
        auto geometry = map->GetMap().GetRoad(road_id).GetInfo<RoadInfoGeometry>(s);
        if (geometry != nullptr)
          ++total_geometries;
      }
      ASSERT_EQ(total_geometries, total_geometries_parser);
    }
  }
}

/// \brief 获取车道节点中所有道路标记的总数，并对比解析得到的道路标记数量。
/// 
/// 该函数遍历给定的车道节点范围，对每个车道节点中的道路标记进行解析和验证。
/// 它计算了两种道路标记的总数：一种是实际解析得到的道路标记数量，另一种是
/// 根据道路信息记录验证得到的道路标记数量。
/// 
/// \param lane_nodes 车道节点的范围，这些节点包含了道路标记信息。
/// \param lane_section 车道段对象，用于获取具体的车道信息。
/// \return 返回一个包含两个整数的pair，第一个整数是验证得到的道路标记总数，
///         第二个整数是解析得到的道路标记总数。
///
/// \note 使用Google Test的EXPECT宏进行断言验证。
///
static auto get_total_road_marks(
    pugi::xml_object_range<pugi::xml_named_node_iterator> &lane_nodes,
    LaneSection& lane_section) {
    /// 定义一个极小的误差值，用于浮点数比较。
  constexpr auto error = 1e-5;
  /// 用于存储验证得到的道路标记总数。
  auto total_road_mark = 0;
  /// 用于存储解析得到的道路标记总数。
  auto total_road_mark_parser = 0;
  /// 遍历车道节点范围。
  for (pugi::xml_node lane_node : lane_nodes) {

    auto road_mark_nodes = lane_node.children("roadMark");
    /// 计算并累加解析得到的道路标记数量。
    total_road_mark_parser += std::distance(road_mark_nodes.begin(), road_mark_nodes.end());
    /// 获取当前车道节点的ID。
    const int lane_id = lane_node.attribute("id").as_int();
    Lane* lane = nullptr;
    /// 根据车道ID获取对应的车道对象。
    lane = lane_section.GetLane(lane_id);
    /// 断言车道对象不为空。
    EXPECT_NE(lane, nullptr);

    /// 遍历当前车道节点中的所有道路标记节点。
    for (pugi::xml_node road_mark_node : road_mark_nodes) {
      const auto s_offset = road_mark_node.attribute("sOffset").as_double();/// 获取道路标记的sOffset属性值。
      const auto type = road_mark_node.attribute("type").as_string();/// 获取道路标记的type属性值。
      const auto material = road_mark_node.attribute("material").as_string();/// 获取道路标记的material属性值。
      const auto color = road_mark_node.attribute("color").as_string();/// 获取道路标记的color属性值。
      const auto road_mark = lane->GetInfo<RoadInfoMarkRecord>(lane->GetDistance() + s_offset);/// 根据距离和sOffset获取对应的道路信息记录。
      /// 如果道路信息记录不为空，则进行验证。
      if (road_mark != nullptr) {
        EXPECT_NEAR(lane->GetDistance() + s_offset, road_mark->GetDistance(), error); /// 断言距离误差在允许范围内。
        EXPECT_EQ(type, road_mark->GetType());/// 断言道路标记类型相同。
        EXPECT_EQ(material, road_mark->GetMaterial());/// 断言道路标记材料相同。
        EXPECT_EQ(color, road_mark->GetColor());/// 断言道路标记颜色相同。
        ++total_road_mark;/// 累加验证得到的道路标记数量。
      }
    }
  }/// 返回一个包含两个整数的pair，分别表示验证和解析得到的道路标记总数。
  return std::make_pair(total_road_mark, total_road_mark_parser);
}

/// 定义一个名为test_roads的函数，它接受一个const引用的pugi::xml_document对象和一个boost::optional<Map>对象作为参数
static void test_roads(const pugi::xml_document &xml, boost::optional<Map>& map) {
 /// 从XML文档中获取名为"OpenDRIVE"的子节点
  pugi::xml_node open_drive_node = xml.child("OpenDRIVE");

 // 解析"OpenDRIVE"节点下所有的"road"子节点，并计算它们的数量
  auto roads_parser = open_drive_node.children("road");
  auto total_roads_parser = std::distance(roads_parser.begin(), roads_parser.end());
  auto total_roads = map->GetMap().GetRoads().size();/// 从Map对象中获取当前存储的道路数量。
  ASSERT_EQ(total_roads, total_roads_parser);///用于验证XML中的道路数量与Map中存储的道路数量是否一致。
// 遍历每一个"road"节点
  for (pugi::xml_node road_node : roads_parser) {
    RoadId road_id = road_node.attribute("id").as_uint();
// 遍历该道路下的所有"lanes"子节点
    for (pugi::xml_node lanes_node : road_node.children("lanes")) {

   // 解析"lanes"节点下所有的"laneSection"子节点，并计算它们的数量
      auto lane_sections_parser = lanes_node.children("laneSection");
      auto total_lane_sections_parser = std::distance(lane_sections_parser.begin(), lane_sections_parser.end());
      auto total_lane_sections = map->GetMap().GetRoad(road_id).GetLaneSections().size(); /// 从Map对象中获取当前道路的所有车道段数量
      ASSERT_EQ(total_lane_sections, total_lane_sections_parser);

      for (pugi::xml_node lane_section_node : lane_sections_parser) {

        // Check total Lanes
        const double s = lane_section_node.attribute("s").as_double();/// 从"laneSection"节点中获取s属性
        auto lane_section = map->GetMap().GetRoad(road_id).GetLaneSectionsAt(s); // 从Map对象中获取对应位置的车道段
        size_t total_lanes = 0u;/// 初始化车道总数为0
        //遍历车道段中的每一个车道组
        for (auto it = lane_section.begin(); it != lane_section.end(); ++it) {
          total_lanes = it->GetLanes().size();  // 更新车道总数为当前车道组中的车道数量
        }
        // 解析"laneSection"节点下的"left"、"center"和"right"子节点中的"lane"子节点数量
        auto left_nodes = lane_section_node.child("left").children("lane");
        auto center_nodes = lane_section_node.child("center").children("lane");
        auto right_nodes = lane_section_node.child("right").children("lane");
        auto total_lanes_parser = std::distance(left_nodes.begin(), left_nodes.end());
        total_lanes_parser += std::distance(right_nodes.begin(), right_nodes.end());
        total_lanes_parser += std::distance(center_nodes.begin(), center_nodes.end());

        ASSERT_EQ(total_lanes, total_lanes_parser);//用于验证XML中的车道数量与Map中存储的车道数量是否一致

        // 初始化道路标记总数为0
        auto total_road_mark = 0;
        auto total_road_mark_parser = 0;
        // 遍历车道段中的每一个车道组
        for (auto it = lane_section.begin(); it != lane_section.end(); ++it) {
        // 计算左侧、中心和右侧车道中的道路标记总数
          auto total_left = get_total_road_marks(left_nodes, *it);
          auto total_center = get_total_road_marks(center_nodes, *it);
          auto total_right = get_total_road_marks(right_nodes, *it);
          total_road_mark = total_left.first + total_center.first + total_right.first;
          total_road_mark_parser = total_left.first + total_center.first + total_right.first;
        }
        ASSERT_EQ(total_road_mark, total_road_mark_parser);
      }
    }
  }
}

// 定义一个测试函数，用于验证XML文档中的交叉路口数据与Map对象中的交叉路口数据是否一致
static void test_junctions(const pugi::xml_document &xml, boost::optional<Map>& map) {
  pugi::xml_node open_drive_node = xml.child("OpenDRIVE"); // 从XML文档中获取OpenDRIVE根节点

  // 获取Map对象中的交叉路口集合，并检查XML文档中声明的交叉路口总数是否与Map对象中的一致
  auto& junctions = map->GetMap().GetJunctions();
  // 计算XML文档中声明的交叉路口总数
  auto total_junctions_parser = std::distance(open_drive_node.children("junction").begin(), open_drive_node.children("junction").end());
  // 使用ASSERT_EQ宏来断言两者数量相等
  ASSERT_EQ(junctions.size(), total_junctions_parser);
  // 遍历XML文档中的每一个交叉路口节点
  for (pugi::xml_node junction_node : open_drive_node.children("junction")) {
   // 检查当前交叉路口的连接总数是否与XML文档中声明的连接总数一致
    auto total_connections_parser = std::distance(junction_node.children("connection").begin(), junction_node.children("connection").end());
  // 从交叉路口节点的id属性中获取交叉路口ID
    JuncId junction_id = junction_node.attribute("id").as_int();
    auto& junction = junctions.find(junction_id)->second; // 在Map对象的交叉路口集合中查找对应的交叉路口
  // 获取当前交叉路口的连接集合
    auto& connections = junction.GetConnections();
    // 使用ASSERT_EQ宏来断言两者数量相等
    ASSERT_EQ(connections.size(), total_connections_parser);
// 遍历当前交叉路口的每一个连接节点
    for (pugi::xml_node connection_node : junction_node.children("connection")) {
      // 检查当前连接的车道链接总数是否与XML文档中声明的车道链接总数一致
      auto total_lane_links_parser = std::distance(connection_node.children("laneLink").begin(), connection_node.children("laneLink").end()); 
      // 从连接节点的id属性中获取连接ID
      ConId connection_id = connection_node.attribute("id").as_uint();
      // 在当前交叉路口的连接集合中查找对应的连接
      auto& connection = connections.find(connection_id)->second;
    // 获取当前连接的车道链接集合
    auto& lane_links = connection.lane_links;
   // 使用ASSERT_EQ宏来断言两者数量相等
    ASSERT_EQ(lane_links.size(), total_lane_links_parser);

    }
  }
}
// 定义一个名为 test_road_links 的函数，它接受一个 boost::optional<Map> 类型的引用作为参数
static void test_road_links(boost::optional<Map>& map) {

    // 使用 for 循环遍历 map 中的所有道路
  for (auto &road : map->GetMap().GetRoads()) {
     // 对于每条道路，遍历其所有的车道段section
    for (auto &section : road.second.GetLaneSections()) {
       // 对于每个车道段，遍历其所有的车道lane
      for (auto &lane : section.GetLanes()) {
        // 对于每个车道，检查其所有后续车道next lanes
        for (auto link : lane.second.GetNextLanes()) {
          ASSERT_TRUE(link != nullptr);
        }
       // 对于每个车道，检查其所有前置车道previous lanes
        for (auto link : lane.second.GetPreviousLanes()) {
          ASSERT_TRUE(link != nullptr);// 每个车道都应该有一个有效的前置车道链接
        }
      }
    }
  }
}
// 定义一个测试用例
TEST(road, parse_files) {
   // 使用 util::OpenDrive::Load 函数加载文件
  for (const auto &file : util::OpenDrive::GetAvailableFiles()) {
    // std::cerr << file << std::endl;
    auto map = OpenDriveParser::Load(util::OpenDrive::Load(file));
   // 使用 ASSERT_TRUE 断言来确保 map 不是 nullptr
    ASSERT_TRUE(map);
    // print_roads(map, file);
  }
}

TEST(road, parse_road_links) {
  for (const auto &file : util::OpenDrive::GetAvailableFiles()) {
    // std::cerr << file << std::endl;
    auto map = OpenDriveParser::Load(util::OpenDrive::Load(file));
    ASSERT_TRUE(map);
    test_road_links(map);
  }
}

TEST(road, parse_junctions) {
  for (const auto& file : util::OpenDrive::GetAvailableFiles()) {
    auto map = OpenDriveParser::Load(util::OpenDrive::Load(file));
    ASSERT_TRUE(map.has_value());

    const std::string full_path = BASE_PATH + file;
    pugi::xml_document xml;

    pugi::xml_parse_result result = xml.load_file( full_path.c_str());
    ASSERT_TRUE(result);

    test_junctions(xml, map);
  }

}

TEST(road, parse_road) {
  for (const auto& file : util::OpenDrive::GetAvailableFiles()) {
    auto map = OpenDriveParser::Load(util::OpenDrive::Load(file));
    ASSERT_TRUE(map.has_value());

    const std::string full_path = BASE_PATH + file;
    pugi::xml_document xml;
    pugi::xml_parse_result result = xml.load_file( full_path.c_str());
    ASSERT_TRUE(result);

    test_roads(xml, map);
  }

}

TEST(road, parse_road_elevation) {
  for (const auto& file : util::OpenDrive::GetAvailableFiles()) {
    auto map = OpenDriveParser::Load(util::OpenDrive::Load(file));
    ASSERT_TRUE(map.has_value());

    const std::string full_path = BASE_PATH + file;
    pugi::xml_document xml;
    pugi::xml_parse_result result = xml.load_file( full_path.c_str());
    ASSERT_TRUE(result);

    test_road_elevation(xml, map);
  }

}

TEST(road, parse_geometry) {
  for (const auto& file : util::OpenDrive::GetAvailableFiles()) {
    auto map = OpenDriveParser::Load(util::OpenDrive::Load(file));
    ASSERT_TRUE(map.has_value());

    const std::string full_path = BASE_PATH + file;
    pugi::xml_document xml;
    pugi::xml_parse_result result = xml.load_file( full_path.c_str());
    ASSERT_TRUE(result);

    test_geometry(xml, map);
  }

}

TEST(road, iterate_waypoints) {
  // 创建一个线程池
  carla::ThreadPool pool;
  pool.AsyncRun();
  // 用于存储异步任务结果的向量
  std::vector<std::future<void>> results;
  // 遍历所有可用的OpenDrive文件
  for (const auto& file : util::OpenDrive::GetAvailableFiles()) {
    carla::logging::log("Parsing", file);// 日志记录开始解析的文件名
     // 向线程池提交一个异步任务，任务内容是解析和验证地图
    results.push_back(pool.Post([file]() {
      carla::StopWatch stop_watch;  // 创建一个计时器，用于测量解析和验证地图所需的时间
      auto m = OpenDriveParser::Load(util::OpenDrive::Load(file)); // 加载OpenDrive文件并解析为地图
      ASSERT_TRUE(m.has_value()); // 断言解析成功，m不为空
      auto &map = *m;
      // 生成地图的拓扑结构，并断言拓扑不为空
      const auto topology = map.GenerateTopology();
      ASSERT_FALSE(topology.empty());
      auto count = 0u;
      // 生成地图的轨迹点（waypoints），每个轨迹点间隔0.5米
      auto waypoints = map.GenerateWaypoints(0.5);
      ASSERT_FALSE(waypoints.empty());
      // 随机打乱轨迹点顺序
      Random::Shuffle(waypoints);
      // 确定要探索的轨迹点数量，最多2000个
      const auto number_of_waypoints_to_explore =
          std::min<size_t>(2000u, waypoints.size());
       // 遍历选定的轨迹点进行探索
      for (auto i = 0u; i < number_of_waypoints_to_explore; ++i) {
        auto wp = waypoints[i];
       // 计算轨迹点的变换
        map.ComputeTransform(wp);
       // 对于非第一个轨迹点，断言它与第一个轨迹点不同
        if (i != 0u) {
          ASSERT_NE(wp, waypoints[0u]);
        }
        // 遍历当前轨迹点的所有后继轨迹点
        for (auto &&successor : map.GetSuccessors(wp)) {
          ASSERT_TRUE(
              successor.road_id != wp.road_id ||
              successor.section_id != wp.section_id ||
              successor.lane_id != wp.lane_id ||
              successor.s != wp.s);
        }
        auto origin = wp;
        // 从当前轨迹点出发，探索最多200次后续轨迹点
        for (auto j = 0u; j < 200u; ++j) {
          // 获取从当前轨迹点出发，在0.0001到150米范围内的后续轨迹点
          auto next_wps = map.GetNext(origin, Random::Uniform(0.0001, 150.0));
          if (next_wps.empty()) {
            break;
          }
          // 确定要探索的后续轨迹点数量，最多10个
          const auto number_of_next_wps_to_explore =
              std::min<size_t>(10u, next_wps.size());
          // 随机打乱后续轨迹点顺序
          Random::Shuffle(next_wps);
          // 遍历选定的后续轨迹点进行探索
          for (auto k = 0u; k < number_of_next_wps_to_explore; ++k) {
            auto next = next_wps[k];
            ++count;
            // 断言后续轨迹点与当前轨迹点至少有一个属性不同
            ASSERT_TRUE(
                next.road_id != wp.road_id ||
                next.section_id != wp.section_id ||
                next.lane_id != wp.lane_id ||
                next.s != wp.s);
            // 获取当前后续轨迹点的右侧轨迹点
            auto right = map.GetRight(next);
            if (right.has_value()) {
              ASSERT_EQ(right->road_id, next.road_id);
              ASSERT_EQ(right->section_id, next.section_id);
              ASSERT_NE(right->lane_id, next.lane_id);
              ASSERT_EQ(right->s, next.s);
            }
             // 获取当前后续轨迹点的左侧轨迹点
            auto left = map.GetLeft(next);
            if (left.has_value()) {
              // 断言左侧轨迹点与当前后续轨迹点在同一道路和路段，但车道不同
              ASSERT_EQ(left->road_id, next.road_id);
              ASSERT_EQ(left->section_id, next.section_id);
              ASSERT_NE(left->lane_id, next.lane_id);
              ASSERT_EQ(left->s, next.s);
            }
          }
          origin = next_wps[0u];  // 将下一个探索的起点设置为当前探索的后续轨迹点中的第一个
        }
      }
      ASSERT_GT(count, 0u);// 断言至少探索了一个轨迹点
      float seconds = 1e-3f * stop_watch.GetElapsedTime();   // 获取解析和验证地图所需的时间，并记录日志
      carla::logging::log(file, "done in", seconds, "seconds.");
    }));
  }
  // 等待所有异步任务完成
  for (auto &result : results) {
    result.get();
  }
}

TEST(road, get_waypoint) {
  // 创建一个线程池
  carla::ThreadPool pool;
  // 启动线程池中的异步任务执行
  pool.AsyncRun();
  // 创建一个容器来存储异步任务的返回值
  std::vector<std::future<void>> results;
  // 遍历所有可用的OpenDrive文件 
  for (const auto& file : util::OpenDrive::GetAvailableFiles()) {
    carla::logging::log("Parsing", file);
    results.push_back(pool.Post([file]() {
      // 创建一个计时器，用于测量任务执行时间
      carla::StopWatch stop_watch;
      // 使用OpenDriveParser加载OpenDrive文件
      auto m = OpenDriveParser::Load(util::OpenDrive::Load(file));
      ASSERT_TRUE(m.has_value());// 确保地图被成功加载
      auto &map = *m;// 获取地图的引用
      // 进行10000次随机位置测试
      for (auto i = 0u; i < 10'000u; ++i) {
        const auto location = Random::Location(-500.0f, 500.0f);  // 在指定的范围内生成一个随机位置
        auto owp = map.GetClosestWaypointOnRoad(location); // 在地图上找到离该位置最近的道路点
        ASSERT_TRUE(owp.has_value());
        auto &wp = *owp;// 获取道路点的引用
        // 获取当前道路点的下一个道路点
        for (auto &next : map.GetNext(wp, 0.5)) {
          ASSERT_TRUE(
              next.road_id != wp.road_id ||
              next.section_id != wp.section_id ||
              next.lane_id != wp.lane_id ||
              next.s != wp.s);
        }
        // 获取当前道路点的左侧相邻道路点
        auto left = map.GetLeft(wp);
        if (left.has_value()) {
          ASSERT_EQ(left->road_id, wp.road_id);
          ASSERT_EQ(left->section_id, wp.section_id);
          ASSERT_NE(left->lane_id, wp.lane_id);
          ASSERT_EQ(left->s, wp.s);
        }
        // 获取当前道路点的右侧相邻道路点
        auto right = map.GetRight(wp);
        if (right.has_value()) {
          ASSERT_EQ(right->road_id, wp.road_id);
          ASSERT_EQ(right->section_id, wp.section_id);
          ASSERT_NE(right->lane_id, wp.lane_id);
          ASSERT_EQ(right->s, wp.s);
        }
      }
      // 计算并记录任务执行时间
      float seconds = 1e-3f * stop_watch.GetElapsedTime();
      carla::logging::log(file, "done in", seconds, "seconds.");
    }));
  }
    // 等待所有异步任务完成
  for (auto &result : results) {
    result.get();
  }
}
