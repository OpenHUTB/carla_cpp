// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <fstream>
#include <vector>

#include "UnrealString.h"
#include "CarlaRecorderHelpers.h"

// 创建一个临时缓冲区，用于在 FString 和字节数组 (bytes) 之间转换
static std::vector<uint8_t> CarlaRecorderHelperBuffer;

// 获取最终的路径和文件名
std::string GetRecorderFilename(std::string Filename)
{
  std::string Filename2;

  // 检查是否指定了相对路径
  if (Filename.find("\\") != std::string::npos || Filename.find("/") != std::string::npos || Filename.find(":") != std::string::npos)
    Filename2 = Filename;
  else
  {
    FString Path = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir());
    Filename2 = TCHAR_TO_UTF8(*Path) + Filename;
  }

  return Filename2;
}

// ------
// write
// ------

// 将FVector中的二进制数据写出
void WriteFVector(std::ostream &OutFile, const FVector &InObj)
{
  WriteValue<float>(OutFile, InObj.X);
  WriteValue<float>(OutFile, InObj.Y);
  WriteValue<float>(OutFile, InObj.Z);
}

// 将FTransform中的二进制数据写出
void WriteFTransform(std::ofstream &OutFile, const FTransform &InObj)
{
  WriteFVector(OutFile, InObj.GetTranslation());
  WriteFVector(OutFile, InObj.GetRotation().Euler());
}

// 将FString中的二进制数据写出（包括长度和文本内容）
void WriteFString(std::ostream &OutFile, const FString &InObj)
{
  // 将字符串编码为UTF-8以获知最终长度
  FTCHARToUTF8 EncodedString(*InObj);
  int16_t Length = EncodedString.Length();
  // write
  WriteValue<uint16_t>(OutFile, Length);
  OutFile.write(reinterpret_cast<char *>(TCHAR_TO_UTF8(*InObj)), Length);
}

// -----
// read
// -----

// 从二进制数据中读取到 FVector
void ReadFVector(std::istream &InFile, FVector &OutObj)
{
  ReadValue<float>(InFile, OutObj.X);
  ReadValue<float>(InFile, OutObj.Y);
  ReadValue<float>(InFile, OutObj.Z);
}

// 从二进制数据中读取到 FVector
void ReadFTransform(std::ifstream &InFile, FTransform &OutObj)
{
  FVector Vec;
  ReadFVector(InFile, Vec);
  OutObj.SetTranslation(Vec);
  ReadFVector(InFile, Vec);
  OutObj.GetRotation().MakeFromEuler(Vec);
}

// 将FString中的二进制数据写出（包括长度和文本内容）
void ReadFString(std::istream &InFile, FString &OutObj)
{
  uint16_t Length;
  ReadValue<uint16_t>(InFile, Length);
  // 在向量缓冲区中腾出空间
  if (CarlaRecorderHelperBuffer.capacity() < Length + 1)
  {
    CarlaRecorderHelperBuffer.reserve(Length + 1);
  }
  CarlaRecorderHelperBuffer.clear();
  // 将向量空间初始化为0
  CarlaRecorderHelperBuffer.resize(Length + 1);
  // 读取
  InFile.read(reinterpret_cast<char *>(CarlaRecorderHelperBuffer.data()), Length);
  // 将UTF-8编码的字符串转换为FString
  OutObj = FString(UTF8_TO_TCHAR(CarlaRecorderHelperBuffer.data()));
}
