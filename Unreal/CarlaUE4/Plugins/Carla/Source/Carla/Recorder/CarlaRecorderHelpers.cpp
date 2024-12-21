// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <fstream>
#include <vector>

#include "UnrealString.h"
#include "CarlaRecorderHelpers.h"

// create a temporal buffer to convert from and to FString and bytes
static std::vector<uint8_t> CarlaRecorderHelperBuffer;

// get the final path + filename
std::string GetRecorderFilename(std::string Filename)
{
    std::string Filename2;  // 定义一个新的字符串来存储最终的文件路径

    // 检查 Filename 是否包含相对路径或绝对路径的元素
    // 通过判断路径中是否包含反斜杠（\）、正斜杠（/）或者冒号（:）来判断是否包含路径
    if (Filename.find("\\") != std::string::npos || Filename.find("/") != std::string::npos || Filename.find(":") != std::string::npos)
        Filename2 = Filename;  // 如果 Filename 已经包含路径，则直接将其赋值给 Filename2
    else
    {
        // 如果 Filename 没有包含路径，说明是相对路径或仅是文件名
        // 获取当前项目的保存目录的完整路径
        FString Path = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir());

        // 将路径转换为 UTF-8 编码的字符串并拼接文件名
        // `TCHAR_TO_UTF8(*Path)` 将 Unreal Engine 的 FString 类型转换为 UTF-8 编码的 std::string
        Filename2 = TCHAR_TO_UTF8(*Path) + Filename;  // 拼接路径和文件名，得到完整路径
    }

    return Filename2;  // 返回最终的完整文件路径
}

// ------
// write
// ------

// write binary data from FVector
void WriteFVector(std::ostream &OutFile, const FVector &InObj)
{
  WriteValue<float>(OutFile, InObj.X);
  WriteValue<float>(OutFile, InObj.Y);
  WriteValue<float>(OutFile, InObj.Z);
}

// write binary data from FTransform
void WriteFTransform(std::ofstream &OutFile, const FTransform &InObj)
{
  WriteFVector(OutFile, InObj.GetTranslation());
  WriteFVector(OutFile, InObj.GetRotation().Euler());
}

// write binary data from FString (length + text)
void WriteFString(std::ostream &OutFile, const FString &InObj)
{
  // encode the string to UTF8 to know the final length
  FTCHARToUTF8 EncodedString(*InObj);
  int16_t Length = EncodedString.Length();
  // write
  WriteValue<uint16_t>(OutFile, Length);
  OutFile.write(reinterpret_cast<char *>(TCHAR_TO_UTF8(*InObj)), Length);
}

// -----
// read
// -----

// read binary data to FVector
void ReadFVector(std::istream &InFile, FVector &OutObj)
{
  ReadValue<float>(InFile, OutObj.X);
  ReadValue<float>(InFile, OutObj.Y);
  ReadValue<float>(InFile, OutObj.Z);
}

// read binary data to FTransform
void ReadFTransform(std::ifstream &InFile, FTransform &OutObj)
{
  FVector Vec;
  ReadFVector(InFile, Vec);
  OutObj.SetTranslation(Vec);
  ReadFVector(InFile, Vec);
  OutObj.GetRotation().MakeFromEuler(Vec);
}

// read binary data to FString (length + text)
void ReadFString(std::istream &InFile, FString &OutObj)
{
  uint16_t Length;
  ReadValue<uint16_t>(InFile, Length);
  // make room in vector buffer
  if (CarlaRecorderHelperBuffer.capacity() < Length + 1)
  {
    CarlaRecorderHelperBuffer.reserve(Length + 1);
  }
  CarlaRecorderHelperBuffer.clear();
  // initialize the vector space with 0
  CarlaRecorderHelperBuffer.resize(Length + 1);
  // read
  InFile.read(reinterpret_cast<char *>(CarlaRecorderHelperBuffer.data()), Length);
  // convert from UTF8 to FString
  OutObj = FString(UTF8_TO_TCHAR(CarlaRecorderHelperBuffer.data()));
}
