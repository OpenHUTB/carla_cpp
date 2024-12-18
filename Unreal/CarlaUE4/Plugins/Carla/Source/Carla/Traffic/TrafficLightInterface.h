// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 这是一个头文件保护指令，用于防止头文件被重复包含。
// 一般来说，在整个项目的编译过程中，同一个头文件只会被编译一次。
#pragma once

// 引入UE4的核心最小化头文件，包含了一些基础的类型、宏等常用定义，是UE4编程中常用的基础头文件。
#include "CoreMinimal.h"
// 引入一个名为“Object”的头文件，推测可能是定义了一些基础对象相关的类或者结构体等内容，具体要看对应项目中的实现。
#include "Object.h"
// 引入“TrafficLightState”头文件，大概率是用于定义交通灯状态相关的枚举或者结构体等，与交通灯状态的表示有关。
#include "TrafficLightState.h"
// 引入用于生成虚幻引擎蓝图类型接口的宏等相关定义，表明下面定义的接口可以在蓝图中使用。
#include "TrafficLightInterface.generated.h"

// 使用UINTERFACE宏定义一个名为UTrafficLightInterface的接口类，这个接口在UE4的蓝图系统中可用（BlueprintType表示其可用于蓝图）。
// 它继承自UInterface，UInterface是UE4中用于定义接口的基类。
// CARLA_API可能是用于控制该接口的导出等相关属性的宏定义（具体取决于项目中对其的实现）。
UINTERFACE(BlueprintType)
class CARLA_API UTrafficLightInterface : public UInterface
{
  // 这是UE4用于生成接口类代码的宏，一般不用手动修改里面的内容，由UE4的代码生成机制来处理。
  GENERATED_BODY()
};

// 定义实现UTrafficLightInterface接口的类的具体接口方法，遵循UE4接口实现的规范。
// 这个类定义了接口具体需要实现的函数等内容，具体的类可以继承该接口并实现这里定义的方法。
class ITrafficLightInterface
{
  // 同样是UE4用于生成代码的宏，遵循其代码生成规则，不用手动调整内部逻辑。
  GENERATED_BODY()

public:
  // 使用UFUNCTION宏声明一个函数，该函数可以在蓝图中实现（BlueprintImplementableEvent表示蓝图可实现此函数），
  // 同时也可以在C++代码中被调用（BlueprintCallable表示可在蓝图中调用）。
  // 函数所属的类别被标记为“Traffic Light Interface”，方便在蓝图等编辑器中进行分类查找。
  // 这个函数名为LightChanged，它接收一个ETrafficLightState类型的参数NewLightState，
  // 推测是用于当交通灯状态发生改变时，通知相关对象交通灯状态已经更新了。
  UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Traffic Light Interface")
  void LightChanged(ETrafficLightState NewLightState);
};
