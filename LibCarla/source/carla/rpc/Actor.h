// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// 包含Carla相关的调试头文件，可能用于输出调试信息等功能
#include "carla/Debug.h"
// 包含用于表示几何边界框（Bounding Box）的头文件，通常用于定义物体在空间中的范围等
#include "carla/geom/BoundingBox.h"
// 包含用于描述Actor（在Carla模拟环境中可能指代各种实体对象，比如车辆、行人等）的相关信息的头文件
#include "carla/rpc/ActorDescription.h"
// 包含用于表示Actor唯一标识符（ID）的头文件
#include "carla/rpc/ActorId.h"
// 包含用于处理流相关的Token（令牌，可能在数据传输、流处理等场景使用）的头文件
#include "carla/streaming/Token.h"

#include <cstring>

namespace carla {
namespace rpc {

// 定义Actor类，在Carla的RPC（远程过程调用）机制相关场景下，该类大概率用于表示模拟环境中的各种可交互实体
class Actor {
public:
    // 默认构造函数，使用编译器默认生成的版本，不进行额外的初始化操作
    Actor() = default;

    // Actor的唯一标识符，用于在整个模拟环境中区分不同的Actor，初始化为0（可能后续会有赋值操作来赋予其真实的ID值）
    ActorId id = 0u;

    // 父Actor的ID，用于表示当前Actor所属的父级对象（例如在有层级关系的场景中，子物体关联到父物体的情况），初始化为0
    ActorId parent_id = 0u;

    // 用于描述Actor详细信息的结构体，包含诸如类型、属性等相关描述内容
    ActorDescription description;

    // 用于表示Actor在空间中的边界框信息，比如位置、大小范围等，可用于碰撞检测等功能
    geom::BoundingBox bounding_box;

    // 语义标签的向量，可能用于给Actor赋予特定的语义含义，方便进行分类、筛选等操作，例如区分不同类型的车辆或者行人等
    std::vector<uint8_t> semantic_tags;

    /// @todo This is only used by sensors actually.
    /// @name Sensor functionality
    /// @{
    // 存储流相关的Token数据，这里提到只被传感器实际使用，可能用于传感器数据的流传输时的标识、认证等功能
    std::vector<unsigned char> stream_token;

    // 函数用于判断当前Actor是否具有有效的流（通过检查流令牌的大小是否符合预期来判断）
    bool HasAStream() const {
        return stream_token.size() == sizeof(streaming::Token::data);
    }

    // 函数用于获取流令牌（Token），前提是已经通过HasAStream函数判断当前Actor存在有效的流。
    // 它通过内存复制的方式将存储的流令牌数据复制到新的Token对象中并返回该对象
    streaming::Token GetStreamToken() const {
        DEBUG_ASSERT(HasAStream());
        streaming::Token token;
        std::memcpy(&token.data[0u], stream_token.data(), stream_token.size());
        return token;
    }
    /// @}

    // 使用MSGPACK_DEFINE_ARRAY宏来定义Actor类在进行消息打包（可能用于网络传输、序列化等场景）时包含的成员变量
    MSGPACK_DEFINE_ARRAY(id, parent_id, description, bounding_box, semantic_tags, stream_token);
};

} // namespace rpc
} // namespace carla
