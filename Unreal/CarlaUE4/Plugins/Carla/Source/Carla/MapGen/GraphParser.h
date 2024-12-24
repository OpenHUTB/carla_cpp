// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 这是一个预处理指令，用于保证头文件只被包含一次，避免重复定义等问题
#pragma once  

// 包含 CityAreaDescription 头文件，可能定义了与城市区域描述相关的类、结构体、函数等内容
#include "CityAreaDescription.h"  
// 包含 GraphTypes 头文件，推测是定义了一些图相关的类型等
#include "GraphTypes.h"  
// 包含 RoadSegmentDescription 头文件，也许是关于路段描述方面的定义
#include "RoadSegmentDescription.h"  

// 包含标准库中 vector 头文件，用于使用 std::vector 容器
#include <vector>  

// 定义名为 MapGen 的命名空间，里面的类、函数等都在这个命名空间作用域内
namespace MapGen {  

    // 前置声明一个名为 DoublyConnectedEdgeList 的类，
    // 告诉编译器有这么一个类存在，但具体定义在后面或者其他地方，
    // 这里先允许使用该类的指针或引用等情况
    class DoublyConnectedEdgeList;  

    // 定义 GraphParser 类，它私有继承自 NonCopyable（可能是禁止拷贝构造和赋值操作的基类）
    class GraphParser : private NonCopyable
    {
    public:
        // 显式构造函数，接受一个 DoublyConnectedEdgeList 类型的引用作为参数，
        // 用于初始化该类对象与相关的 DoublyConnectedEdgeList 实例关联起来
        explicit GraphParser(DoublyConnectedEdgeList &Dcel);  

        // 成员函数，用于判断是否存在路段（RoadSegments），
        // 如果 RoadSegments 容器不为空则返回 true，否则返回 false
        bool HasRoadSegments() const {
            return!RoadSegments.empty();
        }

        // 成员函数，用于判断是否存在城市区域（CityAreas），
        // 如果 CityAreas 容器不为空则返回 true，否则返回 false
        bool HasCityAreas() const {
            return!CityAreas.empty();
        }

        // 成员函数，返回路段（RoadSegments）的数量，即 RoadSegments 容器的大小
        size_t RoadSegmentCount() const {
            return RoadSegments.size();
        }

        // 成员函数，返回城市区域（CityAreas）的数量，即 CityAreas 容器的大小
        size_t CityAreaCount() const {
            return CityAreas.size();
        }

        // 成员函数，返回指定索引位置 i 处的路段描述（RoadSegmentDescription）的常量引用，
        // 通过解引用存储在 RoadSegments 容器中对应位置的智能指针来获取
        const RoadSegmentDescription &GetRoadSegmentAt(size_t i) const {
            return *RoadSegments[i];
        }

        // 成员函数，返回指定索引位置 i 处的城市区域描述（CityAreaDescription）的常量引用，
        // 通过解引用存储在 CityAreas 容器中对应位置的智能指针来获取
        const CityAreaDescription &GetCityAreaAt(size_t i) const {
            return *CityAreas[i];
        }

        // 成员函数，从 RoadSegments 容器末尾取出一个路段描述（RoadSegmentDescription）的智能指针，
        // 先释放容器中最后一个元素（智能指针所管理对象的所有权），然后返回这个智能指针，
        // 同时容器大小减 1，相当于移除了末尾的元素
        TUniquePtr<RoadSegmentDescription> PopRoadSegment() {
            TUniquePtr<RoadSegmentDescription> ptr{RoadSegments.back().Release()};
            RoadSegments.pop_back();
            return ptr;
        }

        // 成员函数，从 CityAreas 容器末尾取出一个城市区域描述（CityAreaDescription）的智能指针，
        // 先释放容器中最后一个元素（智能指针所管理对象的所有权），然后返回这个智能指针，
        // 同时容器大小减 1，相当于移除了末尾的元素
        TUniquePtr<CityAreaDescription> PopCityArea() {
            TUniquePtr<CityAreaDescription> ptr{CityAreas.back().Release()};
            CityAreas.pop_back();
            return ptr;
        }

    private:
        // 定义一个类型别名 RoadSegmentList，它表示一个存储 TUniquePtr<RoadSegmentDescription> 类型智能指针的 vector 容器，
        // 用于管理路段描述的相关对象
        using RoadSegmentList = std::vector<TUniquePtr<RoadSegmentDescription>>;  

        // 定义一个类型别名 CityAreaList，它表示一个存储 TUniquePtr<CityAreaDescription> 类型智能指针的 vector 容器，
        // 用于管理城市区域描述的相关对象
        using CityAreaList = std::vector<TUniquePtr<CityAreaDescription>>;  

        // 定义一个 RoadSegmentList 类型的私有成员变量 RoadSegments，
        // 用于存储路段描述相关的智能指针，管理一系列路段相关的数据
        RoadSegmentList RoadSegments;  

        // 定义一个 CityAreaList 类型的私有成员变量 CityAreas，
        // 用于存储城市区域描述相关的智能指针，管理一系列城市区域相关的数据
        CityAreaList CityAreas;  
    };

} // namespace MapGen
