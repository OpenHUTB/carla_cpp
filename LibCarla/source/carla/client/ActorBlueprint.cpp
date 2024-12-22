// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/ActorBlueprint.h"//引入ActorBlueprint类相关头文件

#include "carla/Exception.h"//引入异常处理头文件
#include "carla/StringUtil.h"//引入字符串工具头文件，用于字符串操作

#include <algorithm>//引入算法库，提供算法支持

namespace carla {
namespace client {
  //函数：FillMap
  //作用：将 source 容器中的元素移动到 destination map 中，并根据 item.id 作为 key
  // 参数：
  //   destination - 目标 map 容器，用于存储按 id 映射的元素
  //   source - 源容器，包含待移动的元素
  template <typename Map, typename Container>
  static void FillMap(Map &destination, Container &source) {
  	//预先为 destination map 分配空间，提高性能
    destination.reserve(source.size());
    //遍历 source 容器中的每个元素，将元素按 id 作为键，移动到 destination map 中
    for (auto &item : source) {
      auto id = item.id; // 获取元素的 id
      //将 item.id 作为 map 的键，并将 item 的所有权转移到 map 中去
      destination.emplace(id, std::move(item));
    }
  }
  // 构造函数：ActorBlueprint
  // 作用：根据 rpc::ActorDefinition 初始化 ActorBlueprint 对象
  // 参数：
  //   definition - rpc::ActorDefinition 类型的初始化定义，包含 ActorBlueprint 的初始数据
  ActorBlueprint::ActorBlueprint(rpc::ActorDefinition definition)
    : _uid(definition.uid), //初始化成员变量 _uid 为传入的 uid
      _id(std::move(definition.id)) { //将 definition 中的 id 移动赋值给 _id
    StringUtil::Split(_tags, definition.tags, ","); //将 tags 按逗号分隔，并储存到 _tags容器中
    
    FillMap(_attributes, definition.attributes); // 使用 FillMap 函数将 definition 中的 attributes 按 id 映射并存储到 _attributes 中
    
  }
  // 函数：MatchTags
  // 作用：检查 _id 或 _tags 列表中的任意 tag 是否匹配指定的通配符模式
  // 参数：
  //   wildcard_pattern - 用于匹配的通配符模式
  // 返回值：
  //   如果 _id 或 _tags 中任意 tag 匹配模式，则返回 true；否则返回 false
  bool ActorBlueprint::MatchTags(const std::string &wildcard_pattern) const {
  	//返回结果为：_id 匹配模式或 _tags 列表中的任意一个 tag 匹配模式
    return
        StringUtil::Match(_id, wildcard_pattern) || // 检查 _id 是否匹配通配符模式
        
        std::any_of(_tags.begin(), _tags.end(), [&](const auto &tag) {
        	// 遍历 _tags 列表，检查是否有任何 tag 匹配通配符模式
          return StringUtil::Match(tag, wildcard_pattern);
        });
  }
  // 函数：GetAttribute
  // 作用：根据指定的属性 id 获取 ActorBlueprint 对象中的 ActorAttribute
  // 参数：
  //   id - 要查询的属性的 id
  // 返回值：
  //   返回指定 id 对应的 ActorAttribute
  // 异常：
  //   如果指定 id 的属性不存在，抛出 std::out_of_range 异常
  const ActorAttribute &ActorBlueprint::GetAttribute(const std::string &id) const {
  	//在_attributes映射中查找 id 对应的属性
    auto it = _attributes.find(id);
    if (it == _attributes.end()) {
      // 如果未找到属性，抛出异常
      using namespace std::string_literals;
      throw_exception(std::out_of_range("attribute '"s + id + "' not found"));
    }
    return it->second; //返回找到的属性
  }
  // 函数：SetAttribute
  // 作用：根据指定的属性 id 设置对应属性的值
  // 参数：
  //   id - 要设置的属性的 id
  //   value - 要设置的新属性值
  void ActorBlueprint::SetAttribute(const std::string &id, std::string value) {
    const_cast<ActorAttribute &>(GetAttribute(id)).Set(std::move(value));
    // 使用 const_cast 去除 const 限制，修改找到的属性值
  }
  // 函数：MakeActorDescription
  // 作用：生成一个 rpc::ActorDescription 对象，用于描述当前的 ActorBlueprint
  // 返回值：
  //   返回生成的 rpc::ActorDescription 描述对象
  rpc::ActorDescription ActorBlueprint::MakeActorDescription() const {
    rpc::ActorDescription description;
    description.uid = _uid; //设置唯一标识符
    description.id = _id; //设置 actor 的 id
    //为属性容器预留空间，提高性能
    description.attributes.reserve(_attributes.size());
   //将所有属性添加到描述对象的 attributes 向量中
    for (const auto &attribute : *this) {
      description.attributes.push_back(attribute);
    }
    return description; //返回生成的描述对象
  }

} // namespace client
} // namespace carla
