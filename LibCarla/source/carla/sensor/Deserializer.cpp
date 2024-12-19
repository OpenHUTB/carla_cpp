// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// 表明该代码遵循MIT许可协议，可通过对应链接查看协议具体内容
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 引入Carla项目中传感器相关的Deserializer头文件，该文件应该定义了Deserializer类，这里的Deserializer类可能用于将传感器数据从某种序列化格式反序列化回其原始的数据结构，
// 它可能包含了反序列化的具体逻辑和相关的数据成员，用于处理传感器数据的解码等操作，此处只是使用该类的定义，而不负责其具体实现（类的定义在其他文件中，这里通过包含头文件获取其声明）
#include "carla/sensor/Deserializer.h"

// 引入Carla项目中传感器注册相关的SensorRegistry头文件，该文件可能定义了SensorRegistry类，用于管理传感器相关的注册信息，
// 比如记录可用的传感器类型、传感器数据的序列化和反序列化方法等，在这里用于调用其反序列化函数来实现实际的数据反序列化操作，同样只包含头文件获取其声明，不涉及具体实现细节
#include "carla/sensor/SensorRegistry.h"

namespace carla {
namespace sensor {

  // 定义Deserializer类的静态成员函数Deserialize，该函数接收一个右值引用类型的Buffer对象（buffer）作为参数，
  // 其功能是将传入的缓冲区数据（buffer）进行反序列化操作，返回一个指向SensorData类型的共享指针（SharedPtr<SensorData>），
  // 表示反序列化后得到的传感器数据对象，如果反序列化失败，可能返回nullptr或者抛出异常（具体取决于SensorRegistry::Deserialize函数的实现）
  SharedPtr<SensorData> Deserializer::Deserialize(Buffer &&buffer) {
    // 调用SensorRegistry类的静态成员函数Deserialize（该函数应该是在SensorRegistry头文件中定义的用于根据给定的缓冲区数据进行传感器数据反序列化的函数），
    // 并将移动后的缓冲区数据（std::move(buffer)）传递给它，将反序列化得到的传感器数据对象转换为指向SensorData类型的共享指针并返回，
    // 这样调用者就可以通过返回的共享指针获取反序列化后的传感器数据，并进行后续的操作，比如访问传感器数据的属性、进行数据处理等
    return SensorRegistry::Deserialize(std::move(buffer));
  }

} // namespace sensor
} // namespace carla
