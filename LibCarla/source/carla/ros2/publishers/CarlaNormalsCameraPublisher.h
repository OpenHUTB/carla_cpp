// 版权所有 (c) 2022 巴塞罗那自治大学 (UAB) 计算机视觉中心 (CVC)。  
// 本作品根据 MIT 许可证进行许可。  
// 许可证副本请参见 <https://opensource.org/licenses/MIT>。  
#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0
#include <memory>
#include <vector>
#include "CarlaPublisher.h"
namespace carla {
namespace ros2 {
// 前向声明实现类，隐藏实现细节
  struct CarlaNormalsCameraPublisherImpl;
  struct CarlaCameraInfoPublisherImpl;
// CarlaOpticalFlowCameraPublisher 类继承自 CarlaPublisher，用于发布光流相机数据  
  class CarlaNormalsCameraPublisher : public CarlaPublisher {
    public:
// 构造函数，接受 ROS 名称和父节点名称，默认值为空
      CarlaNormalsCameraPublisher(const char* ros_name = "", const char* parent = "");
// 析构函数
      ~CarlaNormalsCameraPublisher();
// 拷贝和移动构造函数及赋值运算符，支持资源管理
      CarlaNormalsCameraPublisher(const CarlaNormalsCameraPublisher&);
      CarlaNormalsCameraPublisher& operator=(const CarlaNormalsCameraPublisher&);
      CarlaNormalsCameraPublisher(CarlaNormalsCameraPublisher&&);
      CarlaNormalsCameraPublisher& operator=(CarlaNormalsCameraPublisher&&);
 // 初始化方法
      bool Init();
// 初始化相机信息数据  
      void InitInfoData(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, float fov, bool do_rectify);
// 发布数据
      bool Publish();
// 查询对象是否已初始化
      bool HasBeenInitialized() const;
// 设置图像数据
      void SetImageData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, const uint8_t* data);
// 设置相机信息数据
      void SetCameraInfoData(int32_t seconds, uint32_t nanoseconds);
// 返回光流相机类型  
      const char* type() const override { return "normals camera"; }
    private:
 // 初始化图像和信息的私有方法
      bool InitImage();
      bool InitInfo();
// 发布图像和信息的私有方法
      bool PublishImage();
      bool PublishInfo();
// 设置区域相关信息
      void SetInfoRegionOfInterest( uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, bool do_rectify);
  // 设置数据
      void SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, std::vector<uint8_t>&& data);
    private:
// 使用智能指针管理实现类的实例
      std::shared_ptr<CarlaNormalsCameraPublisherImpl> _impl;
      std::shared_ptr<CarlaCameraInfoPublisherImpl> _impl_info;
  };
}
}
