// 版权 (c) 2022 巴萨大学计算机视觉中心 (CVC)。  
// 本作品根据MIT许可证条款授权。  
// 许可证副本见 <https://opensource.org/licenses/MIT>。  

#pragma once  
#define _GLIBCXX_USE_CXX11_ABI 0  

#include <memory>  
#include <vector>  

#include "CarlaPublisher.h"  

namespace carla {  
namespace ros2 {  

// 前向声明  
struct CarlaOpticalFlowCameraPublisherImpl;  
struct CarlaCameraInfoPublisherImpl;  

// Carla光流相机发布器类  
class CarlaOpticalFlowCameraPublisher : public CarlaPublisher {  
public:  
    // 构造函数，默认ROS名称和父节点  
    CarlaOpticalFlowCameraPublisher(const char* ros_name = "", const char* parent = "");  
    // 析构函数  
    ~CarlaOpticalFlowCameraPublisher();  
    
    // 拷贝构造函数  
    CarlaOpticalFlowCameraPublisher(const CarlaOpticalFlowCameraPublisher&);  
    // 拷贝赋值运算符  
    CarlaOpticalFlowCameraPublisher& operator=(const CarlaOpticalFlowCameraPublisher&);  
    
    // 移动构造函数  
    CarlaOpticalFlowCameraPublisher(CarlaOpticalFlowCameraPublisher&&);  
    // 移动赋值运算符  
    CarlaOpticalFlowCameraPublisher& operator=(CarlaOpticalFlowCameraPublisher&&);  

    // 初始化函数  
    bool Init();  
    // 初始化相机信息数据  
    void InitInfoData(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, float fov, bool do_rectify);  
    // 发布函数  
    bool Publish();  

    // 检查是否已初始化  
    bool HasBeenInitialized() const;  
    // 设置图像数据  
    void SetImageData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, const float* data);  
    // 设置相机信息数据  
    void SetCameraInfoData(int32_t seconds, uint32_t nanoseconds);  
    // 重写类型函数  
    const char* type() const override { return "optical flow camera"; }  

private:  
    // 初始化图像的私有方法  
    bool InitImage();  
    // 初始化信息的私有方法  
    bool InitInfo();  
    // 发布图像的私有方法  
    bool PublishImage();  
    // 发布信息的私有方法  
    bool PublishInfo();  

    // 设置感兴趣区域的私有方法  
    void SetInfoRegionOfInterest(uint32_t x_offset, uint32_t y_offset, uint32_t height, uint32_t width, bool do_rectify);  
    // 设置数据的私有方法  
    void SetData(int32_t seconds, uint32_t nanoseconds, size_t height, size_t width, std::vector<uint8_t>&& data);  

private:  
    // 实现指针  
    std::shared_ptr<CarlaOpticalFlowCameraPublisherImpl> _impl;  
    // 相机信息实现指针  
    std::shared_ptr<CarlaCameraInfoPublisherImpl> _impl_info;  
};  
}  
}