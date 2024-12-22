// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// 包含Carla/MapGen/CityMapDefinitions.h头文件，可能其中定义了与城市地图相关的一些常量、类型等内容
#include "Carla/MapGen/CityMapDefinitions.h"
// 包含Carla/Util/NonCopyable.h头文件，推测其中定义了不可复制相关的类或功能
#include "Carla/Util/NonCopyable.h"
// 引入标准库中的vector容器，用于存放动态大小的同类型元素集合
#include <vector>

namespace MapGen {
    // 如果定义了CARLA_ROAD_GENERATOR_EXTRA_LOG这个宏，以下结构体用于调试目的
    #ifdef CARLA_ROAD_GENERATOR_EXTRA_LOG
    /// 仅用于调试目的的模板结构体，用于给数据添加索引标识
    /// 它继承自NonCopyable，意味着不能进行拷贝操作（通过私有继承实现）
    template <char C>
    struct DataIndex : private NonCopyable
    {
        // 构造函数，初始化时给当前对象的索引赋值为自增后的索引值
        DataIndex() : index(++NEXT_INDEX) {}

        // 静态函数，用于重置索引值，将其置为0
        static void ResetIndex() {
            NEXT_INDEX = 0u;
        }

        // 重载输出流运算符，用于方便地将该结构体对象输出到流中，格式为特定字符加上索引值
        template <typename OSTREAM>
        friend OSTREAM &operator<<(OSTREAM &os, const DataIndex &d) {
            os << C << d.index;
            return os;
        }

    // private:
        // 用于记录当前对象的索引值，初始化为0
        uint32 index = 0u;
        // 静态成员变量，用于记录下一个可用的索引值，初始为0
        static uint32 NEXT_INDEX;
    };
    // 根据是否定义了CARLA_ROAD_GENERATOR_EXTRA_LOG宏来定义不同的继承方式
    #define INHERIT_GRAPH_TYPE_BASE_CLASS(c) : public DataIndex<c>
    #else
    #define INHERIT_GRAPH_TYPE_BASE_CLASS(c) : private NonCopyable
    #endif // CARLA_ROAD_GENERATOR_EXTRA_LOG

    // 定义表示图节点的结构体，继承方式由上面的宏决定（可能是继承DataIndex用于调试，或者直接继承NonCopyable）
    struct GraphNode INHERIT_GRAPH_TYPE_BASE_CLASS('n')
    {
        // 记录节点的边的数量
        uint32 EdgeCount;
        // 初始假设每个节点都是交叉路口，后续可能会根据实际情况改变，用于标识节点是否为交叉路口
        bool bIsIntersection = true; 
        // 交叉路口的类型，其类型应该在CityMapDefinitions.h中定义（推测）
        EIntersectionType IntersectionType;
        // 节点的旋转角度（可能是在地图中的角度相关信息）
        float Rotation;
        // 存储多个旋转相关的值的向量，具体用途需结合更多上下文确定
        std::vector<float> Rots;
    };

    // 定义表示图的半边（Half Edge）的结构体，同样继承方式由宏决定
    struct GraphHalfEdge INHERIT_GRAPH_TYPE_BASE_CLASS('e')
    {
        // 半边的角度信息，具体含义要看该图结构的应用场景
        float Angle;
    };

    // 定义表示图的面（Face）的结构体，继承方式由宏决定
    struct GraphFace INHERIT_GRAPH_TYPE_BASE_CLASS('f')
    {
    };

    // 取消之前定义的宏，避免后续代码出现意外的影响
    #undef INHERIT_GRAPH_TYPE_BASE_CLASS

} // namespace MapGen
