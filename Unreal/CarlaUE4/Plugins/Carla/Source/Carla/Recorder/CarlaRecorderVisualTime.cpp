// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaRecorderVisualTime.h"
// 包含 "CarlaRecorderVisualTime.h" 头文件，可能在此头文件中定义了 CarlaRecorderVisualTime 类的相关声明，
// 比如类的成员变量、成员函数原型等内容
#include "CarlaRecorder.h"
// 包含 "CarlaRecorder.h" 头文件，该头文件可能涉及整个Carla记录器的核心功能、数据结构等方面的定义，
// 为当前代码提供与Carla记录器整体相关的一些基础支持
#include "CarlaRecorderHelpers.h"
// 包含 "CarlaRecorderHelpers.h" 头文件，推测此头文件中提供了一些辅助函数或工具函数，用于协助实现
// Carla记录器相关的各种操作，比如下面代码中会用到的读写数据相关的辅助函数等

void CarlaRecorderVisualTime::SetTime(double ThisTime)
// 定义一个名为SetTime的成员函数，属于CarlaRecorderVisualTime类。
// 该函数的作用是设置类中的时间相关数据，接受一个双精度浮点数类型的参数ThisTime，表示要设置的时间值
{
  Time = ThisTime;
  // 将传入的参数ThisTime的值赋给类中的Time成员变量，从而完成对时间的设置操作
}

void CarlaRecorderVisualTime::Read(std::ifstream &InFile)
// 定义一个名为Read的成员函数，属于CarlaRecorderVisualTime类。
// 此函数用于从输入文件流InFile中读取数据，并将读取到的数据设置为类中的相关成员变量的值
{
  ReadValue<double>(InFile, this->Time);
  // 调用一个可能在CarlaRecorderHelpers.h中定义的名为ReadValue的模板函数（根据函数名推测）。
  // 该函数的作用是从给定的输入文件流InFile中读取一个双精度浮点数类型的数据，
  // 并将其赋给当前类（this）的Time成员变量，以完成从文件读取时间数据并设置的操作
}

void CarlaRecorderVisualTime::Write(std::ofstream &OutFile)
// 定义一个名为Write的成员函数，属于CarlaRecorderVisualTime类。
// 此函数用于将类中与视觉时间相关的数据写入到输出文件流OutFile中
{
  // 写入数据包标识符
  WriteValue<char>(OutFile, static_cast<char>(CarlaRecorderPacketId::VisualTime));
  // 调用一个可能在CarlaRecorderHelpers.h中定义的名为WriteValue的模板函数（根据函数名推测）。
  // 首先将CarlaRecorderPacketId::VisualTime（可能是一个枚举值，表示视觉时间相关的数据包ID）
  // 转换为字符类型，然后通过WriteValue函数将其写入到输出文件流OutFile中，以此来标识这个数据包是与视觉时间相关的

  // 写入数据包大小
  uint32_t Total = sizeof(double);
  // 计算要写入的数据包的大小，这里因为要写入的主要数据是双精度浮点数类型的Time变量，
  // 所以通过sizeof(double)来获取双精度浮点数所占的字节数，将其赋值给Total变量，表示数据包的大小
  WriteValue<uint32_t>(OutFile, Total);
  // 再次调用WriteValue函数，将表示数据包大小的Total变量（类型为uint32_t）写入到输出文件流OutFile中

  WriteValue<double>(OutFile, this->Time);
  // 最后一次调用WriteValue函数，将当前类的Time成员变量（双精度浮点数类型）的值写入到输出文件流OutFile中，
  // 从而完成将与视觉时间相关的完整数据写入文件的操作
}
