// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Waypoint.h"  // 引入Waypoint头文件
#include "carla/client/Map.h"  // 引入Map头文件
#include "carla/client/Junction.h"  // 引入Junction头文件
#include "carla/client/Landmark.h"  // 引入Landmark头文件

#include <unordered_set>  // 引入unordered_set头文件，用于哈希集合

namespace carla {
namespace client {

  // Waypoint构造函数，初始化一个Waypoint对象
  Waypoint::Waypoint(SharedPtr<const Map> parent, road::element::Waypoint waypoint)
    : _parent(std::move(parent)),  // 移动构造父Map对象
      _waypoint(std::move(waypoint)),  // 移动构造Waypoint对象
      _transform(_parent->GetMap().ComputeTransform(_waypoint)),  // 计算并初始化变换
      _mark_record(_parent->GetMap().GetMarkRecord(_waypoint)) {}  // 获取标记记录

  Waypoint::~Waypoint() = default;  // 默认析构函数

  // 获取Waypoint所属的路口ID
  road::JuncId Waypoint::GetJunctionId() const {
    return _parent->GetMap().GetJunctionId(_waypoint.road_id);  // 从地图获取路口ID
  }

  // 判断Waypoint是否为路口
  bool Waypoint::IsJunction() const {
    return _parent->GetMap().IsJunction(_waypoint.road_id);  // 判断该Waypoint的道路ID是否为路口
  }

  // 获取该Waypoint对应的路口对象
  SharedPtr<Junction> Waypoint::GetJunction() const {
    if (IsJunction()) {  // 如果是路口
      return _parent->GetJunction(*this);  // 获取并返回对应的路口对象
    }
    return nullptr;  // 如果不是路口，返回空指针
  }

  // 获取Waypoint所在车道的宽度
  double Waypoint::GetLaneWidth() const {
    return _parent->GetMap().GetLaneWidth(_waypoint);  // 从地图中获取车道宽度
  }

  // 获取Waypoint所在车道的类型
  road::Lane::LaneType Waypoint::GetType() const {
    return _parent->GetMap().GetLaneType(_waypoint);  // 从地图中获取车道类型
  }

  // 获取Waypoint的下一个Waypoint列表
  std::vector<SharedPtr<Waypoint>> Waypoint::GetNext(double distance) const {
    auto waypoints = _parent->GetMap().GetNext(_waypoint, distance);  // 获取下一个Waypoint列表
    std::vector<SharedPtr<Waypoint>> result;  // 结果存储容器
    result.reserve(waypoints.size());  // 预留空间
    for (auto &waypoint : waypoints) {  // 遍历每个Waypoint
      result.emplace_back(SharedPtr<Waypoint>(new Waypoint(_parent, std::move(waypoint))));  // 创建并添加到结果中
    }
    return result;  // 返回下一个Waypoint的列表
  }

  // 获取Waypoint的前一个Waypoint列表
  std::vector<SharedPtr<Waypoint>> Waypoint::GetPrevious(double distance) const {
    auto waypoints = _parent->GetMap().GetPrevious(_waypoint, distance);  // 获取前一个Waypoint列表
    std::vector<SharedPtr<Waypoint>> result;  // 结果存储容器
    result.reserve(waypoints.size());  // 预留空间
    for (auto &waypoint : waypoints) {  // 遍历每个Waypoint
      result.emplace_back(SharedPtr<Waypoint>(new Waypoint(_parent, std::move(waypoint))));  // 创建并添加到结果中
    }
    return result;  // 返回前一个Waypoint的列表
  }

  // 获取Waypoint的下一个Waypoint直到车道结束
  std::vector<SharedPtr<Waypoint>> Waypoint::GetNextUntilLaneEnd(double distance) const {
    std::vector<SharedPtr<Waypoint>> result;  // 结果存储容器
    std::vector<SharedPtr<Waypoint>> next = GetNext(distance);  // 获取下一个Waypoint列表

    // 如果下一个Waypoint还在同一条路上，继续获取下一个
    while (next.size() == 1 && next.front()->GetRoadId() == GetRoadId()) {
      result.emplace_back(next.front());  // 将下一个Waypoint添加到结果中
      next = result.back()->GetNext(distance);  // 获取该Waypoint的下一个Waypoint
    }
    
    double current_s = GetDistance();  // 获取当前Waypoint的位置
    if(result.size()) {  // 如果结果中有Waypoint
      current_s = result.back()->GetDistance();  // 获取最后一个Waypoint的位置
    }
    
    double remaining_length;  // 剩余长度
    double road_length = _parent->GetMap().GetLane(_waypoint).GetRoad()->GetLength();  // 获取道路的总长度
    if(_waypoint.lane_id < 0) {  // 如果车道ID为负数
      remaining_length = road_length - current_s;  // 剩余长度为道路总长减去当前距离
    } else {
      remaining_length = current_s;  // 否则剩余长度为当前距离
    }
    remaining_length -= std::numeric_limits<double>::epsilon();  // 减去一个非常小的数值以避免浮动误差
    
    // 如果结果中有Waypoint，获取其下一个Waypoint直到车道结束
    if(result.size()) {
      result.emplace_back(result.back()->GetNext(remaining_length).front());  // 添加最后一个Waypoint
    } else {
      result.emplace_back(GetNext(remaining_length).front());  // 如果没有Waypoint，则获取当前Waypoint的下一个
    }

    return result;  // 返回直到车道结束的Waypoint列表
  }

  // 获取Waypoint的前一个Waypoint直到车道开始
  std::vector<SharedPtr<Waypoint>> Waypoint::GetPreviousUntilLaneStart(double distance) const {
    std::vector<SharedPtr<Waypoint>> result;  // 结果存储容器
    std::vector<SharedPtr<Waypoint>> prev = GetPrevious(distance);  // 获取前一个Waypoint列表

    // 如果前一个Waypoint还在同一条路上，继续获取前一个
    while (prev.size() == 1 && prev.front()->GetRoadId() == GetRoadId()) {
      result.emplace_back(prev.front());  // 将前一个Waypoint添加到结果中
      prev = result.back()->GetPrevious(distance);  // 获取该Waypoint的前一个Waypoint
    }

    double current_s = GetDistance();  // 获取当前Waypoint的位置
    if(result.size()) {  // 如果结果中有Waypoint
      current_s = result.back()->GetDistance();  // 获取最后一个Waypoint的位置
    }

    double remaining_length;  // 剩余长度
    double road_length = _parent->GetMap().GetLane(_waypoint).GetRoad()->GetLength();  // 获取道路的总长度
    if(_waypoint.lane_id < 0) {  // 如果车道ID为负数
      remaining_length = road_length - current_s;  // 剩余长度为道路总长减去当前距离
    } else {
      remaining_length = current_s;  // 否则剩余长度为当前距离
    }
    remaining_length -= std::numeric_limits<double>::epsilon();  // 减去一个非常小的数值以避免浮动误差
    
    // 如果结果中有Waypoint，获取其前一个Waypoint直到车道开始
    if(result.size()) {
      result.emplace_back(result.back()->GetPrevious(remaining_length).front());  // 添加最后一个Waypoint
    } else {
      result.emplace_back(GetPrevious(remaining_length).front());  // 如果没有Waypoint，则获取当前Waypoint的前一个
    }

    return result;  // 返回直到车道开始的Waypoint列表
  }

  SharedPtr<Waypoint> Waypoint::GetRight() const {
    // 获取右侧的Waypoint
    auto right_lane_waypoint =
        _parent->GetMap().GetRight(_waypoint);// 调用_parent的地图对象，获取右侧的Waypoint
    if (right_lane_waypoint.has_value()) {  // 如果右侧Waypoint存在
      return SharedPtr<Waypoint>(new Waypoint(_parent, std::move(*right_lane_waypoint)));// 创建并返回右侧Waypoint的智能指针
    }
    return nullptr;// 如果右侧Waypoint不存在，返回nullptr
  }

  SharedPtr<Waypoint> Waypoint::GetLeft() const {
     // 获取左侧的Waypoint
    auto left_lane_waypoint =
        _parent->GetMap().GetLeft(_waypoint);// 调用_parent的地图对象，获取左侧的Waypoint
    if (left_lane_waypoint.has_value()) {// 如果左侧Waypoint存在
      return SharedPtr<Waypoint>(new Waypoint(_parent, std::move(*left_lane_waypoint)));// 创建并返回左侧Waypoint的智能指针
    }
    return nullptr;// 如果左侧Waypoint不存在，返回nullptr
  }

  boost::optional<road::element::LaneMarking> Waypoint::GetRightLaneMarking() const {
     // 获取右侧车道标记
    if (_mark_record.first != nullptr) {  // 如果右侧标记存在
      return road::element::LaneMarking(*_mark_record.first);// 返回右侧车道标记
    }
    return boost::optional<road::element::LaneMarking>{};// 如果右侧标记不存在，返回空的可选值
  }

  boost::optional<road::element::LaneMarking> Waypoint::GetLeftLaneMarking() const {
     // 获取左侧车道标记
    if (_mark_record.second != nullptr) {// 如果左侧标记存在
      return road::element::LaneMarking(*_mark_record.second);// 返回左侧车道标记
    }
    return boost::optional<road::element::LaneMarking>{};// 如果左侧标记不存在，返回空的可选值
  }

  template <typename EnumT>
  static EnumT operator&(EnumT lhs, EnumT rhs) {
    // 按位与运算符重载（适用于枚举类型）
    return static_cast<EnumT>(
        static_cast<typename std::underlying_type<EnumT>::type>(lhs) &
        static_cast<typename std::underlying_type<EnumT>::type>(rhs));// 将枚举转换为底层类型，执行按位与运算，并转换回枚举类型
  }

  template <typename EnumT>
  static EnumT operator|(EnumT lhs, EnumT rhs) {
     // 按位或运算符重载（适用于枚举类型）
    return static_cast<EnumT>(
        static_cast<typename std::underlying_type<EnumT>::type>(lhs) |
        static_cast<typename std::underlying_type<EnumT>::type>(rhs));// 将枚举转换为底层类型，执行按位或运算，并转换回枚举类型
  }

  road::element::LaneMarking::LaneChange Waypoint::GetLaneChange() const {
    // 获取车道变更信息
    using lane_change_type = road::element::LaneMarking::LaneChange;// 定义车道变更类型别名

    const auto lane_change_right_info = _mark_record.first;// 获取右侧车道变更信息
    lane_change_type c_right;// 定义右侧车道变更类型
    if (lane_change_right_info != nullptr) {// 如果右侧车道变更信息存在
      const auto lane_change_right = lane_change_right_info->GetLaneChange();// 获取右侧车道变更
      c_right = static_cast<lane_change_type>(lane_change_right);// 将右侧车道变更信息转换为LaneChange类型
    } else {
      c_right = lane_change_type::Both;// 如果没有右侧车道变更信息，默认车道变更为Both
    }

    const auto lane_change_left_info = _mark_record.second;// 获取左侧车道变更信息
    lane_change_type c_left;// 定义左侧车道变更类型
    if (lane_change_left_info != nullptr) {// 如果左侧车道变更信息存在
      const auto lane_change_left = lane_change_left_info->GetLaneChange();// 获取左侧车道变更
      c_left = static_cast<lane_change_type>(lane_change_left);// 将左侧车道变更信息转换为LaneChange类型
    } else {
      c_left = lane_change_type::Both;// 如果没有左侧车道变更信息，默认车道变更为Both
    }

    if (_waypoint.lane_id > 0) {// 如果当前Waypoint的车道ID大于0
       // 如果路段是倒行（车道ID大于0），则右侧车道变更方向反转
      if (c_right == lane_change_type::Right) {
        c_right = lane_change_type::Left;// 右侧车道变更为左侧
      } else if (c_right == lane_change_type::Left) {
        c_right = lane_change_type::Right;// 右侧车道变更为右侧
      }
    }

    if (((_waypoint.lane_id > 0) ? _waypoint.lane_id - 1 : _waypoint.lane_id + 1) > 0) {// 判断左侧车道是否有变化
      // 如果路段是倒行（车道ID大于0），则左侧车道变更方向反转
      if (c_left == lane_change_type::Right) {
        c_left = lane_change_type::Left;// 左侧车道变更为左侧
      } else if (c_left == lane_change_type::Left) {
        c_left = lane_change_type::Right;// 左侧车道变更为右侧
      }
    }

    // 返回右侧和左侧车道变更的组合
    return (c_right & lane_change_type::Right) | (c_left & lane_change_type::Left);
  }

  std::vector<SharedPtr<Landmark>> Waypoint::GetAllLandmarksInDistance(
      double distance, bool stop_at_junction) const {
      // 获取指定距离范围内的所有地标
    std::vector<SharedPtr<Landmark>> result;  // 存储结果地标
    auto signals = _parent->GetMap().GetSignalsInDistance(
        _waypoint, distance, stop_at_junction);// 获取指定距离内的所有信号
    std::unordered_set<const road::element::RoadInfoSignal*> added_signals; // 用于检查是否已添加过信号
    for(auto &signal_data : signals){// 遍历信号数据
      if(added_signals.count(signal_data.signal) > 0) {// 如果信号已添加过，跳过
        continue;
      }
      added_signals.insert(signal_data.signal);// 将信号标记为已添加
      auto waypoint = SharedPtr<Waypoint>(new Waypoint(_parent, signal_data.waypoint));// 创建信号对应的Waypoint
      result.emplace_back(
          new Landmark(waypoint, _parent, signal_data.signal, signal_data.accumulated_s));// 创建地标并加入结果中
    }
    return result;// 返回地标列表
  }

  std::vector<SharedPtr<Landmark>> Waypoint::GetLandmarksOfTypeInDistance(
        double distance, std::string filter_type, bool stop_at_junction) const {
     // 创建一个空的 Landmark 向量，用于存储符合条件的 Landmark 对象
    std::vector<SharedPtr<Landmark>> result;
    // 创建一个 unordered_set，用于存储已经添加过的信号，避免重复添加相同的信号
    std::unordered_set<const road::element::RoadInfoSignal*> added_signals; 
    // 获取在指定距离范围内的所有信号数据
    auto signals = _parent->GetMap().GetSignalsInDistance(
        _waypoint, distance, stop_at_junction);
    // 遍历每一个信号数据
    for(auto &signal_data : signals){
      // 检查信号的类型是否与指定的 filter_type 匹配
      if(signal_data.signal->GetSignal()->GetType() == filter_type) {
        if(added_signals.count(signal_data.signal) > 0) {
          continue;
        }
         // 创建一个新的 Waypoint 对象，传入父对象和信号的 waypoint 数据
        auto waypoint = SharedPtr<Waypoint>(new Waypoint(_parent, signal_data.waypoint));
        result.emplace_back(
            // 将新的 Landmark 对象添加到结果向量中
            new Landmark(waypoint, _parent, signal_data.signal, signal_data.accumulated_s));
      }
    }
    return result;// 返回符合条件的 Landmark 向量
  }

} // namespace client
} // namespace carla
