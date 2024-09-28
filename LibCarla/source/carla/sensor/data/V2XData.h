// Copyright (c) 2024 Institut fuer Technik der Informationsverarbeitung (ITIV) at the
// Karlsruhe Institute of Technology
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
#pragma once // 防止头文件被多次包含

#include <cstdint> // 包含整数类型定义
#include <vector>  // 包含向量容器
#include <cstdio>  // 包含C标准输入输出库
#include "LibITS.h" // 包含LibITS库的头文件

namespace carla // 定义命名空间carla
{
    namespace sensor // 定义命名空间sensor
    {

        namespace s11n // 定义命名空间s11n
        {
            class CAMDataSerializer; // 前向声明CAMDataSerializer类
            class CustomV2XDataSerializer; // 前向声明CustomV2XDataSerializer类
        }

        namespace data // 定义命名空间data
        {
            class CAMData // 定义CAMData类
            {
            public:
                float Power; // 电源功率
                CAM_t Message; // CAM消息
            };

            class CustomV2XData // 定义CustomV2XData类
            {
            public:
                float Power; // 电源功率
                CustomV2XM Message; // 自定义V2X消息
            };

            class CAMDataS // 定义CAMDataS类
            {

            public:
                explicit CAMDataS() = default; // 默认构造函数

                CAMDataS &operator=(CAMDataS &&) = default; // 移动赋值运算符

                // 返回当前接收到的消息数量
                size_t GetMessageCount() const 
                {
                    return MessageList.size(); // 返回消息列表的大小
                }

                // 删除当前消息
                void Reset() 
                {
                    MessageList.clear(); // 清空消息列表
                }

                // 添加新检测消息
                void WriteMessage(CAMData message) 
                {
                    MessageList.push_back(message); // 将新消息添加到列表中
                }

            private:
                std::vector<CAMData> MessageList; // 存储CAMData消息的向量

                friend class s11n::CAMDataSerializer; // 声明s11n::CAMDataSerializer为友元类
            };

            class CustomV2XDataS // 定义CustomV2XDataS类
            {

            public:
                explicit CustomV2XDataS() = default; // 默认构造函数

                CustomV2XDataS &operator=(CustomV2XDataS &&) = default; // 移动赋值运算符

                // 返回当前接收到的消息数量
                size_t GetMessageCount() const 
                {
                    return MessageList.size(); // 返回消息列表的大小
                }

                // 删除当前消息
                void Reset() 
                {
                    MessageList.clear(); // 清空消息列表
                }

                // 添加新检测消息
                void WriteMessage(CustomV2XData message) 
                {
                    MessageList.push_back(message); // 将新消息添加到列表中
                }

            private:
                std::vector<CustomV2XData> MessageList; // 存储CustomV2XData消息的向量

                friend class s11n::CustomV2XDataSerializer; // 声明s11n::CustomV2XDataSerializer为友元类
            };

        } // namespace data
    }     // namespace sensor
} // namespace carla
