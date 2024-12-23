// 确保头文件只被包含一次，避免重复定义等问题
#pragma once

// 引入C++标准库中的随机数相关头文件，用于生成随机数相关功能
#include <random>
// 引入无序映射相关头文件，虽然在此代码片段中未体现其具体使用，但可能在更广泛的上下文中会涉及
#include <unordered_map>

// 引入Carla项目中定义ActorId相关的头文件，此处代码未直接体现其使用，但可能与之相关联
#include "carla/rpc/ActorId.h"

namespace carla {
namespace traffic_manager {

// 定义随机数生成器类，用于生成特定范围内的随机数
class RandomGenerator {
public:
    // 构造函数，接收一个无符号64位整数作为随机数生成器的种子
    // 使用该种子初始化一个基于梅森旋转算法的伪随机数生成器（std::mt19937），并设定生成的随机数范围为0.0到100.0
    RandomGenerator(const uint64_t seed): mt(std::mt19937(seed)), dist(0.0, 100.0) {}
    
    // 生成并返回下一个随机数，通过调用std::uniform_real_distribution的操作符，利用已初始化的随机数生成器（mt）来生成符合设定范围（0.0到100.0）的随机数
    double next() { return dist(mt); }

private:
    // 基于梅森旋转算法的伪随机数生成器对象，用于生成伪随机数序列的基础，其状态由传入的种子决定
    std::mt19937 mt;
    // 均匀分布的实数随机数分布对象，定义了生成随机数的范围（在此为0.0到100.0），与随机数生成器（mt）配合使用来生成符合该范围的随机数
    std::uniform_real_distribution<double> dist;
};

} // namespace traffic_manager
} // namespace carla
