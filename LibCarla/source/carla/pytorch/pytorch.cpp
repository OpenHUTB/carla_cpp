// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#define _GLIBCXX_USE_CXX11_ABI 0

#include "pytorch.h"

#include <torch/torch.h>
#include <torch/script.h>
#include <torchscatter/scatter.h>
#include <torchcluster/cluster.h>
#include <torch/csrc/jit/passes/tensorexpr_fuser.h>
#include <c10/cuda/CUDACachingAllocator.h>
#include <string>
#include <vector>
#include <ostream>
#include <sstream>
// 定义一个空的函数，用于添加标记
void add_mark(const std::string &text) {

}
// 定义一个命名空间 carla，其中包含另一个命名空间 learning
namespace carla {
namespace learning {
  // 测试学习功能的函数，打印CUDA版本信息
  void test_learning()
  {
    std::ostringstream ss;// 创建一个字符串流
    std::cout << "cuda version " << cluster::cuda_version() << std::endl;// 打印cluster命名空间中定义的cuda_version()函数的返回值
    std::cout << "cuda version " << scatter::cuda_version() << std::endl;// 打印scatter命名空间中定义的cuda_version()函数的返回值
    // 下面的代码被注释掉了，如果取消注释，将创建一个3x3的单位张量并打印
    // torch::Tensor tensor = torch::eye(3);
    // std::cout << tensor << std::endl;
  }
// 定义一个函数，用于将WheelInput结构体中的数据转换为PyTorch张量，并将这些张量打包成一个IValue元组返回
  torch::jit::IValue GetWheelTensorInputs(WheelInput& wheel) {
    // 从wheel结构体的particles_positions成员变量创建一个张量，表示粒子的位置。  
    // particles_positions是一个指向粒子位置数据的指针，num_particles是粒子的数量，每个粒子有3个位置坐标（x, y, z）
    at::Tensor particles_position_tensor = 
        torch::from_blob(wheel.particles_positions, 
            {wheel.num_particles, 3}, torch::kFloat32);
    // 从wheel结构体的particles_velocities成员变量创建一个张量，表示粒子的速度。  
    // particles_velocities是一个指向粒子速度数据的指针，格式与粒子位置相同
    at::Tensor particles_velocity_tensor = 
        torch::from_blob(wheel.particles_velocities, 
            {wheel.num_particles, 3}, torch::kFloat32);
    // 从wheel结构体的wheel_positions成员变量创建一个张量，表示车轮的位置。  
    // wheel_positions是一个指向车轮位置数据的指针，车轮位置由3个坐标（x, y, z）表示
    at::Tensor wheel_positions_tensor = 
        torch::from_blob(wheel.wheel_positions, 
            {3}, torch::kFloat32);
    // 从wheel结构体的wheel_oritentation成员变量创建一个张量，表示车轮的朝向
    at::Tensor wheel_oritentation_tensor = 
        torch::from_blob(wheel.wheel_oritentation, 
            {4}, torch::kFloat32);
    // 从wheel结构体的wheel_linear_velocity成员变量创建一个张量，表示车轮的线速度。  
    // wheel_linear_velocity是一个指向车轮线速度数据的指针，由3个分量（x, y, z）表示
    at::Tensor wheel_linear_velocity_tensor = 
        torch::from_blob(wheel.wheel_linear_velocity, 
            {3}, torch::kFloat32);
    // 从wheel结构体的wheel_angular_velocity成员变量创建一个张量，表示车轮的角速度。  
    // wheel_angular_velocity是一个指向车轮角速度数据的指针，同样由3个分量（x, y, z）表示
    at::Tensor wheel_angular_velocity_tensor = 
        torch::from_blob(wheel.wheel_angular_velocity, 
            {3}, torch::kFloat32);
    // 将上述所有张量放入一个IValue向量中
    std::vector<torch::jit::IValue> Tuple 
        {particles_position_tensor, particles_velocity_tensor, wheel_positions_tensor, 
         wheel_oritentation_tensor, wheel_linear_velocity_tensor, wheel_angular_velocity_tensor};
    return torch::ivalue::Tuple::create(Tuple);// 使用torch::ivalue::Tuple::create方法将IValue向量打包成一个IValue元组，并返回
  }
// 定义一个函数，用于从粒子力和轮力张量中提取信息，并填充到一个WheelOutput结构体中
  WheelOutput GetWheelTensorOutput(
      const at::Tensor &particle_forces, // 输入参数：粒子力的张量
      const at::Tensor &wheel_forces ) {// 输入参数：轮力的张量
    WheelOutput result;
    // 获取轮力张量的数据指针，并假定数据类型为float
    const float* wheel_forces_data = wheel_forces.data_ptr<float>();
    // 从轮力张量中提取x, y, z方向的轮力和轮扭矩，并存储到result结构体中
    result.wheel_forces_x = wheel_forces_data[0];
    result.wheel_forces_y = wheel_forces_data[1];
    result.wheel_forces_z = wheel_forces_data[2];
    result.wheel_torque_x = wheel_forces_data[3];
    result.wheel_torque_y = wheel_forces_data[4];
    result.wheel_torque_z = wheel_forces_data[5];
    // 获取粒子力张量的数据指针，并假定数据类型为float 
    const float* particle_forces_data = particle_forces.data_ptr<float>();
    // 定义粒子力的维度数量（假设为3D空间，即x, y, z三个方向）
    int num_dimensions = 3;
    // 获取粒子力张量中粒子的数量
    int num_particles = particle_forces.sizes()[0];
    // 为存储粒子力的向量预留空间，大小为粒子数量乘以每个粒子的维度数量
    result._particle_forces.reserve(num_particles*num_dimensions);
    // 遍历每个粒子，将其x, y, z方向的力添加到result结构体中的粒子力向量中
    for (int i = 0; i < num_particles; i++) {
      result._particle_forces.emplace_back(
          particle_forces_data[i*num_dimensions + 0]);
      result._particle_forces.emplace_back(
          particle_forces_data[i*num_dimensions + 1]);
      result._particle_forces.emplace_back(
          particle_forces_data[i*num_dimensions + 2]);
    }
    return result;
  }
// 该函数接收两个常量引用类型的at::Tensor参数，并返回一个WheelOutput 类型的结果
  WheelOutput GetWheelTensorOutputDynamic(
      const at::Tensor &particle_forces, 
      const at::Tensor &wheel_forces) {
    WheelOutput result;
    const float* wheel_forces_data = wheel_forces.data_ptr<float>();
    // 从wheel_forces中提取轮子X Y Z方向的力
    result.wheel_forces_x = wheel_forces_data[0];
    result.wheel_forces_y = wheel_forces_data[1];
    result.wheel_forces_z = wheel_forces_data[2];
    // 从 particle_forces中提取每个粒子的力
    const float* particle_forces_data = particle_forces.data_ptr<float>();
    int num_dimensions = 3;
    int num_particles = particle_forces.sizes()[0];
    result._particle_forces.reserve(num_particles*num_dimensions);
    for (int i = 0; i < num_particles; i++) {
      result._particle_forces.emplace_back(
          particle_forces_data[i*num_dimensions + 0]);
      result._particle_forces.emplace_back(
          particle_forces_data[i*num_dimensions + 1]);
      result._particle_forces.emplace_back(
          particle_forces_data[i*num_dimensions + 2]);
    }
    return result;
  }

  // 定义一个名为NeuralModelImpl的结构体，它封装了与神经网络模型相关的数据和操作
  struct NeuralModelImpl
  {
    NeuralModelImpl(){}
    // 成员变量：一个PyTorch JIT编译的脚本模块，用于加载和执行神经网络
    torch::jit::script::Module module;
    ~NeuralModelImpl(){}
    // 成员变量：一个存储粒子位置张量的向量，每个张量代表一组粒子的位置信息 
    std::vector<at::Tensor> particles_position_tensors;
    // 成员变量：一个存储粒子速度张量的向量，每个张量代表一组粒子的速度信息
    std::vector<at::Tensor> particles_velocity_tensors;
    // 成员函数：获取与指定车轮相关的输入张量，这些张量将作为神经网络的输入  
    // 参数：  
    //   - wheel：一个引用传递的WheelInput结构体，包含了车轮的输入信息  
    //   - wheel_idx：指定车轮的索引，用于从可能的多组车轮输入中选择一组  
    // 返回值：  
    //   - 一个torch::jit::IValue对象，它封装了神经网络所需的输入张量（或张量的组合）  
    //       这个返回值可以直接被传递给torch::jit::script::Module的forward方法
    torch::jit::IValue GetWheelTensorInputsCUDA(WheelInput& wheel, int wheel_idx);
  };
  torch::jit::IValue NeuralModelImpl::GetWheelTensorInputsCUDA(WheelInput& wheel, int wheel_idx)
  {// 从WheelInput结构体中的粒子位置数组创建一个张量
    at::Tensor particles_position_tensor = 
        torch::from_blob(wheel.particles_positions, // 指向数据的指针
            {wheel.num_particles, 3}, torch::kFloat32);// 张量的形状
 // 从WheelInput结构体中的粒子速度数组创建一个张量，过程与位置张量类似
    at::Tensor particles_velocity_tensor = 
        torch::from_blob(wheel.particles_velocities, 
            {wheel.num_particles, 3}, torch::kFloat32);
// 创建轮子位置张量
    at::Tensor wheel_positions_tensor = 
        torch::from_blob(wheel.wheel_positions, // 原始数据，表示轮子的位置
            {3}, torch::kFloat32);// 张量的维度为3
// 创建轮子方向张量
    at::Tensor wheel_oritentation_tensor = 
        torch::from_blob(wheel.wheel_oritentation, // 原始数据，表示轮子的方向
            {4}, torch::kFloat32);// 表示张量维度为4
// 创建轮子线性速度张量
    at::Tensor wheel_linear_velocity_tensor = 
        torch::from_blob(wheel.wheel_linear_velocity, // 原始数据，表示轮子的线性速度
            {3}, torch::kFloat32);// 表示张量的维度为3
// 创建轮子角速度张量
    at::Tensor wheel_angular_velocity_tensor = 
        torch::from_blob(wheel.wheel_angular_velocity, // 原始数据，表示轮子的角速度
            {3}, torch::kFloat32);// 表示张量的维度为3
// 将所有准备好的张量以及粒子数量（作为一个标量张量或直接作为整数）放入一个向量中
    std::vector<torch::jit::IValue> Tuple 
        {particles_position_tensor.cuda(), particles_velocity_tensor.cuda(), wheel_positions_tensor.cuda(), 
        // 修正了变量名以匹配之前的声明
         wheel_oritentation_tensor.cuda(), wheel_linear_velocity_tensor.cuda(), wheel_angular_velocity_tensor.cuda(),
         wheel.num_particles};// 直接作为整数传递，而不是张量  
    };  
    return torch::ivalue::Tuple::create(Tuple);
  }

  NeuralModel::NeuralModel() {
    // 使用std::make_unique初始化Model成员变量，它是一个指向NeuralModelImpl类型的unique_ptr
    Model = std::make_unique<NeuralModelImpl>();
  }
  void NeuralModel::LoadModel(char* filename, int device) {
    // 禁用TensorExpr融合器，可能是为了避免某些与模型加载或执行不兼容的问题
    torch::jit::setTensorExprFuserEnabled(false);
    // 将传入的char*类型文件名转换为std::string，便于后续操作。
    std::string filename_str(filename);
    // 打印正在加载的模型文件名
    std::cout << "loading " << filename_str << std::endl;
    try {
      // 使用torch::jit::load加载模型文件，并将其存储在ModelImpl类的module成员中
      Model->module = torch::jit::load(filename_str);
      // 构造CUDA设备字符串，格式为"cuda:X"，其中X是传入的设备ID
      std::string cuda_str = "cuda:" + std::to_string(device);
      // 将模型移动到指定的CUDA设备上执行
      // std::cout << "Using CUDA device " << cuda_str << std::endl;
      // Model->module.to(at::Device(cuda_str));
    } catch (const c10::Error& e) {
      // 如果加载模型过程中发生异常（例如文件不存在、格式错误等），则捕获异常并打印错误信息
      std::cout << "Error loading model: " << e.msg() << std::endl;
    }
    // 打印模型加载完成的消息
    std::cout << "loaded " << filename_str <<  std::endl;
  }

  void NeuralModel::SetInputs(Inputs input) {
    _input = input;
  }

// 定义一个成员函数 Forward，用于在神经网络模型中执行前向传播
  void NeuralModel::Forward() {
    // 创建一个存储PyTorch输入数据的向量
    std::vector<torch::jit::IValue> TorchInputs;
    // 将四个轮子的输入数据转换为Tensor并添加到输入向量中  
    TorchInputs.push_back(GetWheelTensorInputs(_input.wheel0));
    TorchInputs.push_back(GetWheelTensorInputs(_input.wheel1));
    TorchInputs.push_back(GetWheelTensorInputs(_input.wheel2));
    TorchInputs.push_back(GetWheelTensorInputs(_input.wheel3));
    // 将驾驶控制输入（转向、油门、刹车）转换为Tensor并添加到输入向量中
    auto drv_inputs = torch::tensor(
        {_input.steering, _input.throttle, _input.braking}, torch::kFloat32); //steer, throtle, brake
    TorchInputs.push_back(drv_inputs);
    // 如果地形类型输入有效，则将其添加到输入向量中
    if (_input.terrain_type >= 0) {
      TorchInputs.push_back(_input.terrain_type);
    }
    // 将是否输出详细信息的标志添加到输入向量中
    TorchInputs.push_back(_input.verbose);
// 定义一个变量用于存储模型的输出
    torch::jit::IValue Output;
    // 尝试执行模型的前向传播，并捕获可能发生的错误
    try {
      Output = Model->module.forward(TorchInputs);
    } catch (const c10::Error& e) {
       // 如果发生错误，打印错误信息  
      std::cout << "Error running model: " << e.msg() << std::endl;
    }
// 将模型的输出转换为元组，并提取其中的Tensor
    std::vector<torch::jit::IValue> Tensors =  Output.toTuple()->elements();
     // 对每个轮子的输出Tensor进行处理，并更新输出结构体中的相应字段
    _output.wheel0 = GetWheelTensorOutput(
        Tensors[0].toTensor().cpu(), Tensors[4].toTensor().cpu() );
    _output.wheel1 = GetWheelTensorOutput(
        Tensors[1].toTensor().cpu(), Tensors[5].toTensor().cpu() );
    _output.wheel2 = GetWheelTensorOutput(
        Tensors[2].toTensor().cpu(), Tensors[6].toTensor().cpu() );
    _output.wheel3 = GetWheelTensorOutput(
        Tensors[3].toTensor().cpu(), Tensors[7].toTensor().cpu() );

  }
  void NeuralModel::ForwardDynamic() {
    {

      std::vector<torch::jit::IValue> TorchInputs;
      TorchInputs.push_back(GetWheelTensorInputs(_input.wheel0));
      TorchInputs.push_back(GetWheelTensorInputs(_input.wheel1));
      TorchInputs.push_back(GetWheelTensorInputs(_input.wheel2));
      TorchInputs.push_back(GetWheelTensorInputs(_input.wheel3));
      auto drv_inputs = torch::tensor(
       // 输入参数：转向、油门、刹车
          {_input.steering, _input.throttle, _input.braking}, torch::kFloat32); //steer, throtle, brake
       // 数据类型为float32
      TorchInputs.push_back(drv_inputs);
      if (_input.terrain_type >= 0) {
        TorchInputs.push_back(_input.terrain_type);
      }
      TorchInputs.push_back(_input.verbose);

      torch::jit::IValue Output;
      try {
         // 调用Model的forward函数，传入TorchInputs，结果存储在Output中
        Output = Model->module.forward(TorchInputs);
      } catch (const c10::Error& e) {
        // 如果捕获到错误，打印错误信息
        std::cout << "Error running model: " << e.msg() << std::endl;
      }

      std::vector<torch::jit::IValue> Tensors =  Output.toTuple()->elements();
      // 获取车轮0的输出动态张量
      _output.wheel0 = GetWheelTensorOutputDynamic(
          Tensors[0].toTensor().cpu(), Tensors[4].toTensor().cpu());
      // 获取车轮1的输出动态张量
      _output.wheel1 = GetWheelTensorOutputDynamic(
          Tensors[1].toTensor().cpu(), Tensors[5].toTensor().cpu());
      // 获取车轮2的输出动态张量
      _output.wheel2 = GetWheelTensorOutputDynamic(
          Tensors[2].toTensor().cpu(), Tensors[6].toTensor().cpu());
      // 获取车轮3的输出动态张量
      _output.wheel3 = GetWheelTensorOutputDynamic(
          Tensors[3].toTensor().cpu(), Tensors[7].toTensor().cpu());

    }

    c10::cuda::CUDACachingAllocator::emptyCache();

  }
// NeuralModel类的ForwardDynamic成员函数，用于执行模型的动态前向传播 
  void NeuralModel::ForwardCUDATensors()
  {
    // 创建一个用于存储模型输入数据的向量TorchInputs
    std::vector<torch::jit::IValue> TorchInputs;
    // 将四个轮子的输入数据（可能是传感器数据）转换为PyTorch张量，并添加到TorchInputs中
    TorchInputs.push_back(Model->GetWheelTensorInputsCUDA(_input.wheel0, 0));
    TorchInputs.push_back(Model->GetWheelTensorInputsCUDA(_input.wheel1, 1));
    TorchInputs.push_back(Model->GetWheelTensorInputsCUDA(_input.wheel2, 2));
    TorchInputs.push_back(Model->GetWheelTensorInputsCUDA(_input.wheel3, 3));
    // 将驾驶控制输入（方向盘转角、油门、刹车）组合成一个PyTorch张量，并添加到TorchInputs中
    auto drv_inputs = torch::tensor(
        {_input.steering, _input.throttle, _input.braking}, torch::kFloat32); //steer, throtle, brake
    TorchInputs.push_back(drv_inputs.cuda());
    // 如果地形类型有效（非负值），则将其作为标量添加到TorchInputs中 
    if (_input.terrain_type >= 0) {
      // 将verbose标志（可能用于控制模型输出的详细程度）添加到TorchInputs中
      TorchInputs.push_back(_input.terrain_type);
    }
    TorchInputs.push_back(_input.verbose);
// 定义一个变量Output，用于存储模型的前向传播结果
    torch::jit::IValue Output;
    // 尝试执行模型的前向传播，如果发生错误（如模型不匹配、输入数据问题等），则捕获异常并打印错误信息
    try {
      Output = Model->module.forward(TorchInputs);
    } catch (const c10::Error& e) {
      std::cout << "Error running model: " << e.msg() << std::endl;
    }
// 将Output转换为包含多个元素的元组，并提取其中的张量数据
    std::vector<torch::jit::IValue> Tensors =  Output.toTuple()->elements();
    // 对每个轮子的输出数据，调用GetWheelTensorOutputDynamic函数进行处理（可能是提取特定的输出值或进行格式转换）
    _output.wheel0 = GetWheelTensorOutput(
        Tensors[0].toTensor().cpu(), Tensors[4].toTensor().cpu() );
    _output.wheel1 = GetWheelTensorOutput(
        Tensors[1].toTensor().cpu(), Tensors[5].toTensor().cpu() );
    _output.wheel2 = GetWheelTensorOutput(
        Tensors[2].toTensor().cpu(), Tensors[6].toTensor().cpu() );
    _output.wheel3 = GetWheelTensorOutput(
        Tensors[3].toTensor().cpu(), Tensors[7].toTensor().cpu() );
  }
  
  Outputs& NeuralModel::GetOutputs() {
    return _output;
  }

  NeuralModel::~NeuralModel() {}

}
}
