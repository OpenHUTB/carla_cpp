//
// Created by flo on 09.11.20.
//

#pragma once
// 预处理指令
#include "carla/Memory.h"
// 引入carla项目中与内存管理相关的头文件
#include "carla/sensor/RawData.h"
// 引入carla项目里关于传感器原始数据（RawData）的头文件

#include <cstdint>
// 引入C++标准库中定义固定宽度整数类型的头文件
#include <cstring>
// 引入C++标准库中用于字符串和内存操作相关函数

namespace carla {
  namespace sensor {

    class SensorData;
 // 前向声明SensorData类，告知编译器后续会有这个类的完整定义

    namespace s11n {

      /// Serializes image buffers generated by camera sensors.
      class OpticalFlowImageSerializer {
      public:

#pragma pack(push, 1)
// 这是一个预处理指令，用于设置结构体的字节对齐方式为1字节对齐。
        struct ImageHeader {
          uint32_t width;
// 定义一个32位无符号整数类型的成员变量，用于存储图像的宽度信息，单位可能是像素等
          uint32_t height;
 // 同样是32位无符号整数类型，用于存储图像的高度信息，单位通常也是像素。
          float fov_angle;
// 定义一个单精度浮点数类型的成员变量
        };
#pragma pack(pop)
// 恢复之前的默认字节对齐方式

        constexpr static auto header_offset = sizeof(ImageHeader);
// 使用 constexpr 关键字定义一个静态常量，其值为 ImageHeader 结构体的大小（字节数）。

        static const ImageHeader &DeserializeHeader(const RawData &data) {
          return *reinterpret_cast<const ImageHeader *>(data.begin());
// 定义一个静态函数 DeserializeHeader，用于将传入的原始数据（RawData 类型）进行反序列化操作，以获取图像头部信息。

        }

        template <typename Sensor>
        static Buffer Serialize(const Sensor &sensor, Buffer &&bitmap);

        static SharedPtr<SensorData> Deserialize(RawData &&data);
// 定义一个静态函数 Deserialize，用于将传入的原始数据（通过右值引用接收 RawData &&data，避免不必要的拷贝）进行反序列化操作
 // 最终返回一个指向 SensorData 类型的共享智能指针（SharedPtr 表示共享所有权的智能指针，便于内存管理和对象生命周期控制）
  // 即将原始数据还原为相应的传感器数据对象。
      };

      template <typename Sensor>
      inline Buffer OpticalFlowImageSerializer::Serialize(const Sensor &sensor, Buffer &&bitmap) {
        DEBUG_ASSERT(bitmap.size() > sizeof(ImageHeader));
// 使用 DEBUG_ASSERT 宏（通常来自项目中用于调试相关的头文件，此处应该是之前引入的相关文件中定义的）进行断言检查
 // 确保传入的图像缓冲区（bitmap）的大小大于 ImageHeader 结构体的大小，这是为了保证缓冲区有足够空间来存放后续要添加的头部信息
// 防止出现缓冲区溢出等错误情况。

        ImageHeader header = {
            sensor.GetImageWidth(),
            sensor.GetImageHeight(),
            sensor.GetFOVAngle()
        };
 // 创建一个 ImageHeader 结构体对象 header，并通过调用传感器对象（sensor）的相关成员函数来获取图像的宽度、高度和视场角信息，对 header 结构体进行初始化赋值，以便后续将这些信息添加到图像缓冲区作为头部信息。
        std::memcpy(bitmap.data(), reinterpret_cast<const void *>(&header), sizeof(header));
// 使用 std::memcpy 函数（来自 <cstring> 头文件）将 header 结构体的数据复制到图像缓冲区（bitmap）的起始位置，
// 实现将图像头部信息添加到缓冲区的操作，这里需要将 header 结构体指针转换为 void* 类型（通过 reinterpret_cast）来满足 memcpy 函数的参数要求，
 // 并且指定复制的字节数为 header 结构体的大小（sizeof(header)），确保完整复制头部信息。

        return std::move(bitmap);
// 通过移动语义（std::move）返回填充好头部信息的图像缓冲区（bitmap），这样可以避免不必要的拷贝操作，提高函数返回的效率

      }

    } // namespace s11n
  } // namespace sensor
} // namespace carla

