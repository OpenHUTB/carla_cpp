// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// 表明该代码遵循MIT许可协议，可通过对应链接查看协议具体内容
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// 引入Carla项目中的Debug头文件，可能用于在代码中进行调试相关的断言操作，确保程序在特定条件下符合预期逻辑，避免出现错误情况
#include "carla/Debug.h"
// 引入Carla项目中传感器数据相关的Array头文件，ImageTmpl类继承自Array<PixelT>，意味着它会复用Array类已有的针对数组类型操作的相关功能和特性，在此基础上构建特定于图像数据的功能
#include "carla/sensor/data/Array.h"
// 引入Carla项目中传感器序列化相关的ImageSerializer头文件，用于对图像数据进行序列化和反序列化操作，方便图像数据在存储、传输等过程中的处理
#include "carla/sensor/s11n/ImageSerializer.h"
// 引入Carla项目中传感器序列化相关的OpticalFlowImageSerializer头文件，推测用于对光流图像数据进行序列化和反序列化操作，可能针对包含光流信息的特殊图像数据进行处理
#include "carla/sensor/s11n/OpticalFlowImageSerializer.h"
// 引入Carla项目中传感器序列化相关的GBufferUint8Serializer头文件，大概率是用于对某种特定格式（可能是无符号8位整数类型相关的缓冲区格式）的图形缓冲区数据进行序列化和反序列化操作
#include "carla/sensor/s11n/GBufferUint8Serializer.h"
// 引入Carla项目中传感器序列化相关的GBufferFloatSerializer头文件，推测是针对浮点型数据格式的图形缓冲区数据进行序列化和反序列化操作的头文件
#include "carla/sensor/s11n/GBufferFloatSerializer.h"
// 引入Carla项目中传感器序列化相关的NormalsImageSerializer头文件，应该是用于对包含法线信息的图像数据进行序列化和反序列化操作，常用于图形学中与光照、渲染等相关的法线贴图等场景
#include "carla/sensor/s11n/NormalsImageSerializer.h"
// 引入Carla项目中与ROS2（机器人操作系统2）相关的头文件，虽然在此处代码中未明确体现其具体使用方式，但可能在更广泛的项目集成中用于和ROS2进行交互、发布图像数据等操作
#include "carla/ros2/ROS2.h"

namespace carla {
namespace sensor {
namespace data {

  // 定义一个模板类ImageTmpl，它是一个针对任意像素类型（由模板参数PixelT指定）的图像类，继承自Array<PixelT>类，以复用数组相关的操作逻辑来处理图像数据（图像可看作是像素的二维数组），并在此基础上拓展图像特有的属性和操作方法。
  /// Templated image for any type of pixel.
  template <typename PixelT>
  class ImageTmpl : public Array<PixelT> {
    // 使用类型别名Super来指代Array<PixelT>类型，方便在后续代码中简洁地表示父类，提高代码可读性和书写便利性，清晰体现继承关系
    using Super = Array<PixelT>;
    // 将carla::ros2::ROS2类声明为友元类，意味着ROS2类可以访问ImageTmpl类的私有成员，可能用于在与ROS2集成时，方便ROS2相关功能对图像数据内部成员的直接操作，比如发布图像数据等情况
    friend class carla::ros2::ROS2;
  protected:
    // 使用类型别名Serializer来指代s11n::ImageSerializer类型，便于后续调用该序列化器相关的函数，用于处理常规图像数据的序列化和反序列化操作，保持代码在数据处理方面的清晰逻辑
    using Serializer = s11n::ImageSerializer;
    // 使用类型别名SerializerOpticalFlow来指代s11n::OpticalFlowImageSerializer类型，方便在处理光流图像数据的序列化和反序列化操作时，更清晰地调用对应的函数，明确针对光流图像的操作逻辑
    using SerializerOpticalFlow = s11n::OpticalFlowImageSerializer;
    // 使用类型别名SerializerNormals来指代s11n::NormalsImageSerializer类型，使得在对包含法线信息的图像数据进行序列化和反序列化操作时，代码能够简洁明了地调用相关函数
    using SerializerNormals = s11n::NormalsImageSerializer;

    // 将Serializer声明为友元类，使其能够访问ImageTmpl类的私有成员，方便在进行常规图像数据的序列化和反序列化操作时，直接操作类内部的数据成员，确保数据处理的完整性和准确性
    friend Serializer;
    // 将SerializerOpticalFlow声明为友元类，便于在处理光流图像数据时，光流图像序列化器能够访问类的私有成员进行相应的数据处理操作
    friend SerializerOpticalFlow;
    // 将s11n::GBufferUint8Serializer声明为友元类，可能用于在涉及无符号8位整数格式的图形缓冲区数据处理时，允许该序列化器访问类内部成员进行数据的序列化和反序列化等操作
    friend s11n::GBufferUint8Serializer;
    // 将s11n::GBufferFloatSerializer声明为友元类，同理，在处理浮点型图形缓冲区数据时，方便该序列化器对类内部数据进行操作
    friend s11n::GBufferFloatSerializer;
    // 将SerializerNormals声明为友元类，使得法线图像序列化器能够访问ImageTmpl类的私有成员，以便在处理包含法线信息的图像数据时进行正确的数据处理
    friend SerializerNormals;

    // 显式定义的模板类构造函数，接收一个右值引用类型的原始数据（RawData &&data），用于根据传入的原始图像数据初始化ImageTmpl对象的相关属性，创建一个有效的图像对象。
    explicit ImageTmpl(RawData &&data)
      // 调用父类（Array<PixelT>）的构造函数，传递序列化器的头部偏移量（Serializer::header_offset）以及移动后的原始数据（std::move(data)），完成父类部分的初始化工作，保证继承体系下的初始化顺序和完整性，同时利用父类的数组相关功能来管理图像数据
      : Super(Serializer::header_offset, std::move(data)) {
      // 使用DEBUG_ASSERT进行调试断言，检查图像的宽度乘以高度是否等于父类（Array<PixelT>）中存储的数据元素个数（即图像像素个数），确保图像数据的一致性和正确性，如果该条件不满足，在调试模式下会触发断言失败提示，便于排查问题
      DEBUG_ASSERT(GetWidth() * GetHeight() == Super::size());
    }

  private:
    // 定义一个私有函数GetHeader，用于获取图像数据的头部信息，通过调用Serializer类的DeserializeHeader方法，从父类存储的原始数据（Super::GetRawData()）中反序列化出图像头部信息，并返回其常量引用，外部代码不能修改该头部信息，头部信息可能包含图像的尺寸、视角等关键元数据
    const auto &GetHeader() const {
      return Serializer::DeserializeHeader(Super::GetRawData());
    }

  public:
    // 使用类型别名pixel_type来指代模板参数PixelT，使得在类内部可以更清晰地表示图像的像素类型，方便代码阅读和书写，例如在一些涉及像素类型相关的操作或函数参数、返回值定义中使用
    using pixel_type = PixelT;

    // 获取图像的宽度（以像素为单位），通过调用GetHeader函数获取图像头部信息，然后返回其中的宽度属性（width），外部代码可以通过调用此方法获取图像在水平方向上的像素数量，用于图像显示、处理等相关操作
    /// Get image width in pixels.
    auto GetWidth() const {
      return GetHeader().width;
    }

    // 获取图像的高度（以像素为单位），同样借助GetHeader函数获取头部信息，并返回其中的高度属性（height），用于确定图像在垂直方向上的像素数量，在诸如图像缩放、裁剪等操作中会经常用到
    /// Get image height in pixels.
    auto GetHeight() const {
      return GetHeader().height;
    }

    // 获取图像的水平视场角（以角度为单位），通过从GetHeader函数获取的头部信息中提取视场角属性（fov_angle）并返回，该视场角信息常用于图形学中的相机模型、图像渲染等场景，反映了图像所覆盖的水平角度范围
    /// Get horizontal field of view of the image in degrees.
    auto GetFOVAngle() const {
      return GetHeader().fov_angle;
    }
  };

} // namespace data
} // namespace sensor
} // namespace carla
