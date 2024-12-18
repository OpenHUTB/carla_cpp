// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// 本作品根据 MIT 许可证的条款进行许可。有关副本，请参阅 <https://opensource.org/licenses/MIT>。

#include "test.h"

// 包含了不同组件的头文件，这些组件实现了客户端和服务器的低级功能。
#include <carla/ThreadGroup.h>
#include <carla/streaming/Client.h>
#include <carla/streaming/Server.h>
#include <carla/streaming/detail/Dispatcher.h>
#include <carla/streaming/detail/tcp/Client.h>
#include <carla/streaming/detail/tcp/Server.h>
#include <carla/streaming/low_level/Client.h>
#include <carla/streaming/low_level/Server.h>

#include <atomic>

using namespace std::chrono_literals;  // 使用chrono字面量来表示时间单位

// io_context_running类用于管理一个io_context的生命周期，并创建多个线程来执行异步任务。
class io_context_running {
public:
  boost::asio::io_context service;  // io_context是Boost Asio库中的核心对象，负责异步I/O操作

  explicit io_context_running(size_t threads = 2u)
    : _work_to_do(service) {
      // 启动指定数量的线程来运行io_context的事件循环
    _threads.CreateThreads(threads, [this]() { service.run(); });
  }

  ~io_context_running() {
    // 在对象销毁时停止io_context的工作
    service.stop();
  }

private:
  boost::asio::io_context::work _work_to_do;  // 保证io_context的event loop继续运行
  carla::ThreadGroup _threads;  // 管理多线程
};

// 第一个测试：验证低级别的字符串发送功能
TEST(streaming, low_level_sending_strings) {
  using namespace util::buffer;
  using namespace carla::streaming;
  using namespace carla::streaming::detail;
  using namespace carla::streaming::low_level;

  constexpr auto number_of_messages = 100u;  // 要发送的消息数量
  const std::string message_text = "Hello client!";  // 要发送的消息内容

  std::atomic_size_t message_count{0u};  // 计数已接收到的消息数量

  io_context_running io;  // 创建并启动io_context

  carla::streaming::low_level::Server<tcp::Server> srv(io.service, TESTING_PORT);  // 创建TCP服务器
  srv.SetTimeout(1s);  // 设置超时时间

  // 构造一个流对象
  auto stream = srv.MakeStream();

  carla::streaming::low_level::Client<tcp::Client> c;
  c.Subscribe(io.service, stream.token(), [&](auto message) {
    ++message_count;  // 收到一条消息时，增加计数
    ASSERT_EQ(message.size(), message_text.size());  // 检查消息大小
    const std::string msg = as_string(message);  // 转换消息为字符串
    ASSERT_EQ(msg, message_text);  // 检查消息内容是否匹配
  });

  carla::Buffer Buf(boost::asio::buffer(message_text.c_str(), message_text.size()));  // 将消息内容转为Buffer
  carla::SharedBufferView BufView = carla::BufferView::CreateFrom(std::move(Buf));  // 创建Buffer视图

  for (auto i = 0u; i < number_of_messages; ++i) {
    std::this_thread::sleep_for(2ms);  // 模拟短暂的延时
    carla::SharedBufferView View = BufView;
    stream.Write(View);  // 写入数据到流
  }

  std::this_thread::sleep_for(2ms);  // 等待消息传递完成

  // 断言接收到的消息数量大于或等于number_of_messages - 3（允许有一些丢失的消息）
  ASSERT_GE(message_count, number_of_messages - 3u);

  io.service.stop();  // 停止io_context
}

// 第二个测试：验证取消订阅功能
TEST(streaming, low_level_unsubscribing) {
  using namespace util::buffer;
  using namespace carla::streaming;
  using namespace carla::streaming::detail;
  using namespace carla::streaming::low_level;

  constexpr auto number_of_messages = 50u;  // 每次传输的消息数量
  const std::string message_text = "Hello client!";  // 消息内容

  io_context_running io;  // 创建io_context

  carla::streaming::low_level::Server<tcp::Server> srv(io.service, TESTING_PORT);  // 创建TCP服务器
  srv.SetTimeout(1s);  // 设置超时时间

  carla::streaming::low_level::Client<tcp::Client> c;
  for (auto n = 0u; n < 10u; ++n) {
    auto stream = srv.MakeStream();  // 创建流
    std::atomic_size_t message_count{0u};  // 计数接收到的消息

    c.Subscribe(io.service, stream.token(), [&](auto message) {
      ++message_count;  // 收到消息时计数
      ASSERT_EQ(message.size(), message_text.size());  // 检查消息大小
      const std::string msg = as_string(message);  // 转换消息为字符串
      ASSERT_EQ(msg, message_text);  // 检查消息内容
    });

    carla::Buffer Buf(boost::asio::buffer(message_text.c_str(), message_text.size()));  // 将消息转为Buffer
    carla::SharedBufferView BufView = carla::BufferView::CreateFrom(std::move(Buf));  // 创建Buffer视图

    for (auto i = 0u; i < number_of_messages; ++i) {
      std::this_thread::sleep_for(4ms);  // 模拟延迟
      carla::SharedBufferView View = BufView;
      stream.Write(View);  // 写入数据
    }

    std::this_thread::sleep_for(4ms);  // 等待数据写入完成
    c.UnSubscribe(stream.token());  // 取消订阅流

    // 再次写入数据，检查取消订阅后的行为
    for (auto i = 0u; i < number_of_messages; ++i) {
      std::this_thread::sleep_for(2ms);  // 模拟延迟
      carla::SharedBufferView View = BufView;
      stream.Write(View);  // 写入数据
    }

    ASSERT_GE(message_count, number_of_messages - 3u);  // 断言接收到的消息数量
  }

  io.service.stop();  // 停止io_context
}

// 第三个测试：验证TCP连接下的小消息传输
TEST(streaming, low_level_tcp_small_message) {
  using namespace carla::streaming;
  using namespace carla::streaming::detail;

  boost::asio::io_context io_context;  // 创建io_context
  tcp::Server::endpoint ep(boost::asio::ip::tcp::v4(), TESTING_PORT);  // 定义TCP端口

  tcp::Server srv(io_context, ep);  // 创建TCP服务器
  srv.SetTimeout(1s);  // 设置超时时间
  std::atomic_bool done{false};  // 标记传输是否完成
  std::atomic_size_t message_count{0u};  // 记录接收到的消息数量

  const std::string msg = "Hola!";  // 要发送的消息

  // 服务器的Session处理
  srv.Listen([&](std::shared_ptr<tcp::ServerSession> session) {
    ASSERT_EQ(session->get_stream_id(), 1u);  // 验证流ID
    carla::Buffer Buf(boost::asio::buffer(msg.c_str(), msg.size()));  // 创建Buffer
    carla::SharedBufferView BufView = carla::BufferView::CreateFrom(std::move(Buf));  // 创建视图
    while (!done) {
      std::this_thread::sleep_for(1ns);  // 极短的延时
      carla::SharedBufferView View = BufView;
      session->Write(View);  // 向客户端写入数据
    }
    std::cout << "done!\n";
  }, [](std::shared_ptr<tcp::ServerSession>) { 
    std::cout << "session closed!\n";  // 会话关闭时输出
  });

  Dispatcher dispatcher{make_endpoint<tcp::Client::protocol_type>(srv.GetLocalEndpoint())};  // 创建Dispatcher
  auto stream = dispatcher.MakeStream();  // 创建流
  auto c = std::make_shared<tcp::Client>(io_context, stream.token(), [&](carla::Buffer message) {
    ++message_count;  // 收到消息时计数
    ASSERT_FALSE(message.empty());  // 确保消息非空
    ASSERT_EQ(message.size(), 5u);  // 验证消息大小
    const std::string received = util::buffer::as_string(message);  // 转换为字符串
    ASSERT_EQ(received, msg);  // 验证消息内容
  });
  c->Connect();  // 连接到服务器

  // 创建
