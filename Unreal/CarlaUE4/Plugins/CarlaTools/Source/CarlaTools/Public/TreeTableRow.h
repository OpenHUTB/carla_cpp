// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
// #pragma once 是一种预处理指令，用于防止头文件被重复包含，确保在编译过程中，该头文件的内容只会被编译一次，避免出现重复定义等编译错误。
#include "CoreMinimal.h"// CoreMinimal.h 是 Unreal Engine 中的一个基础头文件，包含了众多核心的类型定义、宏以及常用的基础功能相关的声明等，
// 几乎所有在 Unreal Engine 中开发的模块都需要包含它来获取这些基础支持。
#include "Engine/DataTable.h"// 引入 Engine/DataTable.h 头文件，因为后续定义的结构体 FTreeTableRow 可能会与虚幻引擎中的数据表格（DataTable）功能相关联，
// 例如可能用于在数据表格中存储特定类型的数据，该头文件提供了相关的数据表格操作以及数据存储的基础类等内容。
#include "TreeTableRow.generated.h"// TreeTableRow.generated.h 是由虚幻引擎的代码生成工具生成的头文件，包含了与代码生成相关的宏等内容，
// 用于支持当前类和结构体的反射等机制，使得它们能够在虚幻引擎的蓝图系统、序列化等功能中正常工作。

/**
 *
 */

UENUM(BlueprintType)
enum class ELaneDescriptor : uint8 {// None 枚举值，表示没有特定的车道描述，初始值设为 0x0，常用于初始化或表示不存在有效车道描述的情况。
  None          = 0x0,// Town 枚举值，表示车道属于城镇类型，其值通过位运算（0x1 << 0）设置，可用于标记与城镇环境相关的车道特征。
  Town          = 0x1 << 0,// Motorway 枚举值，代表车道属于高速公路类型，同样通过位运算（0x1 << 1）得到对应的值，用于区分高速公路场景下的车道。
  Motorway      = 0x1 << 1,// Rural 枚举值，意味着车道处于乡村环境，由位运算（0x1 << 2）确定其值，便于在处理乡村道路相关车道时进行标识。
  Rural         = 0x1 << 2,// Any 枚举值，设置为 255（十六进制表示为 0xFE），可用于表示任意类型的车道，在一些需要涵盖所有车道可能性的场景中使用。
  Any           = 255 // 0xFE
};

USTRUCT(BlueprintType)
struct FTreeTableRow : public FTableRowBase {
// GENERATED_BODY() 宏用于支持虚幻引擎的代码生成机制，像反射、序列化等功能都依赖于此宏来正确处理该结构体相关的逻辑，
    // 确保结构体能够在虚幻引擎的蓝图系统以及其他相关功能中正常工作。
  GENERATED_BODY()

public:
// UPROPERTY 宏用于将成员变量暴露给虚幻引擎的蓝图系统以及编辑器，使得这些变量可以在蓝图中进行读写操作，并且在编辑器中进行相应的配置。
    // EditAnywhere 表示该属性可以在虚幻引擎编辑器的任何地方进行编辑修改，BlueprintReadWrite 说明它既能在蓝图中读取也能写入，
    // Category = "Meshes" 则指定了该属性在编辑器中的所属分类为“Meshes”，便于在编辑器界面上进行分类展示和查找。
    // Meshes 成员变量是一个数组，数组元素类型为 TSoftObjectPtr<UStaticMesh>，它用于存储指向静态网格（UStaticMesh）的软对象指针数组，
    // 软对象指针意味着这些资源可能不会立即加载到内存中，而是根据实际需求动态加载，常用于存储一组相关的静态网格资源，
    // 比如可能用于在特定的数据表格行中关联多个不同的静态网格模型。
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Meshes")
  TArray<TSoftObjectPtr<UStaticMesh>> Meshes;
// 类似地，此 UPROPERTY 宏定义的属性也具备在编辑器任意处编辑、在蓝图中可读写的特性，所属分类为“Blueprints”。
    // Blueprints 成员变量同样是一个数组，其元素类型为 TSoftClassPtr<AActor>，用于保存指向演员类（AActor）的软类指针数组，
    // 软类指针常用于在运行时动态加载演员类，例如可以通过这些指针根据不同的情况创建对应的演员实例，
    // 这里可能用于在数据表格行中关联多个不同的演员蓝图相关的类信息，方便后续在游戏逻辑中基于这些蓝图创建具体的演员对象。
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprints")
  TArray<TSoftClassPtr<AActor>> Blueprints;

};
