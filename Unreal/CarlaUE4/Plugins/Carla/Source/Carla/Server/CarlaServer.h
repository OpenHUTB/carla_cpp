// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 确保头文件只被包含一次，避免重复定义等问题
#pragma once

// 包含Carla相关的演员（Actor）基础类头文件，可能用于后续与游戏场景中的各种可交互实体相关操作
#include "Carla/Actor/CarlaActor.h"
// 包含Carla传感器相关的数据流转（DataStream）头文件，用于处理传感器数据的传输等逻辑
#include "Carla/Sensor/DataStream.h"

// 包含UE4核心的最小化头文件集合，包含了一些基础的UE4常用类型、宏等定义
#include "CoreMinimal.h"

// 以下三个头文件相关的包含，先禁用UE4特定宏（可能是为了避免一些宏定义冲突等情况），引入carla相关的多GPU路由（multigpu/router）和流服务器（streaming/Server）的功能相关头文件，最后再启用UE4宏
#include <compiler/disable-ue4-macros.h>
#include <carla/multigpu/router.h>
#include <carla/streaming/Server.h>
#include <compiler/enable-ue4-macros.h>

// 前向声明UCarlaEpisode类，告知编译器存在这样一个类，但具体定义在后续代码中再给出，避免编译时找不到完整类定义的错误
class UCarlaEpisode;

// 定义FCarlaServer类，从名字看可能是Carla项目中用于服务器相关功能实现的类
class FCarlaServer
{
public:
    // 默认构造函数，用于创建FCarlaServer类的对象实例，可能在这里进行一些成员变量的初始化等操作
    FCarlaServer();

    // 析构函数，用于在对象销毁时进行资源清理、释放等相关操作，比如关闭网络连接、释放内存等
    ~FCarlaServer();

    // 启动服务器相关功能，传入RPC端口号（RPCPort）、流数据端口号（StreamingPort）以及备用端口号（SecondaryPort），返回一个多数据流对象（FDataMultiStream），可能用于后续的多种数据传输交互场景
    FDataMultiStream Start(uint16_t RPCPort, uint16_t StreamingPort, uint16_t SecondaryPort);

    // 用于通知服务器开始一个Carla Episode（可能是一个模拟场景、任务等的阶段），传入对应的UCarlaEpisode对象引用，以便服务器知晓相关信息进行对应处理
    void NotifyBeginEpisode(UCarlaEpisode &Episode);

    // 通知服务器结束当前的Carla Episode，让服务器进行相应的清理、收尾等操作
    void NotifyEndEpisode();

    // 以异步方式运行服务器，传入工作线程数量（NumberOfWorkerThreads）参数，可让服务器利用多线程来高效处理各种任务，比如数据接收、处理和发送等
    void AsyncRun(uint32 NumberOfWorkerThreads);

    // 运行服务器一段时间，传入时间（以毫秒为单位，Milliseconds）参数，在指定时长内执行服务器相关的逻辑，比如处理数据、更新状态等
    void RunSome(uint32 Milliseconds);

    // 执行服务器的一次“滴答”操作，通常用于周期性地更新服务器状态、处理数据等，类似于游戏循环里的每一帧更新逻辑
    void Tick();
    
    // 检查是否接收到了“滴答”提示（Tick Cue），返回布尔值表示是否收到，可用于判断是否需要进行下一步相关操作等
    bool TickCueReceived();

    // 停止服务器运行，释放相关资源，关闭各种连接等，将服务器置于停止状态
    void Stop();

    // 打开一个数据流，返回一个数据流转对象（FDataStream），用于后续的数据读取、写入等操作，可能是针对特定的数据传输通道进行操作
    FDataStream OpenStream() const;

    // 获取指向多GPU路由器（multigpu::Router）的共享指针，这个路由器可能用于在多GPU环境下对数据等进行路由转发、调度等功能，返回的是智能指针便于内存管理
    std::shared_ptr<carla::multigpu::Router> GetSecondaryServer();

    // 获取流服务器（streaming::Server）的引用，以便能直接操作流服务器对象，比如配置服务器参数、获取服务器状态信息等
    carla::streaming::Server &GetStreamingServer();

private:
    // 定义一个名为FPimpl的私有内部类，通常这种方式用于实现“编译防火墙”（Pimpl，即“Pointer to implementation”），将类的实现细节隐藏在内部类中，对外只暴露接口
    class FPimpl;
    // 定义一个智能指针（TUniquePtr），用于管理FPimpl类对象的生命周期，通过这种方式实现了对内部实现细节的良好封装
    TUniquePtr<FPimpl> Pimpl;
};
