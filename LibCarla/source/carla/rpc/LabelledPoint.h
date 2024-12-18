// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once //防止头文件多次包含

#include "carla/MsgPack.h"
#include "carla/rpc/Location.h" //引入位置相关头文件
#include "carla/rpc/ObjectLabel.h" //引入定义城市标签相关头文件

namespace carla {
namespace rpc {

  struct LabelledPoint { //定义一个结构体，用于表示带标签的点
    //构造函数
    LabelledPoint () {}
    LabelledPoint (Location location, CityObjectLabel label)
     : _location(location), _label(label)
     {}

    Location _location; //表示点的位置，类型为Location

    CityObjectLabel _label; //表示点的标签，类型为CityObjectLabel

    MSGPACK_DEFINE_ARRAY(_location, _label); //使用MsgPack库定义该结构体的序列化格式，便于网络传输或持久化存储

  };

}
}
