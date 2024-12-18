  // Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#define _USE_MATH_DEFINES // 避免 M_PI 未定义错误（Visual Studio 2015 和 2017 中的错误）

#include <cmath>

#include "carla/Logging.h"
#include "carla/nav/Navigation.h"
#include "carla/nav/WalkerManager.h"
#include "carla/geom/Math.h"

#include <iterator>
#include <fstream>
#include <mutex>

namespace carla {
namespace nav {

  enum UpdateFlags {
    DT_CROWD_ANTICIPATE_TURNS   = 1,
    DT_CROWD_OBSTACLE_AVOIDANCE = 2,
    DT_CROWD_SEPARATION         = 4,
    DT_CROWD_OPTIMIZE_VIS       = 8,
    DT_CROWD_OPTIMIZE_TOPO      = 16
  };

  // 这些设置与 RecastBuilder 中的设置相同，因此如果您更改代理的高度，则应该在 RecastBuilder 中执行相同的操作
  static const int   MAX_POLYS = 256;
  static const int   MAX_AGENTS = 500;
  static const int   MAX_QUERY_SEARCH_NODES = 2048;
  static const float AGENT_HEIGHT = 1.8f;
  static const float AGENT_RADIUS = 0.3f;

  static const float AGENT_UNBLOCK_DISTANCE = 0.5f;
  static const float AGENT_UNBLOCK_DISTANCE_SQUARED = AGENT_UNBLOCK_DISTANCE * AGENT_UNBLOCK_DISTANCE;
  static const float AGENT_UNBLOCK_TIME = 4.0f;

  static const float AREA_GRASS_COST =  1.0f;
  static const float AREA_ROAD_COST  = 10.0f;

  // 返回一个随机的浮点数 float
  static float frand() {
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
  }

  Navigation::Navigation() {
    // 指定行人管理器
    _walker_manager.SetNav(this);
  }

  Navigation::~Navigation() { // 析构函数
    _ready = false;// 在析构函数中，首先将_ready标志设置为false，表示导航系统不再可用。
    _time_to_unblock = 0.0f;  //将_time_to_unblock设置为0.0f，表示没有阻塞时间。
    _mapped_walkers_id.clear();// 清除所有映射的行人ID。
    _mapped_vehicles_id.clear();// 清除所有映射的车辆ID。
    _mapped_by_index.clear();// 清除所有按索引映射的数据。
    _walkers_blocked_position.clear();// 清除所有阻塞行人的位置
    _yaw_walkers.clear();// 清除所有行人的偏航数据。
    _binary_mesh.clear();// 清除二进制网格数据。
    dtFreeCrowd(_crowd);// 释放Recast导航中的Crowd管理器资源。
    dtFreeNavMeshQuery(_nav_query);// 释放导航网格查询资源。
    dtFreeNavMesh(_nav_mesh);// 释放导航网格资源。
  }

  // 参考模拟器访问API函数
  void Navigation::SetSimulator(std::weak_ptr<carla::client::detail::Simulator> simulator)
  {
    _simulator = simulator;
    _walker_manager.SetSimulator(simulator);// 将simulator弱引用传递给_walker_manager，以便行人管理器可以使用模拟器API。
  }

  // 设置要使用的随机数种子
  void Navigation::SetSeed(unsigned int seed) {
    srand(seed);
  }

  // 加载导航数据
  bool Navigation::Load(const std::string &filename) {
    std::ifstream f;// 创建一个文件输入流对象f。
    std::istream_iterator<uint8_t> start(f), end;// 创建输入流迭代器，用于遍历文件内容。

    // 读取整个文件
    f.open(filename, std::ios::binary);// 以二进制模式打开文件。
    if (!f.is_open()) {
      return false;// 如果文件未能成功打开，返回false。
    }
    std::vector<uint8_t> content(start, end);// 读取文件内容到content向量中
    f.close();

    // 解析内容
    return Load(std::move(content));
  }

  // 从内存中加载导航数据
  bool Navigation::Load(std::vector<uint8_t> content) {
    const int NAVMESHSET_MAGIC = 'M' << 24 | 'S' << 16 | 'E' << 8 | 'T'; // 'MSET';// 定义导航网格集合的魔术数字。
    const int NAVMESHSET_VERSION = 1;//  定义导航网格集合的版本号。
#pragma pack(push, 1)// 确保结构体按照1字节对齐。

    // 导航网格集合头的结构体
    struct NavMeshSetHeader {
      int magic;       // 魔术
      int version;     // 版本
      int num_tiles;   // 瓦片数
      dtNavMeshParams params;
    } header;
    // 导航网格瓦片头的结构体
    struct NavMeshTileHeader {
      dtTileRef tile_ref;
      int data_size;        // 数据大小
    };
#pragma pack(pop)//恢复默认的对齐设置。

    // 检查 导航网格集合头的结构体大小
    // 如果内存中导航数据 都小于 头的大小，则报错
    if (content.size() < sizeof(header)) {
      logging::log("Nav: failed loading binary");
      return false;
    }

    // 读取文件的头
    unsigned long pos = 0;// 定义一个位置变量pos，用于跟踪读取的位置。
    memcpy(&header, &content[pos], sizeof(header));// 从content中复制头信息到header结构体。
    pos += sizeof(header);// 更新pos，跳过已读取的头信息。

    // 检查文件的魔术和版本
    if (header.magic != NAVMESHSET_MAGIC || header.version != NAVMESHSET_VERSION) {
      return false;
    }

    // 分配导航网格对象的内存
    dtNavMesh *mesh = dtAllocNavMesh();
    if (!mesh) {
      return false;
    }

    // 设置瓦片的数目和原点
    dtStatus status = mesh->init(&header.params);
    if (dtStatusFailed(status)) {
      return false;
    }

    // 读取瓦片数据
    for (int i = 0; i < header.num_tiles; ++i) {
      NavMeshTileHeader tile_header;

      // 读取瓦片头
      memcpy(&tile_header, &content[pos], sizeof(tile_header));// 从content中复制瓦片头信息到tile_header结构体
      pos += sizeof(tile_header);// 更新pos变量，跳过已读取的瓦片头信息
      if (pos >= content.size()) {// 检查当前读取位置是否已经到达或超过了内容的末尾
        dtFreeNavMesh(mesh);
        return false;// 如果超出范围，释放之前分配的导航网格资源，返回false，表示加载过程中出现问题
      }

      // 检查瓦片的有效性
      if (!tile_header.tile_ref || !tile_header.data_size) {
        break;
      }

      // 分配缓冲区内存
      char *data = static_cast<char *>(dtAlloc(static_cast<size_t>(tile_header.data_size), DT_ALLOC_PERM));
      if (!data) {
        break;
      }

      // 读取瓦片
      memcpy(data, &content[pos], static_cast<size_t>(tile_header.data_size));// 复制瓦片数据到内存
      pos += static_cast<unsigned long>(tile_header.data_size);// 更新读取位置
      if (pos > content.size()) {//  检查更新后的读取位置`pos`是否超过了`content`数组的大小。
        dtFree(data);
        dtFreeNavMesh(mesh);
        return false;
      }

      // 添加瓦片数据
      mesh->addTile(reinterpret_cast<unsigned char *>(data), tile_header.data_size, DT_TILE_FREE_DATA,
      tile_header.tile_ref, 0);
    }

    // 交换
    dtFreeNavMesh(_nav_mesh);
    _nav_mesh = mesh;

    // 准备查询对象
    dtFreeNavMeshQuery(_nav_query);
    _nav_query = dtAllocNavMeshQuery();
    _nav_query->init(_nav_mesh, MAX_QUERY_SEARCH_NODES);

    // 拷贝
    _binary_mesh = std::move(content);
    _ready = true;

    // 创建并初始化人群管理器
    CreateCrowd();

    return true;
  }

  void Navigation::CreateCrowd(void) {

    // 检查是否一切就绪
    if (!_ready) {
      return;
    }

    DEBUG_ASSERT(_crowd == nullptr);

    // 创建并初始化
    _crowd = dtAllocCrowd();
    // 这些半径应该是车辆的最大尺寸 (CarlaCola for Carla)
    const float max_agent_radius = AGENT_RADIUS * 20;
    if (!_crowd->init(MAX_AGENTS, max_agent_radius, _nav_mesh)) {
      logging::log("Nav: failed to create crowd");
      return;
    }

    // 设置不同的过滤器
    // 过滤器 0 不能在道路上行走
    _crowd->getEditableFilter(0)->setIncludeFlags(CARLA_TYPE_WALKABLE);
    _crowd->getEditableFilter(0)->setExcludeFlags(CARLA_TYPE_ROAD);
    _crowd->getEditableFilter(0)->setAreaCost(CARLA_AREA_ROAD, AREA_ROAD_COST);
    _crowd->getEditableFilter(0)->setAreaCost(CARLA_AREA_GRASS, AREA_GRASS_COST);
    // 过滤器 1 可以在道路上行走
    _crowd->getEditableFilter(1)->setIncludeFlags(CARLA_TYPE_WALKABLE);
    _crowd->getEditableFilter(1)->setExcludeFlags(CARLA_TYPE_NONE);
    _crowd->getEditableFilter(1)->setAreaCost(CARLA_AREA_ROAD, AREA_ROAD_COST);
    _crowd->getEditableFilter(1)->setAreaCost(CARLA_AREA_GRASS, AREA_GRASS_COST);

    // 设置不同品质的局部避让参数。
    dtObstacleAvoidanceParams params;
    // 主要使用默认设置，从 dtCrowd 复制。
    memcpy(&params, _crowd->getObstacleAvoidanceParams(0), sizeof(dtObstacleAvoidanceParams));

    // 低等级避让Low (11)
    params.velBias = 0.5f;// 设置速度偏差参数，影响行人在避让障碍物时的速度调整。值为0.5表示行人在避让时会适度调整速度。
    params.adaptiveDivs = 5;// 设置自适应分割数，影响避让行为的平滑程度。值为5表示在计算避让路径时会进行5次自适应分割。
    params.adaptiveRings = 2;// 设置自适应环的数量，影响避让行为的预测范围。值为2表示避让时会考虑周围两个环的障碍物。
    params.adaptiveDepth = 1;// 设置自适应深度，影响避让行为的预测深度。值为1表示避让时会预测1步的避让路径。
    _crowd->setObstacleAvoidanceParams(0, &params);// 应用于crowd管理器的第0个避让等级。

    // 中等级避让Medium (22)
    params.velBias = 0.5f;
    params.adaptiveDivs = 5;
    params.adaptiveRings = 2;
    params.adaptiveDepth = 2;
    _crowd->setObstacleAvoidanceParams(1, &params);// 应用于crowd管理器的第1个避让等级。

    // Good (45)
    params.velBias = 0.5f;
    params.adaptiveDivs = 7;
    params.adaptiveRings = 2;
    params.adaptiveDepth = 3;
    _crowd->setObstacleAvoidanceParams(2, &params);

    // High (66)
    params.velBias = 0.5f;
    params.adaptiveDivs = 7;
    params.adaptiveRings = 3;
    params.adaptiveDepth = 3;

    _crowd->setObstacleAvoidanceParams(3, &params);
  }

  // 返回从一个位置到另一个位置的路径点
  bool Navigation::GetPath(carla::geom::Location from,
                           carla::geom::Location to,
                           dtQueryFilter * filter,
                           std::vector<carla::geom::Location> &path,
                           std::vector<unsigned char> &area) {
    // 找到路径
    float straight_path[MAX_POLYS * 3];// 定义一个浮点数数组`straight_path`，用于存储直线路径上的点坐标。
    unsigned char straight_path_flags[MAX_POLYS];// 定义路径标志数组
    dtPolyRef straight_path_polys[MAX_POLYS];// 定义路径多边形引用数组
    int num_straight_path;// 定义路径点数量变量
    int straight_path_options = DT_STRAIGHTPATH_AREA_CROSSINGS;// 设置路径查找选项

    // 路径中的多边形
    dtPolyRef polys[MAX_POLYS];
    int num_polys;

    // 检查是否一切就绪
    if (!_ready) {
      return false;
    }

    DEBUG_ASSERT(_nav_query != nullptr);

    // 点的延伸
    float poly_pick_ext[3];// // 定义一个名为poly_pick_ext的浮点型数组，该数组包含3个元素。
    poly_pick_ext[0] = 2;
    poly_pick_ext[1] = 4;
    poly_pick_ext[2] = 2;

    // 筛选
    dtQueryFilter filter2;
    if (filter == nullptr) {
      filter2.setAreaCost(CARLA_AREA_ROAD, AREA_ROAD_COST);
      filter2.setAreaCost(CARLA_AREA_GRASS, AREA_GRASS_COST);
      filter2.setIncludeFlags(CARLA_TYPE_WALKABLE);
      filter2.setExcludeFlags(CARLA_TYPE_NONE);
      filter = &filter2;
    }

    // 设置点
    dtPolyRef start_ref = 0;
    dtPolyRef end_ref = 0;
    float start_pos[3] = { from.x, from.z, from.y };
    float end_pos[3] = { to.x, to.z, to.y };
    {
      // 关键部分，强制单线程运行这里
      std::lock_guard<std::mutex> lock(_mutex);
      _nav_query->findNearestPoly(start_pos, poly_pick_ext, filter, &start_ref, 0);
      _nav_query->findNearestPoly(end_pos, poly_pick_ext, filter, &end_ref, 0);
    }
    if (!start_ref || !end_ref) {
      return false;
    }

    {
      // 关键部分，强制单线程运行这里
      std::lock_guard<std::mutex> lock(_mutex);
      // 获取节点的路径
      _nav_query->findPath(start_ref, end_ref, start_pos, end_pos, filter, polys, &num_polys, MAX_POLYS);
    }

    // 获得点的路径
    num_straight_path = 0;
    if (num_polys == 0) {
      return false;
    }

    // 如果是部分路径，请确保终点与最后一个多边形相接
    float end_pos2[3];
    dtVcopy(end_pos2, end_pos);
    if (polys[num_polys - 1] != end_ref) {
      // 关键部分，强制单线程运行这里
      std::lock_guard<std::mutex> lock(_mutex);
      _nav_query->closestPointOnPoly(polys[num_polys - 1], end_pos, end_pos2, 0);
    }

    // 获得点
    {
      // 关键部分，强制单线程运行这里
      std::lock_guard<std::mutex> lock(_mutex);
      _nav_query->findStraightPath(start_pos, end_pos2, polys, num_polys,
      straight_path, straight_path_flags,
      straight_path_polys, &num_straight_path, MAX_POLYS, straight_path_options);
    }

    // 将路径复制到输出缓冲区
    path.clear();
    path.reserve(static_cast<unsigned long>(num_straight_path));
    unsigned char area_type;
    for (int i = 0, j = 0; j < num_straight_path; i += 3, ++j) {
      // 保存虚幻轴的坐标（x，z，y）
      path.emplace_back(straight_path[i], straight_path[i + 2], straight_path[i + 1]);
      // 保存区域类型
      {
        // 关键部分，强制单线程运行这里
        std::lock_guard<std::mutex> lock(_mutex);
        _nav_mesh->getPolyArea(straight_path_polys[j], &area_type);
      }
      area.emplace_back(area_type);
    }

    return true;
  }

  bool Navigation::GetAgentRoute(ActorId id, carla::geom::Location from, carla::geom::Location to,
  std::vector<carla::geom::Location> &path, std::vector<unsigned char> &area) {
    // 找到路径
    float straight_path[MAX_POLYS * 3];
    unsigned char straight_path_flags[MAX_POLYS];
    dtPolyRef straight_path_polys[MAX_POLYS];
    int num_straight_path = 0;
    int straight_path_options = DT_STRAIGHTPATH_AREA_CROSSINGS;

    // 路径中的多边形
    dtPolyRef polys[MAX_POLYS];
    int num_polys;

    // 检查是否一切就绪
    if (!_ready) {
      return false;
    }

    DEBUG_ASSERT(_nav_query != nullptr);

    // 点的扩展
    float poly_pick_ext[3] = {2,4,2};

    // 从代理获取当前过滤器
    auto it = _mapped_walkers_id.find(id);
    if (it == _mapped_walkers_id.end())
      return false;

    const dtQueryFilter *filter;
    {
      // 关键部分，强制单线程运行这里
      std::lock_guard<std::mutex> lock(_mutex);
      filter = _crowd->getFilter(_crowd->getAgent(it->second)->params.queryFilterType);
    }

    // 设置点
    dtPolyRef start_ref = 0;
    dtPolyRef end_ref = 0;
    float start_pos[3] = { from.x, from.z, from.y };
    float end_pos[3] = { to.x, to.z, to.y };
    {
      // 关键部分，强制单线程运行这里
      std::lock_guard<std::mutex> lock(_mutex);
      _nav_query->findNearestPoly(start_pos, poly_pick_ext, filter, &start_ref, 0);
      _nav_query->findNearestPoly(end_pos, poly_pick_ext, filter, &end_ref, 0);
    }
    if (!start_ref || !end_ref) {
      return false;
    }

    // 获取点的路径
    {
      // 关键部分，强制单线程运行这里
      std::lock_guard<std::mutex> lock(_mutex);
      _nav_query->findPath(start_ref, end_ref, start_pos, end_pos, filter, polys, &num_polys, MAX_POLYS);
    }

    // 获取点的路径
    if (num_polys == 0) {
      return false;
    }

    // 如果是部分路径，请确保终点与最后一个多边形相接
    float end_pos2[3];
    dtVcopy(end_pos2, end_pos);
    if (polys[num_polys - 1] != end_ref) {
      // 关键部分，强制单线程运行这里
      std::lock_guard<std::mutex> lock(_mutex);
      _nav_query->closestPointOnPoly(polys[num_polys - 1], end_pos, end_pos2, 0);
    }

    // 获取点
    {
      // 关键部分，强制单线程运行这里
      std::lock_guard<std::mutex> lock(_mutex);
      _nav_query->findStraightPath(start_pos, end_pos2, polys, num_polys,
      straight_path, straight_path_flags,
      straight_path_polys, &num_straight_path, MAX_POLYS, straight_path_options);
    }

    // 将路径复制到输出缓冲区
    path.clear();
    path.reserve(static_cast<unsigned long>(num_straight_path));
    unsigned char area_type;
    for (int i = 0, j = 0; j < num_straight_path; i += 3, ++j) {
      // 保存虚幻轴的坐标（x，z，y）
      path.emplace_back(straight_path[i], straight_path[i + 2], straight_path[i + 1]);
      // 保存面积类型
      {
        // 关键部分，强制单线程运行这里
        std::lock_guard<std::mutex> lock(_mutex);
        _nav_mesh->getPolyArea(straight_path_polys[j], &area_type);
      }
      area.emplace_back(area_type);
    }

    return true;
  }

  // 在人群中创造新的行人
  bool Navigation::AddWalker(ActorId id, carla::geom::Location from) {
    dtCrowdAgentParams params;

    // 检查是否一切就绪
    if (!_ready) {
      return false;
    }

    DEBUG_ASSERT(_crowd != nullptr);

    // 设置参数
    memset(&params, 0, sizeof(params));
    params.radius = AGENT_RADIUS;
    params.height = AGENT_HEIGHT;
    params.maxAcceleration = 160.0f;
    params.maxSpeed = 1.47f;
    params.collisionQueryRange = 10;
    params.obstacleAvoidanceType = 3;
    params.separationWeight = 0.5f;
    
    // 设置代理是否可以过马路
    if (frand() <= _probability_crossing) {
      params.queryFilterType = 1;
    } else {
      params.queryFilterType = 0;
    }

    // flags
    params.updateFlags = 0;
    params.updateFlags |= DT_CROWD_ANTICIPATE_TURNS;
    params.updateFlags |= DT_CROWD_OBSTACLE_AVOIDANCE;
    params.updateFlags |= DT_CROWD_SEPARATION;

    // 来自虚幻坐标（减去一半高度以将枢轴从中心（虚幻）移动到底部（recast））
    float point_from[3] = { from.x, from.z - (AGENT_HEIGHT / 2.0f), from.y };
    // 添加行人
    int index;
    {
      // 关键部分，强制单线程运行这里
      std::lock_guard<std::mutex> lock(_mutex);
      index = _crowd->addAgent(point_from, &params);
      if (index == -1) {
        return false;
      }
    }

    // 保存 id
    _mapped_walkers_id[id] = index;
    _mapped_by_index[index] = id;

    // 初始化偏航角
    _yaw_walkers[id] = 0.0f;

    // 添加行人进行路线规划
    _walker_manager.AddWalker(id);

    return true;
  }

  // 在人群中创造一种新的车辆，以便行人避开
  bool Navigation::AddOrUpdateVehicle(VehicleCollisionInfo &vehicle) {
    namespace cg = carla::geom;
    dtCrowdAgentParams params;

    // 检查是否一切就绪
    if (!_ready) {
      return false;
    }

    DEBUG_ASSERT(_crowd != nullptr);

    // 获取边界框扩展以及周围的一些空间
    float marge = 0.8f;
    float hx = vehicle.bounding.extent.x + marge;
    float hy = vehicle.bounding.extent.y + marge;
    // 定义边界框的 4 个角
    cg::Vector3D box_corner1 {-hx, -hy, 0};
    cg::Vector3D box_corner2 { hx + 0.2f, -hy, 0};
    cg::Vector3D box_corner3 { hx + 0.2f,  hy, 0};
    cg::Vector3D box_corner4 {-hx,  hy, 0};
    // 旋转点
    float angle = cg::Math::ToRadians(vehicle.transform.rotation.yaw);
    box_corner1 = cg::Math::RotatePointOnOrigin2D(box_corner1, angle);
    box_corner2 = cg::Math::RotatePointOnOrigin2D(box_corner2, angle);
    box_corner3 = cg::Math::RotatePointOnOrigin2D(box_corner3, angle);
    box_corner4 = cg::Math::RotatePointOnOrigin2D(box_corner4, angle);
    // 转换为世界位置
    box_corner1 += vehicle.transform.location;
    box_corner2 += vehicle.transform.location;
    box_corner3 += vehicle.transform.location;
    box_corner4 += vehicle.transform.location;

    // 检查该参与者是否存在
    auto it = _mapped_vehicles_id.find(vehicle.id);
    if (it != _mapped_vehicles_id.end()) {
      // 获得找到的索引
      int index = it->second;
      if (index != -1) {
        // 获得智能体
        dtCrowdAgent *agent;
        {
          // 关键部分，强制单线程运行这里
          std::lock_guard<std::mutex> lock(_mutex);
          agent = _crowd->getEditableAgent(index);
        }
        if (agent) {
          // 更新它的位置
          agent->npos[0] = vehicle.transform.location.x;
          agent->npos[1] = vehicle.transform.location.z;
          agent->npos[2] = vehicle.transform.location.y;
          // 更新其朝向的边界框
          agent->params.obb[0]  = box_corner1.x;
          agent->params.obb[1]  = box_corner1.z;
          agent->params.obb[2]  = box_corner1.y;
          agent->params.obb[3]  = box_corner2.x;
          agent->params.obb[4]  = box_corner2.z;
          agent->params.obb[5]  = box_corner2.y;
          agent->params.obb[6]  = box_corner3.x;
          agent->params.obb[7]  = box_corner3.z;
          agent->params.obb[8]  = box_corner3.y;
          agent->params.obb[9]  = box_corner4.x;
          agent->params.obb[10] = box_corner4.z;
          agent->params.obb[11] = box_corner4.y;
        }
        return true;
      }
    }

    // 设置参数
    memset(&params, 0, sizeof(params));
    params.radius = 2;
    params.height = AGENT_HEIGHT;
    params.maxAcceleration = 0.0f;
    params.maxSpeed = 1.47f;
    params.collisionQueryRange = 0;
    params.obstacleAvoidanceType = 0;
    params.separationWeight = 100.0f;

    // 标志
    params.updateFlags = 0;
    params.updateFlags |= DT_CROWD_SEPARATION;

    // 更新其朝向的边界框
    // 数据: [x][y][z] [x][y][z] [x][y][z] [x][y][z]
    params.useObb = true;
    params.obb[0]  = box_corner1.x;
    params.obb[1]  = box_corner1.z;
    params.obb[2]  = box_corner1.y;
    params.obb[3]  = box_corner2.x;
    params.obb[4]  = box_corner2.z;
    params.obb[5]  = box_corner2.y;
    params.obb[6]  = box_corner3.x;
    params.obb[7]  = box_corner3.z;
    params.obb[8]  = box_corner3.y;
    params.obb[9]  = box_corner4.x;
    params.obb[10] = box_corner4.z;
    params.obb[11] = box_corner4.y;

    // 从虚幻坐标（垂直为 Z）到 Recast 坐标（垂直为 Y，右手坐标系）
    float point_from[3] = { vehicle.transform.location.x,
                            vehicle.transform.location.z,
                            vehicle.transform.location.y };

    // 添加行人
    int index;
    {
      // 关键部分，强制单线程运行这里
      std::lock_guard<std::mutex> lock(_mutex);
      index = _crowd->addAgent(point_from, &params);
      if (index == -1) {
        logging::log("Vehicle agent not added to the crowd by some problem!");
        return false;
      }

      // 标记为有效
      dtCrowdAgent *agent = _crowd->getEditableAgent(index);
      if (agent) {
        agent->state = DT_CROWDAGENT_STATE_WALKING;
      }
    }

    // 保存 id
    _mapped_vehicles_id[vehicle.id] = index;
    _mapped_by_index[index] = vehicle.id;

    return true;
  }

  // 移除代理
  bool Navigation::RemoveAgent(ActorId id) {

    // 检查是否一切就绪
    if (!_ready) {
      return false;
    }

    DEBUG_ASSERT(_crowd != nullptr);

    // 获取内部行人索引
    auto it = _mapped_walkers_id.find(id);
    if (it != _mapped_walkers_id.end()) {
      // 从人群中移除
      {
        // 关键部分，强制单线程运行这里
        std::lock_guard<std::mutex> lock(_mutex);
        _crowd->removeAgent(it->second);
      }
      _walker_manager.RemoveWalker(id);
      // remove from mapping
      _mapped_walkers_id.erase(it);
      _mapped_by_index.erase(it->second);

      return true;
    }

    // get the internal vehicle index
    it = _mapped_vehicles_id.find(id);
    if (it != _mapped_vehicles_id.end()) {
      // 从人群中移除
      {
        // 关键部分，强制单线程运行这里
        std::lock_guard<std::mutex> lock(_mutex);
        _crowd->removeAgent(it->second);
      }
      // 从映射中移除
      _mapped_vehicles_id.erase(it);
      _mapped_by_index.erase(it->second);

      return true;
    }

    return false;
  }

  // 在人群中添加/更新/删除车辆
  bool Navigation::UpdateVehicles(std::vector<VehicleCollisionInfo> vehicles) {
    std::unordered_set<carla::rpc::ActorId> updated;

    // 添加所有当前已映射的车辆
    for (auto &&entry : _mapped_vehicles_id) {
      updated.insert(entry.first);
    }

    // 添加所有车辆（如果已经存在，则仅更新）
    for (auto &&entry : vehicles) {
      // 尝试添加或更新车辆
      AddOrUpdateVehicle(entry);
      // 标记为已更新（以避免在此帧中删除它）
      updated.erase(entry.id);
    }

    // 删除所有未更新的车辆（它们不存在于此帧中）
    for (auto &&entry : updated) {
      // 删除未更新的代理
      RemoveAgent(entry);
    }

    return true;
  }

  // 设置新的最大速度
  bool Navigation::SetWalkerMaxSpeed(ActorId id, float max_speed) {

    // 检查是否一切就绪
    if (!_ready) {
      return false;
    }

    DEBUG_ASSERT(_crowd != nullptr);

    // 获取内部索引
    auto it = _mapped_walkers_id.find(id);
    if (it == _mapped_walkers_id.end()) {
      return false;
    }

    // 获得智能体
    {
      // 关键部分，强制单线程运行这里
      std::lock_guard<std::mutex> lock(_mutex);
      dtCrowdAgent *agent = _crowd->getEditableAgent(it->second);
      if (agent) {
        agent->params.maxSpeed = max_speed;
        return true;
      }
    }

    return false;
  }

  // 设定新的目标点
  bool Navigation::SetWalkerTarget(ActorId id, carla::geom::Location to) {

    // 检查是否一切就绪
    if (!_ready) {
      return false;
    }

    // 获取内部索引
    auto it = _mapped_walkers_id.find(id);
    if (it == _mapped_walkers_id.end()) {
      return false;
    }

    return _walker_manager.SetWalkerRoute(id, to);
  }

  // 设置新的目标点，直接前往没有事件发生的地方
  bool Navigation::SetWalkerDirectTarget(ActorId id, carla::geom::Location to) {

    // 检查是否一切就绪
    if (!_ready) {
      return false;
    }

    // 获取内部索引
    auto it = _mapped_walkers_id.find(id);
    if (it == _mapped_walkers_id.end()) {
      return false;
    }

    return SetWalkerDirectTargetIndex(it->second, to);
  }

  // 设置新的目标点，直接前往没有事件发生的地方
  bool Navigation::SetWalkerDirectTargetIndex(int index, carla::geom::Location to) {

    // 检查是否一切就绪
    if (!_ready) {
      return false;
    }

    DEBUG_ASSERT(_crowd != nullptr);
    DEBUG_ASSERT(_nav_query != nullptr);

    if (index == -1) {
      return false;
    }

    // 设定目标位置
    float point_to[3] = { to.x, to.z, to.y };
    float nearest[3];
    bool res;
    {
      // 关键部分，强制单线程运行这里
      std::lock_guard<std::mutex> lock(_mutex);
      const dtQueryFilter *filter = _crowd->getFilter(0);
      dtPolyRef target_ref;
      _nav_query->findNearestPoly(point_to, _crowd->getQueryHalfExtents(), filter, &target_ref, nearest);
      if (!target_ref) {
        return false;
      }

      res = _crowd->requestMoveTarget(index, target_ref, point_to);
    }

    return res;
  }

  // 更新人群中的所有行人
  void Navigation::UpdateCrowd(const client::detail::EpisodeState &state) {

    // 检查是否一切就绪
    if (!_ready) {
      return;
    }

    DEBUG_ASSERT(_crowd != nullptr);

    // 更新人群代理
    _delta_seconds = state.GetTimestamp().delta_seconds;
    {
      // 关键部分，强制单线程运行这里
      std::lock_guard<std::mutex> lock(_mutex);
      _crowd->update(static_cast<float>(_delta_seconds), nullptr);
    }

    // 更新行人路线
    _walker_manager.Update(_delta_seconds);

    // 更新检查被堵塞代理的时间
    _time_to_unblock += _delta_seconds;

    // 查看所有活跃代理
    int total_unblocked = 0;
    int total_agents;
    {
      // 关键部分，强制单线程运行这里
      std::lock_guard<std::mutex> lock(_mutex);
      total_agents = _crowd->getAgentCount();
    }
    const dtCrowdAgent *ag;
    for (int i = 0; i < total_agents; ++i) {
      {
        // 关键部分，强制单线程运行这里
        std::lock_guard<std::mutex> lock(_mutex);
        ag = _crowd->getAgent(i);
      }

      if (!ag->active || ag->paused || ag->dead) {
        continue;
      }

      // 仅检查未暂停的行人，不检查车辆
      if (!ag->params.useObb && !ag->paused) {
        bool reset_target_pos = false;
        bool use_same_filter = false;

        // 检查参与者是否解除堵塞
        if (_time_to_unblock >= AGENT_UNBLOCK_TIME) {
          // 获取每个参与者移动的距离
          carla::geom::Vector3D previous = _walkers_blocked_position[i];
          carla::geom::Vector3D current = carla::geom::Vector3D(ag->npos[0], ag->npos[1], ag->npos[2]);
          carla::geom::Vector3D distance = current - previous;
          float d = distance.SquaredLength();
          if (d < AGENT_UNBLOCK_DISTANCE_SQUARED) {
            ++total_unblocked;
            reset_target_pos = true;
            use_same_filter = true;
          }
          // 更新当前位置
          _walkers_blocked_position[i] = current;

          // check to assign a new target position
          if (reset_target_pos) {
            // set if the agent can cross roads or not
            if (!use_same_filter) {
              if (frand() <= _probability_crossing) {
                SetAgentFilter(i, 1);
              } else {
                SetAgentFilter(i, 0);
              }
            }
            // 设置新的随机目标
            carla::geom::Location location;
            GetRandomLocation(location, nullptr);
            _walker_manager.SetWalkerRoute(_mapped_by_index[i], location);
          }
        }
      }
    }

    // 检查重置时间
    if (_time_to_unblock >= AGENT_UNBLOCK_TIME) {
      _time_to_unblock = 0.0f;
    }
  }

  // 获取行人当前变换
  bool Navigation::GetWalkerTransform(ActorId id, carla::geom::Transform &trans) {

    // 检查是否一切就绪
    if (!_ready) {
      return false;
    }

    DEBUG_ASSERT(_crowd != nullptr);

    // 获取内部索引
    auto it = _mapped_walkers_id.find(id);
    if (it == _mapped_walkers_id.end()) {
      return false;
    }

    // 找到索引
    int index = it->second;
    if (index == -1) {
      return false;
    }

    // 获得行人
    const dtCrowdAgent *agent;
    {
      // 关键部分，强制单线程运行这里
      std::lock_guard<std::mutex> lock(_mutex);
      agent = _crowd->getAgent(index);
    }

    if (!agent->active) {
      return false;
    }

    // 在虚幻坐标中设置其位置
    trans.location.x = agent->npos[0];
    trans.location.y = agent->npos[2];
    trans.location.z = agent->npos[1];

    // 设置其旋转
    float yaw;
    float speed = 0.0f;
    float min = 0.1f;
    if (agent->vel[0] < -min || agent->vel[0] > min ||
        agent->vel[2] < -min || agent->vel[2] > min) {
      yaw = atan2f(agent->vel[2], agent->vel[0]) * (180.0f / static_cast<float>(M_PI));
      speed = sqrtf(agent->vel[0] * agent->vel[0] + agent->vel[1] * agent->vel[1] + agent->vel[2] * agent->vel[2]);
    } else {
      yaw = atan2f(agent->dvel[2], agent->dvel[0]) * (180.0f / static_cast<float>(M_PI));
      speed = sqrtf(agent->dvel[0] * agent->dvel[0] + agent->dvel[1] * agent->dvel[1] + agent->dvel[2] * agent->dvel[2]);
    }

    // 插入当前角度和目标角度
    float shortest_angle = fmod(yaw - _yaw_walkers[id] + 540.0f, 360.0f) - 180.0f;
    float per = (speed / 1.5f);
    if (per > 1.0f) per = 1.0f;
    float rotation_speed = per * 6.0f;
    trans.rotation.yaw = _yaw_walkers[id] +
    (shortest_angle * rotation_speed * static_cast<float>(_delta_seconds));
    _yaw_walkers[id] = trans.rotation.yaw;

    return true;
  }

  // 获取行人的当前位置
  bool Navigation::GetWalkerPosition(ActorId id, carla::geom::Location &location) {

    // 检查是否一切就绪
    if (!_ready) {
      return false;
    }

    DEBUG_ASSERT(_crowd != nullptr);

    // 获取内部索引
    auto it = _mapped_walkers_id.find(id);
    if (it == _mapped_walkers_id.end()) {
      return false;
    }

    // 找到索引
    int index = it->second;
    if (index == -1) {
      return false;
    }

    // 获得行人
    const dtCrowdAgent *agent;
    {
      // 关键部分，强制单线程运行这里
      std::lock_guard<std::mutex> lock(_mutex);
      agent = _crowd->getAgent(index);
    }

    if (!agent->active) {
      return false;
    }

    // 在虚幻坐标中设置其位置
    location.x = agent->npos[0];
    location.y = agent->npos[2];
    location.z = agent->npos[1];

    return true;
  }

  float Navigation::GetWalkerSpeed(ActorId id) {

    // 检查是否一切就绪
    if (!_ready) {
      return 0.0f;
    }

    DEBUG_ASSERT(_crowd != nullptr);

    // 获取内部索引
    auto it = _mapped_walkers_id.find(id);
    if (it == _mapped_walkers_id.end()) {
      return 0.0f;
    }

    // 找到索引
    int index = it->second;
    if (index == -1) {
      return 0.0f;
    }

    // 获得行人
    const dtCrowdAgent *agent;
    {
      // 关键部分，强制单线程运行这里
      std::lock_guard<std::mutex> lock(_mutex);
      agent = _crowd->getAgent(index);
    }

    return sqrt(agent->vel[0] * agent->vel[0] + agent->vel[1] * agent->vel[1] + agent->vel[2] *
    agent->vel[2]);
  }

  // 获取随机的导航位置
  bool Navigation::GetRandomLocation(carla::geom::Location &location, dtQueryFilter * filter) const {

    // 检查是否一切就绪
    if (!_ready) {
      return false;
    }

    DEBUG_ASSERT(_nav_query != nullptr);

    // 过滤器
    dtQueryFilter filter2;
    if (filter == nullptr) {
      filter2.setIncludeFlags(CARLA_TYPE_SIDEWALK);
      filter2.setExcludeFlags(CARLA_TYPE_NONE);
      filter = &filter2;
    }

    // 我们会尝试最多 10 轮，否则我们就找不到好的位置
    dtPolyRef random_ref { 0 };
    float point[3] { 0.0f, 0.0f, 0.0f };
    int rounds = 10;
    {
      dtStatus status;
      // 关键部分，强制单线程运行这里
      std::lock_guard<std::mutex> lock(_mutex);
      do {
        status = _nav_query->findRandomPoint(filter, frand, &random_ref, point);
        // 在虚幻坐标中设置位置
        if (status == DT_SUCCESS) {
          location.x = point[0];
          location.y = point[2];
          location.z = point[1];
        }
        --rounds;
      } while (status != DT_SUCCESS && rounds > 0);
    }

    return (rounds > 0);
  }

  // 为代理分配过滤索引
  void Navigation::SetAgentFilter(int agent_index, int filter_index)
  {
    // 获得行人
    dtCrowdAgent *agent;
    {
      // 关键部分，强制单线程运行这里
      std::lock_guard<std::mutex> lock(_mutex);
      agent = _crowd->getEditableAgent(agent_index);
    }
    agent->params.queryFilterType = static_cast<unsigned char>(filter_index);
  }

  // 设置代理在其路径上过马路的概率 0.0 表示没有行人可以过马路，
  // 0.5 表示 50% 的行人可以过马路
  // 1.0 表示所有行人都可以在需要时过马路
  void Navigation::SetPedestriansCrossFactor(float percentage)
  {
    _probability_crossing = percentage;
  }

  // 将人群中的代理设置为暂停
  void Navigation::PauseAgent(ActorId id, bool pause) {
    // 检查是否一切就绪
    if (!_ready) {
      return;
    }

    DEBUG_ASSERT(_crowd != nullptr);

    // 获取内部索引
    auto it = _mapped_walkers_id.find(id);
    if (it == _mapped_walkers_id.end()) {
      return;
    }

    // 找到索引
    int index = it->second;
    if (index == -1) {
      return;
    }

    // 获取行人
    dtCrowdAgent *agent;
    {
      // 关键部分，强制单线程运行这里
      std::lock_guard<std::mutex> lock(_mutex);
      agent = _crowd->getEditableAgent(index);
    }

    // 标记为暂停
    agent->paused = pause;
  }

  bool Navigation::HasVehicleNear(ActorId id, float distance, carla::geom::Location direction) {
    // 获取内部索引（行人或者车辆）
    auto it = _mapped_walkers_id.find(id);
    if (it == _mapped_walkers_id.end()) {
      it = _mapped_vehicles_id.find(id);
      if (it == _mapped_vehicles_id.end()) {
        return false;
      }
    }

    float dir[3] = { direction.x, direction.z, direction.y };
    bool result;
    {
      // 关键部分，强制单线程运行这里
      std::lock_guard<std::mutex> lock(_mutex);
      result = _crowd->hasVehicleNear(it->second, distance * distance, dir, false);
    }
    return result;
  }

  /// 让代理查看某个位置
  bool Navigation::SetWalkerLookAt(ActorId id, carla::geom::Location location) {
    // 获取内部索引（行人或车辆）
    auto it = _mapped_walkers_id.find(id);
    if (it == _mapped_walkers_id.end()) {
      it = _mapped_vehicles_id.find(id);
      if (it == _mapped_vehicles_id.end()) {
        return false;
      }
    }

    dtCrowdAgent *agent;
    {
      // 关键部分，强制单线程运行这里
      std::lock_guard<std::mutex> lock(_mutex);
      agent = _crowd->getEditableAgent(it->second);
    }

    // 获取位置
    float x = (location.x - agent->npos[0]) * 0.0001f;
    float y = (location.y - agent->npos[2]) * 0.0001f;
    float z = (location.z - agent->npos[1]) * 0.0001f;

    // 设置其速度
    agent->vel[0] = x;
    agent->vel[2] = y;
    agent->vel[1] = z;
    agent->nvel[0] = x;
    agent->nvel[2] = y;
    agent->nvel[1] = z;
    agent->dvel[0] = x;
    agent->dvel[2] = y;
    agent->dvel[1] = z;

    return true;
  }

  bool Navigation::IsWalkerAlive(ActorId id, bool &alive) {
    // 检查是否所有都就绪
    if (!_ready) {
      return false;
    }

    DEBUG_ASSERT(_crowd != nullptr);

    // 获取内部索引
    auto it = _mapped_walkers_id.find(id);
    if (it == _mapped_walkers_id.end()) {
      return false;
    }

    // 找到索引
    int index = it->second;
    if (index == -1) {
      return false;
    }

    // 获取行人
    const dtCrowdAgent *agent;
    {
      // 关键部分，强制单线程运行这里
      std::lock_guard<std::mutex> lock(_mutex);
      agent = _crowd->getAgent(index);
    }

    // 标记
    alive = !agent->dead;

    return true;
  }

} // namespace nav
} // namespace carla
