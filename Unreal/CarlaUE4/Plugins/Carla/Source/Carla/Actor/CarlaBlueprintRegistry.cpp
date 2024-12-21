// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Actor/CarlaBlueprintRegistry.h"
#include "Carla/Game/CarlaStatics.h"

#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace CommonAttributes {
  static const FString PATH = FPaths::ProjectContentDir();
  static const FString DEFAULT = TEXT("/Carla/Config/Default");
  static const FString DEFINITIONS = TEXT("props");
}

namespace PropAttributes {
  static const FString REGISTRY_FORMAT = TEXT(".Package.json");
  static const FString NAME = TEXT("name");
  static const FString MESH_PATH = TEXT("path");
  static const FString SIZE = TEXT("size");
}

static FString PropSizeTypeToString(EPropSize PropSizeType)
{
  const UEnum *ptr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EPropSize"), true);
  if (!ptr)
  {
    return FString("unknown");
  }
  return ptr->GetNameStringByIndex(static_cast<int32>(PropSizeType));
}

static EPropSize StringToPropSizeType(FString PropSize)
{
  const UEnum *EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EPropSize"), true);
  if (EnumPtr)
  {
    return (EPropSize) EnumPtr->GetIndexByName(FName(*PropSize));
  }
  return EPropSize::INVALID;
}

void UCarlaBlueprintRegistry::AddToCarlaBlueprintRegistry(const TArray<FPropParameters> &PropParametersArray)
{
  TArray<TSharedPtr<FJsonValue>> ResultPropJsonArray;

  // 加载默认属性文件
  FString DefaultPropFilePath = CommonAttributes::PATH + CommonAttributes::DEFAULT +
      PropAttributes::REGISTRY_FORMAT;
  FString JsonString;
  FFileHelper::LoadFileToString(JsonString, *DefaultPropFilePath);

  TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
  TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);

  //填充属性数组并将每个属性的索引保存到 PropIndexes 地图中
  TMap<FString, int> PropIndexes;
  if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
  {
    ResultPropJsonArray = JsonObject->GetArrayField(CommonAttributes::DEFINITIONS);
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
