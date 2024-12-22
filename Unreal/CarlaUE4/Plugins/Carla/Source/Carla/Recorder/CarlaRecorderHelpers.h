// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <sstream>
#include <vector>

// 获取最终路径 + 文件名
std::string GetRecorderFilename(std::string Filename);

// ---------
// 录音机
// ---------

// 写入二进制数据（使用 sizeOf（））
template <typename T>
void WriteValue(std::ostream &OutFile, const T &InObj)
{
  OutFile.write(reinterpret_cast<const char *>(&InObj), sizeof(T));
}

template <typename T>
void WriteStdVector(std::ostream &OutFile, const std::vector<T> &InVec)
{
  WriteValue<uint32_t>(OutFile, InVec.size());
  for (const auto& InObj : InVec)
  {
    WriteValue<T>(OutFile, InObj);
  }
}

template <typename T>
void WriteTArray(std::ostream &OutFile, const TArray<T> &InVec)
{
  WriteValue<uint32_t>(OutFile, InVec.Num());
  for (const auto& InObj : InVec)
  {
    WriteValue<T>(OutFile, InObj);
  }
}

// 从 FVector 写入二进制数据
void WriteFVector(std::ostream &OutFile, const FVector &InObj);

// 从 FTransform 写入二进制数据
void WriteFTransform(std::ofstream &OutFile, const FTransform &InObj);
// write binary data from FString (length + text)
void WriteFString(std::ostream &OutFile, const FString &InObj);

// ---------
// 回放
// ---------

// read binary data (using sizeof())
template <typename T>
void ReadValue(std::istream &InFile, T &OutObj)
{
  InFile.read(reinterpret_cast<char *>(&OutObj), sizeof(T));
}

template <typename T>
void ReadStdVector(std::istream &InFile, std::vector<T> &OutVec)
{
  uint32_t VecSize;
  ReadValue<uint32_t>(InFile, VecSize);
  OutVec.clear();
  for (uint32_t i = 0; i < VecSize; ++i)
  {
    T InObj;
    ReadValue<T>(InFile, InObj);
    OutVec.push_back(InObj);
  }
}

template <typename T>
void ReadTArray(std::istream &InFile, TArray<T> &OutVec)
{
  uint32_t VecSize;
  ReadValue<uint32_t>(InFile, VecSize);
  OutVec.Empty();
  for (uint32_t i = 0; i < VecSize; ++i)
  {
    T InObj;
    ReadValue<T>(InFile, InObj);
    OutVec.Add(InObj);
  }
}

// 从 FVector 读取二进制数据
void ReadFVector(std::istream &InFile, FVector &OutObj);

// 从 FTransform 读取二进制数据
void ReadTransform(std::ifstream &InFile, FTransform &OutObj);
// 从 FString 读取二进制数据（长度 + 文本）
void ReadFString(std::istream &InFile, FString &OutObj);
