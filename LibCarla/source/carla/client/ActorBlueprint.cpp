// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/ActorBlueprint.h"//引入ActorBlueprint类相关头文件

#include "carla/Exception.h"//引入异常处理头文件
#include "carla/StringUtil.h"//引入字符串工具头文件，用于字符串操作

#include <algorithm>//引入算法库

namespace carla {
namespace client {
//函数：FillMap
//作用：将 source 容器中的元素移动到 destination map 中，并根据 item.id 作为 key
  template <typename Map, typename Container>
  static void FillMap(Map &destination, Container &source) {
  	//预先分配空间，提高效率
    destination.reserve(source.size());
    //遍历 source 容器中的每个元素
    for (auto &item : source) {
      auto id = item.id;
      //将 item.id 作为 map 的键，并将 item 的所有权转移到 map 中去
      destination.emplace(id, std::move(item));
    }
  }
//ActorBlueprint 构造函数
//作用：根据 rpc::ActorBlueprint 初始化ActorBlueprint对象
  ActorBlueprint::ActorBlueprint(rpc::ActorDefinition definition)
    : _uid(definition.uid), //初始化成员变量 _uid
      _id(std::move(definition.id)) { //将定义中的 id 移动赋值给 _id
    StringUtil::Split(_tags, definition.tags, ",");
    //将 tags 按逗号分隔，并储存到 _tags列表中
    FillMap(_attributes, definition.attributes);
    //调用 FillMap 函数，将 attributes 从 definition 移动到 _attributes 中
  }

  bool ActorBlueprint::MatchTags(const std::string &wildcard_pattern) const {
  	//返回结果为：_id 匹配模式或 _tags 列表中的任意一个 tag 匹配模式
    return
        StringUtil::Match(_id, wildcard_pattern) ||
        //检查 _id 是否匹配模式
        std::any_of(_tags.begin(), _tags.end(), [&](const auto &tag) {
        	//遍历每个 tag 检查是否匹配模式
          return StringUtil::Match(tag, wildcard_pattern);
        });
  }

  const ActorAttribute &ActorBlueprint::GetAttribute(const std::string &id) const {
  	//在_attributes映射中查找 id 对应的属性
    auto it = _attributes.find(id);
    if (it == _attributes.end()) {
      using namespace std::string_literals;
      throw_exception(std::out_of_range("attribute '"s + id + "' not found"));
    } //如果未找到则抛出异常
    return it->second; //返回找到的属性
  }
//指定 id 的属性值
  void ActorBlueprint::SetAttribute(const std::string &id, std::string value) {
    const_cast<ActorAttribute &>(GetAttribute(id)).Set(std::move(value));
    // const_cast 用于移除常量性，将找到的属性设置为新的值
  }
//生成一个 rpc::ActorDescription 对象，用于描述当前的 ActorBlueprint
  rpc::ActorDescription ActorBlueprint::MakeActorDescription() const {
    rpc::ActorDescription description;
    description.uid = _uid; //设置唯一标识符
    description.id = _id; //设置 id
     //为属性预留空间以优化性能
    description.attributes.reserve(_attributes.size());
   //将所有属性添加到描述对象的 attributes 向量中
    for (const auto &attribute : *this) {
      description.attributes.push_back(attribute);
    }
    return description; //返回生成的描述对象
  }

} // namespace client
} // namespace carla
