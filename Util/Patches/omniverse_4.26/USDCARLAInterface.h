// 这是一个预处理指令，用于确保该头文件只会被包含一次，避免重复定义等问题
#pragma once  

// 包含UObject相关的头文件，UObject是虚幻引擎中用于实现对象系统基础功能的类
#include "UObject/Object.h"  
// 包含核心最小化相关的头文件，包含了一些常用的基础类型等定义
#include "CoreMinimal.h"  
// 这是一个虚幻引擎的宏，用于生成特定的代码，比如反射相关的代码等，由虚幻引擎的代码生成系统处理
#include "USDCARLAInterface.generated.h"  

// 使用USTRUCT宏定义一个结构体，并且标记为可在蓝图中使用的类型（BlueprintType）
// 该结构体用于存储车辆轮子相关的数据信息
USTRUCT(BlueprintType)  
struct FUSDCARLAWheelData
{
    GENERATED_BODY()  
    // 使用UPROPERTY宏声明一个属性，该属性可在蓝图中读写，可在编辑器中任意编辑，并且所属类别为"USD Wheeel Data"
    // 表示轮子的最大刹车扭矩，初始值为0
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")  
    float MaxBrakeTorque = 0;  
    // 表示轮子的最大手刹扭矩，初始值为0
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")  
    float MaxHandBrakeTorque = 0;  
    // 表示轮子的最大转向角度，初始值为0
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")  
    float MaxSteerAngle = 0;  
    // 表示悬挂弹簧的强度，初始值为0
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")  
    float SpringStrength = 0;  
    // 表示悬挂的最大压缩量，初始值为0
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")  
    float MaxCompression = 0;  
    // 表示悬挂的最大下垂量，初始值为0
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")  
    float MaxDroop = 0;  
    // 表示横向刚度（X方向），初始值为0
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")  
    float LateralStiffnessX = 0;  
    // 表示横向刚度（Y方向），初始值为0
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")  
    float LateralStiffnessY = 0;  
    // 表示纵向刚度，初始值为0
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")  
    float LongitudinalStiffness = 0;  
};

// 使用USTRUCT宏定义一个结构体，并且标记为可在蓝图中使用的类型（BlueprintType）
// 该结构体用于存储灯光相关的数据信息
USTRUCT(BlueprintType)  
struct FUSDCARLALight
{
    GENERATED_BODY()  
    // 灯光的名称，可在蓝图中读写、在编辑器中任意编辑，所属类别为"USD Light"
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Light")  
    FString Name;  
    // 灯光在三维空间中的位置，可在蓝图中读写、在编辑器中任意编辑，所属类别为"USD Light"
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Light")  
    FVector Location;  
    // 灯光的颜色，使用FLinearColor类型表示，可在蓝图中读写、在编辑器中任意编辑，所属类别为"USD Light"
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Light")  
    FLinearColor Color;  
};

// 使用UCLASS宏定义一个类，该类继承自UObject，并且属于OMNIVERSEUSD命名空间
// 此类作为与USD和CARLA相关的接口类，提供了一些静态函数用于导入USD数据以及获取USD中灯光、车轮等相关数据
UCCLASS()  
class OMNIVERSEUSD_API UUSDCARLAInterface : public UObject
{
    GENERATED_BODY()  

public:
    // 使用UFUNCTION宏定义一个静态函数，该函数可在蓝图中被调用，所属类别为"USD Carla Interface"
    // 用于从指定路径导入USD文件到目标路径，可选择是否导入未使用的引用以及是否作为蓝图导入
    UFUNCTION(BlueprintCallable, Category="USD Carla Interface")  
    static bool ImportUSD(
        const FString& Path, const FString& Dest,
        bool bImportUnusedReferences, bool bImportAsBlueprint);  

    // 使用UFUNCTION宏定义一个静态函数，该函数可在蓝图中被调用，所属类别为"USD Carla Interface"
    // 用于从指定的USD文件路径中获取所有灯光相关的数据信息，并以数组形式返回
    UFUNCTION(BlueprintCallable, Category="USD Carla Interface")  
    static TArray<FUSDCARLALight> GetUSDLights(const FString& Path);  

    // 使用UFUNCTION宏定义一个静态函数，该函数可在蓝图中被调用，所属类别为"USD Carla Interface"
    // 用于从指定的USD文件路径中获取所有车轮相关的数据信息，并以数组形式返回
    UFUNCTION(BlueprintCallable, Category="USD Carla Interface")  
    static TArray<FUSDCARLAWheelData> GetUSDWheelData(const FString& Path);  
};
