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
  static geom::Location Rotate(float yaw, const geom::Location &location) {
  	// 将偏航角从度转换为弧度
    yaw *= geom::Math::Pi<float>() / 180.0f;
    const float c = std::cos(yaw);
    const float s = std::sin(yaw);
    // 进行旋转计算并返回新的位置
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
    LaneInvasionCallback(
        const Vehicle &vehicle,
        SharedPtr<Map> &&map,
        Sensor::CallbackFunctionType &&user_callback)
      : _parent(vehicle.GetId()),
        _parent_bounding_box(vehicle.GetBoundingBox()),
        _map(std::move(map)),
        _callback(std::move(user_callback)) {
      DEBUG_ASSERT(_map != nullptr);
    }
// 处理每一帧数据的函数
    void Tick(const WorldSnapshot &snapshot) const;

  private:
// 内部结构体，用于存储一帧的边界信息
    struct Bounds {
      size_t frame;
      std::array<geom::Location, 4u> corners;
    };
 // 创建边界信息的函数
    std::shared_ptr<const Bounds> MakeBounds(
        size_t frame,
        const geom::Transform &vehicle_transform) const;

    ActorId _parent;
// 存储父类车辆的边界框信息
    geom::BoundingBox _parent_bounding_box;
// 地图的智能指针
    SharedPtr<const Map> _map;
// 用户定义的回调函数
    Sensor::CallbackFunctionType _callback;
// 可变的原子共享指针，用于存储边界信息
    mutable AtomicSharedPtr<const Bounds> _bounds;
  };
// 处理每一帧数据，检查车辆是否压线并调用用户回调函数
  void LaneInvasionCallback::Tick(const WorldSnapshot &snapshot) const {
    // 确保父类还存活。
    auto parent = snapshot.Find(_parent);
    if (!parent) {
      return;
    }
// 创建当前帧的边界信息
    auto next = MakeBounds(snapshot.GetFrame(), parent->transform);
    auto prev = _bounds.load();

    // 第一帧它将为空。
    if ((prev == nullptr) && _bounds.compare_exchange(&prev, next)) {
      return;
    }

    // 确保距离足够长。
    constexpr float distance_threshold = 10.0f * std::numeric_limits<float>::epsilon();
    for (auto i = 0u; i < 4u; ++i) {
      if ((next->corners[i] - prev->corners[i]).Length() < distance_threshold) {
        return;
      }
    }

    // 确保当前帧是最新的。
    do {
      if (prev->frame >= next->frame) {
        return;
      }
    } while (!_bounds.compare_exchange(&prev, next));

    // 最后，可以安全地计算交叉车道。
    std::vector<road::element::LaneMarking> crossed_lanes;
    for (auto i = 0u; i < 4u; ++i) {
      const auto lanes = _map->CalculateCrossedLanes(prev->corners[i], next->corners[i]);
      crossed_lanes.insert(crossed_lanes.end(), lanes.begin(), lanes.end());
    }
// 如果有交叉车道，调用用户回调函数
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
    return std::make_shared<Bounds>(Bounds{frame, {
        location + Rotate(yaw, geom::Location( box.extent.x,  box.extent.y, 0.0f)),
        location + Rotate(yaw, geom::Location(-box.extent.x,  box.extent.y, 0.0f)),
        location + Rotate(yaw, geom::Location( box.extent.x, -box.extent.y, 0.0f)),
        location + Rotate(yaw, geom::Location(-box.extent.x, -box.extent.y, 0.0f))}});
  }

  // ===========================================================================
  // -- 压线传感器 LaneInvasionSensor -------------------------------------------
  // ===========================================================================
 // 压线传感器的析构函数，停止监听
  LaneInvasionSensor::~LaneInvasionSensor() {
    Stop();
  }
// 开始监听，接收用户回调函数
  void LaneInvasionSensor::Listen(CallbackFunctionType callback) {
    auto vehicle = boost::dynamic_pointer_cast<Vehicle>(GetParent());
    if (vehicle == nullptr) {
      log_error(GetDisplayId(), ": not attached to a vehicle");
      return;
    }

    auto episode = GetEpisode().Lock();
    // 创建压线回调对象
    auto cb = std::make_shared<LaneInvasionCallback>(
        *vehicle,
        episode->GetCurrentMap(),
        std::move(callback));

    const size_t callback_id = episode->RegisterOnTickEvent([cb=std::move(cb)](const auto &snapshot) {
      try {
        cb->Tick(snapshot);
      } catch (const std::exception &e) {
        log_error("LaneInvasionSensor:", e.what());
      }
    });

    const size_t previous = _callback_id.exchange(callback_id);
    if (previous != 0u) {
      episode->RemoveOnTickEvent(previous);
    }
  }
 // 停止监听
  void LaneInvasionSensor::Stop() {
    const size_t previous = _callback_id.exchange(0u);
    auto episode = GetEpisode().TryLock();
    if ((previous != 0u) && (episode != nullptr)) {
      episode->RemoveOnTickEvent(previous);
    }
  }

} // namespace client
} // namespace carla
