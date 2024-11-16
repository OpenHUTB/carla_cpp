// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// 上面两行意思是版权所有 (c) 2019 巴塞罗那自治大学 (UAB) 计算机视觉中心 (CVC)
//
// This work is licensed under the terms of the MIT license.（本工作遵循 MIT 许可证条款进行授权）
// For a copy, see <https://opensource.org/licenses/MIT>.（如需副本，请访问 <https://opensource.org/licenses/MIT>）

// 引入 Carla 项目的头文件
#include "Carla.h"

// 引入 Carla 中 Actor 蓝图功能库的头文件
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"

// 引入 Carla 中激光雷达描述的头文件
#include "Carla/Sensor/LidarDescription.h"

// 引入 Carla 中场景捕获传感器的头文件
#include "Carla/Sensor/SceneCaptureSensor.h"

// 引入 Carla 中基于着色器的传感器的头文件
#include "Carla/Sensor/ShaderBasedSensor.h"

// 引入 Carla 中 V2X 路径损耗模型的头文件
#include "Carla/Sensor/V2X/PathLossModel.h"

// 引入 Carla 中作用域栈工具的头文件
#include "Carla/Util/ScopedStack.h"

// 引入标准算法库
#include <algorithm>

// 引入标准库中的极限值
#include <limits>

// 引入标准栈库
#include <stack>

/// Checks validity of FActorDefinition.（检查 FActorDefinition 的有效性）
class FActorDefinitionValidator
{
public:

  /// Iterate all actor definitions and their properties and display messages on
  /// error.
  /// 上面两行代码意思是遍历所有 actor 定义及其属性，并在出错时显示消息
  bool AreValid(const TArray<FActorDefinition> &ActorDefinitions)
  {
    // 调用重载的 AreValid 方法，传入 "Actor Definition" 字符串和 actor 定义数组
    return AreValid(TEXT("Actor Definition"), ActorDefinitions);
  }

  /// Validate @a ActorDefinition and display messages on error.（验证@a ActorDefinition的有效性，并在出现错误时显示消息）
  bool SingleIsValid(const FActorDefinition& Definition)
  {
      // 使用Definition的Id构造一个作用域文本
      auto ScopeText = FString::Printf(TEXT("[Actor Definition : %s]"), *Definition.Id);

      // 将作用域文本推入Stack的作用域栈中
      auto Scope = Stack.PushScope(ScopeText);

      // 调用IsValid函数验证Definition的有效性
      return IsValid(Definition);
  }

private:

  /// If @a Predicate is false, print an error message. If possible the message
  /// is printed to the editor window.
  /// 上面两行代码意思是如果@a Predicate为false，则打印一条错误消息。如果可能，消息将被打印到编辑器窗口中
  template <typename T, typename ... ARGS>
  bool OnScreenAssert(bool Predicate, const T &Format, ARGS && ... Args) const
  {
    if (!Predicate)
    {
      FString Message;
      // 遍历Stack中的所有字符串，并将它们添加到Message中
      for (auto &String : Stack)
      {
        Message += String;
      }
      // 在Message末尾添加一个空格
      Message += TEXT(" ");
      // 使用Format和参数Args格式化字符串，并追加到Message中
      Message += FString::Printf(Format, std::forward<ARGS>(Args) ...);
 
      // 使用UE_LOG记录错误消息
      UE_LOG(LogCarla, Error, TEXT("%s"), *Message);


#if WITH_EDITOR
     // 如果在编辑器模式下，且GEngine对象存在
      if (GEngine)
      {
        // 在屏幕上显示一条调试消息，消息颜色为红色
        GEngine->AddOnScreenDebugMessage(42, 15.0f, FColor::Red, Message);
      }
#endif // WITH_EDITOR（被用来检查是否正在编辑器环境中运行）
    }
    // 返回Predicate的值
    return Predicate;
  }

  /// 为给定类型的项目生成显示ID。
  template <typename T>
  FString GetDisplayId(const FString &Type, size_t Index, const T &Item)
  {
    // 使用Type、Index和Item的Id构造并返回一个格式化的字符串
    return FString::Printf(TEXT("[%s %d : %s]"), *Type, Index, *Item.Id);
  }
 
  /// 为给定类型的字符串项目生成显示ID的重载版本。
  FString GetDisplayId(const FString &Type, size_t Index, const FString &Item)
  {
    // 使用Type、Index和Item字符串构造并返回一个格式化的字符串
    return FString::Printf(TEXT("[%s %d : %s]"), *Type, Index, *Item);
  }

  /// Applies @a Validator to each item in @a Array. Pushes a new context to the
  /// stack for each item.
  /// 上面两行的意思是对@a Array中的每个元素应用@a Validator。为每个元素向堆栈推送一个新的上下文
  template <typename T, typename F>
bool ForEach(const FString &Type, const TArray<T> &Array, F Validator)
{
  bool Result = true; // 初始化结果为true，假设所有元素都通过验证。
  auto Counter = 0u;  // 初始化计数器，用于追踪当前正在验证的元素位置。
 
  // 遍历数组中的每个元素
  for (const auto &Item : Array)
  {
    // 为当前元素生成一个显示ID，并将其推送到堆栈的新上下文中。
    // 这里假设Stack是一个能够管理上下文的某种堆栈结构，而PushScope是一个向堆栈添加新上下文的方法。
    // GetDisplayId是一个函数，用于生成包含元素类型、索引和ID的格式化字符串。
    auto Scope = Stack.PushScope(GetDisplayId(Type, Counter, Item));
 
    // 对当前元素应用Validator进行验证，并将验证结果与当前Result进行逻辑与运算。
    // 如果Validator返回false，则Result也将变为false。
    Result &= Validator(Item);
 
    // 计数器递增，准备验证下一个元素。
    ++Counter;
  }
 
  // 返回最终的验证结果。如果所有元素都通过验证，则Result为true；否则为false。
  return Result;
}


  /// Applies @a IsValid to each item in @a Array. Pushes a new context to the
  /// stack for each item.
  /// 上面两行代码的意思是对@a Array中的每个元素应用验证函数，为每个元素向堆栈推送一个新的上下文
  template <typename T>
bool AreValid(const FString &Type, const TArray<T> &Array)
{
  // 调用ForEach函数，传入类型名称、元素数组和一个lambda表达式作为验证函数。
  // lambda表达式捕获当前对象（this），并调用IsValid成员函数来验证每个元素
  return ForEach(Type, Array, [this](const auto &Item) { return IsValid(Item); });
}

  /// 验证ID是否有效
  bool IsIdValid(const FString &Id)
  {
    /// @todo Do more checks.（@todo 执行更多检查）
    // 使用OnScreenAssert函数来断言ID不为空且不是"."。如果失败，则显示错误信息
    return OnScreenAssert((!Id.IsEmpty() && Id != TEXT(".")), TEXT("Id cannot be empty"));
  }

  /// 验证标签是否有效
  bool AreTagsValid(const FString &Tags)
  {
    /// @todo Do more checks.（@todo 执行更多检查）
    // 使用OnScreenAssert函数来断言标签不为空。如果失败，则显示错误信息
    return OnScreenAssert(!Tags.IsEmpty(), TEXT("Tags cannot be empty"));
  }

  /// 验证类型是否有效
  bool IsValid(const EActorAttributeType Type)
  {
    /// @todo Do more checks.（@todo 执行更多检查）
    // 使用OnScreenAssert函数来断言类型值小于EActorAttributeType枚举的大小。如果失败，则显示错误信息
    return OnScreenAssert(Type < EActorAttributeType::SIZE, TEXT("Invalid type"));
  }

  /// 验证值是否有效
  bool ValueIsValid(const EActorAttributeType Type, const FString &Value)
  {
    /// @todo Do more checks.（@todo 执行更多检查）
    // 当前版本未执行任何检查，直接返回true
    return true;
  }

  // 判断给定的因子变化是否有效
  bool IsValid(const FActorVariation &Variation)
  {
    // 返回以下条件都为真的结果
    return

      //因子变化的ID有效
      IsIdValid(Variation.Id) &&

      //因子变化的类型有效
      IsValid(Variation.Type) &&

      //因子变化的推荐值数量大于0，且推荐值不能为空
      OnScreenAssert(Variation.RecommendedValues.Num() > 0, TEXT("Recommended values cannot be empty")) &&

      //对每个推荐值，调用一个lambda函数检查其是否有效，该函数根据因子变化的类型检查值的有效性
      ForEach(TEXT("Recommended Value"), Variation.RecommendedValues, [&](auto &Value) {
      return ValueIsValid(Variation.Type, Value);
    });
  }

  // 判断给定的因子属性是否有效
  bool IsValid(const FActorAttribute &Attribute)
  {
    // 返回以下条件都为真的结果
    return

      //因子属性的ID有效
      IsIdValid(Attribute.Id) &&

      //因子属性的类型有效
      IsValid(Attribute.Type) &&

      //根据因子属性的类型，其值有效
      ValueIsValid(Attribute.Type, Attribute.Value);
  }

  // 判断给定的角色定义是否有效
  bool IsValid(const FActorDefinition &ActorDefinition)
  {
    /// @todo Validate Class and make sure IDs are not repeated.（@todo验证类别并确保ID不重复）
    //返回以下条件都为真的结果
    return

      //角色定义的ID有效
      IsIdValid(ActorDefinition.Id) &&

      //角色定义的标签有效
      AreTagsValid(ActorDefinition.Tags) &&

      //角色定义的变化（因子变化）集合有效
      AreValid(TEXT("Variation"), ActorDefinition.Variations) &&

      //角色定义的属性集合有效
      AreValid(TEXT("Attribute"), ActorDefinition.Attributes);
  }
  // 一个FScopedStack<FString>类型的栈实例，用于特定的字符串管理或操作
  FScopedStack<FString> Stack;
};

// 定义一个模板函数，用于将多个字符串使用指定的分隔符连接起来
template <typename ... ARGS>
static FString JoinStrings(const FString &Separator, ARGS && ... Args)
{
  // 使用FString的Join方法，将Args中的字符串使用Separator连接起来
  // std::forward<ARGS>(Args) ... 是完美转发，用于保持参数的左值或右值属性
  // TArray<FString>{std::forward<ARGS>(Args) ...} 创建了一个包含所有参数的FString数组
  return FString::Join(TArray<FString>{std::forward<ARGS>(Args) ...}, *Separator);
}
 
// 定义一个函数，用于将FColor颜色对象转换为FString字符串
// 字符串格式为 "R,G,B"，其中R、G、B是颜色的红、绿、蓝分量
static FString ColorToFString(const FColor &Color)
{
  // 调用JoinStrings函数，将颜色的红、绿、蓝分量转换为字符串并用逗号连接
  return JoinStrings(
      TEXT(","), // 使用逗号作为分隔符
      FString::FromInt(Color.R), // 将红色分量转换为字符串
      FString::FromInt(Color.G), // 将绿色分量转换为字符串
      FString::FromInt(Color.B)); // 将蓝色分量转换为字符串
}
 
/// ============================================================================
/// -- Actor definition validators（Actor定义验证器） --------------------------
/// ============================================================================
 
// UActorBlueprintFunctionLibrary类中的成员函数，用于验证单个Actor定义的有效性
bool UActorBlueprintFunctionLibrary::CheckActorDefinition(const FActorDefinition &ActorDefinition)
{
  // 创建FActorDefinitionValidator验证器对象
  FActorDefinitionValidator Validator;

  // 调用验证器的SingleIsValid方法，验证单个Actor定义的有效性
  return Validator.SingleIsValid(ActorDefinition);
}
 
// UActorBlueprintFunctionLibrary类中的成员函数，用于验证多个Actor定义的有效性
bool UActorBlueprintFunctionLibrary::CheckActorDefinitions(const TArray<FActorDefinition> &ActorDefinitions)
{
  // 创建FActorDefinitionValidator验证器对象
  FActorDefinitionValidator Validator;

  // 调用验证器的AreValid方法，验证多个Actor定义的有效性
  return Validator.AreValid(ActorDefinitions);
}
 
/// ============================================================================
/// -- Helpers to create actor definitions （创建Actor定义的辅助函数）----------
/// ============================================================================

// 定义一个模板函数，接受任意数量的字符串参数（可变参数模板）
template <typename ... TStrs>

// 静态函数，用于填充参与者定义（FActorDefinition）的ID和标签（Tags），以及添加一些默认属性
static void FillIdAndTags(FActorDefinition& Def, TStrs && ... Strings)
{
    // 将传入的字符串参数用"."连接，并转换为小写，作为参与者的ID
    Def.Id = JoinStrings(TEXT("."), std::forward<TStrs>(Strings) ...).ToLower();

    // 将传入的字符串参数用","连接，并转换为小写，作为参与者的标签
    Def.Tags = JoinStrings(TEXT(","), std::forward<TStrs>(Strings) ...).ToLower();

  // 每个参与者都会有一个角色名称属性（默认为空）
  FActorVariation ActorRole;
  ActorRole.Id = TEXT("role_name"); // 属性ID
  ActorRole.Type = EActorAttributeType::String; // 属性类型：字符串
  ActorRole.RecommendedValues = { TEXT("default") }; // 推荐值：默认
  ActorRole.bRestrictToRecommended = false; // 是否限制为推荐值：否
  Def.Variations.Emplace(ActorRole); // 将参与者名称属性添加到角色的属性列表中

  // ROS2相关的属性设置
  FActorVariation Var;
  Var.Id = TEXT("ros_name"); // 属性ID：ros名称
  Var.Type = EActorAttributeType::String; // 属性类型：字符串
  Var.RecommendedValues = { Def.Id }; // 推荐值：参与者的ID
  Var.bRestrictToRecommended = false; // 是否限制为推荐值：否
  Def.Variations.Emplace(Var); // 将ROS2名称属性添加到参与者的属性列表中
}

// 定义一个静态函数，用于为参与者名称属性添加推荐值
static void AddRecommendedValuesForActorRoleName(
    FActorDefinition &Definition, // 参与者定义引用
    TArray<FString> &&RecommendedValues) // 推荐值的数组（右值引用）
{
  // 遍历参与者的属性列表
  for (auto &&ActorVariation: Definition.Variations)
  {
    // 如果找到ID为"role_name"的属性
    if (ActorVariation.Id == "role_name")
    {
      // 将该属性的推荐值设置为传入的推荐值
      ActorVariation.RecommendedValues = RecommendedValues;
      return; // 找到后直接返回，不再继续遍历
    }
  }
}

// 定义一个函数，用于为传感器的参与者名称添加推荐值
static void AddRecommendedValuesForSensorRoleNames(FActorDefinition& Definition)
{
    // 为参与者定义的参与者名称添加推荐的传感器位置名称
    AddRecommendedValuesForActorRoleName(Definition, { TEXT("front"), TEXT("back"), TEXT("left"), TEXT("right"), TEXT("front_left"), TEXT("front_right"), TEXT("back_left"), TEXT("back_right") });
}

// 定义一个函数，用于为传感器添加变化属性
static void AddVariationsForSensor(FActorDefinition& Def)
{
    // 创建一个参与者变化对象
    FActorVariation Tick;

    // 设置变化对象的ID为"sensor_tick"
    Tick.Id = TEXT("sensor_tick");

    // 设置变化对象的类型为浮点型
    Tick.Type = EActorAttributeType::Float;

    // 设置变化对象的推荐值为"0.0"
    Tick.RecommendedValues = { TEXT("0.0") };

    // 设置是否限制只能使用推荐值，这里为false，表示不限制
    Tick.bRestrictToRecommended = false;

    // 将变化对象添加到参与者定义的变化列表中
    Def.Variations.Emplace(Tick);
}

// 定义一个函数，用于为触发器添加变化属性
static void AddVariationsForTrigger(FActorDefinition& Def)
{
    // Friction（摩擦力）
    FActorVariation Friction;

    // 设置摩擦力变化对象的ID为"friction"
    Friction.Id = FString("friction");

    // 设置摩擦力变化对象的类型为浮点型
    Friction.Type = EActorAttributeType::Float;

    // 设置摩擦力变化对象的推荐值为"3.5f"
    Friction.RecommendedValues = { TEXT("3.5f") };

    // 设置是否限制只能使用推荐值，这里为false，表示不限制
    Friction.bRestrictToRecommended = false;

    // 将摩擦力变化对象添加到参与者定义的变化列表中
    Def.Variations.Emplace(Friction);

    // Extent（范围）
    FString Extent("extent");
    FString Coordinates[3] = { FString("x"), FString("y"), FString("z") }; // 定义三个坐标轴x, y, z

    // 遍历坐标轴数组
    for (auto Coordinate : Coordinates)
    {
        FActorVariation ExtentCoordinate; // 为每个坐标轴创建一个变化对象

        // 设置变化对象的ID，格式为"extent_x", "extent_y", "extent_z"
        ExtentCoordinate.Id = JoinStrings(TEXT("_"), Extent, Coordinate);

        // 设置变化对象的类型为浮点型
        ExtentCoordinate.Type = EActorAttributeType::Float;

        // 设置变化对象的推荐值为"1.0f"
        ExtentCoordinate.RecommendedValues = { TEXT("1.0f") };

        // 设置是否限制只能使用推荐值，这里为false，表示不限制
        ExtentCoordinate.bRestrictToRecommended = false;

        // 将变化对象添加到参与者定义的变化列表中
        Def.Variations.Emplace(ExtentCoordinate);
    }
}

// 在UActorBlueprintFunctionLibrary类中定义一个成员函数，用于创建一个通用的Actor定义。
// 它接收三个参数：分类（Category）、类型（Type）和ID（Id），并返回一个参与者对象。
FActorDefinition UActorBlueprintFunctionLibrary::MakeGenericDefinition(

    // 分类名称
    const FString& Category, 

    // 参与者的类型
    const FString& Type,

    // 参与者的唯一标识符
    const FString& Id)       
{
    // 创建一个参与者对象，用于存储Actor的定义
    FActorDefinition Definition;

    // 调用FillIdAndTags函数，填充定义中的ID和标签
    FillIdAndTags(Definition, Category, Type, Id); 

    // 返回填充后的定义
    return Definition; 
}

// 在UActorBlueprintFunctionLibrary类中定义一个成员函数，专门用于创建传感器类型的Actor定义。
// 它接收两个参数：类型（Type）和ID（Id），并返回一个FActorDefinition对象。
FActorDefinition UActorBlueprintFunctionLibrary::MakeGenericSensorDefinition(

    // 传感器的类型
    const FString& Type, 

    // 传感器的唯一标识符
    const FString& Id) 
{
    // 调用MakeGenericDefinition函数，创建一个分类为“sensor”的通用定义
    auto Definition = MakeGenericDefinition(TEXT("sensor"), Type, Id); 

    // 调用AddRecommendedValuesForSensorRoleNames函数，为定义添加建议的传感器角色名称值
    AddRecommendedValuesForSensorRoleNames(Definition); 

    // 返回填充后的定义
    return Definition; 
}

// 在UActorBlueprintFunctionLibrary类中定义一个成员函数，用于创建一个相机Actor的定义。
// 它接收两个参数：ID（Id）和一个布尔值（bEnableModifyingPostProcessEffects），指示是否允许修改后处理效果。
// 函数返回一个FActorDefinition对象。
FActorDefinition UActorBlueprintFunctionLibrary::MakeCameraDefinition(

    // 相机的唯一标识符
    const FString& Id, 

    // 是否允许修改后处理效果的标志
    const bool bEnableModifyingPostProcessEffects)
{
    // 创建一个参与者对象，用于存储相机的定义
    FActorDefinition Definition;

    // 定义一个布尔变量，用于指示定义创建是否成功
    bool Success;

    // 调用一个重载版本的MakeCameraDefinition函数（未在代码片段中给出），该版本接受一个额外的Success参数用于输出操作结果
    MakeCameraDefinition(Id, bEnableModifyingPostProcessEffects, Success, Definition);

    // 使用check宏确保定义创建成功，如果失败则触发断言
    check(Success); 
    
    // 返回填充后的定义
    return Definition; 
}

// 定义一个函数，用于创建相机定义
// 该函数属于UActorBlueprintFunctionLibrary类
void UActorBlueprintFunctionLibrary::MakeCameraDefinition(

    // 相机的唯一标识符
    const FString& Id, 

    // 是否允许修改后处理效果
    const bool bEnableModifyingPostProcessEffects, 

    // 函数执行成功与否的标志，通过引用传递，函数内部可以修改其值
    bool& Success, 

    // 相机定义的对象，通过引用传递，函数内部会对其进行填充
    FActorDefinition& Definition) 
{
    // 填充相机定义的基本信息，包括Id和标签（此处标签为"sensor"和"camera"）
    FillIdAndTags(Definition, TEXT("sensor"), TEXT("camera"), Id);

    // 为相机定义添加推荐的参与者名称值，这些值通常与传感器的参与者有关
    AddRecommendedValuesForSensorRoleNames(Definition);

    // 为相机定义添加传感器相关的变体（可能是不同的配置或参数集合）
    AddVariationsForSensor(Definition);

    // 下面的代码块用于添加相机的视野（Field of View, FOV）定义

    // 创建一个FActorVariation对象，用于表示FOV的定义
    FActorVariation FOV; 

    // 设置FOV定义的标识符为"fov"
    FOV.Id = TEXT("fov"); 

    // 设置FOV的类型为浮点型
    FOV.Type = EActorAttributeType::Float; 

    // 为FOV设置一个推荐的值，这里是90.0度
    FOV.RecommendedValues = { TEXT("90.0") }; 

    // 设置是否限制用户只能使用推荐的值，这里设置为false，表示用户可以选择其他值
    FOV.bRestrictToRecommended = false; 

    // 关于分辨率
    // 定义图像的宽度（X轴）变化的结构体
    FActorVariation ResX;

    // 设置该变化项的标识符为"image_size_x"
    ResX.Id = TEXT("image_size_x");

    // 设置该变化项的类型为整数（Int）
    ResX.Type = EActorAttributeType::Int;

    // 设置推荐的值为"800"
    ResX.RecommendedValues = { TEXT("800") };

    // 设置是否限制用户只能使用推荐值，这里设置为false，意味着用户可以选择其他值
    ResX.bRestrictToRecommended = false;

    // 定义图像的高度（Y轴）变化的结构体
    FActorVariation ResY;

    // 设置该变化项的标识符为"image_size_y"
    ResY.Id = TEXT("image_size_y");

    // 设置该变化项的类型为整数（Int）
    ResY.Type = EActorAttributeType::Int;

    // 设置推荐的值为"600"（同样以文本形式给出，但实际为整数）
    ResY.RecommendedValues = { TEXT("600") };

    // 设置是否限制用户只能使用推荐值，这里同样设置为false
    ResY.bRestrictToRecommended = false;

    // 镜头参数
    // 定义镜头圆形衰减参数的结构体
    FActorVariation LensCircleFalloff;

    // 设置该变化项的标识符为"lens_circle_falloff"
    LensCircleFalloff.Id = TEXT("lens_circle_falloff");

    // 设置该变化项的类型为浮点数（Float）
    LensCircleFalloff.Type = EActorAttributeType::Float;

    // 设置推荐的值为"5.0"
    LensCircleFalloff.RecommendedValues = { TEXT("5.0") };

    // 设置是否限制用户只能使用推荐值，这里设置为false
    LensCircleFalloff.bRestrictToRecommended = false;

    // 定义镜头圆形倍增参数的结构体
    FActorVariation LensCircleMultiplier;

    // 设置该变化项的标识符为"lens_circle_multiplier"
    LensCircleMultiplier.Id = TEXT("lens_circle_multiplier");

    // 设置该变化项的类型为浮点数（Float）
    LensCircleMultiplier.Type = EActorAttributeType::Float;

    // 设置推荐的值为"0.0"（以文本形式给出，但实际为浮点数）
    LensCircleMultiplier.RecommendedValues = { TEXT("0.0") };

    // 设置是否限制用户只能使用推荐值，这里同样设置为false
    LensCircleMultiplier.bRestrictToRecommended = false;

    // 定义一个FActorVariation类型的变量LensK，用于表示某种属性或参数的变化
    FActorVariation LensK;

    // 为LensK设置唯一标识符，这里是一个文本字符串"lens_k"
    LensK.Id = TEXT("lens_k");

    // 设置LensK的属性类型为浮点型
    LensK.Type = EActorAttributeType::Float;

    // 为LensK设置一个推荐值列表，这里只有一个值"-1.0"
    LensK.RecommendedValues = { TEXT("-1.0") };

    // 设置是否将LensK的值限制在推荐值之内，这里设置为false，表示不限制
    LensK.bRestrictToRecommended = false;

    // 定义一个FActorVariation类型的变量LensKcube，与LensK类似，但表示不同的属性或参数
    FActorVariation LensKcube;

    // 为LensKcube设置唯一标识符"lens_kcube"
    LensKcube.Id = TEXT("lens_kcube");

    // 设置LensKcube的属性类型也为浮点型
    LensKcube.Type = EActorAttributeType::Float;

    // 为LensKcube设置一个推荐值列表，这里只有一个值"0.0"
    LensKcube.RecommendedValues = { TEXT("0.0") };

    // 设置是否将LensKcube的值限制在推荐值之内，这里也设置为false
    LensKcube.bRestrictToRecommended = false;

    // 定义一个FActorVariation类型的变量LensXSize，表示透镜在X轴方向上的尺寸变化
    FActorVariation LensXSize;

    // 为LensXSize设置唯一标识符"lens_x_size"
    LensXSize.Id = TEXT("lens_x_size");

    // 设置LensXSize的属性类型也为浮点型
    LensXSize.Type = EActorAttributeType::Float;

    // 为LensXSize设置一个推荐值列表，这里只有一个值"0.08"
    LensXSize.RecommendedValues = { TEXT("0.08") };

    // 设置是否将LensXSize的值限制在推荐值之内，这里也设置为false
    LensXSize.bRestrictToRecommended = false;

    // 定义一个FActorVariation类型的变量LensYSize，表示透镜在Y轴方向上的尺寸变化
    FActorVariation LensYSize;

    // 为LensYSize设置唯一标识符"lens_y_size"
    LensYSize.Id = TEXT("lens_y_size");

    // 设置LensYSize的属性类型也为浮点型
    LensYSize.Type = EActorAttributeType::Float;

    // 为LensYSize设置一个推荐值列表，这里只有一个值"0.08"
    LensYSize.RecommendedValues = { TEXT("0.08") };

    // 设置是否将LensYSize的值限制在推荐值之内，这里也设置为false
    LensYSize.bRestrictToRecommended = false;


 // 将一系列变量（如分辨率、视野等）添加到定义的变化列表中
Definition.Variations.Append({
    ResX,           // 分辨率X轴
    ResY,           // 分辨率Y轴
    FOV,            // 视野（Field of View）
    LensCircleFalloff, // 镜头圆形衰减
    LensCircleMultiplier, // 镜头圆形倍增器
    LensK,          // 镜头K值（一种镜头畸变参数）
    LensKcube,      // 镜头K立方值（另一种镜头畸变参数）
    LensXSize,      // 镜头X轴尺寸
    LensYSize});    // 镜头Y轴尺寸
 
// 如果启用了修改后处理效果的功能
if (bEnableModifyingPostProcessEffects)
{
    // 创建一个后处理效果的变化定义
    FActorVariation PostProccess;
    PostProccess.Id = TEXT("enable_postprocess_effects"); // 设置变化的标识符
    PostProccess.Type = EActorAttributeType::Bool;        // 设置变化类型为布尔值
    PostProccess.RecommendedValues = { TEXT("true") };    // 设置推荐的值为"true"（启用）
    PostProccess.bRestrictToRecommended = false;          // 不限制用户只能使用推荐值
 
    // 创建一个关于Gamma值的变化定义
    FActorVariation Gamma;
    Gamma.Id = TEXT("gamma");         // 设置变化的标识符为"gamma"
    Gamma.Type = EActorAttributeType::Float; // 设置变化类型为浮点数
    Gamma.RecommendedValues = { TEXT("2.2") }; // 设置推荐的Gamma值为2.2
    Gamma.bRestrictToRecommended = false;      // 不限制用户只能使用推荐的Gamma值
}

    // 运动模糊配置
   // 定义运动模糊强度的变化属性
    FActorVariation MBIntesity; 
    MBIntesity.Id = TEXT("motion_blur_intensity"); // 设置属性的唯一标识符
    MBIntesity.Type = EActorAttributeType::Float; // 指定属性类型为浮点数
    MBIntesity.RecommendedValues = { TEXT("0.45") }; // 设置推荐的运动模糊强度值
    MBIntesity.bRestrictToRecommended = false; // 允许用户选择不使用推荐值
 
    // 定义运动模糊最大扭曲的变化属性
    FActorVariation MBMaxDistortion;
    MBMaxDistortion.Id = TEXT("motion_blur_max_distortion"); // 设置属性的唯一标识符
    MBMaxDistortion.Type = EActorAttributeType::Float; // 指定属性类型为浮点数
    MBMaxDistortion.RecommendedValues = { TEXT("0.35") }; // 设置推荐的运动模糊最大扭曲值
    MBMaxDistortion.bRestrictToRecommended = false; // 允许用户选择不使用推荐值
 
    // 定义运动模糊最小对象屏幕尺寸的变化属性
    FActorVariation MBMinObjectScreenSize;
    MBMinObjectScreenSize.Id = TEXT("motion_blur_min_object_screen_size"); // 设置属性的唯一标识符
    MBMinObjectScreenSize.Type = EActorAttributeType::Float; // 指定属性类型为浮点数
    MBMinObjectScreenSize.RecommendedValues = { TEXT("0.1") }; // 设置推荐的运动模糊最小对象屏幕尺寸值
    MBMinObjectScreenSize.bRestrictToRecommended = false; // 允许用户选择不使用推荐值

    // 关于镜头光晕效果的设置
    FActorVariation LensFlareIntensity; // 声明一个FActorVariation类型的对象，用于存储镜头光晕强度的配置

    // 设置对象的Id属性，用于唯一标识这个变量
    LensFlareIntensity.Id = TEXT("lens_flare_intensity"); // 将Id设置为"lens_flare_intensity"，这是一个字符串标识符

    // 设置对象的Type属性，指定变量的数据类型
    LensFlareIntensity.Type = EActorAttributeType::Float; // 将类型设置为浮点型（Float），意味着镜头光晕的强度是一个浮点数

    // 设置RecommendedValues属性，提供推荐的变量值列表
    LensFlareIntensity.RecommendedValues = { TEXT("0.1") }; // 这里只提供了一个推荐值"0.1"，意味着默认情况下镜头光晕的强度被设置为0.1

    // 设置bRestrictToRecommended属性，决定变量值是否必须为推荐值之一
    LensFlareIntensity.bRestrictToRecommended = false; // 设置为false，意味着镜头光晕的强度值不仅限于推荐值，可以是任意浮点数。


    // Bloom
    FActorVariation BloomIntensity;
    BloomIntensity.Id = TEXT("bloom_intensity");
    BloomIntensity.Type = EActorAttributeType::Float;
    BloomIntensity.RecommendedValues = { TEXT("0.675") };
    BloomIntensity.bRestrictToRecommended = false;

    // More info at:
    // https://docs.unrealengine.com/en-US/Engine/Rendering/PostProcessEffects/AutomaticExposure/index.html
    // https://docs.unrealengine.com/en-US/Engine/Rendering/PostProcessEffects/DepthOfField/CinematicDOFMethods/index.html
    // https://docs.unrealengine.com/en-US/Engine/Rendering/PostProcessEffects/ColorGrading/index.html

    // Exposure
    FActorVariation ExposureMode;
    ExposureMode.Id = TEXT("exposure_mode");
    ExposureMode.Type = EActorAttributeType::String;
    ExposureMode.RecommendedValues = { TEXT("histogram"), TEXT("manual") };
    ExposureMode.bRestrictToRecommended = true;

    // Logarithmic adjustment for the exposure. Only used if a tonemapper is
    // specified.
    //  0 : no adjustment
    // -1 : 2x darker
    // -2 : 4x darker
    //  1 : 2x brighter
    //  2 : 4x brighter.
    FActorVariation ExposureCompensation;
    ExposureCompensation.Id = TEXT("exposure_compensation");
    ExposureCompensation.Type = EActorAttributeType::Float;
    ExposureCompensation.RecommendedValues = { TEXT("0.0") };
    ExposureCompensation.bRestrictToRecommended = false;

    // - Manual ------------------------------------------------

    // The formula used to compute the camera exposure scale is:
    // Exposure = 1 / (1.2 * 2^(log2( N²/t * 100/S )))

    // The camera shutter speed in seconds.
    FActorVariation ShutterSpeed; // (1/t)
    ShutterSpeed.Id = TEXT("shutter_speed");
    ShutterSpeed.Type = EActorAttributeType::Float;
    ShutterSpeed.RecommendedValues = { TEXT("200.0") };
    ShutterSpeed.bRestrictToRecommended = false;

    // The camera sensor sensitivity.
    FActorVariation ISO; // S
    ISO.Id = TEXT("iso");
    ISO.Type = EActorAttributeType::Float;
    ISO.RecommendedValues = { TEXT("100.0") };
    ISO.bRestrictToRecommended = false;

    // Defines the size of the opening for the camera lens.
    // Using larger numbers will reduce the DOF effect.
    FActorVariation Aperture; // N
    Aperture.Id = TEXT("fstop");
    Aperture.Type = EActorAttributeType::Float;
    Aperture.RecommendedValues = { TEXT("1.4") };
    Aperture.bRestrictToRecommended = false;

    // - Histogram ---------------------------------------------

    // The minimum brightness for auto exposure that limits the lower
    // brightness the eye can adapt within
    FActorVariation ExposureMinBright;
    ExposureMinBright.Id = TEXT("exposure_min_bright");
    ExposureMinBright.Type = EActorAttributeType::Float;
    ExposureMinBright.RecommendedValues = { TEXT("10.0") };
    ExposureMinBright.bRestrictToRecommended = false;

    // The maximum brightness for auto exposure that limits the upper
    // brightness the eye can adapt within
    FActorVariation ExposureMaxBright;
    ExposureMaxBright.Id = TEXT("exposure_max_bright");
    ExposureMaxBright.Type = EActorAttributeType::Float;
    ExposureMaxBright.RecommendedValues = { TEXT("12.0") };
    ExposureMaxBright.bRestrictToRecommended = false;

    // The speed at which the adaptation occurs from a dark environment
    // to a bright environment.
    FActorVariation ExposureSpeedUp;
    ExposureSpeedUp.Id = TEXT("exposure_speed_up");
    ExposureSpeedUp.Type = EActorAttributeType::Float;
    ExposureSpeedUp.RecommendedValues = { TEXT("3.0") };
    ExposureSpeedUp.bRestrictToRecommended = false;

    // The speed at which the adaptation occurs from a bright environment
    // to a dark environment.
    FActorVariation ExposureSpeedDown;
    ExposureSpeedDown.Id = TEXT("exposure_speed_down");
    ExposureSpeedDown.Type = EActorAttributeType::Float;
    ExposureSpeedDown.RecommendedValues = { TEXT("1.0") };
    ExposureSpeedDown.bRestrictToRecommended = false;

    // Calibration constant for 18% Albedo.
    FActorVariation CalibrationConstant;
    CalibrationConstant.Id = TEXT("calibration_constant");
    CalibrationConstant.Type = EActorAttributeType::Float;
    CalibrationConstant.RecommendedValues = { TEXT("16.0") };
    CalibrationConstant.bRestrictToRecommended = false;

    // Distance in which the Depth of Field effect should be sharp,
    // in unreal units (cm)
    FActorVariation FocalDistance;
    FocalDistance.Id = TEXT("focal_distance");
    FocalDistance.Type = EActorAttributeType::Float;
    FocalDistance.RecommendedValues = { TEXT("1000.0") };
    FocalDistance.bRestrictToRecommended = false;

    // Depth blur km for 50%
    FActorVariation DepthBlurAmount;
    DepthBlurAmount.Id = TEXT("blur_amount");
    DepthBlurAmount.Type = EActorAttributeType::Float;
    DepthBlurAmount.RecommendedValues = { TEXT("1.0") };
    DepthBlurAmount.bRestrictToRecommended = false;

    // Depth blur radius in pixels at 1920x
    FActorVariation DepthBlurRadius;
    DepthBlurRadius.Id = TEXT("blur_radius");
    DepthBlurRadius.Type = EActorAttributeType::Float;
    DepthBlurRadius.RecommendedValues = { TEXT("0.0") };
    DepthBlurRadius.bRestrictToRecommended = false;

    // Defines the opening of the camera lens, Aperture is 1.0/fstop,
    // typical lens go down to f/1.2 (large opening),
    // larger numbers reduce the DOF effect
    FActorVariation MaxAperture;
    MaxAperture.Id = TEXT("min_fstop");
    MaxAperture.Type = EActorAttributeType::Float;
    MaxAperture.RecommendedValues = { TEXT("1.2") };
    MaxAperture.bRestrictToRecommended = false;

    // Defines the number of blades of the diaphragm within the
    // lens (between 4 and 16)
    FActorVariation BladeCount;
    BladeCount.Id = TEXT("blade_count");
    BladeCount.Type = EActorAttributeType::Int;
    BladeCount.RecommendedValues = { TEXT("5") };
    BladeCount.bRestrictToRecommended = false;

    // - Tonemapper Settings -----------------------------------
    // You can adjust these tonemapper controls to emulate other
    // types of film stock for your project
    FActorVariation FilmSlope;
    FilmSlope.Id = TEXT("slope");
    FilmSlope.Type = EActorAttributeType::Float;
    FilmSlope.RecommendedValues = { TEXT("0.88") };
    FilmSlope.bRestrictToRecommended = false;

    FActorVariation FilmToe;
    FilmToe.Id = TEXT("toe");
    FilmToe.Type = EActorAttributeType::Float;
    FilmToe.RecommendedValues = { TEXT("0.55") };
    FilmToe.bRestrictToRecommended = false;

    FActorVariation FilmShoulder;
    FilmShoulder.Id = TEXT("shoulder");
    FilmShoulder.Type = EActorAttributeType::Float;
    FilmShoulder.RecommendedValues = { TEXT("0.26") };
    FilmShoulder.bRestrictToRecommended = false;

    FActorVariation FilmBlackClip;
    FilmBlackClip.Id = TEXT("black_clip");
    FilmBlackClip.Type = EActorAttributeType::Float;
    FilmBlackClip.RecommendedValues = { TEXT("0.0") };
    FilmBlackClip.bRestrictToRecommended = false;

    FActorVariation FilmWhiteClip;
    FilmWhiteClip.Id = TEXT("white_clip");
    FilmWhiteClip.Type = EActorAttributeType::Float;
    FilmWhiteClip.RecommendedValues = { TEXT("0.04") };
    FilmWhiteClip.bRestrictToRecommended = false;

    // Color
    FActorVariation Temperature;
    Temperature.Id = TEXT("temp");
    Temperature.Type = EActorAttributeType::Float;
    Temperature.RecommendedValues = { TEXT("6500.0") };
    Temperature.bRestrictToRecommended = false;

    FActorVariation Tint;
    Tint.Id = TEXT("tint");
    Tint.Type = EActorAttributeType::Float;
    Tint.RecommendedValues = { TEXT("0.0") };
    Tint.bRestrictToRecommended = false;

    FActorVariation ChromaticIntensity;
    ChromaticIntensity.Id = TEXT("chromatic_aberration_intensity");
    ChromaticIntensity.Type = EActorAttributeType::Float;
    ChromaticIntensity.RecommendedValues = { TEXT("0.0") };
    ChromaticIntensity.bRestrictToRecommended = false;

    FActorVariation ChromaticOffset;
    ChromaticOffset.Id = TEXT("chromatic_aberration_offset");
    ChromaticOffset.Type = EActorAttributeType::Float;
    ChromaticOffset.RecommendedValues = { TEXT("0.0") };
    ChromaticOffset.bRestrictToRecommended = false;

    Definition.Variations.Append({
      ExposureMode,
      ExposureCompensation,
      ShutterSpeed,
      ISO,
      Aperture,
      PostProccess,
      Gamma,
      MBIntesity,
      MBMaxDistortion,
      LensFlareIntensity,
      BloomIntensity,
      MBMinObjectScreenSize,
      ExposureMinBright,
      ExposureMaxBright,
      ExposureSpeedUp,
      ExposureSpeedDown,
      CalibrationConstant,
      FocalDistance,
      MaxAperture,
      BladeCount,
      DepthBlurAmount,
      DepthBlurRadius,
      FilmSlope,
      FilmToe,
      FilmShoulder,
      FilmBlackClip,
      FilmWhiteClip,
      Temperature,
      Tint,
      ChromaticIntensity,
      ChromaticOffset});
  }

  Success = CheckActorDefinition(Definition);
}

FActorDefinition UActorBlueprintFunctionLibrary::MakeNormalsCameraDefinition()
{
  FActorDefinition Definition;
  bool Success;
  MakeNormalsCameraDefinition(Success, Definition);
  check(Success);
  return Definition;
}

void UActorBlueprintFunctionLibrary::MakeNormalsCameraDefinition(bool &Success, FActorDefinition &Definition)
{
  FillIdAndTags(Definition, TEXT("sensor"), TEXT("camera"), TEXT("normals"));
  AddRecommendedValuesForSensorRoleNames(Definition);
  AddVariationsForSensor(Definition);

  // FOV
  FActorVariation FOV;
  FOV.Id = TEXT("fov");
  FOV.Type = EActorAttributeType::Float;
  FOV.RecommendedValues = { TEXT("90.0") };
  FOV.bRestrictToRecommended = false;

  // Resolution
  FActorVariation ResX;
  ResX.Id = TEXT("image_size_x");
  ResX.Type = EActorAttributeType::Int;
  ResX.RecommendedValues = { TEXT("800") };
  ResX.bRestrictToRecommended = false;

  FActorVariation ResY;
  ResY.Id = TEXT("image_size_y");
  ResY.Type = EActorAttributeType::Int;
  ResY.RecommendedValues = { TEXT("600") };
  ResY.bRestrictToRecommended = false;

  // Lens parameters
  FActorVariation LensCircleFalloff;
  LensCircleFalloff.Id = TEXT("lens_circle_falloff");
  LensCircleFalloff.Type = EActorAttributeType::Float;
  LensCircleFalloff.RecommendedValues = { TEXT("5.0") };
  LensCircleFalloff.bRestrictToRecommended = false;

  FActorVariation LensCircleMultiplier;
  LensCircleMultiplier.Id = TEXT("lens_circle_multiplier");
  LensCircleMultiplier.Type = EActorAttributeType::Float;
  LensCircleMultiplier.RecommendedValues = { TEXT("0.0") };
  LensCircleMultiplier.bRestrictToRecommended = false;

  FActorVariation LensK;
  LensK.Id = TEXT("lens_k");
  LensK.Type = EActorAttributeType::Float;
  LensK.RecommendedValues = { TEXT("-1.0") };
  LensK.bRestrictToRecommended = false;

  FActorVariation LensKcube;
  LensKcube.Id = TEXT("lens_kcube");
  LensKcube.Type = EActorAttributeType::Float;
  LensKcube.RecommendedValues = { TEXT("0.0") };
  LensKcube.bRestrictToRecommended = false;

  FActorVariation LensXSize;
  LensXSize.Id = TEXT("lens_x_size");
  LensXSize.Type = EActorAttributeType::Float;
  LensXSize.RecommendedValues = { TEXT("0.08") };
  LensXSize.bRestrictToRecommended = false;

  FActorVariation LensYSize;
  LensYSize.Id = TEXT("lens_y_size");
  LensYSize.Type = EActorAttributeType::Float;
  LensYSize.RecommendedValues = { TEXT("0.08") };
  LensYSize.bRestrictToRecommended = false;

  Definition.Variations.Append({
      ResX,
      ResY,
      FOV,
      LensCircleFalloff,
      LensCircleMultiplier,
      LensK,
      LensKcube,
      LensXSize,
      LensYSize});

  Success = CheckActorDefinition(Definition);
}

FActorDefinition UActorBlueprintFunctionLibrary::MakeIMUDefinition()
{
  FActorDefinition Definition;
  bool Success;
  MakeIMUDefinition(Success, Definition);
  check(Success);
  return Definition;
}

void UActorBlueprintFunctionLibrary::MakeIMUDefinition(
    bool &Success,
    FActorDefinition &Definition)
{
  FillIdAndTags(Definition, TEXT("sensor"), TEXT("other"), TEXT("imu"));
  AddVariationsForSensor(Definition);

  // - Noise seed --------------------------------
  FActorVariation NoiseSeed;
  NoiseSeed.Id = TEXT("noise_seed");
  NoiseSeed.Type = EActorAttributeType::Int;
  NoiseSeed.RecommendedValues = { TEXT("0") };
  NoiseSeed.bRestrictToRecommended = false;

  // - Accelerometer Standard Deviation ----------
  // X Component
  FActorVariation StdDevAccelX;
  StdDevAccelX.Id = TEXT("noise_accel_stddev_x");
  StdDevAccelX.Type = EActorAttributeType::Float;
  StdDevAccelX.RecommendedValues = { TEXT("0.0") };
  StdDevAccelX.bRestrictToRecommended = false;
  // Y Component
  FActorVariation StdDevAccelY;
  StdDevAccelY.Id = TEXT("noise_accel_stddev_y");
  StdDevAccelY.Type = EActorAttributeType::Float;
  StdDevAccelY.RecommendedValues = { TEXT("0.0") };
  StdDevAccelY.bRestrictToRecommended = false;
  // Z Component
  FActorVariation StdDevAccelZ;
  StdDevAccelZ.Id = TEXT("noise_accel_stddev_z");
  StdDevAccelZ.Type = EActorAttributeType::Float;
  StdDevAccelZ.RecommendedValues = { TEXT("0.0") };
  StdDevAccelZ.bRestrictToRecommended = false;

  // - Gyroscope Standard Deviation --------------
  // X Component
  FActorVariation StdDevGyroX;
  StdDevGyroX.Id = TEXT("noise_gyro_stddev_x");
  StdDevGyroX.Type = EActorAttributeType::Float;
  StdDevGyroX.RecommendedValues = { TEXT("0.0") };
  StdDevGyroX.bRestrictToRecommended = false;
  // Y Component
  FActorVariation StdDevGyroY;
  StdDevGyroY.Id = TEXT("noise_gyro_stddev_y");
  StdDevGyroY.Type = EActorAttributeType::Float;
  StdDevGyroY.RecommendedValues = { TEXT("0.0") };
  StdDevGyroY.bRestrictToRecommended = false;
  // Z Component
  FActorVariation StdDevGyroZ;
  StdDevGyroZ.Id = TEXT("noise_gyro_stddev_z");
  StdDevGyroZ.Type = EActorAttributeType::Float;
  StdDevGyroZ.RecommendedValues = { TEXT("0.0") };
  StdDevGyroZ.bRestrictToRecommended = false;

  // - Gyroscope Bias ----------------------------
  // X Component
  FActorVariation BiasGyroX;
  BiasGyroX.Id = TEXT("noise_gyro_bias_x");
  BiasGyroX.Type = EActorAttributeType::Float;
  BiasGyroX.RecommendedValues = { TEXT("0.0") };
  BiasGyroX.bRestrictToRecommended = false;
  // Y Component
  FActorVariation BiasGyroY;
  BiasGyroY.Id = TEXT("noise_gyro_bias_y");
  BiasGyroY.Type = EActorAttributeType::Float;
  BiasGyroY.RecommendedValues = { TEXT("0.0") };
  BiasGyroY.bRestrictToRecommended = false;
  // Z Component
  FActorVariation BiasGyroZ;
  BiasGyroZ.Id = TEXT("noise_gyro_bias_z");
  BiasGyroZ.Type = EActorAttributeType::Float;
  BiasGyroZ.RecommendedValues = { TEXT("0.0") };
  BiasGyroZ.bRestrictToRecommended = false;

  Definition.Variations.Append({
    NoiseSeed,
    StdDevAccelX,
    StdDevAccelY,
    StdDevAccelZ,
    StdDevGyroX,
    StdDevGyroY,
    StdDevGyroZ,
    BiasGyroX,
    BiasGyroY,
    BiasGyroZ});

  Success = CheckActorDefinition(Definition);
}

FActorDefinition UActorBlueprintFunctionLibrary::MakeRadarDefinition()
{
  FActorDefinition Definition;
  bool Success;
  MakeRadarDefinition(Success, Definition);
  check(Success);
  return Definition;
}

void UActorBlueprintFunctionLibrary::MakeRadarDefinition(
    bool &Success,
    FActorDefinition &Definition)
{
  FillIdAndTags(Definition, TEXT("sensor"), TEXT("other"), TEXT("radar"));
  AddVariationsForSensor(Definition);

  FActorVariation HorizontalFOV;
  HorizontalFOV.Id = TEXT("horizontal_fov");
  HorizontalFOV.Type = EActorAttributeType::Float;
  HorizontalFOV.RecommendedValues = { TEXT("30") };
  HorizontalFOV.bRestrictToRecommended = false;

  FActorVariation VerticalFOV;
  VerticalFOV.Id = TEXT("vertical_fov");
  VerticalFOV.Type = EActorAttributeType::Float;
  VerticalFOV.RecommendedValues = { TEXT("30") };
  VerticalFOV.bRestrictToRecommended = false;

  FActorVariation Range;
  Range.Id = TEXT("range");
  Range.Type = EActorAttributeType::Float;
  Range.RecommendedValues = { TEXT("100") };
  Range.bRestrictToRecommended = false;

  FActorVariation PointsPerSecond;
  PointsPerSecond.Id = TEXT("points_per_second");
  PointsPerSecond.Type = EActorAttributeType::Int;
  PointsPerSecond.RecommendedValues = { TEXT("1500") };
  PointsPerSecond.bRestrictToRecommended = false;

  // Noise seed
  FActorVariation NoiseSeed;
  NoiseSeed.Id = TEXT("noise_seed");
  NoiseSeed.Type = EActorAttributeType::Int;
  NoiseSeed.RecommendedValues = { TEXT("0") };
  NoiseSeed.bRestrictToRecommended = false;

  Definition.Variations.Append({
    HorizontalFOV,
    VerticalFOV,
    Range,
    PointsPerSecond,
    NoiseSeed});

  Success = CheckActorDefinition(Definition);
}

FActorDefinition UActorBlueprintFunctionLibrary::MakeLidarDefinition(
    const FString &Id)
{
  FActorDefinition Definition;
  bool Success;
  MakeLidarDefinition(Id, Success, Definition);
  check(Success);
  return Definition;
}

void UActorBlueprintFunctionLibrary::MakeLidarDefinition(
    const FString &Id,
    bool &Success,
    FActorDefinition &Definition)
{
  FillIdAndTags(Definition, TEXT("sensor"), TEXT("lidar"), Id);
  AddRecommendedValuesForSensorRoleNames(Definition);
  AddVariationsForSensor(Definition);
  // Number of channels.
  FActorVariation Channels;
  Channels.Id = TEXT("channels");
  Channels.Type = EActorAttributeType::Int;
  Channels.RecommendedValues = { TEXT("32") };
  // Range.
  FActorVariation Range;
  Range.Id = TEXT("range");
  Range.Type = EActorAttributeType::Float;
  Range.RecommendedValues = { TEXT("10.0") }; // 10 meters
  // Points per second.
  FActorVariation PointsPerSecond;
  PointsPerSecond.Id = TEXT("points_per_second");
  PointsPerSecond.Type = EActorAttributeType::Int;
  PointsPerSecond.RecommendedValues = { TEXT("56000") };
  // Frequency.
  FActorVariation Frequency;
  Frequency.Id = TEXT("rotation_frequency");
  Frequency.Type = EActorAttributeType::Float;
  Frequency.RecommendedValues = { TEXT("10.0") };
  // Upper FOV limit.
  FActorVariation UpperFOV;
  UpperFOV.Id = TEXT("upper_fov");
  UpperFOV.Type = EActorAttributeType::Float;
  UpperFOV.RecommendedValues = { TEXT("10.0") };
  // Lower FOV limit.
  FActorVariation LowerFOV;
  LowerFOV.Id = TEXT("lower_fov");
  LowerFOV.Type = EActorAttributeType::Float;
  LowerFOV.RecommendedValues = { TEXT("-30.0") };
  // Horizontal FOV.
  FActorVariation HorizontalFOV;
  HorizontalFOV.Id = TEXT("horizontal_fov");
  HorizontalFOV.Type = EActorAttributeType::Float;
  HorizontalFOV.RecommendedValues = { TEXT("360.0") };
  // Atmospheric Attenuation Rate.
  FActorVariation AtmospAttenRate;
  AtmospAttenRate.Id = TEXT("atmosphere_attenuation_rate");
  AtmospAttenRate.Type = EActorAttributeType::Float;
  AtmospAttenRate.RecommendedValues = { TEXT("0.004") };
  // Noise seed
  FActorVariation NoiseSeed;
  NoiseSeed.Id = TEXT("noise_seed");
  NoiseSeed.Type = EActorAttributeType::Int;
  NoiseSeed.RecommendedValues = { TEXT("0") };
  NoiseSeed.bRestrictToRecommended = false;
  // Dropoff General Rate
  FActorVariation DropOffGenRate;
  DropOffGenRate.Id = TEXT("dropoff_general_rate");
  DropOffGenRate.Type = EActorAttributeType::Float;
  DropOffGenRate.RecommendedValues = { TEXT("0.45") };
  // Dropoff intensity limit.
  FActorVariation DropOffIntensityLimit;
  DropOffIntensityLimit.Id = TEXT("dropoff_intensity_limit");
  DropOffIntensityLimit.Type = EActorAttributeType::Float;
  DropOffIntensityLimit.RecommendedValues = { TEXT("0.8") };
  // Dropoff at zero intensity.
  FActorVariation DropOffAtZeroIntensity;
  DropOffAtZeroIntensity.Id = TEXT("dropoff_zero_intensity");
  DropOffAtZeroIntensity.Type = EActorAttributeType::Float;
  DropOffAtZeroIntensity.RecommendedValues = { TEXT("0.4") };
  // Noise in lidar cloud points.
  FActorVariation StdDevLidar;
  StdDevLidar.Id = TEXT("noise_stddev");
  StdDevLidar.Type = EActorAttributeType::Float;
  StdDevLidar.RecommendedValues = { TEXT("0.0") };

  if (Id == "ray_cast") {
    Definition.Variations.Append({
      Channels,
      Range,
      PointsPerSecond,
      Frequency,
      UpperFOV,
      LowerFOV,
      AtmospAttenRate,
      NoiseSeed,
      DropOffGenRate,
      DropOffIntensityLimit,
      DropOffAtZeroIntensity,
      StdDevLidar,
      HorizontalFOV});
  }
  else if (Id == "ray_cast_semantic") {
    Definition.Variations.Append({
      Channels,
      Range,
      PointsPerSecond,
      Frequency,
      UpperFOV,
      LowerFOV,
      HorizontalFOV});
  }
  else {
    DEBUG_ASSERT(false);
  }

  Success = CheckActorDefinition(Definition);
}

FActorDefinition UActorBlueprintFunctionLibrary::MakeV2XDefinition()
{
  FActorDefinition Definition;
  bool Success;
  MakeV2XDefinition(Success, Definition);
  check(Success);
  return Definition;
}

void UActorBlueprintFunctionLibrary::MakeV2XDefinition(
    bool &Success,
    FActorDefinition &Definition)
{
  FillIdAndTags(Definition, TEXT("sensor"), TEXT("other"), TEXT("v2x"));
  AddVariationsForSensor(Definition);

  // - Noise seed --------------------------------
  FActorVariation NoiseSeed;
  NoiseSeed.Id = TEXT("noise_seed");
  NoiseSeed.Type = EActorAttributeType::Int;
  NoiseSeed.RecommendedValues = { TEXT("0") };
  NoiseSeed.bRestrictToRecommended = false;  

  //Frequency
  FActorVariation Frequency;
  Frequency.Id = TEXT("frequency_ghz");
  Frequency.Type = EActorAttributeType::Float;
  Frequency.RecommendedValues = { TEXT("5.9")};

  //TransmitPower
  FActorVariation TransmitPower;
  TransmitPower.Id = TEXT("transmit_power");
  TransmitPower.Type = EActorAttributeType::Float;
  TransmitPower.RecommendedValues = { TEXT("21.5")};

  //ReceiveSensitivity
  FActorVariation ReceiverSensitivity;
  ReceiverSensitivity.Id = TEXT("receiver_sensitivity");
  ReceiverSensitivity.Type = EActorAttributeType::Float;
  ReceiverSensitivity.RecommendedValues = { TEXT("-99.0")};

  //Combined Antenna Gain in dBi
  FActorVariation CombinedAntennaGain;
  CombinedAntennaGain.Id = TEXT("combined_antenna_gain");
  CombinedAntennaGain.Type = EActorAttributeType::Float;
  CombinedAntennaGain.RecommendedValues = { TEXT("10.0")};  

  //Scenario
  FActorVariation Scenario;
  Scenario.Id = TEXT("scenario");
  Scenario.Type = EActorAttributeType::String;
  Scenario.RecommendedValues = { TEXT("highway"), TEXT("rural"), TEXT("urban")};
  Scenario.bRestrictToRecommended = true;

  //Path loss exponent
  FActorVariation PLE;
  PLE.Id = TEXT("path_loss_exponent");
  PLE.Type = EActorAttributeType::Float;
  PLE.RecommendedValues = { TEXT("2.7")};
  

  //FSPL reference distance for LDPL calculation
  FActorVariation FSPL_RefDistance;
  FSPL_RefDistance.Id = TEXT("d_ref");
  FSPL_RefDistance.Type = EActorAttributeType::Float;
  FSPL_RefDistance.RecommendedValues = { TEXT("1.0")};

  //filter distance to speed up calculation
  FActorVariation FilterDistance;
  FilterDistance.Id = TEXT("filter_distance");
  FilterDistance.Type = EActorAttributeType::Float;
  FilterDistance.RecommendedValues = { TEXT("500.0")};

  //etsi fading
  FActorVariation EtsiFading;
  EtsiFading.Id = TEXT("use_etsi_fading");
  EtsiFading.Type = EActorAttributeType::Bool;
  EtsiFading.RecommendedValues = { TEXT("true")};

  //custom fading std deviation
  FActorVariation CustomFadingStddev;
  CustomFadingStddev.Id = TEXT("custom_fading_stddev");
  CustomFadingStddev.Type = EActorAttributeType::Float;
  CustomFadingStddev.RecommendedValues = { TEXT("0.0")};

  // Min Cam Generation
  FActorVariation GenCamMin;
  GenCamMin.Id = TEXT("gen_cam_min");
  GenCamMin.Type = EActorAttributeType::Float;
  GenCamMin.RecommendedValues = { TEXT("0.1")};

  // Max Cam Generation
  FActorVariation GenCamMax;
  GenCamMax.Id = TEXT("gen_cam_max");
  GenCamMax.Type = EActorAttributeType::Float;
  GenCamMax.RecommendedValues = { TEXT("1.0")};

  //Fixed Rate
  FActorVariation FixedRate;
  FixedRate.Id = TEXT("fixed_rate");
  FixedRate.Type = EActorAttributeType::Bool;
  FixedRate.RecommendedValues = { TEXT("false")};

  //path loss model
  FActorVariation PLModel;
  PLModel.Id = TEXT("path_loss_model");
  PLModel.Type = EActorAttributeType::String;
  PLModel.RecommendedValues = { TEXT("winner"), TEXT("geometric")};
  PLModel.bRestrictToRecommended = true;

  //V2x Sensor sends GNSS position in CAM messages
  // - Latitude ----------------------------------
  FActorVariation StdDevLat;
  StdDevLat.Id = TEXT("noise_lat_stddev");
  StdDevLat.Type = EActorAttributeType::Float;
  StdDevLat.RecommendedValues = { TEXT("0.0") };
  StdDevLat.bRestrictToRecommended = false;
  FActorVariation BiasLat;
  BiasLat.Id = TEXT("noise_lat_bias");
  BiasLat.Type = EActorAttributeType::Float;
  BiasLat.RecommendedValues = { TEXT("0.0") };
  BiasLat.bRestrictToRecommended = false;

  // - Longitude ---------------------------------
  FActorVariation StdDevLong;
  StdDevLong.Id = TEXT("noise_lon_stddev");
  StdDevLong.Type = EActorAttributeType::Float;
  StdDevLong.RecommendedValues = { TEXT("0.0") };
  StdDevLong.bRestrictToRecommended = false;
  FActorVariation BiasLong;
  BiasLong.Id = TEXT("noise_lon_bias");
  BiasLong.Type = EActorAttributeType::Float;
  BiasLong.RecommendedValues = { TEXT("0.0") };
  BiasLong.bRestrictToRecommended = false;

  // - Altitude ----------------------------------
  FActorVariation StdDevAlt;
  StdDevAlt.Id = TEXT("noise_alt_stddev");
  StdDevAlt.Type = EActorAttributeType::Float;
  StdDevAlt.RecommendedValues = { TEXT("0.0") };
  StdDevAlt.bRestrictToRecommended = false;
  FActorVariation BiasAlt;
  BiasAlt.Id = TEXT("noise_alt_bias");
  BiasAlt.Type = EActorAttributeType::Float;
  BiasAlt.RecommendedValues = { TEXT("0.0") };
  BiasAlt.bRestrictToRecommended = false;

    // - Heading ----------------------------------
  FActorVariation StdDevHeading;
  StdDevHeading.Id = TEXT("noise_head_stddev");
  StdDevHeading.Type = EActorAttributeType::Float;
  StdDevHeading.RecommendedValues = { TEXT("0.0") };
  StdDevHeading.bRestrictToRecommended = false;
  FActorVariation BiasHeading;
  BiasHeading.Id = TEXT("noise_head_bias");
  BiasHeading.Type = EActorAttributeType::Float;
  BiasHeading.RecommendedValues = { TEXT("0.0") };
  BiasHeading.bRestrictToRecommended = false;
  
  //V2x Sensor sends acceleration in CAM messages
  // - Accelerometer Standard Deviation ----------
  // X Component
  FActorVariation StdDevAccelX;
  StdDevAccelX.Id = TEXT("noise_accel_stddev_x");
  StdDevAccelX.Type = EActorAttributeType::Float;
  StdDevAccelX.RecommendedValues = { TEXT("0.0") };
  StdDevAccelX.bRestrictToRecommended = false;
  // Y Component
  FActorVariation StdDevAccelY;
  StdDevAccelY.Id = TEXT("noise_accel_stddev_y");
  StdDevAccelY.Type = EActorAttributeType::Float;
  StdDevAccelY.RecommendedValues = { TEXT("0.0") };
  StdDevAccelY.bRestrictToRecommended = false;
  // Z Component
  FActorVariation StdDevAccelZ;
  StdDevAccelZ.Id = TEXT("noise_accel_stddev_z");
  StdDevAccelZ.Type = EActorAttributeType::Float;
  StdDevAccelZ.RecommendedValues = { TEXT("0.0") };
  StdDevAccelZ.bRestrictToRecommended = false;

  // Yaw rate
  FActorVariation StdDevYawrate;
  StdDevYawrate.Id = TEXT("noise_yawrate_stddev");
  StdDevYawrate.Type = EActorAttributeType::Float;
  StdDevYawrate.RecommendedValues = { TEXT("0.0") };
  StdDevYawrate.bRestrictToRecommended = false;
  FActorVariation BiasYawrate;
  BiasYawrate.Id = TEXT("noise_yawrate_bias");
  BiasYawrate.Type = EActorAttributeType::Float;
  BiasYawrate.RecommendedValues = { TEXT("0.0") };
  BiasYawrate.bRestrictToRecommended = false; 

  //V2x Sensor sends speed in CAM messages
  // X Component
  FActorVariation StdDevVelX;
  StdDevVelX.Id = TEXT("noise_vel_stddev_x");
  StdDevVelX.Type = EActorAttributeType::Float;
  StdDevVelX.RecommendedValues = { TEXT("0.0") };
  StdDevVelX.bRestrictToRecommended = false;

  Definition.Variations.Append({
    NoiseSeed,
    TransmitPower,
    ReceiverSensitivity,
    Frequency,
    CombinedAntennaGain,
    Scenario,
    PLModel,
    PLE,
    FSPL_RefDistance,
    FilterDistance,
    EtsiFading,
    CustomFadingStddev,
    GenCamMin,
    GenCamMax,
    FixedRate,
    StdDevLat,
    BiasLat,
    StdDevLong,
    BiasLong,
    StdDevAlt,
    BiasAlt,
    StdDevHeading,
    BiasHeading,
    StdDevAccelX,
    StdDevAccelY,
    StdDevAccelZ,
    StdDevYawrate,
    BiasYawrate,
    StdDevVelX});
  Success = CheckActorDefinition(Definition);
}    

FActorDefinition UActorBlueprintFunctionLibrary::MakeCustomV2XDefinition()
{
  FActorDefinition Definition;
  bool Success;
  MakeCustomV2XDefinition(Success, Definition);
  check(Success);
  return Definition;
}

void UActorBlueprintFunctionLibrary::MakeCustomV2XDefinition(
    bool &Success,
    FActorDefinition &Definition)
{
  FillIdAndTags(Definition, TEXT("sensor"), TEXT("other"), TEXT("v2x_custom"));
  AddVariationsForSensor(Definition);

  // - Noise seed --------------------------------
  FActorVariation NoiseSeed;
  NoiseSeed.Id = TEXT("noise_seed");
  NoiseSeed.Type = EActorAttributeType::Int;
  NoiseSeed.RecommendedValues = { TEXT("0") };
  NoiseSeed.bRestrictToRecommended = false;  

  //TransmitPower
  FActorVariation TransmitPower;
  TransmitPower.Id = TEXT("transmit_power");
  TransmitPower.Type = EActorAttributeType::Float;
  TransmitPower.RecommendedValues = { TEXT("21.5")};

  //ReceiveSensitivity
  FActorVariation ReceiverSensitivity;
  ReceiverSensitivity.Id = TEXT("receiver_sensitivity");
  ReceiverSensitivity.Type = EActorAttributeType::Float;
  ReceiverSensitivity.RecommendedValues = { TEXT("-99.0")};

  //Frequency
  FActorVariation Frequency;
  Frequency.Id = TEXT("frequency_ghz");
  Frequency.Type = EActorAttributeType::Float;
  Frequency.RecommendedValues = { TEXT("5.9")};

  //Combined Antenna Gain in dBi
  FActorVariation CombinedAntennaGain;
  CombinedAntennaGain.Id = TEXT("combined_antenna_gain");
  CombinedAntennaGain.Type = EActorAttributeType::Float;
  CombinedAntennaGain.RecommendedValues = { TEXT("10.0")};

  //Scenario
  FActorVariation Scenario;
  Scenario.Id = TEXT("scenario");
  Scenario.Type = EActorAttributeType::String;
  Scenario.RecommendedValues = { TEXT("highway"), TEXT("rural"), TEXT("urban")};
  Scenario.bRestrictToRecommended = true;

  //Path loss exponent
  FActorVariation PLE;
  PLE.Id = TEXT("path_loss_exponent");
  PLE.Type = EActorAttributeType::Float;
  PLE.RecommendedValues = { TEXT("2.7")};
  

  //FSPL reference distance for LDPL calculation
  FActorVariation FSPL_RefDistance;
  FSPL_RefDistance.Id = TEXT("d_ref");
  FSPL_RefDistance.Type = EActorAttributeType::Float;
  FSPL_RefDistance.RecommendedValues = { TEXT("1.0")};

  //filter distance to speed up calculation
  FActorVariation FilterDistance;
  FilterDistance.Id = TEXT("filter_distance");
  FilterDistance.Type = EActorAttributeType::Float;
  FilterDistance.RecommendedValues = { TEXT("500.0")};

  //etsi fading
  FActorVariation EtsiFading;
  EtsiFading.Id = TEXT("use_etsi_fading");
  EtsiFading.Type = EActorAttributeType::Bool;
  EtsiFading.RecommendedValues = { TEXT("true")};

  //custom fading std deviation
  FActorVariation CustomFadingStddev;
  CustomFadingStddev.Id = TEXT("custom_fading_stddev");
  CustomFadingStddev.Type = EActorAttributeType::Float;
  CustomFadingStddev.RecommendedValues = { TEXT("0.0")};

  //path loss model
  FActorVariation PLModel;
  PLModel.Id = TEXT("path_loss_model");
  PLModel.Type = EActorAttributeType::String;
  PLModel.RecommendedValues = { TEXT("winner"), TEXT("geometric")};
  PLModel.bRestrictToRecommended = true;
  
  
  Definition.Variations.Append({
    NoiseSeed,
    TransmitPower,
    ReceiverSensitivity,
    Frequency,
    CombinedAntennaGain,
    Scenario,
    PLModel,
    PLE,
    FSPL_RefDistance,
    FilterDistance,
    EtsiFading,
    CustomFadingStddev
});

  Success = CheckActorDefinition(Definition);
}


FActorDefinition UActorBlueprintFunctionLibrary::MakeGnssDefinition()
{
  FActorDefinition Definition;
  bool Success;
  MakeGnssDefinition(Success, Definition);
  check(Success);
  return Definition;
}

void UActorBlueprintFunctionLibrary::MakeGnssDefinition(
    bool &Success,
    FActorDefinition &Definition)
{
  FillIdAndTags(Definition, TEXT("sensor"), TEXT("other"), TEXT("gnss"));
  AddVariationsForSensor(Definition);

  // - Noise seed --------------------------------
  FActorVariation NoiseSeed;
  NoiseSeed.Id = TEXT("noise_seed");
  NoiseSeed.Type = EActorAttributeType::Int;
  NoiseSeed.RecommendedValues = { TEXT("0") };
  NoiseSeed.bRestrictToRecommended = false;

  // - Latitude ----------------------------------
  FActorVariation StdDevLat;
  StdDevLat.Id = TEXT("noise_lat_stddev");
  StdDevLat.Type = EActorAttributeType::Float;
  StdDevLat.RecommendedValues = { TEXT("0.0") };
  StdDevLat.bRestrictToRecommended = false;
  FActorVariation BiasLat;
  BiasLat.Id = TEXT("noise_lat_bias");
  BiasLat.Type = EActorAttributeType::Float;
  BiasLat.RecommendedValues = { TEXT("0.0") };
  BiasLat.bRestrictToRecommended = false;

  // - Longitude ---------------------------------
  FActorVariation StdDevLong;
  StdDevLong.Id = TEXT("noise_lon_stddev");
  StdDevLong.Type = EActorAttributeType::Float;
  StdDevLong.RecommendedValues = { TEXT("0.0") };
  StdDevLong.bRestrictToRecommended = false;
  FActorVariation BiasLong;
  BiasLong.Id = TEXT("noise_lon_bias");
  BiasLong.Type = EActorAttributeType::Float;
  BiasLong.RecommendedValues = { TEXT("0.0") };
  BiasLong.bRestrictToRecommended = false;

  // - Altitude ----------------------------------
  FActorVariation StdDevAlt;
  StdDevAlt.Id = TEXT("noise_alt_stddev");
  StdDevAlt.Type = EActorAttributeType::Float;
  StdDevAlt.RecommendedValues = { TEXT("0.0") };
  StdDevAlt.bRestrictToRecommended = false;
  FActorVariation BiasAlt;
  BiasAlt.Id = TEXT("noise_alt_bias");
  BiasAlt.Type = EActorAttributeType::Float;
  BiasAlt.RecommendedValues = { TEXT("0.0") };
  BiasAlt.bRestrictToRecommended = false;

  Definition.Variations.Append({
    NoiseSeed,
    StdDevLat,
    BiasLat,
    StdDevLong,
    BiasLong,
    StdDevAlt,
    BiasAlt});

  Success = CheckActorDefinition(Definition);
}

void UActorBlueprintFunctionLibrary::MakeVehicleDefinition(
    const FVehicleParameters &Parameters,
    bool &Success,
    FActorDefinition &Definition)
{
  /// @todo We need to validate here the params.
  FillIdAndTags(Definition, TEXT("vehicle"), Parameters.Make, Parameters.Model);
  AddRecommendedValuesForActorRoleName(Definition,
      {TEXT("autopilot"), TEXT("scenario"), TEXT("ego_vehicle")});
  Definition.Class = Parameters.Class;

  if (Parameters.RecommendedColors.Num() > 0)
  {
    FActorVariation Colors;
    Colors.Id = TEXT("color");
    Colors.Type = EActorAttributeType::RGBColor;
    Colors.bRestrictToRecommended = false;
    for (auto &Color : Parameters.RecommendedColors)
    {
      Colors.RecommendedValues.Emplace(ColorToFString(Color));
    }
    Definition.Variations.Emplace(Colors);
  }

  if (Parameters.SupportedDrivers.Num() > 0)
  {
    FActorVariation Drivers;
    Drivers.Id = TEXT("driver_id");
    Drivers.Type = EActorAttributeType::Int;
    Drivers.bRestrictToRecommended = true;
    for (auto &Id : Parameters.SupportedDrivers)
    {
      Drivers.RecommendedValues.Emplace(FString::FromInt(Id));
    }
    Definition.Variations.Emplace(Drivers);
  }

  FActorVariation StickyControl;
  StickyControl.Id = TEXT("sticky_control");
  StickyControl.Type = EActorAttributeType::Bool;
  StickyControl.bRestrictToRecommended = false;
  StickyControl.RecommendedValues.Emplace(TEXT("true"));
  Definition.Variations.Emplace(StickyControl);

  FActorVariation TerramechanicsAttribute;
  TerramechanicsAttribute.Id = TEXT("terramechanics");
  TerramechanicsAttribute.Type = EActorAttributeType::Bool;
  TerramechanicsAttribute.bRestrictToRecommended = false;
  TerramechanicsAttribute.RecommendedValues.Emplace(TEXT("false"));
  Definition.Variations.Emplace(TerramechanicsAttribute);

  Definition.Attributes.Emplace(FActorAttribute{
    TEXT("object_type"),
    EActorAttributeType::String,
    Parameters.ObjectType});

  Definition.Attributes.Emplace(FActorAttribute{
    TEXT("base_type"),
    EActorAttributeType::String,
    Parameters.BaseType});
  Success = CheckActorDefinition(Definition);

  Definition.Attributes.Emplace(FActorAttribute{
    TEXT("special_type"),
    EActorAttributeType::String,
    Parameters.SpecialType});
  Success = CheckActorDefinition(Definition);

  Definition.Attributes.Emplace(FActorAttribute{
    TEXT("number_of_wheels"),
    EActorAttributeType::Int,
    FString::FromInt(Parameters.NumberOfWheels)});
  Success = CheckActorDefinition(Definition);

  Definition.Attributes.Emplace(FActorAttribute{
    TEXT("generation"),
    EActorAttributeType::Int,
    FString::FromInt(Parameters.Generation)});
  Success = CheckActorDefinition(Definition);

  Definition.Attributes.Emplace(FActorAttribute{
    TEXT("has_dynamic_doors"),
    EActorAttributeType::Bool,
    Parameters.HasDynamicDoors ? TEXT("true") : TEXT("false")});
  Success = CheckActorDefinition(Definition);

  Definition.Attributes.Emplace(FActorAttribute{
    TEXT("has_lights"),
    EActorAttributeType::Bool,
    Parameters.HasLights ? TEXT("true") : TEXT("false")});
  Success = CheckActorDefinition(Definition);
}

template <typename T, typename Functor>
static void FillActorDefinitionArray(
    const TArray<T> &ParameterArray,
    TArray<FActorDefinition> &Definitions,
    Functor Maker)
{
  for (auto &Item : ParameterArray)
  {
    FActorDefinition Definition;
    bool Success = false;
    Maker(Item, Success, Definition);
    if (Success)
    {
      Definitions.Emplace(std::move(Definition));
    }
  }
}

void UActorBlueprintFunctionLibrary::MakeVehicleDefinitions(
    const TArray<FVehicleParameters> &ParameterArray,
    TArray<FActorDefinition> &Definitions)
{
  FillActorDefinitionArray(ParameterArray, Definitions, &MakeVehicleDefinition);
}

void UActorBlueprintFunctionLibrary::MakePedestrianDefinition(
    const FPedestrianParameters &Parameters,
    bool &Success,
    FActorDefinition &Definition)
{
  /// @todo We need to validate here the params.
  FillIdAndTags(Definition, TEXT("walker"),  TEXT("pedestrian"), Parameters.Id);
  AddRecommendedValuesForActorRoleName(Definition, {TEXT("pedestrian")});
  Definition.Class = Parameters.Class;

  auto GetGender = [](EPedestrianGender Value) {
    switch (Value)
    {
      case EPedestrianGender::Female: return TEXT("female");
      case EPedestrianGender::Male:   return TEXT("male");
      default:                        return TEXT("other");
    }
  };

  auto GetAge = [](EPedestrianAge Value) {
    switch (Value)
    {
      case EPedestrianAge::Child:     return TEXT("child");
      case EPedestrianAge::Teenager:  return TEXT("teenager");
      case EPedestrianAge::Elderly:   return TEXT("elderly");
      default:                        return TEXT("adult");
    }
  };

  Definition.Attributes.Emplace(FActorAttribute{
    TEXT("gender"),
    EActorAttributeType::String,
    GetGender(Parameters.Gender)});

  Definition.Attributes.Emplace(FActorAttribute{
    TEXT("generation"),
    EActorAttributeType::Int,
    FString::FromInt(Parameters.Generation)});

  Definition.Attributes.Emplace(FActorAttribute{
    TEXT("age"),
    EActorAttributeType::String,
    GetAge(Parameters.Age)});


  Definition.Attributes.Emplace(FActorAttribute{
    TEXT("can_use_wheelchair"),
    EActorAttributeType::Bool,
    Parameters.bCanUseWheelChair ? TEXT("true") : TEXT("false") });

  if (Parameters.Speed.Num() > 0)
  {
    FActorVariation Speed;
    Speed.Id = TEXT("speed");
    Speed.Type = EActorAttributeType::Float;
    for (auto &Value : Parameters.Speed)
    {
      Speed.RecommendedValues.Emplace(FString::SanitizeFloat(Value));
    }
    Speed.bRestrictToRecommended = false;
    Definition.Variations.Emplace(Speed);
  }

  bool bCanUseWheelChair = Parameters.bCanUseWheelChair;

  FActorVariation IsInvincible;
  IsInvincible.Id = TEXT("is_invincible");
  IsInvincible.Type = EActorAttributeType::Bool;
  IsInvincible.RecommendedValues = { TEXT("true") };
  IsInvincible.bRestrictToRecommended = false;
  Definition.Variations.Emplace(IsInvincible);

  FActorVariation WheelChairVariation;
  WheelChairVariation.Id = TEXT("use_wheelchair");
  WheelChairVariation.Type = EActorAttributeType::Bool;
  if(bCanUseWheelChair)
  {
    WheelChairVariation.RecommendedValues = { TEXT("false"), TEXT("true") };
  }
  else
  {
    WheelChairVariation.RecommendedValues = { TEXT("false") };
  }
  WheelChairVariation.bRestrictToRecommended = true;
  Definition.Variations.Emplace(WheelChairVariation);

  Success = CheckActorDefinition(Definition);
}

void UActorBlueprintFunctionLibrary::MakePedestrianDefinitions(
    const TArray<FPedestrianParameters> &ParameterArray,
    TArray<FActorDefinition> &Definitions)
{
  FillActorDefinitionArray(ParameterArray, Definitions, &MakePedestrianDefinition);
}

void UActorBlueprintFunctionLibrary::MakeTriggerDefinitions(
    const TArray<FString> &ParameterArray,
    TArray<FActorDefinition> &Definitions)
{
  FillActorDefinitionArray(ParameterArray, Definitions, &MakeTriggerDefinition);
}

void UActorBlueprintFunctionLibrary::MakeTriggerDefinition(
    const FString &Id,
    bool &Success,
    FActorDefinition &Definition)
{
  FillIdAndTags(Definition, TEXT("static"), TEXT("trigger"), Id);
  AddVariationsForTrigger(Definition);
  Success = CheckActorDefinition(Definition);
  check(Success);
}

void UActorBlueprintFunctionLibrary::MakePropDefinition(
    const FPropParameters &Parameters,
    bool &Success,
    FActorDefinition &Definition)
{
  /// @todo We need to validate here the params.
  FillIdAndTags(Definition, TEXT("static"),  TEXT("prop"), Parameters.Name);
  AddRecommendedValuesForActorRoleName(Definition, {TEXT("prop")});

  auto GetSize = [](EPropSize Value) {
    switch (Value)
    {
      case EPropSize::Tiny:    return TEXT("tiny");
      case EPropSize::Small:   return TEXT("small");
      case EPropSize::Medium:  return TEXT("medium");
      case EPropSize::Big:     return TEXT("big");
      case EPropSize::Huge:    return TEXT("huge");
      default:                 return TEXT("unknown");
    }
  };

  Definition.Attributes.Emplace(FActorAttribute{
    TEXT("size"),
    EActorAttributeType::String,
    GetSize(Parameters.Size)});

  Success = CheckActorDefinition(Definition);
}

void UActorBlueprintFunctionLibrary::MakePropDefinitions(
    const TArray<FPropParameters> &ParameterArray,
    TArray<FActorDefinition> &Definitions)
{
  FillActorDefinitionArray(ParameterArray, Definitions, &MakePropDefinition);
}

void UActorBlueprintFunctionLibrary::MakeObstacleDetectorDefinitions(
    const FString &Type,
    const FString &Id,
    FActorDefinition &Definition)
{
  Definition = MakeGenericSensorDefinition(TEXT("other"), TEXT("obstacle"));
  AddVariationsForSensor(Definition);
  // Distance.
  FActorVariation distance;
  distance.Id = TEXT("distance");
  distance.Type = EActorAttributeType::Float;
  distance.RecommendedValues = { TEXT("5.0") };
  distance.bRestrictToRecommended = false;
  // HitRadius.
  FActorVariation hitradius;
  hitradius.Id = TEXT("hit_radius");
  hitradius.Type = EActorAttributeType::Float;
  hitradius.RecommendedValues = { TEXT("0.5") };
  hitradius.bRestrictToRecommended = false;
  // Only Dynamics
  FActorVariation onlydynamics;
  onlydynamics.Id = TEXT("only_dynamics");
  onlydynamics.Type = EActorAttributeType::Bool;
  onlydynamics.RecommendedValues = { TEXT("false") };
  onlydynamics.bRestrictToRecommended = false;
  // Debug Line Trace
  FActorVariation debuglinetrace;
  debuglinetrace.Id = TEXT("debug_linetrace");
  debuglinetrace.Type = EActorAttributeType::Bool;
  debuglinetrace.RecommendedValues = { TEXT("false") };
  debuglinetrace.bRestrictToRecommended = false;

  Definition.Variations.Append({
    distance,
    hitradius,
    onlydynamics,
    debuglinetrace
  });

}
/// ============================================================================
/// -- Helpers to retrieve attribute values ------------------------------------
/// ============================================================================

bool UActorBlueprintFunctionLibrary::ActorAttributeToBool(
    const FActorAttribute &ActorAttribute,
    bool Default)
{
  if (ActorAttribute.Type != EActorAttributeType::Bool)
  {
    UE_LOG(LogCarla, Error, TEXT("ActorAttribute '%s' is not a bool"), *ActorAttribute.Id);
    return Default;
  }
  return ActorAttribute.Value.ToBool();
}

int32 UActorBlueprintFunctionLibrary::ActorAttributeToInt(
    const FActorAttribute &ActorAttribute,
    int32 Default)
{
  if (ActorAttribute.Type != EActorAttributeType::Int)
  {
    UE_LOG(LogCarla, Error, TEXT("ActorAttribute '%s' is not an int"), *ActorAttribute.Id);
    return Default;
  }
  return FCString::Atoi(*ActorAttribute.Value);
}

float UActorBlueprintFunctionLibrary::ActorAttributeToFloat(
    const FActorAttribute &ActorAttribute,
    float Default)
{
  if (ActorAttribute.Type != EActorAttributeType::Float)
  {
    UE_LOG(LogCarla, Error, TEXT("ActorAttribute '%s' is not a float"), *ActorAttribute.Id);
    return Default;
  }
  return FCString::Atof(*ActorAttribute.Value);
}

FString UActorBlueprintFunctionLibrary::ActorAttributeToString(
    const FActorAttribute &ActorAttribute,
    const FString &Default)
{
  if (ActorAttribute.Type != EActorAttributeType::String)
  {
    UE_LOG(LogCarla, Error, TEXT("ActorAttribute '%s' is not a string"), *ActorAttribute.Id);
    return Default;
  }
  return ActorAttribute.Value;
}

FColor UActorBlueprintFunctionLibrary::ActorAttributeToColor(
    const FActorAttribute &ActorAttribute,
    const FColor &Default)
{
  if (ActorAttribute.Type != EActorAttributeType::RGBColor)
  {
    UE_LOG(LogCarla, Error, TEXT("ActorAttribute '%s' is not a color"), *ActorAttribute.Id);
    return Default;
  }
  TArray<FString> Channels;
  ActorAttribute.Value.ParseIntoArray(Channels, TEXT(","), false);
  if (Channels.Num() != 3)
  {
    UE_LOG(LogCarla,
        Error,
        TEXT("ActorAttribute '%s': invalid color '%s'"),
        *ActorAttribute.Id,
        *ActorAttribute.Value);
    return Default;
  }
  TArray<uint8> Colors;
  for (auto &Str : Channels)
  {
    auto Val = FCString::Atoi(*Str);
    if ((Val < 0) || (Val > std::numeric_limits<uint8>::max()))
    {
      UE_LOG(LogCarla,
          Error,
          TEXT("ActorAttribute '%s': invalid color '%s'"),
          *ActorAttribute.Id,
          *ActorAttribute.Value);
      return Default;
    }
    Colors.Add(Val);
  }
  FColor Color;
  Color.R = Colors[0u];
  Color.G = Colors[1u];
  Color.B = Colors[2u];
  return Color;
}

bool UActorBlueprintFunctionLibrary::RetrieveActorAttributeToBool(
    const FString &Id,
    const TMap<FString, FActorAttribute> &Attributes,
    bool Default)
{
  return Attributes.Contains(Id) ?
         ActorAttributeToBool(Attributes[Id], Default) :
         Default;
}

int32 UActorBlueprintFunctionLibrary::RetrieveActorAttributeToInt(
    const FString &Id,
    const TMap<FString, FActorAttribute> &Attributes,
    int32 Default)
{
  return Attributes.Contains(Id) ?
         ActorAttributeToInt(Attributes[Id], Default) :
         Default;
}

float UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
    const FString &Id,
    const TMap<FString, FActorAttribute> &Attributes,
    float Default)
{
  return Attributes.Contains(Id) ?
         ActorAttributeToFloat(Attributes[Id], Default) :
         Default;
}

FString UActorBlueprintFunctionLibrary::RetrieveActorAttributeToString(
    const FString &Id,
    const TMap<FString, FActorAttribute> &Attributes,
    const FString &Default)
{
  return Attributes.Contains(Id) ?
         ActorAttributeToString(Attributes[Id], Default) :
         Default;
}

FColor UActorBlueprintFunctionLibrary::RetrieveActorAttributeToColor(
    const FString &Id,
    const TMap<FString, FActorAttribute> &Attributes,
    const FColor &Default)
{
  return Attributes.Contains(Id) ?
         ActorAttributeToColor(Attributes[Id], Default) :
         Default;
}

/// ============================================================================
/// -- Helpers to set Actors ---------------------------------------------------
/// ============================================================================

// Here we do different checks when we are in editor because we don't want the
// editor crashing while people are testing new actor definitions.
#if WITH_EDITOR
#  define CARLA_ABFL_CHECK_ACTOR(ActorPtr)                    \
  if ((ActorPtr == nullptr) || ActorPtr->IsPendingKill())     \
  {                                                           \
    UE_LOG(LogCarla, Error, TEXT("Cannot set empty actor!")); \
    return;                                                   \
  }
#else
#  define CARLA_ABFL_CHECK_ACTOR(ActorPtr) \
  check((ActorPtr != nullptr) && !ActorPtr->IsPendingKill());
#endif // WITH_EDITOR

void UActorBlueprintFunctionLibrary::SetCamera(
    const FActorDescription &Description,
    ASceneCaptureSensor *Camera)
{
  CARLA_ABFL_CHECK_ACTOR(Camera);
  Camera->SetImageSize(
      RetrieveActorAttributeToInt("image_size_x", Description.Variations, 800),
      RetrieveActorAttributeToInt("image_size_y", Description.Variations, 600));
  Camera->SetFOVAngle(
      RetrieveActorAttributeToFloat("fov", Description.Variations, 90.0f));
  if (Description.Variations.Contains("enable_postprocess_effects"))
  {
    Camera->EnablePostProcessingEffects(
        ActorAttributeToBool(
        Description.Variations["enable_postprocess_effects"],
        true));
    Camera->SetTargetGamma(
        RetrieveActorAttributeToFloat("gamma", Description.Variations, 2.2f));
    Camera->SetMotionBlurIntensity(
        RetrieveActorAttributeToFloat("motion_blur_intensity", Description.Variations, 0.5f));
    Camera->SetMotionBlurMaxDistortion(
        RetrieveActorAttributeToFloat("motion_blur_max_distortion", Description.Variations, 5.0f));
    Camera->SetMotionBlurMinObjectScreenSize(
        RetrieveActorAttributeToFloat("motion_blur_min_object_screen_size", Description.Variations, 0.5f));
    Camera->SetLensFlareIntensity(
        RetrieveActorAttributeToFloat("lens_flare_intensity", Description.Variations, 0.1f));
    Camera->SetBloomIntensity(
        RetrieveActorAttributeToFloat("bloom_intensity", Description.Variations, 0.675f));
    // Exposure, histogram mode by default
    if (RetrieveActorAttributeToString("exposure_mode", Description.Variations, "histogram") == "histogram")
    {
      Camera->SetExposureMethod(EAutoExposureMethod::AEM_Histogram);
    }
    else
    {
      Camera->SetExposureMethod(EAutoExposureMethod::AEM_Manual);
    }
    Camera->SetExposureCompensation(
        RetrieveActorAttributeToFloat("exposure_compensation", Description.Variations, 0.0f));
    Camera->SetShutterSpeed(
        RetrieveActorAttributeToFloat("shutter_speed", Description.Variations, 200.0f));
    Camera->SetISO(
        RetrieveActorAttributeToFloat("iso", Description.Variations, 100.0f));
    Camera->SetAperture(
        RetrieveActorAttributeToFloat("fstop", Description.Variations, 1.4f));

    Camera->SetExposureMinBrightness(
        RetrieveActorAttributeToFloat("exposure_min_bright", Description.Variations, 7.0f));
    Camera->SetExposureMaxBrightness(
        RetrieveActorAttributeToFloat("exposure_max_bright", Description.Variations, 9.0f));
    Camera->SetExposureSpeedUp(
        RetrieveActorAttributeToFloat("exposure_speed_up", Description.Variations, 3.0f));
    Camera->SetExposureSpeedDown(
        RetrieveActorAttributeToFloat("exposure_speed_down", Description.Variations, 1.0f));
    Camera->SetExposureCalibrationConstant(
        RetrieveActorAttributeToFloat("calibration_constant", Description.Variations, 16.0f));

    Camera->SetFocalDistance(
        RetrieveActorAttributeToFloat("focal_distance", Description.Variations, 1000.0f));
    Camera->SetDepthBlurAmount(
        RetrieveActorAttributeToFloat("blur_amount", Description.Variations, 1.0f));
    Camera->SetDepthBlurRadius(
        RetrieveActorAttributeToFloat("blur_radius", Description.Variations, 0.0f));
    Camera->SetDepthOfFieldMinFstop(
        RetrieveActorAttributeToFloat("min_fstop", Description.Variations, 1.2f));
    Camera->SetBladeCount(
        RetrieveActorAttributeToInt("blade_count", Description.Variations, 5));

    Camera->SetFilmSlope(
        RetrieveActorAttributeToFloat("slope", Description.Variations, 0.88f));
    Camera->SetFilmToe(
        RetrieveActorAttributeToFloat("toe", Description.Variations, 0.55f));
    Camera->SetFilmShoulder(
        RetrieveActorAttributeToFloat("shoulder", Description.Variations, 0.26f));
    Camera->SetFilmBlackClip(
        RetrieveActorAttributeToFloat("black_clip", Description.Variations, 0.0f));
    Camera->SetFilmWhiteClip(
        RetrieveActorAttributeToFloat("white_clip", Description.Variations, 0.04f));

    Camera->SetWhiteTemp(
        RetrieveActorAttributeToFloat("temp", Description.Variations, 6500.0f));
    Camera->SetWhiteTint(
        RetrieveActorAttributeToFloat("tint", Description.Variations, 0.0f));

    Camera->SetChromAberrIntensity(
        RetrieveActorAttributeToFloat("chromatic_aberration_intensity", Description.Variations, 0.0f));
    Camera->SetChromAberrOffset(
        RetrieveActorAttributeToFloat("chromatic_aberration_offset", Description.Variations, 0.0f));
  }
}

void UActorBlueprintFunctionLibrary::SetCamera(
    const FActorDescription &Description,
    AShaderBasedSensor *Camera)
{
  CARLA_ABFL_CHECK_ACTOR(Camera);
  Camera->SetFloatShaderParameter(0, TEXT("CircleFalloff_NState"),
      RetrieveActorAttributeToFloat("lens_circle_falloff", Description.Variations, 5.0f));
  Camera->SetFloatShaderParameter(0, TEXT("CircleMultiplier_NState"),
      RetrieveActorAttributeToFloat("lens_circle_multiplier", Description.Variations, 0.0f));
  Camera->SetFloatShaderParameter(0, TEXT("K_NState"),
      RetrieveActorAttributeToFloat("lens_k", Description.Variations, -1.0f));
  Camera->SetFloatShaderParameter(0, TEXT("kcube"),
      RetrieveActorAttributeToFloat("lens_kcube", Description.Variations, 0.0f));
  Camera->SetFloatShaderParameter(0, TEXT("XSize_NState"),
      RetrieveActorAttributeToFloat("lens_x_size", Description.Variations, 0.08f));
  Camera->SetFloatShaderParameter(0, TEXT("YSize_NState"),
      RetrieveActorAttributeToFloat("lens_y_size", Description.Variations, 0.08f));
}

void UActorBlueprintFunctionLibrary::SetLidar(
    const FActorDescription &Description,
    FLidarDescription &Lidar)
{
  constexpr float TO_CENTIMETERS = 1e2;
  Lidar.Channels =
      RetrieveActorAttributeToInt("channels", Description.Variations, Lidar.Channels);
  Lidar.Range =
      RetrieveActorAttributeToFloat("range", Description.Variations, 10.0f) * TO_CENTIMETERS;
  Lidar.PointsPerSecond =
      RetrieveActorAttributeToInt("points_per_second", Description.Variations, Lidar.PointsPerSecond);
  Lidar.RotationFrequency =
      RetrieveActorAttributeToFloat("rotation_frequency", Description.Variations, Lidar.RotationFrequency);
  Lidar.UpperFovLimit =
      RetrieveActorAttributeToFloat("upper_fov", Description.Variations, Lidar.UpperFovLimit);
  Lidar.LowerFovLimit =
      RetrieveActorAttributeToFloat("lower_fov", Description.Variations, Lidar.LowerFovLimit);
  Lidar.HorizontalFov =
      RetrieveActorAttributeToFloat("horizontal_fov", Description.Variations, Lidar.HorizontalFov);
  Lidar.AtmospAttenRate =
      RetrieveActorAttributeToFloat("atmosphere_attenuation_rate", Description.Variations, Lidar.AtmospAttenRate);
  Lidar.RandomSeed =
      RetrieveActorAttributeToInt("noise_seed", Description.Variations, Lidar.RandomSeed);
  Lidar.DropOffGenRate =
      RetrieveActorAttributeToFloat("dropoff_general_rate", Description.Variations, Lidar.DropOffGenRate);
  Lidar.DropOffIntensityLimit =
      RetrieveActorAttributeToFloat("dropoff_intensity_limit", Description.Variations, Lidar.DropOffIntensityLimit);
  Lidar.DropOffAtZeroIntensity =
      RetrieveActorAttributeToFloat("dropoff_zero_intensity", Description.Variations, Lidar.DropOffAtZeroIntensity);
  Lidar.NoiseStdDev =
      RetrieveActorAttributeToFloat("noise_stddev", Description.Variations, Lidar.NoiseStdDev);
}

void UActorBlueprintFunctionLibrary::SetGnss(
    const FActorDescription &Description,
    AGnssSensor *Gnss)
{
  CARLA_ABFL_CHECK_ACTOR(Gnss);
  if (Description.Variations.Contains("noise_seed"))
  {
    Gnss->SetSeed(
      RetrieveActorAttributeToInt("noise_seed", Description.Variations, 0));
  }
  else
  {
    Gnss->SetSeed(Gnss->GetRandomEngine()->GenerateRandomSeed());
  }

  Gnss->SetLatitudeDeviation(
      RetrieveActorAttributeToFloat("noise_lat_stddev", Description.Variations, 0.0f));
  Gnss->SetLongitudeDeviation(
      RetrieveActorAttributeToFloat("noise_lon_stddev", Description.Variations, 0.0f));
  Gnss->SetAltitudeDeviation(
      RetrieveActorAttributeToFloat("noise_alt_stddev", Description.Variations, 0.0f));
  Gnss->SetLatitudeBias(
      RetrieveActorAttributeToFloat("noise_lat_bias", Description.Variations, 0.0f));
  Gnss->SetLongitudeBias(
      RetrieveActorAttributeToFloat("noise_lon_bias", Description.Variations, 0.0f));
  Gnss->SetAltitudeBias(
      RetrieveActorAttributeToFloat("noise_alt_bias", Description.Variations, 0.0f));
}

void UActorBlueprintFunctionLibrary::SetIMU(
    const FActorDescription &Description,
    AInertialMeasurementUnit *IMU)
{
  CARLA_ABFL_CHECK_ACTOR(IMU);
  if (Description.Variations.Contains("noise_seed"))
  {
    IMU->SetSeed(
        RetrieveActorAttributeToInt("noise_seed", Description.Variations, 0));
  }
  else
  {
    IMU->SetSeed(IMU->GetRandomEngine()->GenerateRandomSeed());
  }

  IMU->SetAccelerationStandardDeviation({
      RetrieveActorAttributeToFloat("noise_accel_stddev_x", Description.Variations, 0.0f),
      RetrieveActorAttributeToFloat("noise_accel_stddev_y", Description.Variations, 0.0f),
      RetrieveActorAttributeToFloat("noise_accel_stddev_z", Description.Variations, 0.0f)});

  IMU->SetGyroscopeStandardDeviation({
      RetrieveActorAttributeToFloat("noise_gyro_stddev_x", Description.Variations, 0.0f),
      RetrieveActorAttributeToFloat("noise_gyro_stddev_y", Description.Variations, 0.0f),
      RetrieveActorAttributeToFloat("noise_gyro_stddev_z", Description.Variations, 0.0f)});

  IMU->SetGyroscopeBias({
      RetrieveActorAttributeToFloat("noise_gyro_bias_x", Description.Variations, 0.0f),
      RetrieveActorAttributeToFloat("noise_gyro_bias_y", Description.Variations, 0.0f),
      RetrieveActorAttributeToFloat("noise_gyro_bias_z", Description.Variations, 0.0f)});
}

void UActorBlueprintFunctionLibrary::SetRadar(
    const FActorDescription &Description,
    ARadar *Radar)
{
  CARLA_ABFL_CHECK_ACTOR(Radar);
  constexpr float TO_CENTIMETERS = 1e2;

  if (Description.Variations.Contains("noise_seed"))
  {
    Radar->SetSeed(
      RetrieveActorAttributeToInt("noise_seed", Description.Variations, 0));
  }
  else
  {
    Radar->SetSeed(Radar->GetRandomEngine()->GenerateRandomSeed());
  }

  Radar->SetHorizontalFOV(
      RetrieveActorAttributeToFloat("horizontal_fov", Description.Variations, 30.0f));
  Radar->SetVerticalFOV(
      RetrieveActorAttributeToFloat("vertical_fov", Description.Variations, 30.0f));
  Radar->SetRange(
      RetrieveActorAttributeToFloat("range", Description.Variations, 100.0f) * TO_CENTIMETERS);
  Radar->SetPointsPerSecond(
      RetrieveActorAttributeToInt("points_per_second", Description.Variations, 1500));
}

void UActorBlueprintFunctionLibrary::SetV2X(
    const FActorDescription &Description,
    AV2XSensor* V2X)
{
  CARLA_ABFL_CHECK_ACTOR(V2X);
  if (Description.Variations.Contains("noise_seed"))
  {
    V2X->SetSeed(
      RetrieveActorAttributeToInt("noise_seed", Description.Variations, 0));
  }
  else
  {
    V2X->SetSeed(V2X->GetRandomEngine()->GenerateRandomSeed());
  }

  V2X->SetPropagationParams(
    RetrieveActorAttributeToFloat("transmit_power", Description.Variations, 21.5),
    RetrieveActorAttributeToFloat("receiver_sensitivity", Description.Variations, -99.0),
    RetrieveActorAttributeToFloat("frequency_ghz", Description.Variations, 5.9),
    RetrieveActorAttributeToFloat("combined_antenna_gain", Description.Variations, 10.0),
    RetrieveActorAttributeToFloat("path_loss_exponent", Description.Variations, 2.7),
    RetrieveActorAttributeToFloat("d_ref", Description.Variations, 1.0),
    RetrieveActorAttributeToFloat("filter_distance", Description.Variations, 500.0),
    RetrieveActorAttributeToBool("use_etsi_fading", Description.Variations, true),
    RetrieveActorAttributeToFloat("custom_fading_stddev", Description.Variations, 0.0f)
    );

    if (RetrieveActorAttributeToString("scenario", Description.Variations, "urban") == "urban")
    {
        V2X->SetScenario(EScenario::Urban);
    }
    else if (RetrieveActorAttributeToString("scenario", Description.Variations, "urban") == "rural")
    {
        V2X->SetScenario(EScenario::Rural);
    }
    else
    {
      V2X->SetScenario(EScenario::Highway);
    }    

    V2X->SetCaServiceParams(
        RetrieveActorAttributeToFloat("gen_cam_min", Description.Variations, 0.1), 
        RetrieveActorAttributeToFloat("gen_cam_max", Description.Variations, 1.0),
        RetrieveActorAttributeToBool("fixed_rate", Description.Variations, false));

    V2X->SetAccelerationStandardDeviation({
        RetrieveActorAttributeToFloat("noise_accel_stddev_x", Description.Variations, 0.0f),
        RetrieveActorAttributeToFloat("noise_accel_stddev_y", Description.Variations, 0.0f),
        RetrieveActorAttributeToFloat("noise_accel_stddev_z", Description.Variations, 0.0f)});    

    V2X->SetGNSSDeviation(
        RetrieveActorAttributeToFloat("noise_lat_stddev", Description.Variations, 0.0f),
        RetrieveActorAttributeToFloat("noise_lon_stddev", Description.Variations, 0.0f),
        RetrieveActorAttributeToFloat("noise_alt_stddev", Description.Variations, 0.0f),
        RetrieveActorAttributeToFloat("noise_head_stddev", Description.Variations, 0.0f),
        RetrieveActorAttributeToFloat("noise_lat_bias", Description.Variations, 0.0f),
        RetrieveActorAttributeToFloat("noise_lon_bias", Description.Variations, 0.0f),
        RetrieveActorAttributeToFloat("noise_alt_bias", Description.Variations, 0.0f),
        RetrieveActorAttributeToFloat("noise_head_bias", Description.Variations, 0.0f)); 

    V2X->SetVelDeviation(
        RetrieveActorAttributeToFloat("noise_vel_stddev_x", Description.Variations, 0.0f)
    );
    V2X->SetYawrateDeviation(
        RetrieveActorAttributeToFloat("noise_yawrate_stddev", Description.Variations, 0.0f),
        RetrieveActorAttributeToFloat("noise_yawrate_bias", Description.Variations, 0.0f)
    );

    if (RetrieveActorAttributeToString("path_loss_model", Description.Variations, "geometric") == "winner")
    {
        V2X->SetPathLossModel(EPathLossModel::Winner);
    }
    else if(RetrieveActorAttributeToString("path_loss_model", Description.Variations, "geometric") == "geometric")
    {
        V2X->SetPathLossModel(EPathLossModel::Geometric);
    }


}

void UActorBlueprintFunctionLibrary::SetCustomV2X(
    const FActorDescription &Description,
    ACustomV2XSensor* V2X)
{
  CARLA_ABFL_CHECK_ACTOR(V2X);
  if (Description.Variations.Contains("noise_seed"))
  {
    V2X->SetSeed(
      RetrieveActorAttributeToInt("noise_seed", Description.Variations, 0));
  }
  else
  {
    V2X->SetSeed(V2X->GetRandomEngine()->GenerateRandomSeed());
  }

  V2X->SetPropagationParams(
    RetrieveActorAttributeToFloat("transmit_power", Description.Variations, 21.5),
    RetrieveActorAttributeToFloat("receiver_sensitivity", Description.Variations, -99.0),
    RetrieveActorAttributeToFloat("frequency_ghz", Description.Variations, 5.9),
    RetrieveActorAttributeToFloat("combined_antenna_gain", Description.Variations, 10.0),
    RetrieveActorAttributeToFloat("path_loss_exponent", Description.Variations, 2.7),
    RetrieveActorAttributeToFloat("d_ref", Description.Variations, 1.0),
    RetrieveActorAttributeToFloat("filter_distance", Description.Variations, 500.0),
    RetrieveActorAttributeToBool("use_etsi_fading", Description.Variations, true),
    RetrieveActorAttributeToFloat("custom_fading_stddev", Description.Variations, 0.0f)
    );

    if (RetrieveActorAttributeToString("scenario", Description.Variations, "urban") == "urban")
    {
        V2X->SetScenario(EScenario::Urban);
    }
    else if (RetrieveActorAttributeToString("scenario", Description.Variations, "urban") == "rural")
    {
        V2X->SetScenario(EScenario::Rural);
    }
    else
    {
      V2X->SetScenario(EScenario::Highway);
    }    


    if (RetrieveActorAttributeToString("path_loss_model", Description.Variations, "geometric") == "winner")
    {
        V2X->SetPathLossModel(EPathLossModel::Winner);
    }
    else if(RetrieveActorAttributeToString("path_loss_model", Description.Variations, "geometric") == "geometric")
    {
        V2X->SetPathLossModel(EPathLossModel::Geometric);
    }


}
#undef CARLA_ABFL_CHECK_ACTOR
