// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/ActorList.h"  // 引入ActorList类的头文件

#include "carla/StringUtil.h"  // 引入字符串工具类的头文件
#include "carla/client/detail/ActorFactory.h"  // 引入角色工厂类的头文件

#include <iterator>  // 引入迭代器相关的标准库

namespace carla {
namespace client {

   ActorList::ActorList(  // ActorList构造函数
       detail::EpisodeProxy episode,  // 传入的场景代理对象
       std::vector<rpc::Actor> actors)  // 传入的参与者列表
      : _episode(std::move(episode)),  // 移动语义传递场景代理
        _actors(std::make_move_iterator(actors.begin()), std::make_move_iterator(actors.end())) {}  // 使用移动迭代器初始化参与者列表

   SharedPtr<Actor> ActorList::Find(const ActorId actor_id) const {  // 查找指定ID的参与者
     for (auto& actor : _actors) {  // 遍历所有参与者
        if (actor_id == actor.GetId()) {     // 如果找到匹配的ID
          return actor.Get(_episode);  // 返回参与者的共享指针
        }
     }
     return nullptr;  // 如果未找到，返回空指针
   }

   SharedPtr<ActorList> ActorList::Filter(const std::string& wildcard_pattern) const {  // 根据通配符模式过滤参与者
     SharedPtr<ActorList> filtered(new ActorList(_episode, {}));  // 创建一个新的ActorList用于存放过滤后的参与者
     for (auto&& actor : _actors) {  // 遍历所有参与者
        if (StringUtil::Match(actor.GetTypeId(), wildcard_pattern)) {  // 如果参与者类型与通配符匹配
           filtered->_actors.push_back(actor);  // 将匹配的参与者加入到过滤后的列表中
        }
     }
     return filtered;  // 返回过滤后的参与者列表
   }

} // namespace client
} // namespace carla
