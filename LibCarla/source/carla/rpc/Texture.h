// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"// MsgPack相关头文件，可能用于序列化/反序列化等操作
#include "carla/rpc/FloatColor.h"// FloatColor相关头文件，应该是用于表示浮点数类型的颜色相关结构体或类
#include "carla/sensor/data/Color.h"// Color相关头文件，用于表示颜色相关的数据结构，可能是传感器数据中的颜色定义

#include <vector>// 引入标准库中向量容器的头文件，用于存储数据元素的动态数组

namespace carla {
namespace rpc {

  template<typename T>// 定义模板类Texture，用于表示一种纹理数据结构，它可以存储特定类型T的数据
  class Texture {// 这里的T是一个模板参数，意味着可以根据实际需求实例化为不同类型
  public:

    Texture() = default;// 默认构造函数，使用默认的初始化方式，对于内置类型成员会初始化为默认值（如这里的_width和_height会初始化为0）

    Texture(uint32_t width, uint32_t height)// 构造函数，用于创建具有指定宽度和高度的纹理对象
      : _width(width), _height(height) {// 参数width表示纹理的宽度（以像素等为单位，具体取决于使用场景）参数height表示纹理的高度（同样以合适的单位衡量）
      _texture_data.resize(_width*_height);
    }

    uint32_t GetWidth() const {// 获取纹理的宽度，返回值：纹理的宽度值，类型为uint32_t
      return _width;
    }

    uint32_t GetHeight() const {// 获取纹理的高度，返回值：纹理的高度值，类型为uint32_t
      return _height;
    }

    void SetDimensions(uint32_t width, uint32_t height) {// 设置纹理的尺寸（宽度和高度）
      _width = width;// 参数width表示新的宽度值
      _height = height;// 参数height表示新的高度值
      _texture_data.resize(_width*_height);// 该函数会先更新_width和_height成员变量的值，然后根据新的尺寸重新调整_texture_data向量的大小
    }

    T& At (uint32_t x, uint32_t y) {// 返回对纹理中指定坐标（x, y）位置元素的引用，可用于读写该位置的数据
      return _texture_data[y*_width + x];// 参数x表示水平坐标（通常从左到右，起始为0）参数y表示垂直坐标（通常从上到下，起始为0）
    }

    const T& At (uint32_t x, uint32_t y) const {// 返回对纹理中指定坐标（x, y）位置元素的常量引用，用于只读访问该位置的数据
      return _texture_data[y*_width + x];// 参数x表示水平坐标（通常从左到右，起始为0）参数y表示垂直坐标（通常从上到下，起始为0）
    }

    const T* GetDataPtr() const {// 获取指向纹理数据存储区域的指针（常量版本），用于在需要直接访问底层数据数组的场景（比如传递给其他函数进行处理等）
      return _texture_data.data();// 返回值：指向纹理数据存储区域的常量指针，类型为const T*，通过这个指针可以读取数据，但不能修改
    }

  private:

    uint32_t _width = 0;// 纹理的宽度，以合适的单位（如像素等）计量，初始化为0
    uint32_t _height = 0;// 纹理的高度，以合适的单位（如像素等）计量，初始化为0
    std::vector<T> _texture_data;    // 用于存储纹理数据的向量容器，其元素类型为模板参数T，实际存储的数据取决于Texture实例化时指定的类型

  public:

    MSGPACK_DEFINE_ARRAY(_width, _height, _texture_data);// 使用MsgPack定义如何序列化/反序列化这个类的对象，将_width、_height和_texture_data成员变量纳入序列化/反序列化的范围
  };

  using TextureColor = Texture<sensor::data::Color>;// 使用Texture模板类定义一个具体的类型TextureColor，其实例化Texture类时将类型参数T指定为sensor::data::Color
  using TextureFloatColor = Texture<FloatColor>;// 使用Texture模板类定义一个具体的类型TextureFloatColor，其实例化Texture类时将类型参数T指定为FloatColor

}
}
