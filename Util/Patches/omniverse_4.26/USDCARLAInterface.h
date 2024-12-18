#pragma once

// 引入UE4中的UObject基类相关头文件，这是UE4中所有对象的基类，提供了诸如垃圾回收、反射等基础功能
#include "UObject/Object.h"
// 引入UE4的核心最小化头文件，包含了UE4基本类型、宏定义等常用的基础内容
#include "CoreMinimal.h"
// 这个头文件是用于支持UE4的代码生成系统，处理如反射相关的代码生成等功能，通常与UE4的蓝图系统结合使用
#include "USDCARLAInterface.generated.h"

// 使用USTRUCT宏定义一个结构体，标记为可以在蓝图中使用（BlueprintType），用于存储车辆轮子相关的数据
USTRUCT(BlueprintType)
struct FUSDCARLAWheelData
{
    GENERATED_BODY()
    // UPROPERTY宏用于将结构体中的成员变量暴露给UE4的蓝图系统，使其可以在蓝图中进行读写操作
    // 表示可以在蓝图中读写、在编辑器中任意编辑，所属分类为"USD Wheeel Data"，用于设置轮子的最大刹车扭矩
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")
    float MaxBrakeTorque = 0;
    // 同样可在蓝图中读写、在编辑器中任意编辑，所属分类为"USD Wheeel Data"，用于设置轮子的最大手刹扭矩
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")
    float MaxHandBrakeTorque = 0;
    // 可在蓝图中读写、在编辑器中任意编辑，所属分类为"USD Wheeel Data"，用于设置轮子的最大转向角度
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")
    float MaxSteerAngle = 0;
    // 可在蓝图中读写、在编辑器中任意编辑，所属分类为"USD Wheeel Data"，用于设置轮子的弹簧强度
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")
    float SpringStrength = 0;
    // 可在蓝图中读写、在编辑器中任意编辑，所属分类为"USD Wheeel Data"，用于设置轮子的最大压缩量
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")
    float MaxCompression = 0;
    // 可在蓝图中读写、在编辑器中任意编辑，所属分类为"USD Wheeel Data"，用于设置轮子的最大下垂量
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")
    float MaxDroop = 0;
    // 可在蓝图中读写、在编辑器中任意编辑，所属分类为"USD Wheeel Data"，用于设置轮子横向X方向的刚度
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")
    float LateralStiffnessX = 0;
    // 可在蓝图中读写、在编辑器中任意编辑，所属分类为"USD Wheeel Data"，用于设置轮子横向Y方向的刚度
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")
    float LateralStiffnessY = 0;
    // 可在蓝图中读写、在编辑器中任意编辑，所属分类为"USD Wheeel Data"，用于设置轮子纵向的刚度
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")
    float LongitudinalStiffness = 0;
};

// 使用USTRUCT宏定义一个结构体，标记为可以在蓝图中使用（BlueprintType），用于存储灯光相关的数据
USTRUCT(BlueprintType)
struct FUSDCARLALight
{
    GENERATED_BODY()
    // 可在蓝图中读写、在编辑器中任意编辑，所属分类为"USD Light"，用于存储灯光的名称
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Light")
    FString Name;
    // 可在蓝图中读写、在编辑器中任意编辑，所属分类为"USD Light"，用于存储灯光在三维空间中的位置，以FVector类型表示
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Light")
    FVector Location;
    // 可在蓝图中读写、在编辑器中任意编辑，所属分类为"USD Light"，用于存储灯光的颜色，以FLinearColor类型表示
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Light")
    FLinearColor Color;
};

// 使用UCLASS宏定义一个类，继承自UObject基类，属于OMNIVERSEUSD模块（命名空间相关概念，用于组织代码结构），该类作为与USD CARLA交互的接口类
class OMNIVERSEUSD_API UUSDCARLAInterface : public UObject
{
    GENERATED_BODY()

public:
    // 使用UFUNCTION宏定义一个静态函数，标记为可以在蓝图中调用（BlueprintCallable），所属分类为"USD Carla Interface"
    // 功能是导入USD文件，参数Path表示要导入的USD文件的路径，Dest表示导入的目标位置，
    // bImportUnusedReferences表示是否导入未使用的引用，bImportAsBlueprint表示是否作为蓝图导入，返回值表示导入是否成功
    UFUNCTION(BlueprintCallable, Category="USD Carla Interface")
    static bool ImportUSD(
        const FString& Path, const FString& Dest,
        bool bImportUnusedReferences, bool bImportAsBlueprint);
    // 使用UFUNCTION宏定义一个静态函数，标记为可以在蓝图中调用（BlueprintCallable），所属分类为"USD Carla Interface"
    // 功能是从指定路径的USD文件中获取灯光相关的数据，返回值是一个包含FUSDCARLALight结构体的数组，每个元素代表一个灯光的相关数据
    UFUNCTION(BlueprintCallable, Category="USD Carla Interface")
    static TArray<FUSDCARLALight> GetUSDLights(const FString& Path);
    // 使用UFUNCTION宏定义一个静态函数，标记为可以在蓝图中调用（BlueprintCallable），所属分类为"USD Carla Interface"
    // 功能是从指定路径的USD文件中获取车辆轮子相关的数据，返回值是一个包含FUSDCARLAWheelData结构体的数组，每个元素代表一个轮子的相关数据
    UFUNCTION(BlueprintCallable, Category="USD Carla Interface")
    static TArray<FUSDCARLAWheelData> GetUSDWheelData(const FString& Path);
};
