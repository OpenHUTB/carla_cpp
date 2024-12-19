// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.
// 这是一个预处理指令，用于确保头文件只被包含一次，避免重复定义等问题
#pragma once
// 包含核心最小化相关的头文件，通常包含了一些基础的UE4类型和宏等定义
#include "CoreMinimal.h"// 包含编辑器工具Actor相关的头文件，意味着这个类可能会继承自 AEditorUtilityActor，具备相关特性和功能
#include "EditorUtilityActor.h"// 包含编辑器工具Actor相关的头文件，意味着这个类可能会继承自 AEditorUtilityActor，具备相关特性和功能
#include "ProceduralBuildingUtilities.generated.h"
// 前置声明 USceneCaptureComponent2D 类和 UTexture2D 类，这样在后续代码中可以使用这两个类的指针类型，
// 但在真正使用这两个类的具体成员之前，需要在对应的源文件中包含它们完整的头文件
class USceneCaptureComponent2D;
class UTexture2D;
// 定义一个枚举类型 EBuildingCameraView，用于表示建筑物不同的相机视角方向，并且标记为可以在蓝图中使用的类型（BlueprintType）
UENUM(BlueprintType)
enum EBuildingCameraView
{
  FRONT,// 代表前方视角
  LEFT,// 代表左方视角
  BACK,// 代表后方视角
  RIGHT// 代表右方视角
};

/**
 * 
 */// 定义 AProceduralBuildingUtilities 类，它继承自 AEditorUtilityActor，意味着它可以作为一个在编辑器中使用的工具Actor，具备相应的功能和特性
// 这个类应该是和程序化建筑相关的工具类，用于生成、处理建筑相关的一些资源和操作
UCLASS()
class CARLATOOLS_API AProceduralBuildingUtilities : public AEditorUtilityActor
{
  GENERATED_BODY()

public:// 声明一个可在蓝图中实现的函数（BlueprintImplementableEvent），属于 "Procedural Building Utilities" 分类
    // 作用是将场景捕获的渲染目标（SceneCaptureRT）烘焙到纹理图集（TextureAtlas）中，
    // 参数包括要烘焙的纹理（SceneCaptureRT）以及目标的相机视角（TargetView）
  UFUNCTION(BlueprintImplementableEvent, Category="Procedural Building Utilities")
  void BakeSceneCaptureRTToTextureAtlas(UTexture2D* SceneCaptureRT, EBuildingCameraView TargetView);
// 声明一个可在蓝图中实现的函数（BlueprintImplementableEvent），属于 "Procedural Building Utilities" 分类
    // 作用是设置场景捕获组件（SceneCaptureComponent）的目标纹理，也就是指定场景捕获组件要将捕获的画面渲染到哪个纹理上
  UFUNCTION(BlueprintImplementableEvent, Category="Procedural Building Utilities")
  void SetTargetTextureToSceneCaptureComponent(USceneCaptureComponent2D* SceneCaptureComponent);
// 声明一个可以在蓝图中调用的函数（BlueprintCallable），属于 "Procedural Building Utilities" 分类
    // 作用是根据给定的建筑物尺寸（BuildingSize）生成一个替代纹理（Impostor Texture），
    // 这个替代纹理可能用于在一定距离下模拟建筑物外观等用途
  UFUNCTION(BlueprintCallable, Category="Procedural Building Utilities")
  void GenerateImpostorTexture(const FVector& BuildingSize);
// 声明一个可以在蓝图中调用的函数（BlueprintCallable），属于 "Procedural Building Utilities" 分类
    // 作用是根据给定的建筑物尺寸（BuildingSize）生成一个替代几何体（Impostor Geometry），
    // 这个几何体可能是一种简单的用于在一定条件下代表建筑物形状的网格体，返回值是对应的程序化网格组件（UProceduralMeshComponent）
  UFUNCTION(BlueprintCallable, Category="Procedural Building Utilities")
  class UProceduralMeshComponent* GenerateImpostorGeometry(const FVector& BuildingSize);
// 声明一个可以在蓝图中调用的函数（BlueprintCallable），属于 "Procedural Building Utilities" 分类
    // 作用是将程序化建筑（Procedural Building）烹饪（处理、转换等操作，可能涉及资源生成等）为一个网格文件，
    // 需要指定目标保存路径（DestinationPath）和文件名（FileName）
  UFUNCTION(BlueprintCallable, Category="Procedural Building Utilities")
  void CookProceduralBuildingToMesh(const FString& DestinationPath, const FString& FileName);
// 声明一个可以在蓝图中调用的函数（BlueprintCallable），属于 "Procedural Building Utilities" 分类
    // 作用是将给定的程序化网格组件（Mesh）代表的网格烹饪（类似上面的处理操作）为一个网格文件，
    // 需要指定目标保存路径（DestinationPath）和文件名（FileName）
  UFUNCTION(BlueprintCallable, Category="Procedural Building Utilities")
  void CookProceduralMeshToMesh(class UProceduralMeshComponent* Mesh, const FString& DestinationPath, const FString& FileName);
// 声明一个可以在蓝图中调用的函数（BlueprintCallable），属于 "Procedural Building Utilities" 分类
    // 作用是生成一个建筑材质资产（Building Material Asset），需要指定复制父材质的路径（DuplicateParentPath）、
    // 目标保存路径（DestinationPath）以及文件名（FileName），返回值是生成的材质实例常量（UMaterialInstanceConstant）
  UFUNCTION(BlueprintCallable, Category="Procedural Building Utilities")
  class UMaterialInstanceConstant* GenerateBuildingMaterialAsset(
      const FString& DuplicateParentPath,
      const FString& DestinationPath,
      const FString& FileName);

private:// 私有函数，用于渲染替代视图（Impostor View），通过给定的相机（Camera）、建筑物尺寸（BuildingSize）以及视图视角（View）来进行渲染操作
  void RenderImpostorView(USceneCaptureComponent2D* Camera, const FVector BuildingSize, const EBuildingCameraView View);
  void MoveCameraToViewPosition(USceneCaptureComponent2D* Camera, const FVector BuildingSize, const EBuildingCameraView View);
// 私有函数，用于根据给定的建筑物尺寸（BuildingSize）和视图视角（View）将相机移动到对应的视图位置，
    // 以便从正确的角度进行捕获、渲染等操作
  void CreateBuildingImpostorGeometryForView(
      class UProceduralMeshComponent* Mesh,
      const FVector& BuildingSize,
      const EBuildingCameraView View);
// 私有函数，用于从给定的建筑物尺寸（BuildingSize）和视图视角（View）获取宽度、深度以及视图角度信息，
    // 通过引用（OutWidth、OutDepth、OutViewAngle）返回获取到的值，方便后续根据这些信息进行相关计算和操作
  void GetWidthDepthFromView(
      const FVector& BuildingSize,
      const EBuildingCameraView View,
      float& OutWidth, float& OutDepth,
      float& OutViewAngle);
// 私有函数，用于根据建筑物的宽度（BuildingWidth）、高度（BuildingHeight）以及视图视角（View）计算视图几何体的UV坐标，
    // 将计算得到的UV坐标存储在传入的数组（OutUVs）中，UV坐标对于纹理映射等操作非常重要
  void CalculateViewGeometryUVs(
      const float BuildingWidth,
      const float BuildingHeight,
      const EBuildingCameraView View,
      TArray<FVector2D>& OutUVs);

};
