// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h" // 包含CARLA的主头文件
#include "Carla/Actor/CarlaBlueprintRegistry.h" // 包含CARLA蓝图注册表的头文件
#include "Carla/Game/CarlaStatics.h" // 包含CARLA静态数据的头文件
#include "Dom/JsonObject.h" // 包含JSON对象的头文件
#include "Misc/FileHelper.h" // 包含文件帮助函数的头文件
#include "Serialization/JsonReader.h" // 包含JSON读取器的头文件
#include "Serialization/JsonSerializer.h" // 包含JSON序列化的头文件

namespace CommonAttributes {
  // 定义一些通用属性路径
  static const FString PATH = FPaths::ProjectContentDir(); // 项目内容目录的路径
  static const FString DEFAULT = TEXT("/Carla/Config/Default"); // 默认配置文件的相对路径
  static const FString DEFINITIONS = TEXT("props"); // 定义属性数组的JSON字段名
}

namespace PropAttributes {
  // 定义一些属性相关的JSON字段名
  static const FString REGISTRY_FORMAT = TEXT(".Package.json"); // 属性注册表文件的扩展名
  static const FString NAME = TEXT("name"); // 属性名称的JSON字段名
  static const FString MESH_PATH = TEXT("path"); // 网格路径的JSON字段名
  static const FString SIZE = TEXT("size"); // 属性大小的JSON字段名
}

// 将属性大小枚举转换为字符串
static FString PropSizeTypeToString(EPropSize PropSizeType)
{
  const UEnum *ptr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EPropSize"), true); // 查找EPropSize枚举
  if (!ptr) // 如果枚举未找到
  {
    return FString("unknown"); // 返回未知字符串
  }
  return ptr->GetNameStringByIndex(static_cast<int32>(PropSizeType)); // 返回枚举项的名称字符串
}

// 将字符串转换为属性大小枚举
static EPropSize StringToPropSizeType(FString PropSize)
{
  const UEnum *EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EPropSize"), true); // 查找EPropSize枚举
  if (EnumPtr) // 如果枚举找到
  {
    return (EPropSize)EnumPtr->GetIndexByName(FName(*PropSize)); // 返回枚举项的索引
  }
  return EPropSize::INVALID; // 如果枚举未找到，返回无效值
}

// 将属性添加到CARLA蓝图注册表
void UCarlaBlueprintRegistry::AddToCarlaBlueprintRegistry(const TArray<FPropParameters> &PropParametersArray)
{
  TArray<TSharedPtr<FJsonValue>> ResultPropJsonArray; // 存储属性JSON值的数组
  // 加载默认属性文件
  FString DefaultPropFilePath = CommonAttributes::PATH + CommonAttributes::DEFAULT +
      PropAttributes::REGISTRY_FORMAT; // 构造默认属性文件的完整路径
  FString JsonString; // 存储JSON字符串
  FFileHelper::LoadFileToString(JsonString, *DefaultPropFilePath); // 从文件加载JSON字符串
  TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject()); // 创建JSON对象
  TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString); // 创建JSON读取器
  // 填充属性数组并将每个属性的索引保存到PropIndexes地图中
  TMap<FString, int> PropIndexes; // 存储属性名称和索引的映射
  if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid()) // 如果JSON反序列化成功且对象有效
  {
    ResultPropJsonArray = JsonObject->GetArrayField(CommonAttributes::DEFINITIONS); // 获取属性数组字段
  }
}

    for (int32 i = 0; i < ResultPropJsonArray.Num(); ++i)
    {
      TSharedPtr<FJsonObject> PropJsonObject = ResultPropJsonArray[i]->AsObject();
      FString Name = PropJsonObject->GetStringField(PropAttributes::NAME);
      PropIndexes.Add(Name, i);
    }
  }
  // 添加输入属性或更新它们（如果已存在）
  for (auto &PropParameter : PropParametersArray)
  {
    TSharedPtr<FJsonObject> PropJsonObject;
    // 创建对象或更新现有对象
    int *PropIndex = PropIndexes.Find(PropParameter.Name);
    if (PropIndex)
    {
      PropJsonObject = ResultPropJsonArray[*PropIndex]->AsObject();
    }
    else
    {
      PropJsonObject = MakeShareable(new FJsonObject);
    }
    // 填充属性 JSON
    PropJsonObject->SetStringField(PropAttributes::NAME, PropParameter.Name);
    PropJsonObject->SetStringField(PropAttributes::MESH_PATH, PropParameter.Mesh->GetPathName());
    PropJsonObject->SetStringField(PropAttributes::SIZE, PropSizeTypeToString(PropParameter.Size));
    //添加或更新
    TSharedRef<FJsonValue> PropJsonValue = MakeShareable(new FJsonValueObject(PropJsonObject));
    if (PropIndex)
    {
      ResultPropJsonArray[*PropIndex] = PropJsonValue;
    }
    else
    {
      ResultPropJsonArray.Add(PropJsonValue);
      PropIndexes.Add(PropParameter.Name, ResultPropJsonArray.Num() - 1);
    }
  }
  // 更新Json对象
  JsonObject->SetArrayField(CommonAttributes::DEFINITIONS, ResultPropJsonArray);
  // 序列化文件
  FString OutputString;
  TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
  FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
  // 保存文件
  FFileHelper::SaveStringToFile(OutputString, *DefaultPropFilePath);
}
void UCarlaBlueprintRegistry::LoadPropDefinitions(TArray<FPropParameters> &PropParametersArray)
{
  // 在Unreal Content文件夹中查找所有Package.json文件
  const FString WildCard = FString("*").Append(PropAttributes::REGISTRY_FORMAT);
  TArray<FString> PropFileNames;
  IFileManager::Get().FindFilesRecursive(PropFileNames,
      *CommonAttributes::PATH,
      *WildCard,
      true,
      false,
      false);
  // 如果存在，则先排序并放置默认文件
  PropFileNames.Sort();
  FString DefaultFileName;
  for (int32 i = 0; i < PropFileNames.Num() && DefaultFileName.IsEmpty(); ++i)
  {
    if (PropFileNames[i].Contains(CommonAttributes::DEFAULT))
    {
      DefaultFileName = PropFileNames[i];
      PropFileNames.RemoveAt(i);
    }
  }
  if (!DefaultFileName.IsEmpty())
  {
    PropFileNames.Insert(DefaultFileName, 0);
  }
  // 读取所有注册表文件并用用户值覆盖默认注册表值
  // 注册表文件
  TMap<FString, int> PropIndexes;
  for (int32 i = 0; i < PropFileNames.Num(); ++i)
  {
    FString FileJsonContent;
    if (FFileHelper::LoadFileToString(FileJsonContent, *PropFileNames[i]))
    {
      TSharedPtr<FJsonObject> JsonParsed;
      TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(FileJsonContent);
      if (FJsonSerializer::Deserialize(JsonReader, JsonParsed))
      {
        auto PropJsonArray = JsonParsed->GetArrayField(CommonAttributes::DEFINITIONS);
        for (auto &PropJsonValue : PropJsonArray)
        {
          // 读取属性JSON
          TSharedPtr<FJsonObject> PropJsonObject = PropJsonValue->AsObject();
          FString PropName = PropJsonObject->GetStringField(PropAttributes::NAME);
          FString PropMeshPath = PropJsonObject->GetStringField(PropAttributes::MESH_PATH);
          FString PropSize = PropJsonObject->GetStringField(PropAttributes::SIZE);
          // 构建属性参数
          UStaticMesh *PropMesh = LoadObject<UStaticMesh>(nullptr, *PropMeshPath);
          EPropSize PropSizeType = StringToPropSizeType(PropSize);
          FPropParameters Params {PropName, PropMesh, PropSizeType};
          // 添加或更新
          if (PropIndexes.Contains(PropName))
          {
            PropParametersArray[PropIndexes[PropName]] = Params;
          }
          else
          {
            PropParametersArray.Add(Params);
            PropIndexes.Add(Params.Name, PropParametersArray.Num() - 1);
          }
        }
      }
    }
  }
}
