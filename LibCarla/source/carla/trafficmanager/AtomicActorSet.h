// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <mutex>
#include <map>
#include <vector>

#include "carla/client/Actor.h"
#include "carla/Memory.h"

namespace carla {
namespace traffic_manager {
// 使用命名空间别名简化类型名称
  namespace cc = carla::client;
  using ActorPtr = carla::SharedPtr<cc::Actor>;
  using ActorId = carla::ActorId;
  // 定义名为 AtomicActorSet 的类
  class AtomicActorSet {

  private:
 // 互斥锁，用于保护对 actor_set 的并发访问
    std::mutex modification_mutex;
	// 存储 ActorId 和对应的 ActorPtr 的映射
    std::map<ActorId, ActorPtr> actor_set;
	// 状态计数器，用于跟踪集合的状态变化
    int state_counter;

  public:

    AtomicActorSet(): state_counter(0) {}
// 获取当前集合中的所有 ActorPtr 组成的向量
    std::vector<ActorPtr> GetList() {

      std::lock_guard<std::mutex> lock(modification_mutex);
      std::vector<ActorPtr> actor_list;
	  // 遍历 actor_set，将每个 ActorPtr 添加到向量中
      for (auto it = actor_set.begin(); it != actor_set.end(); ++it) {
        actor_list.push_back(it->second);
      }
      return actor_list;
    }
// 获取当前集合中所有 Actor 的 ID 组成的向量
    std::vector<ActorId> GetIDList() {

      std::lock_guard<std::mutex> lock(modification_mutex);
      std::vector<ActorId> actor_list;
      for (auto it = actor_set.begin(); it != actor_set.end(); ++it) {
        actor_list.push_back(it->first);
      }//将 actor_set 集合中的所有元素的 second 值提取并添加到 actor_list 列表中
      return actor_list;
    }
// 将一个 ActorPtr 向量中的所有元素插入到集合中
    void Insert(std::vector<ActorPtr> actor_list) {

      std::lock_guard<std::mutex> lock(modification_mutex);
	  // 遍历输入的向量，将每个 ActorPtr 插入到 actor_set 中
      for (auto &actor: actor_list) {
        actor_set.insert({actor->GetId(), actor});
      }
      ++state_counter;
    }
// 从集合中移除指定的 ActorId 列表中的元素
    void Remove(std::vector<ActorId> actor_id_list) {

      std::lock_guard<std::mutex> lock(modification_mutex);
	  // 遍历要移除的 ActorId 列表
      for (auto& actor_id: actor_id_list) {
		   // 如果在 actor_set 中找到对应的 ActorId，则移除该元素
        if (actor_set.find(actor_id) != actor_set.end()){
          actor_set.erase(actor_id);
        }
      }
      ++state_counter;
    }
// 销毁指定 ActorId 对应的 Actor
    void Destroy(ActorId actor_id) {

      std::lock_guard<std::mutex> lock(modification_mutex);
	   // 如果在 actor_set 中找到对应的 ActorId
      if (actor_set.find(actor_id) != actor_set.end()) {
		  // 获取对应的 ActorPtr
        ActorPtr actor = actor_set.at(actor_id);
		 // 销毁该 Actor
        actor->Destroy();
		// 从集合中移除该元素
        actor_set.erase(actor_id);
        ++state_counter;
      }
    }
// 获取当前集合的状态计数器值
    int GetState() {

      std::lock_guard<std::mutex> lock(modification_mutex);
      return state_counter;
    }
 // 检查集合中是否包含指定的 ActorId
    bool Contains(ActorId id) {

      std::lock_guard<std::mutex> lock(modification_mutex);
	  // 判断 actor_set 中是否存在指定的 ActorId
      return actor_set.find(id) != actor_set.end();
    }
// 获取集合中元素的数量
    size_t Size() {

      std::lock_guard<std::mutex> lock(modification_mutex);
      return actor_set.size();
    }
// 清空集合
    void Clear() {

      std::lock_guard<std::mutex> lock(modification_mutex);
      return actor_set.clear();
    }

  };

} // namespace traffic_manager
} // namespace carla
