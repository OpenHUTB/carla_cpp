// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // 这是一个预处理指令，用于确保头文件只被包含一次，避免重复定义等问题，提高编译效率。
#define _GLIBCXX_USE_CXX11_ABI 0
// 定义了一个宏，用于设置C++标准库的ABI（应用程序二进制接口）版本为0，通常是为了与特定的编译环境或依赖库的二进制兼容性需求相匹配，决定了C++标准库的一些底层实现方式，比如类的内存布局、函数调用约定等。
#include <memory>
#include <vector>
// 引入C++标准库中的 <memory> 和 <vector> 头文件。
// <memory> 头文件提供了智能指针（如 std::shared_ptr、std::unique_ptr 等）相关的定义和功能，用于管理对象的生命周期，避免手动内存管理带来的问题，如内存泄漏、悬空指针等。
// <vector> 头文件用于定义 std::vector 容器，它是一个动态大小的数组，可以方便地存储和操作一组同类型的元素。
#include "CarlaPublisher.h"
// 引入自定义的头文件 "CarlaPublisher.h"，推测这个头文件中应该包含了基类 CarlaPublisher 的定义，可能有一些通用的发布者相关的功能、成员变量或成员函数的声明等，供当前类继承和使用。
namespace carla {
namespace ros2 {
// 定义了 carla::ros2 命名空间，将后续在此命名空间内定义的所有类型、函数等进行封装，避免与其他命名空间中的同名实体发生命名冲突，使得代码结构更加清晰，便于组织和管理相关代码逻辑。
  struct CarlaRGBCameraPublisherImpl;
  struct CarlaCameraInfoPublisherImpl;
// 前置声明两个结构体类型，告诉编译器这两个结构体稍后会在代码中定义（通常在对应的源文件中完整定义其成员等内容），这样在当前代码位置就可以使用这两个结构体类型的指针或引用了，避免编译时因找不到完整定义而报错。
  class CarlaRGBCameraPublisher : public CarlaPublisher { // 定义一个名为 CarlaRGBCameraPublisher 的类，它继承自 CarlaPublisher 类，意味着它会继承 CarlaPublisher 类中的所有 public 和 protected 成员（成员变量和成员函数），并且可以在此基础上扩展自己特有的功能。
    public:
      CarlaRGBCameraPublisher(const char* ros_name = "", const char* parent = ""); // 构造函数声明，用于创建 CarlaRGBCameraPublisher 类的对象。它接受两个可选的参数，分别是表示 ROS 名称（默认为空字符串）和父名称（默认为空字符串）的字符指针，用于初始化对象相关的属性。
      ~CarlaRGBCameraPublisher(); // 析构函数声明，用于在对象生命周期结束时，释放对象所占用的资源，比如清理在构造函数或者其他成员函数中申请的内存等相关操作。
      CarlaRGBCameraPublisher(const CarlaRGBCameraPublisher&); // 拷贝构造函数声明，用于通过已有的同类型对象创建一个新的对象，进行对象的拷贝操作，会将原对象的各个成员变量的值复制到新创建的对象中。
      CarlaRGBCameraPublisher& operator=(const CarlaRGBCameraPublisher&);// 赋值运算符重载函数声明，用于将一个 CarlaRGBCameraPublisher 对象的值赋给另一个同类型的对象，实现对象之间的赋值操作，使得赋值后目标对象具有和源对象相同的状态。
      CarlaRGBCameraPublisher(CarlaRGBCameraPublisher&&); // 移动构造函数声明，用于利用右值引用实现移动语义，通过“窃取”已有对象（通常是临时对象等右值）的资源来创建新对象，而不是像拷贝构造函数那样进行深拷贝，提高性能，原对象在移动后可能处于一种可析构但不再可用的状态。
      CarlaRGBCameraPublisher& operator=(CarlaRGBCameraPublisher&&);
// 移动赋值运算符重载函数声明，和移动构造函数类似，不过是用于实现已有对象之间的移动赋值操作，将一个对象的资源移动到另一个对象中，避免不必要的资源复制开销。
      bool Init(); // 成员函数声明，用于初始化 CarlaRGBCameraPublisher 对象相关的一些资源或者执行一些必要的初始化操作，比如初始化与发布相关的底层组件等，返回值为 bool 类型，用于表示初始化是否成功。
      void InitInfoData(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, float fov, bool do_rectify); // 成员函数声明，用于初始化相机信息相关的数据，接受相机的一些参数，如坐标偏移量（x_offset、y_offset）、图像高度、宽度、视场角（fov）以及是否进行矫正（do_rectify）等参数，无返回值，主要用于设置内部相关的数据成员。
      bool Publish();
// 成员函数声明，用于执行发布操作，将相机相关的数据（如图像数据、相机信息等）发布出去，返回值为 bool 类型，用于表示发布操作是否成功。
      bool HasBeenInitialized() const; // 成员函数声明，用于判断对象是否已经完成初始化，返回值为 bool 类型，const 关键字表示该函数不会修改对象的内部状态，是一个常成员函数。
      void SetImageData(int32_t seconds, uint32_t nanoseconds, uint32_t height, uint32_t width, const uint8_t* data); // 成员函数声明，用于设置图像数据，接受时间戳（秒和纳秒部分）、图像的高度、宽度以及指向图像数据的字节指针作为参数，将这些数据设置到对象内部对应的成员变量中，无返回值，以便后续进行发布等操作。
      void SetCameraInfoData(int32_t seconds, uint32_t nanoseconds);
      const char* type() const override { return "rgb camera"; }
// 重写了基类（CarlaPublisher）中的虚函数 type()，返回一个表示类型的字符串常量 "rgb camera"，用于标识当前发布者对象对应的是 RGB 相机相关的数据发布，const 关键字确保函数不会修改对象状态，override 关键字表明这是对基类虚函数的重写。

    private:
    private: // 以下是类的私有成员部分，只能在类的内部访问，外部代码无法直接访问这些成员，用于封装类的内部实现细节。
      bool InitImage(); // 私有成员函数声明，用于初始化图像相关的内部资源或者执行图像部分的初始化操作，返回值为 bool 类型，用于表示初始化是否成功，具体实现细节可能涉及创建图像相关的底层数据结构、配置参数等，仅在类内部使用。
      bool InitInfo(); // 私有成员函数声明，类似于 InitImage()，不过是用于初始化相机信息相关的内部资源或执行对应的初始化操作，返回值为 bool 类型，用于表示初始化是否成功，同样仅在类内部使用。
      bool PublishImage(); // 私有成员函数声明，用于执行图像数据的发布操作，将内部存储的图像数据发布出去，返回值为 bool 类型，用于表示发布是否成功，属于类内部具体的发布逻辑实现，外部不需要直接调用。
      bool PublishInfo();
// 私有成员函数声明，用于执行相机信息数据的发布操作，将内部存储的相机信息发布出去，返回值为 bool 类型，用于表示发布是否成功，也是类内部具体的发布逻辑实现，外部不会直接调用。
      void SetImageData(int32_t seconds, uint32_t nanoseconds, uint32_t height, uint32_t width, std::vector<uint8_t>&& data);//重载的私有成员函数声明，和上面的 SetImageData 函数类似，不过这里接受的图像数据参数是右值引用形式的 std::vector<uint8_t>，用于更高效地处理图像数据传递，特别是对于临时的或者可移动的图像数据资源，无返回值，同样用于设置内部图像数据相关成员变量。
      void SetInfoRegionOfInterest(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, bool do_rectify);

    private:
      std::shared_ptr<CarlaRGBCameraPublisherImpl> _impl;
      std::shared_ptr<CarlaCameraInfoPublisherImpl> _impl_info; // 定义了两个私有成员变量，都是智能指针类型（std::shared_ptr），分别指向 CarlaRGBCameraPublisherImpl 和 CarlaCameraInfoPublisherImpl 结构体类型的对象。
      // 这两个结构体应该是用于封装 RGB 相机发布者和相机信息发布者的具体内部实现细节，通过智能指针管理它们的生命周期，便于内存管理和资源共享等操作。
  };
}
}
