// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MsgPack.h"// 包含 MsgPack 相关的头文件，可能用于序列化、反序列化等与消息打包相关的功能（具体取决于其内部实现）
#include "carla/rpc/FloatColor.h"// 包含 FloatColor 相关的头文件，推测 FloatColor 是用于表示某种浮点数类型颜色的数据结构定义
#include "carla/sensor/data/Color.h"// 包含 sensor/data/Color 相关头文件，应该是和传感器数据中的颜色相关的数据结构定义

#include <vector>// 引入标准库中的向量容器，用于存储动态大小的数据集合

namespace carla {// 定义 carla 命名空间，代码应该是属于 carla 项目相关的内容，在这个命名空间下对各类功能进行组织
namespace rpc {// 进一步定义 rpc 子命名空间，可能用于存放远程过程调用（RPC）相关的类、函数等实现

  template<typename T>// 定义一个模板类 Texture，它是一个通用的纹理类，类型参数 T 可以是具体的某种数据类型（比如颜色相关的数据类型）
    // 通过模板的方式，可以方便地创建针对不同类型数据的纹理对象，提高代码复用性
  class Texture {
  public:
 // 默认构造函数，使用默认语法，不进行任何初始化操作，成员变量将使用各自的默认初始化值
    Texture() = default;
// 构造函数，用于创建具有指定宽度（width）和高度（height）的纹理对象
        // 参数 width 表示纹理的宽度，以像素为单位（通常用于图像、纹理等二维数据的横向尺寸描述）
        // 参数 height 表示纹理的高度，同样以像素为单位（用于描述二维数据的纵向尺寸）
    Texture(uint32_t width, uint32_t height)
      : _width(width), _height(height) {// 根据传入的宽度和高度，调整存储纹理数据的向量大小，使其能够容纳对应数量的元素（_width * _height 个 T 类型元素）
      _texture_data.resize(_width*_height);
    }
// 获取纹理宽度的函数，返回纹理对象当前的宽度值（以像素为单位）
    uint32_t GetWidth() const {
      return _width;
    }
// 获取纹理高度的函数，返回纹理对象当前的高度值（以像素为单位）
    uint32_t GetHeight() const {
      return _height;
    }
// 设置纹理尺寸的函数，用于改变纹理对象的宽度和高度
        // 参数 width 是要设置的新宽度值（以像素为单位）
        // 参数 height 是要设置的新高度值（以像素为单位）
        // 内部会更新宽度和高度成员变量，并根据新的尺寸重新调整纹理数据向量的大小
    void SetDimensions(uint32_t width, uint32_t height) {
      _width = width;
      _height = height;
      _texture_data.resize(_width*_height);
    }
// 返回对纹理数据中指定位置（坐标为 (x, y)）元素的引用，可用于修改该位置的元素值
        // 参数 x 表示纹理数据中的横坐标（从左到右的像素位置，范围从 0 到宽度 - 1）
        // 参数 y 表示纹理数据中的纵坐标（从上到下的像素位置，范围从 0 到高度 - 1）
    T& At (uint32_t x, uint32_t y) {
      return _texture_data[y*_width + x];
    }
// 返回对纹理数据中指定位置（坐标为 (x, y)）元素的常引用，用于获取该位置的元素值但不允许修改（适用于 const 对象调用此函数的情况）
        // 参数 x 和 y 的含义与上面的 At 函数中相同，用于定位纹理数据中的具体位置
    const T& At (uint32_t x, uint32_t y) const {
      return _texture_data[y*_width + x];
    }
 // 获取指向纹理数据存储区域的指针（const 版本），通过这个指针可以读取纹理数据，但不能修改
        // 外部代码可以利用这个指针进行高效的数据访问（比如传递给其他需要读取纹理数据的函数等情况）
    const T* GetDataPtr() const {
      return _texture_data.data();
    }

  private:
  	
    uint32_t _width = 0; // 纹理的宽度，以像素为单位，初始值为 0，表示默认未初始化的宽度状态
    uint32_t _height = 0;// 纹理的高度，以像素为单位，初始值为 0，表示默认未初始化的高度状态
    std::vector<T> _texture_data;// 用于存储纹理数据的向量容器，其元素类型为模板参数 T，向量大小会根据纹理的宽度和高度动态调整

  public: // 使用 MsgPack 定义的宏，用于指定在进行消息打包（序列化）和解包（反序列化）时，涉及的成员变量有哪些

    MSGPACK_DEFINE_ARRAY(_width, _height, _texture_data);// 这里指定了宽度、高度以及纹理数据向量这三个成员变量参与相关操作，方便数据的传输和存储等处理
  };

  using TextureColor = Texture<sensor::data::Color>;// 定义一个类型别名 TextureColor，它是基于 Texture 模板类，将模板参数 T 具体指定为 sensor::data::Color 类型的纹理类型
  using TextureFloatColor = Texture<FloatColor>; // 定义一个类型别名 TextureFloatColor，同样基于 Texture 模板类，将模板参数 T 具体指定为 FloatColor 类型的纹理类型

}
}
