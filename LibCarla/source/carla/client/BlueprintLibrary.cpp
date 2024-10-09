// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/BlueprintLibrary.h"

#include "carla/Exception.h"

#include <algorithm>
#include <iterator>

namespace carla {
namespace client {
//构造函数：使用给定的蓝图列表初始化 BlueprintLibary
  BlueprintLibrary::BlueprintLibrary(
      const std::vector<rpc::ActorDefinition> &blueprints) {
    _blueprints.reserve(blueprints.size()); //预留空间以存储蓝图
    for (auto &definition : blueprints) {
      _blueprints.emplace(definition.id, ActorBlueprint{definition});
      //将蓝图添加到映射中
    }
  }
//根据通配符模式过滤蓝图，返回匹配的 BlueprintLibrary
  SharedPtr<BlueprintLibrary> BlueprintLibrary::Filter(
      const std::string &wildcard_pattern) const {
    map_type result; //存储过滤结果的映射
    for (auto &pair : _blueprints) {
      if (pair.second.MatchTags(wildcard_pattern)) { //检查蓝图是否匹配通配符模式
        result.emplace(pair); //如果匹配，则将其添加到结果中
      }
    }
    return SharedPtr<BlueprintLibrary>{new BlueprintLibrary(result)};
    //返回过滤后的蓝图库
  }
//根据指定属性名称和值过滤蓝图，返回匹配的 BlueprintLibrary
  SharedPtr<BlueprintLibrary> BlueprintLibrary::FilterByAttribute(
      const std::string &name, const std::string& value) const {
    map_type result; //存储过滤结果的映射

    for (auto &pair : _blueprints) {
      if (!pair.second.ContainsAttribute(name))
        continue; //如果蓝图不包含指定的属性，则跳过
      const ActorAttribute &Attribute = pair.second.GetAttribute(name); //获取指定的属性
      const std::vector<std::string> &Values = Attribute.GetRecommendedValues();//获取推荐值
      if (Values.empty()) //如果值为空
      {
        const std::string &AttributeValue = Attribute.GetValue(); //获取属性当前值
        if (value == AttributeValue) //如果值匹配
          result.emplace(pair); //将蓝图添加到结果中
      }
      else
      {
        for (const std::string &Value : Values) //如果有推荐值，逐个排查
        {
          if (Value == value) //如果找到匹配的推荐值
          {
            result.emplace(pair); //将蓝图添加到结果中
            break; //退出循环
          }
        }
      }

    }
    return SharedPtr<BlueprintLibrary>{new BlueprintLibrary(result)}; //返回过滤后的蓝图库
  }

  BlueprintLibrary::const_pointer BlueprintLibrary::Find(const std::string &key) const {
    auto it = _blueprints.find(key);
    return it != _blueprints.end() ? &it->second : nullptr;
  }

  BlueprintLibrary::const_reference BlueprintLibrary::at(const std::string &key) const {
    auto it = _blueprints.find(key);
    if (it == _blueprints.end()) {
      using namespace std::string_literals;
      throw_exception(std::out_of_range("blueprint '"s + key + "' not found"));
    }
    return it->second;
  }

  BlueprintLibrary::const_reference BlueprintLibrary::at(size_type pos) const {
    if (pos >= size()) {
      throw_exception(std::out_of_range("index out of range"));
    }
    return operator[](pos);
  }

} // namespace client
} // namespace carla
