// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#define _GLIBCXX_USE_CXX11_ABI 0

#include <memory>
#include <vector> // 包含标准库中的智能指针和向量容器

namespace carla {
namespace learning {

  void test_learning(); // 测试学习功能的函数

  struct NeuralModelImpl; // 神经网络模型的实现细节

  struct WheelInput {  // 车轮输入数据结构
  public:
    int num_particles = 0; // 粒子数量
    float* particles_positions; // 粒子位置数组
    float* particles_velocities; // 粒子速度数组
    float* wheel_positions; // 车轮位置数组
    float* wheel_oritentation; // 车轮方向数组
    float* wheel_linear_velocity; // 车轮线速度数组
    float* wheel_angular_velocity; // 车轮角速度数组
  };

  struct Inputs { // 输入数据结构
  public:
    WheelInput wheel0;
    WheelInput wheel1;
    WheelInput wheel2;
    WheelInput wheel3; // 四个车轮的输入数据
    float steering = 0;
    float throttle = 0;
    float braking = 0; // 转向、油门和刹车的输入值
    int terrain_type = 0; // 地形类型
    bool verbose = false; // 是否输出详细信息
  };
 
  struct WheelOutput { // 车轮输出数据结构
  public:
    float wheel_forces_x = 0;
    float wheel_forces_y = 0;
    float wheel_forces_z = 0;
    float wheel_torque_x = 0;
    float wheel_torque_y = 0;
    float wheel_torque_z = 0; // 车轮的力和扭矩
    std::vector<float> _particle_forces; // 粒子受力数组
  };
  struct Outputs {
  public:
    WheelOutput wheel0;
    WheelOutput wheel1;
    WheelOutput wheel2;
    WheelOutput wheel3; // 四个车轮的输出数据
  };
 
  // 与PyTorch实现的接口
  class NeuralModel {
  public:

    NeuralModel(); // 构造函数
    void LoadModel(char* filename, int device); // 加载模型

    void SetInputs(Inputs input); // 设置输入数据
    void Forward(); // 执行前向传播
    void ForwardDynamic(); // 执行动态前向传播
    void ForwardCUDATensors(); // 使用CUDA张量执行前向传播
    Outputs& GetOutputs(); // 获取输出数据

    ~NeuralModel();  // 析构函数

  private:
    std::unique_ptr<NeuralModelImpl> Model; // 模型的私有实现
    Inputs _input;
    Outputs _output; // 输入和输出数据
  };

}
}
