// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"
//名为“test.h”的头文件。
#include <carla/RecurrentSharedFuture.h>//这里是包含了名为“RecurrentSharedFuture.h”的头文件，这个头文件可能是来自名为“carla”的库或者模块。这个头文件中的内容可能包含与循环共享未来（Recurrent Shared Future，根据文件名推测）相关的类定义、函数声明等内容。
#include <carla/ThreadGroup.h>//包含与线程组相关的定义，例如创建、管理线程组的类或者函数等内容。
#include <atomic>
//包含了<atomic>头文件。
using namespace std::chrono_literals;
// 测试 RecurrentSharedFuture 的用例
TEST(recurrent_shared_future, use_case) {
  using namespace carla;
  // 创建一个线程组
  ThreadGroup threads;
  // 创建一个 RecurrentSharedFuture 对象
  RecurrentSharedFuture<int> future;
// 定义常量，线程数量和打开次数
  constexpr size_t number_of_threads = 12u;
  constexpr size_t number_of_openings = 40u;
// 原子变量，用于计数
  std::atomic_size_t count{0u};
  // 原子变量，用于标记是否完成
  std::atomic_bool done{false};
// 创建多个线程，每个线程执行以下逻辑
  threads.CreateThreads(number_of_threads, [&]() {
    while (!done) {
    	// 等待未来对象的值，超时时间为 1 秒
      auto result = future.WaitFor(1s);
      // 断言结果有值
      ASSERT_TRUE(result.has_value());
      // 断言结果的值为 42
      ASSERT_EQ(*result, 42);
      // 增加计数
      ++count;
    }
  });
// 主线程睡眠 100 毫秒
  std::this_thread::sleep_for(100ms);
  // 设置未来对象的值为 42，多次重复此操作
  for (auto i = 0u; i < number_of_openings - 1u; ++i) {
    future.SetValue(42);
    // 主线程睡眠 10 毫秒
    std::this_thread::sleep_for(10ms);
  }
  // 标记完成
  done = true;
  // 再次设置未来对象的值为 42
  future.SetValue(42);
  // 等待所有线程完成
  threads.JoinAll();
  // 断言计数等于线程数量乘以打开次数
  ASSERT_EQ(count, number_of_openings * number_of_threads);
}
// 测试 RecurrentSharedFuture 的超时情况
TEST(recurrent_shared_future, timeout) {
  using namespace carla;
  // 创建一个 RecurrentSharedFuture 对象
  RecurrentSharedFuture<int> future;
  // 等待未来对象的值，超时时间为 1 纳秒
  auto result = future.WaitFor(1ns);
  // 断言结果没有值，因为超时时间很短
  ASSERT_FALSE(result.has_value());
}
// 测试 RecurrentSharedFuture 的异常情况
TEST(recurrent_shared_future, exception) {
  using namespace carla;
  // 创建一个线程组
  ThreadGroup threads;
  // 创建一个 RecurrentSharedFuture 对象
  RecurrentSharedFuture<int> future;
  // 定义异常消息
  const std::string message = "Uh oh an exception!";
// 创建一个线程，在该线程中设置未来对象的异常
  threads.CreateThread([&]() {
    std::this_thread::sleep_for(10ms);
    future.SetException(std::runtime_error(message));
  });

  try {
  	 // 等待未来对象的值，如果有异常则抛出
    future.WaitFor(1s);
  } catch (const std::exception &e) {
  	// 断言异常消息与预期一致
    ASSERT_STREQ(e.what(), message.c_str());
  }
}
