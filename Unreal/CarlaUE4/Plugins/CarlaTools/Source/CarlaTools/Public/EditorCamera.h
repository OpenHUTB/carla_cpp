// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//版权声明部分，表明这段代码的版权归属于巴塞罗那自治大学的计算机视觉中心（CVC），清晰界定了版权归属主体。
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
// 说明了该作品遵循MIT许可证的条款进行授权，同时给出了获取许可证副本的网址，方便使用者查看具体的许可要求，
// 意味着使用这段代码需要遵循MIT许可证所规定的相关规则，比如保留版权声明等。
#pragma once// 这是一个预处理器指令，它的作用是确保该头文件在整个编译过程中只会被编译一次，避免因多次被包含而出现重复定义等编译问题，
// 以此保证编译过程的顺利进行以及代码逻辑的正确性。
#include "CoreMinimal.h"// 引入UE4的核心基础头文件，这个头文件中包含了大量UE4开发中常用的基础内容，例如基本的数据类型定义（像FString、FVector等）、
// 常用的宏以及一些底层的、通用的功能函数等，是构建UE4项目中其他各类模块和类的重要基础支撑，很多功能实现都会依赖于此。
#include "EditorCamera.generated.h"// 这是UE4代码生成相关的头文件，它与UE4的反射系统配合使用，能够生成一些必要的代码来支持类的序列化、蓝图相关操作等功能，
// 使得定义的类可以在UE4的蓝图环境下方便地进行属性编辑、函数调用以及与其他蓝图或C++代码进行交互等操作




UCLASS(BlueprintType)
class CARLATOOLS_API AEditorCameraUtils :
    public 
    GENERATED_BODY()// 这是UE4中用于标记类的可生成代码部分的宏，借助UE4的代码生成机制，通过这个宏实现类的反射等相关特性，
    // 进而让类能够在蓝图环境下展现出可编辑属性、可调用函数等功能，方便开发人员进行可视化编程和功能拓展。
public:
// 以下是类的公有成员函数，公有成员函数可以被外部的其他类或者蓝图进行访问，用于向外提供类的功能接口。
    UFUNCTION(BlueprintCallable, CallInEditor)
    void Get();
// 声明一个可以在蓝图中调用（通过BlueprintCallable修饰）并且可以在编辑器环境下执行（通过CallInEditor修饰）的函数Get，
    // 虽然目前从代码中看不出具体获取什么内容，但推测其功能可能是获取与编辑器相机相关的某些属性、状态或者对象等信息，
    // 具体的实现逻辑应该在对应的.cpp文件中定义，外部代码（如蓝图脚本）可以调用该函数来获取所需的相机相关数据。
    UFUNCTION(BlueprintCallable, CallInEditor)
    void Set();
// 声明一个同样可在蓝图中调用（BlueprintCallable修饰）且能在编辑器环境下执行（CallInEditor修饰）的函数Set，
    // 结合函数名推测其功能大概率是用于设置与编辑器相机相关的某些属性、状态或者对象等内容，
    // 具体要设置的内容以及相关操作的实现会在对应的.cpp文件中明确，外部代码可通过该函数来改变相机相关的相关设定。


    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FTransform CameraTransform;
// 这是一个使用UPROPERTY宏定义的类的属性，通过BlueprintReadWrite修饰，表示该属性在蓝图环境下既可以被读取也可以被写入操作，
    // 而EditAnywhere修饰则意味着这个属性可以在虚幻编辑器的属性窗口中进行编辑修改，方便开发人员在编辑器里直观地调整属性值。
    // 该属性的类型是FTransform，用于表示变换信息（如位置、旋转、缩放等），在这里推测是用于存储编辑器相机的变换相关信息，
    // 比如相机在场景中的位置、朝向以及缩放比例等，外部代码（如蓝图脚本或者其他类）可以通过访问该属性来获取或修改相机的这些状态。
};
