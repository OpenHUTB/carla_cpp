
#pragma once

#include "UObject/Object.h"
#include "CoreMinimal.h"
#include "USDCARLAInterface.generated.h"

USTRUCT(BlueprintType)
// 定义名为 FUSDCARLAWheelData 的结构体
struct FUSDCARLAWheelData
{
  GENERATED_BODY()
 // 这是一个由虚幻引擎的代码生成系统使用的宏，用于自动生成结构体相关的必要代码
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")
  float MaxBrakeTorque = 0;#定义了最大的手刹扭距成员变量，为float类型，用于表示车辆车轮手刹能施加的最大扭矩值
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")
  // 定义一个名为 MaxBrakeTorque 的浮点型（float）成员变量，初始化为0，
   // 用于表示车辆车轮手刹能施加的最大扭矩值
  float MaxHandBrakeTorque = 0;
 // 同样使用 UPROPERTY 宏声明属性，在蓝图中可读可写、可在编辑器任意位置编辑，所属分类为 "USD Wheeel Data"
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")
 // 同样使用 UPROPERTY 宏声明属性，在蓝图中可读可写、可在编辑器任意位置编辑，所属分类为 "USD Wheeel Data"
  float MaxSteerAngle = 0;
 // 再次使用 UPROPERTY 宏声明属性，具有蓝图可读可写、可编辑等特性，分类为 "USD Wheeel Data"
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")
// 定义名为 MaxSteerAngle 的浮点型成员变量，初始化为0，可能用于表示车轮最大转向角度
  float SpringStrength = 0;
 // 使用 UPROPERTY 宏声明属性，在蓝图中操作及编辑相关设定如前，分类为 "USD Wheeel Data"
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")
// 定义名为 SpringStrength 的浮点型成员变量，初始化为0，也许与车轮弹簧强度相关
  float MaxCompression = 0;
 // 依旧通过 UPROPERTY 宏声明属性，属性特性不变，所属分类为 "USD Wheeel Data"
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")
// 定义名为 MaxCompression 的浮点型成员变量，初始化为0，可能表示车轮最大压缩量相关的值
  float MaxDroop = 0;
// 使用 UPROPERTY 宏声明属性，具备相应蓝图操作和编辑特性，分类为 "USD Wheeel Data"
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")
// 定义名为 MaxDroop 的浮点型成员变量，初始化为0，或许和车轮下垂相关的最大量之类的含义
  float LateralStiffnessX = 0;
 // 通过 UPROPERTY 宏声明属性，符合之前提到的蓝图及编辑相关设定，分类为 "USD Wheeel Data"
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")
// 定义名为 LateralStiffnessX 的浮点型成员变量，初始化为0，可能涉及车轮横向刚度在 X 方向的值
  float LateralStiffnessY = 0;
// 继续使用 UPROPERTY 宏声明属性，相关特性一致，分类为 "USD Wheeel Data"
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Wheeel Data")
// 定义名为 LateralStiffnessY 的浮点型成员变量，初始化为0，也许是车轮横向刚度在 Y 方向的值
  float LongitudinalStiffness = 0;
// 使用 USTRUCT 宏定义结构体，标记该结构体可在蓝图中使用（BlueprintType）
};

USTRUCT(BlueprintType)
// 定义名为 FUSDCARLALight 的结构体
struct FUSDCARLALight
{
  GENERATED_BODY()
// 由虚幻引擎代码生成系统使用的宏，用于自动生成结构体相关必要代码，如默认构造、析构函数等部分

    // 使用 UPROPERTY 宏声明属性，在蓝图中可读可写、可在编辑器任意位置编辑，所属分类为 "USD Light"
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Light")
// 定义名为 Name 的 FString 类型成员变量（FString 通常用于表示字符串类型，在虚幻引擎中常用），
  FString Name;
 // 利用 UPROPERTY 宏声明属性，具备蓝图操作和编辑相关特性，分类为 "USD Light"
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Light")
 // 定义名为 Location 的 FVector 类型成员变量（FVector 用于表示三维向量，在虚幻引擎中常用于表示位置等信息），
    // 用于存储灯光的位置信息
    FVector Location;
  FVector Location;
// 使用 UPROPERTY 宏声明属性，具有相应蓝图和编辑属性设定，分类为 "USD Light"
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="USD Light")
// 定义名为 Color 的 FLinearColor 类型成员变量（FLinearColor 用于表示线性颜色，在虚幻引擎中用于颜色相关操作），
    // 用于存储灯光的颜色信息
  FLinearColor Color;
};

// 使用 UCLASS 宏定义一个类，表明这是一个可以被虚幻引擎识别和管理的类
UCLASS()
// 定义名为 UUSDCARLAInterface 的类，它继承自 UObject（UObject 是虚幻引擎中大多数类的基类，提供了很多基础功能）
class OMNIVERSEUSD_API UUSDCARLAInterface : public UObject
{
  GENERATED_BODY()
// 虚幻引擎代码生成系统使用的宏，用于自动生成类相关的必要代码，比如默认构造函数、析构函数等默认实现部分


public:
 // 使用 UFUNCTION 宏声明一个函数，该函数可以在蓝图中被调用（BlueprintCallable），所属分类为 "USD Carla Interface"
  UFUNCTION(BlueprintCallable, Category="USD Carla Interface")
// 声明一个名为 ImportUSD 的静态函数（static 表示不需要实例化类对象就可以调用），
    // 接收两个 FString 类型的常量引用参数（分别表示文件路径和目标路径之类的含义），
    // 以及两个布尔类型的参数（用于控制是否导入未使用的引用以及是否作为蓝图导入等相关操作），
    // 返回值为布尔类型，表示导入操作是否成功
  static bool ImportUSD(
      const FString& Path, const FString& Dest,
      bool bImportUnusedReferences, bool bImportAsBlueprint);
// 使用 UFUNCTION 宏声明函数，可在蓝图中调用，分类为 "USD Carla Interface"
  UFUNCTION(BlueprintCallable, Category="USD Carla Interface")
// 声明一个名为 GetUSDLights 的静态函数，接收一个 FString 类型的常量引用参数（可能表示文件路径），
    // 返回值是 TArray<FUSDCARLALight> 类型（TArray 是虚幻引擎中的动态数组类型，这里表示存储 FUSDCARLALight 结构体的数组，
    // 即返回一个包含多个 FUSDCARLALight 结构体的数组，可能用于获取指定路径下的灯光相关数据）
  static TArray<FUSDCARLALight> GetUSDLights(const FString& Path);
 // 使用 UFUNCTION 宏声明函数，可在蓝图中调用，分类为 "USD Carla Interface"
  UFUNCTION(BlueprintCallable, Category="USD Carla Interface")
// 声明一个名为 GetUSDWheelData 的静态函数，接收一个 FString 类型的常量引用参数（可能表示文件路径），
    // 返回值是 TArray<FUSDCARLAWheelData> 类型（即返回一个包含多个 FUSDCARLAWheelData 结构体的数组，
    // 用于获取指定路径下的车轮数据相关信息）
  static TArray<FUSDCARLAWheelData> GetUSDWheelData(const FString& Path);
};
