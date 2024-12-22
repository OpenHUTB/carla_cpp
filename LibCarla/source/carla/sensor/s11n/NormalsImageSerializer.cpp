//
// Created by flo on 09.11.20.
//

#include "NormalsImageSerializer.h"
#include "carla/sensor/s11n/NormalsImageSerializer.h"

#include "carla/sensor/data/Image.h"

namespace carla {
  namespace sensor {
    namespace s11n {

      // NormalsImageSerializer类的Deserialize函数定义，该函数接收一个右值引用类型的RawData参数，用于反序列化操作。
      // 其作用可能是将传入的原始数据（RawData）转换为对应的传感器数据结构（这里推测是和法线图像相关的数据结构）。

     SharedPtr<SensorData> NormalsImageSerializer::Deserialize(RawData &&data) {
        // 创建一个指向data::NormalsImage类型对象的智能指针（SharedPtr），通过移动构造函数将传入的原始数据（data）传递给data::NormalsImage的构造函数来初始化该对象。
        // 这里的data::NormalsImage应该是一种表示法线图像数据的自定义类型，它接受移动进来的原始数据来构造自身。
        auto image = SharedPtr<data::NormalsImage>(new data::NormalsImage{std::move(data)});
        // 将创建好的代表法线图像数据的智能指针（image）作为函数返回值返回，意味着该函数返回经过反序列化操作后得到的法线图像相关的传感器数据。
        return image;

    } // namespace s11n
  } // namespace sensor
} // namespace carla
