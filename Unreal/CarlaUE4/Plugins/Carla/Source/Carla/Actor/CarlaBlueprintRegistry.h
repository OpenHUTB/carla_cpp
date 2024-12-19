// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// 这是一个预处理指令，作用是确保该头文件在整个编译过程中只会被包含一次，
// 避免因多次包含同一头文件而导致的重复定义等编译错误。
#include "ActorBlueprintFunctionLibrary.h"
// 包含名为"ActorBlueprintFunctionLibrary.h"的头文件，通常这个头文件中会定义与Actor蓝图相关的函数库内容，
// 例如可能包含一些可以在蓝图中调用的通用函数等，是当前代码实现功能所依赖的外部声明所在的头文件之一。
#include "CarlaBlueprintRegistry.generated.h"
// 这是一个由虚幻引擎（Unreal Engine）的代码生成工具自动生成的头文件，里面包含了和当前类（UCarlaBlueprintRegistry）相关的一些元数据、反射信息等内容，
// 是实现虚幻引擎中类的一些高级特性（如蓝图功能支持等）所必需的。
UCLASS()
class UCarlaBlueprintRegistry : public UBlueprintFunctionLibrary
{ // 使用UCLASS宏定义了一个名为UCarlaBlueprintRegistry的类，并且表明这个类继承自UBlueprintFunctionLibrary类，
  // UCLASS宏是虚幻引擎中用于将一个C++类暴露给蓝图系统以及实现其他面向对象相关特性（如反射等）的关键宏，
  // 通过继承UBlueprintFunctionLibrary，意味着这个类可以在虚幻引擎的蓝图中被调用，用于扩展蓝图可使用的功能。
  GENERATED_BODY()
// GENERATED_BODY宏是虚幻引擎中与代码生成相关的宏，它告诉编译器这个类的一些成员函数、变量等相关的实现细节由代码生成工具自动生成，
  // 一般用于和自动生成的".generated.h"头文件配合，确保类的正确构建以及和虚幻引擎系统的正确交互。
public:

  UFUNCTION(Category = "Carla Blueprint Registry", BlueprintCallable)
  static void AddToCarlaBlueprintRegistry(const TArray<FPropParameters> &PropParametersArray);
// 同样使用UFUNCTION宏定义的另一个静态成员函数LoadPropDefinitions，具备和上面函数类似的蓝图相关属性（分类和可调用性）。
  // 其功能从函数名来看，大概是加载属性定义到给定的FPropParameters类型的数组中，也就是将一些预定义的属性相关信息填充到传入的数组参数中，
  // 同样具体的加载逻辑需要查看函数的实际实现代码以及与之相关的其他代码部分。
  UFUNCTION(Category = "Carla Blueprint Registry", BlueprintCallable)
  static void LoadPropDefinitions(TArray<FPropParameters> &PropParametersArray);
// 同样使用UFUNCTION宏定义的另一个静态成员函数LoadPropDefinitions，具备和上面函数类似的蓝图相关属性（分类和可调用性）。
  // 其功能从函数名来看，大概是加载属性定义到给定的FPropParameters类型的数组中，也就是将一些预定义的属性相关信息填充到传入的数组参数中，
  // 同样具体的加载逻辑需要查看函数的实际实现代码以及与之相关的其他代码部分。
};
