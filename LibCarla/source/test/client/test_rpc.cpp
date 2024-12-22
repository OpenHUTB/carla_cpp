// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"
//包含一个自定义的头文件"test.h"。
// 通常在"test.h"里会有当前源文件所需的函数声明、结构体定义、宏定义等内容，方便在本文件中调用相关功能。
#include <carla/MsgPackAdaptors.h>//包含来自名为"carla"的项目（可能是库等）下的头文件。
#include <carla/ThreadGroup.h>//同样是从"carla"项目中引入头文件，此头文件大概率是关于线程组（ThreadGroup）的相关定义。
// 例如可能包含创建、管理线程组的类，或者操作线程组的函数等，方便在代码中进行多线程相关的编程操作。
#include <carla/rpc/Actor.h>//
#include <carla/rpc/Client.h>
#include <carla/rpc/Response.h>
#include <carla/rpc/Server.h>

#include <thread>

using namespace carla::rpc;
using namespace std::chrono_literals;
// 测试 RPC 功能的编译测试
TEST(rpc, compilation_tests) {
	// 创建一个服务器实例
  Server server(TESTING_PORT);
  // 绑定同步方法，无参数，返回 2.0f
  server.BindSync("bind00", []() { return 2.0f; });
  // 绑定同步方法，一个 int 参数，返回传入的参数
  server.BindSync("bind01", [](int x) { return x; });
  // 绑定同步方法，两个参数（int 和 float），返回 0.0
  server.BindSync("bind02", [](int, float) { return 0.0; });
  // 绑定同步方法，四个参数（int、float、double、char），无返回值
  server.BindSync("bind03", [](int, float, double, char) {});
}
// 测试服务器绑定同步方法并在游戏线程上运行
TEST(rpc, server_bind_sync_run_on_game_thread) {
	// 获取当前主线程的 ID
  const auto main_thread_id = std::this_thread::get_id();
// 定义端口号，如果 TESTING_PORT 不为 0 则使用 TESTING_PORT，否则使用 2017
  const uint16_t port = (TESTING_PORT != 0u ? TESTING_PORT : 2017u);
// 创建服务器实例
  Server server(port);
// 绑定一个同步方法，接收两个 int 参数，返回它们的和，并检查当前线程是否为主线程
  server.BindSync("do_the_thing", [=](int x, int y) -> int {
    EXPECT_EQ(std::this_thread::get_id(), main_thread_id);
    return x + y;
  });
// 异步运行服务器，传入一个线程数参数
  server.AsyncRun(1u);
// 创建一个原子布尔变量，用于标记任务是否完成
  std::atomic_bool done{false};
// 创建线程组
  carla::ThreadGroup threads;
  // 创建一个线程，在该线程中执行以下逻辑
  threads.CreateThread([&]() {
  	// 创建客户端实例，连接到本地主机和指定端口
    Client client("localhost", port);
    // 循环执行 300 次
    for (auto i = 0; i < 300; ++i) {
    	// 调用服务器上的方法，传入两个参数，并将结果转换为 int 类型，检查结果是否正确
      auto result = client.call("do_the_thing", i, 1).as<int>();
      EXPECT_EQ(result, i + 1);
    }
    // 标记任务完成
    done = true;
  });
// 循环计数器
  auto i = 0u;
  // 循环执行，直到任务完成或达到一定次数
  for (; i < 1'000'000u; ++i) {
  	// 在服务器上同步运行一段时间（2 毫秒）
    server.SyncRunFor(2ms);
    // 如果任务完成，则跳出循环
    if (done) {
      break;
    }
  }
  // 输出服务器在游戏线程上运行的次数
  std::cout << "game thread: run " << i << " slices.\n";
  // 断言任务已完成
  ASSERT_TRUE(done);
}
