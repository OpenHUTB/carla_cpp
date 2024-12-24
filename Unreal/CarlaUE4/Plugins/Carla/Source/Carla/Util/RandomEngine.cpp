// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "RandomEngine.h"

#include <limits>

// URandomEngine类的命名空间作用域内的函数，用于生成一个随机的64位无符号整数ID
// 该函数被设计为尽量快速执行，因为可能会相对频繁地被调用
// 使用了线程局部存储（thread_local）来保证每个线程有独立的随机数生成器实例，避免多线程下的冲突
// 基于 std::mt19937_64 这个64位的梅森旋转算法随机数引擎来生成随机数
// 通过 std::uniform_int_distribution 来定义一个均匀分布，范围是整个64位无符号整数的取值范围
// 最后从这个分布中获取一个随机数并返回作为随机ID
uint64 URandomEngine::GenerateRandomId()
{
    // Tuned to be fast as this function might get called relatively often.
    // 创建一个线程局部的64位梅森旋转算法随机数引擎实例，使用 std::random_device 来初始化它，以获取一个随机的初始状态
    static thread_local std::mt19937_64 Engine((std::random_device())());
    // 定义一个均匀分布，其取值范围是从64位无符号整数的最小值到最大值
    std::uniform_int_distribution<uint64> Distribution(
            std::numeric_limits<uint64>::lowest(),
            std::numeric_limits<uint64>::max());
    // 使用定义好的随机数引擎和分布来生成一个随机数，并返回该随机数作为随机ID
    return Distribution(Engine);
}

// URandomEngine类的命名空间作用域内的函数，用于生成一个随机的32位有符号整数种子
// 使用 std::random_device 来获取一个基本的随机源，然后通过 std::uniform_int_distribution 定义一个均匀分布
// 范围是整个32位有符号整数的取值范围，最后从这个分布中获取一个随机数作为种子返回
int32 URandomEngine::GenerateRandomSeed()
{
    // 创建一个随机设备对象，它可以作为一个获取随机数的基础源，不同系统下实现方式不同，但通常会利用系统提供的随机源（如硬件熵源等）
    std::random_device RandomDevice;
    // 定义一个均匀分布，其取值范围是从32位有符号整数的最小值到最大值
    std::uniform_int_distribution<int32> Distribution(
            std::numeric_limits<int32>::lowest(),
            std::numeric_limits<int32>::max());
    // 使用随机设备对象和定义好的分布来生成一个随机数，并返回该随机数作为随机种子
    return Distribution(RandomDevice);
}

// URandomEngine类的命名空间作用域内的函数，用于生成一个随机的32位有符号整数种子
// 调用了 GetUniformIntInRange 函数（这里推测其功能应该是在给定的整数范围内获取一个均匀分布的随机整数）
// 传入的范围是整个32位有符号整数的取值范围，最终返回获取到的随机种子
int32 URandomEngine::GenerateSeed()
{
    return GetUniformIntInRange(
            std::numeric_limits<int32>::lowest(),
            std::numeric_limits<int32>::max());
}
