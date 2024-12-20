// 版权信息
// 版权所有 (c) 2017 巴塞罗那自治大学 (UAB) 计算机视觉中心 (CVC)。
//
// 本工作根据 MIT 许可证的条款进行授权。
// 许可证副本见 <https://opensource.org/licenses/MIT>。

#pragma once

// 引入游戏框架中Actor相关的头文件，表明这个类可能继承自Actor或者与之相关
#include "GameFramework/Actor.h"
// 引入城市地图网格标签相关的头文件，可能用于标识不同类型的地图网格元素
#include "CityMapMeshTag.h"
// 引入一个生成的头文件，通常用于UE4中的反射相关功能（由UE4的代码生成工具生成）
#include "CityMapMeshHolder.generated.h"

// 前向声明一些类，告知编译器这些类在后续会被使用，但暂时不需要知道它们的具体定义，有助于减少编译依赖和提高编译速度
class IDetailLayoutBuilder;
class UInstancedStaticMeshComponent;
class AStaticMeshActor;

// 定义一个名为ACityMapMeshHolder的类，它继承自AActor类，并且使用了UE4的反射相关宏来使其可以被UE4的系统识别和处理
// UCLASS(Abstract)表示这个类是抽象类，意味着不能直接实例化它，通常用于定义一些通用的行为和接口供子类实现
UCLASS(Abstract)
class CARLA_API ACityMapMeshHolder : public AActor
{
  GENERATED_BODY()

  // ===========================================================================
  // -- Construction and update related methods --------------------------------
  // ===========================================================================
public:
  // 构造函数，接受一个FObjectInitializer类型的参数，用于初始化对象的相关属性
  // 调用这个构造函数初始化网格持有者后，可以安全地调用SetStaticMesh函数来设置静态网格，但在调用OnConstruction函数之前不能添加实例
  ACityMapMeshHolder(const FObjectInitializer& ObjectInitializer);

  // 函数定义被注释掉了，原本可能用于布局细节相关的操作（具体功能需要结合上下文和实际使用场景确定）
  //void LayoutDetails( IDetailLayoutBuilder& DetailLayout );

protected:
  // 虚函数，当对象在构造过程中（例如在编辑器中改变了属性等情况触发重新构建时）会被调用，用于初始化实例化相关的操作
  // 通过override关键字表明这是重写了父类（AActor）中的同名虚函数
  virtual void OnConstruction(const FTransform &Transform) override;

  // 虚函数，在组件初始化完成后会被调用，可用于执行一些依赖于组件初始化后的额外操作，同样是重写了父类中的同名虚函数
  virtual void PostInitializeComponents() override;

#if WITH_EDITOR
  // 仅在编辑器环境下生效的虚函数，当编辑过程中某个属性发生改变时会被调用，用于清除和更新实例化相关的内容，也是重写父类函数
  virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

  // ===========================================================================
  // -- Other protected methods ------------------------------------------------
  // ===========================================================================
protected:
  // 获取地图的缩放比例，是一个常量成员函数，返回当前对象的MapScale属性值，该属性表示地图的缩放程度
  float GetMapScale() const
  {
    return MapScale;
  }

  // 根据给定的二维瓦片坐标（X和Y），返回对应的三维世界位置（相对于这个Actor而言），具体计算逻辑可能在函数实现中定义
  FVector GetTileLocation(uint32 X, uint32 Y) const;

  // 设置与给定标签（ECityMapMeshTag类型）相关联的静态网格（UStaticMesh类型），用于指定不同标签对应的网格模型
  void SetStaticMesh(ECityMapMeshTag Tag, UStaticMesh *Mesh);

  // 根据给定的标签（ECityMapMeshTag类型），返回对应的静态网格（UStaticMesh类型）指针，如果不存在可能返回nullptr，用于获取特定标签对应的网格模型
  UStaticMesh *GetStaticMesh(ECityMapMeshTag Tag);

  // 与上面的GetStaticMesh函数类似，但这个是常量版本，用于在不修改对象状态的情况下获取对应标签的静态网格，返回的指针指向的内容不能通过该函数调用修改
  const UStaticMesh *GetStaticMesh(ECityMapMeshTag Tag) const;

  // 根据给定的静态网格（UStaticMesh类型），返回对应的标签（ECityMapMeshTag类型），用于反向查找标签
  ECityMapMeshTag GetTag(const UStaticMesh &StaticMesh) const;

  // 添加一个具有给定瓦片位置的网格实例，需要指定网格的标签（ECityMapMeshTag类型）以及瓦片坐标（X和Y），用于在地图中添加特定网格的实例
  void AddInstance(ECityMapMeshTag Tag, uint32 X, uint32 Y);

  // 添加一个具有给定瓦片位置和绕Z轴旋转角度的网格实例，相比上面的AddInstance函数多了一个旋转角度参数，用于放置有特定旋转角度的网格实例
  void AddInstance(ECityMapMeshTag Tag, uint32 X, uint32 Y, float Angle);

  // 添加一个具有给定变换（FTransform类型，包含位置、旋转、缩放等信息）的网格实例，通过指定完整的变换信息来放置网格实例
  void AddInstance(ECityMapMeshTag Tag, FTransform Transform);

  // ===========================================================================
  // -- Private methods and members --------------------------------------------
  // ===========================================================================
private:
  // 虚函数，在这里没有实际实现内容（函数体为空），需要在派生类中去具体实现，可能用于更新地图相关的操作，具体功能由子类根据需求定义
  virtual void UpdateMap();

  // 清除所有静态网格Actor的实例，用于清理已存在的网格实例，可能在重新生成地图或者重置地图状态等场景下使用
  void DeletePieces();

  // 根据基础网格的尺寸来设置缩放比例，用于更新地图的缩放程度，使其适配基础网格的大小
  void UpdateMapScale();

private:
  // 定义一个场景根组件（USceneComponent类型），用于作为场景中的根节点，其他组件可以挂载在它下面，通过UPROPERTY宏进行属性声明，便于UE4的反射系统管理
  UPROPERTY()
  USceneComponent *SceneRootComponent;

  // 定义一个表示地图缩放比例的浮点型属性，初始值为1.0f，属于“Map Generation”类别，在编辑器中任何地方可见（可查看但不一定可编辑，取决于具体使用情况）
  UPROPERTY(Category = "Map Generation", VisibleAnywhere)
  float MapScale = 1.0f;

  // 定义一个映射表（TMap类型），用于存储不同城市地图网格标签（ECityMapMeshTag类型）与对应的静态网格（UStaticMesh *类型）之间的关联，通过UPROPERTY宏声明，可在“Meshes”类别下编辑
  UPROPERTY(Category = "Meshes", EditAnywhere)
  TMap<ECityMapMeshTag, UStaticMesh *> StaticMeshes;

  // 定义一个反向映射表（TMap类型），用于存储静态网格（UStaticMesh *类型）与对应的城市地图网格标签（ECityMapMeshTag类型）之间的关联，通过UPROPERTY宏声明
  UPROPERTY()
  TMap<UStaticMesh *, ECityMapMeshTag> TagMap;
};
