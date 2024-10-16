//
// Created by flo on 09.11.20.
//

#pragma once// 防止头文件被重复包含

#include "carla/Memory.h"// 引入 carla 命名空间下的 Memory.h 头文件
#include "carla/sensor/RawData.h"// 引入 carla 命名空间下的 sensor 子命名空间中的 RawData.h 头文件

#include <cstdint>// 引入 C++标准库中的整数类型头文件
#include <cstring>// 引入 C 标准库中的字符串操作函数头文件

namespace carla {
  namespace sensor {

    class SensorData;// 声明名为 SensorData 的类，具体定义可能在其他地方

    namespace s11n {// 声明名为 SensorData 的类，具体定义可能在其他地方

      // 对由相机传感器生成的图像缓冲区进行序列化
      class NormalsImageSerializer {
      public:

// 定义结构体时的特殊编译指令，确保结构体按照 1 字节对齐
#pragma pack(push, 1)
        struct ImageHeader {
          uint32_t width;// 图像宽度，无符号 32 位整数类型
          uint32_t height;// 图像高度，无符号 32 位整数类型
          float fov_angle;// 视场角，浮点类型
        };
#pragma pack(pop)// 恢复默认的内存对齐方式

        constexpr static auto header_offset = sizeof(ImageHeader);// 定义一个常量，表示头部偏移量为 ImageHeader 结构体的大小

        static const ImageHeader &DeserializeHeader(const RawData &data) {
          return *reinterpret_cast<const ImageHeader *>(data.begin());
        }// 静态函数，反序列化头部，根据传入的 RawData 对象，将其起始地址转换为 ImageHeader 指针并返回引用

        template <typename Sensor>
        static Buffer Serialize(const Sensor &sensor, Buffer &&bitmap);// 模板静态函数，对传感器数据和缓冲区进行序列化，传入传感器对象和可变右值引用的缓冲区

        static SharedPtr<SensorData> Deserialize(RawData &&data);// 静态函数，反序列化 RawData 对象为 SensorData 的智能指针
      };

      template <typename Sensor>
      inline Buffer NormalsImageSerializer::Serialize(const Sensor &sensor, Buffer &&bitmap) {
        DEBUG_ASSERT(bitmap.size() > sizeof(ImageHeader));// 调试断言，确保缓冲区大小大于头部结构体的大小
        ImageHeader header = {
            sensor.GetImageWidth(),// 初始化头部结构体的宽度为传感器的图像宽度
            sensor.GetImageHeight(),// 初始化头部结构体的高度为传感器的图像高度
            sensor.GetFOVAngle()// 初始化头部结构体的视场角为传感器的视场角
        };
        std::memcpy(bitmap.data(), reinterpret_cast<const void *>(&header), sizeof(header));// 将头部结构体的数据复制到缓冲区中
        return std::move(bitmap);// 返回移动后的缓冲区
      }

    } // namespace s11n
  } // namespace sensor
} // namespace carla
