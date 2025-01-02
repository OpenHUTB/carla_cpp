// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// 压线传感器
// 
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/LaneInvasionSensor.h"

#include "carla/Logging.h"
#include "carla/client/Map.h"
#include "carla/client/Vehicle.h"
#include "carla/client/detail/Simulator.h"
#include "carla/geom/Location.h"
#include "carla/geom/Math.h"
#include "carla/sensor/data/LaneInvasionEvent.h"

#include <exception>

namespace carla {
namespace client {

  // ===========================================================================
  // -- 静态局部方法 ------------------------------------------------------------
  // ===========================================================================
  // 静态方法，根据给定的偏航角和位置进行旋转计算
  // 此方法用于将一个位置按照给定的偏航角进行旋转，常用于坐标转换等相关操作
  // 参数说明：
  // - yaw：偏航角，以度为单位，代表绕Z轴旋转的角度。
  // - location：要进行旋转的位置坐标（geom::Location类型）。
  static geom::Location Rotate(float yaw, const geom::Location &location) {
  	// 将偏航角从度转换为弧度，因为后续的三角函数计算通常需要使用弧度制
    yaw *= geom::Math::Pi<float>() / 180.0f;
    const float c = std::cos(yaw);
    const float s = std::sin(yaw);
    // 按照二维平面旋转的数学公式进行坐标计算，返回旋转后的新位置坐标
    // 这里实现的是绕原点的二维旋转，适用于平面坐标的转换情况
    return {
        c * location.x - s * location.y,
        s * location.x + c * location.y,
        location.z};
  }

  // ===========================================================================
  // -- 压线回调类 LaneInvasionCallback -----------------------------------------
  // ===========================================================================

  class LaneInvasionCallback {
  public:
// 构造函数，接收车辆、地图智能指针和用户回调函数
    // 用于初始化相关成员变量，这些变量将在后续判断车辆是否压线以及触发回调等操作中使用
    // 参数说明：
    // - vehicle：表示要检测压线情况的车辆对象，从中可以获取车辆的相关属性信息，如ID、边界框等。
    // - map：指向地图对象的智能指针，地图包含了道路、车道等信息，用于判断车辆与车道的交叉情况。
    // - user_callback：用户定义的回调函数，当检测到车辆压线时会调用该函数，将压线相关信息传递给用户。
    LaneInvasionCallback(
        const Vehicle &vehicle,
        SharedPtr<Map> &&map,
        Sensor::CallbackFunctionType &&user_callback)
      : _parent(vehicle.GetId()),
        _parent_bounding_box(vehicle.GetBoundingBox()),
        _map(std::move(map)),
        _callback(std::move(user_callback)) {
      DEBUG_ASSERT(_map != nullptr);// 确保传入的地图指针不为空，保证后续操作能正常基于地图进行
    }
// 处理每一帧数据的函数，在每一帧都会被调用，用于检测当前帧车辆是否压线并相应地触发回调
    void Tick(const WorldSnapshot &snapshot) const;

  private:
    // 内部结构体，用于存储一帧的边界信息
    // 包含了帧编号以及车辆边界框四个角在世界坐标系下的位置信息
    struct Bounds {
      size_t frame;
      std::array<geom::Location, 4u> corners;
    };
 // 创建边界信息的函数，根据给定的帧编号和车辆的变换信息生成当前帧车辆的边界信息
    std::shared_ptr<const Bounds> MakeBounds(
        size_t frame,
        const geom::Transform &vehicle_transform) const;

    ActorId _parent;// 存储父类车辆的唯一标识符（ID），用于在场景快照中查找对应的车辆对象
    // 存储父类车辆的边界框信息，用于确定车辆在世界坐标系中的范围，以便后续判断是否压线
    geom::BoundingBox _parent_bounding_box;
// 地图的智能指针，指向整个场景的地图对象，通过地图来获取道路、车道等相关信息用于判断压线情况
    SharedPtr<const Map> _map;
// 用户定义的回调函数，类型为Sensor::CallbackFunctionType，当检测到车辆压线时将调用此函数通知用户
    Sensor::CallbackFunctionType _callback;
// 可变的原子共享指针，用于存储边界信息，原子操作保证了多线程环境下对该指针的安全访问
    mutable AtomicSharedPtr<const Bounds> _bounds;
  };
// 处理每一帧数据，检查车辆是否压线并调用用户回调函数
  void LaneInvasionCallback::Tick(const WorldSnapshot &snapshot) const {
    // 确保父类（对应的车辆）还存活在当前场景中，如果在场景快照中找不到对应的车辆，则直接返回，不进行后续检测
    auto parent = snapshot.Find(_parent);
    if (!parent) {
      return;
    }

    // 创建当前帧的边界信息，根据当前帧编号和车辆的变换信息生成车辆边界框四个角在世界坐标系下的位置
    auto next = MakeBounds(snapshot.GetFrame(), parent->transform);
    auto prev = _bounds.load();

    // 第一帧它将为空，因为还没有上一帧的边界信息，此时将当前帧的边界信息设置为初始值，并直接返回，不进行后续比较等操作
    if ((prev == nullptr) && _bounds.compare_exchange(&prev, next)) {
      return;
    }

    // 确保距离足够长，设置一个距离阈值，用于判断车辆是否移动了足够的距离，避免在车辆几乎没移动的情况下误判压线情况
    // 这里使用了一个极小的浮点数乘以一个倍数作为阈值，根据实际情况判断车辆位置变化是否足够显著
    constexpr float distance_threshold = 10.0f * std::numeric_limits<float>::epsilon();
    for (auto i = 0u; i < 4u; ++i) {
      if ((next->corners[i] - prev->corners[i]).Length() < distance_threshold) {
        return;
      }
    }

    // 确保当前帧是最新的，通过比较当前帧和上一帧的编号来判断，防止出现顺序错乱等情况
    // 如果当前帧编号小于等于上一帧编号，说明数据可能出现问题，直接返回，不进行后续压线判断操作
    do {
      if (prev->frame >= next->frame) {
        return;
      }
    } while (!_bounds.compare_exchange(&prev, next));

    // 最后，可以安全地计算交叉车道，即判断车辆在这两帧之间是否跨越了车道线
    std::vector<road::element::LaneMarking> crossed_lanes;
    for (auto i = 0u; i < 4u; ++i) {
      // 通过地图对象的方法，根据车辆边界框角点的前后位置信息，计算出车辆跨越的车道线信息
      const auto lanes = _map->CalculateCrossedLanes(prev->corners[i], next->corners[i]);
      crossed_lanes.insert(crossed_lanes.end(), lanes.begin(), lanes.end());
    }

    // 如果有交叉车道，说明车辆压线了，此时调用用户回调函数，将压线相关的详细信息传递给用户定义的回调函数进行处理
    if (!crossed_lanes.empty()) {
    if (!crossed_lanes.empty()) {
      _callback(MakeShared<sensor::data::LaneInvasionEvent>(
          snapshot.GetTimestamp().frame,
          snapshot.GetTimestamp().elapsed_seconds,
          parent->transform,
          _parent,
          std::move(crossed_lanes)));
    }
  }
// 创建边界信息的函数实现
  std::shared_ptr<const LaneInvasionCallback::Bounds> LaneInvasionCallback::MakeBounds(
      const size_t frame,
      const geom::Transform &transform) const {
    const auto &box = _parent_bounding_box;
    const auto location = transform.location + box.location;
    const auto yaw = transform.rotation.yaw;
    // 根据车辆的边界框信息以及当前的位置和旋转角度，计算出车辆边界框四个角在世界坐标系下的坐标位置
    // 先获取车辆整体的位置（考虑边界框的偏移），再通过旋转函数计算出四个角经过旋转后的准确位置
    return std::make_shared<Bounds>(Bounds{frame, {
        location + Rotate(yaw, geom::Location( box.extent.x,  box.extent.y, 0.0f)),
        location + Rotate(yaw, geom::Location(-box.extent.x,  box.extent.y, 0.0f)),
        location + Rotate(yaw, geom::Location( box.extent.x, -box.extent.y, 0.0f)),
        location + Rotate(yaw, geom::Location(-box.extent.x, -box.extent.y, 0.0f))}});
  }

  // ===========================================================================
  // -- 压线传感器 LaneInvasionSensor -------------------------------------------
  // ===========================================================================
 // 压线传感器的析构函数，在对象销毁时被调用，用于停止传感器的监听操作，释放相关资源
  LaneInvasionSensor::~LaneInvasionSensor() {
    Stop();
  }

  // 开始监听，接收用户回调函数，用于设置当检测到车辆压线时要执行的回调操作
  // 参数说明：
  // - callback：用户定义的回调函数，符合Sensor::CallbackFunctionType类型要求，当车辆压线时会被调用。
  void LaneInvasionSensor::Listen(CallbackFunctionType callback) {
    // 获取传感器所附着的父对象，并尝试将其转换为车辆类型的智能指针，如果转换失败说明传感器没有正确附着到车辆上
    auto vehicle = boost::dynamic_pointer_cast<Vehicle>(GetParent());
    if (vehicle == nullptr) {
      log_error(GetDisplayId(), ": not attached to a vehicle");
      return;
    }

    auto episode = GetEpisode().Lock();
     // 创建压线回调对象，传入车辆、地图以及用户回调函数等信息，用于后续在每帧数据处理时判断压线情况并触发回调
    auto cb = std::make_shared<LaneInvasionCallback>(
        *vehicle,
        episode->GetCurrentMap(),
        std::move(callback));

    // 在当前的模拟场景（episode）中注册一个每帧触发的事件，将压线回调对象的Tick方法绑定到该事件上，
    // 这样在每一帧数据更新时都会调用Tick方法来检测车辆是否压线，同时返回一个回调事件的唯一标识符
    const size_t callback_id = episode->RegisterOnTickEvent([cb=std::move(cb)](const auto &snapshot) {
      try {
        cb->Tick(snapshot);
      } catch (const std::exception &e) {
        log_error("LaneInvasionSensor:", e.what());
      }
    });

    const size_t previous = _callback_id.exchange(callback_id);
    if (previous != 0u) {
      // 如果之前已经存在注册的回调事件，先移除之前的事件，保证只有最新注册的事件生效
      episode->RemoveOnTickEvent(previous);
    }
  }
 // 停止监听，用于取消之前注册的车辆压线检测相关的回调事件，停止传感器的监听操作，释放相关资源
  void LaneInvasionSensor::Stop() {
    const size_t previous = _callback_id.exchange(0u);
    auto episode = GetEpisode().TryLock();
    if ((previous != 0u) && (episode != nullptr)) {
      episode->RemoveOnTickEvent(previous);
    }
  }

} // namespace client
} // namespace carla
