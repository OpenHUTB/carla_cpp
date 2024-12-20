// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"
// 包含Carla线程组相关的头文件，用于管理和操作多个线程，例如创建线程、等待线程结束等功能
#include <carla/ThreadGroup.h>
// 包含Carla流媒体客户端相关的头文件，用于实现与流媒体服务器进行通信的客户端功能
#include <carla/streaming/Client.h>
// 包含Carla流媒体服务器相关的头文件，用于实现流媒体服务端的相关功能，比如接收客户端连接、发送数据等
#include <carla/streaming/Server.h>
// 包含Carla流媒体细节相关的调度器头文件，可能涉及到对流媒体数据分发、处理等底层逻辑的实现
#include <carla/streaming/detail/Dispatcher.h>
// 包含Carla流媒体基于TCP协议客户端相关的详细实现头文件，提供了具体的TCP客户端功能实现细节
#include <carla/streaming/detail/tcp/Client.h>
// 包含Carla流媒体基于TCP协议服务器相关的详细实现头文件，提供了具体的TCP服务器功能实现细节
#include <carla/streaming/detail/tcp/Server.h>
// 包含Carla流媒体底层客户端相关的头文件，涉及更底层的客户端功能实现，可能与协议交互等基础操作有关
#include <carla/streaming/low_level/Client.h>
// 包含Carla流媒体底层服务器相关的头文件，涉及更底层的服务器功能实现，同样可能侧重于基础的协议处理等方面
#include <carla/streaming/low_level/Server.h>

#include <atomic>
// 使用 std::chrono_literals 命名空间，这样可以方便地使用时间字面量
using namespace std::chrono_literals;

// This is required for low level to properly stop the threads in case of
// exception/assert.
class io_context_running {
public:
    // 定义一个 boost::asio 库的 io_context 对象，用于异步 I/O 操作，例如网络通信中的异步读写等，它是整个异步操作的核心上下文环境
  boost::asio::io_context service;
    // 显式的构造函数，接收一个可选的参数threads，用于指定要创建的工作线程数量，默认值为2
  explicit io_context_running(size_t threads = 2u)
        // 创建一个 io_context::work 对象，它用于保持 io_context 处于运行状态，防止 io_context.run() 立即返回，只要这个对象存在且关联到 io_context，io_context 就会持续运行其事件循环
    : _work_to_do(service) {
    _threads.CreateThreads(threads, [this]() { service.run(); });
  }

  ~io_context_running() {
    service.stop();
  }

private:
    // 定义一个 io_context::work 类型的对象，用于维持 io_context 的运行状态，如前面构造函数中所述
  boost::asio::io_context::work _work_to_do;
    // 定义一个 carla::ThreadGroup 类型的对象，用于管理创建的线程，比如创建线程、等待线程结束等操作
  carla::ThreadGroup _threads;
};

TEST(streaming, low_level_sending_strings) {
      // 使用 util::buffer 命名空间，可能其中包含了与缓冲区操作相关的函数、类型等定义，具体取决于该命名空间的实际内容
  using namespace util::buffer;
      // 使用 carla::streaming 命名空间，包含了流媒体相关的通用功能、类型等定义，便于在测试函数中直接访问相关内容
  using namespace carla::streaming;
      // 使用 carla::streaming::detail 命名空间，涉及流媒体更详细、底层的实现相关的定义，这里使用可能是为了访问一些内部的辅助功能等
  using namespace carla::streaming::detail;
      // 使用 carla::streaming::low_level 命名空间，对应于流媒体底层相关的功能、类型等定义，针对更基础的操作进行测试
  using namespace carla::streaming::low_level;
  
    // 定义一个常量表达式，表示要发送的消息数量，这里设置为100条消息
  constexpr auto number_of_messages = 100u;
      // 定义一个字符串常量，表示要发送的消息内容，这里是"Hello client!"
  const std::string message_text = "Hello client!";
  
    // 定义一个原子类型的变量，用于统计接收到的消息数量，初始值为0，原子类型保证在多线程环境下对其的操作是原子性的，避免数据竞争导致计数错误
  std::atomic_size_t message_count{0u};
  
    // 创建一个 io_context_running 类型的对象，用于管理异步 I/O 上下文以及相关线程，默认会创建2个工作线程来运行 io_context 的事件循环
  io_context_running io;

  carla::streaming::low_level::Server<tcp::Server> srv(io.service, TESTING_PORT);
      // 为服务器设置超时时间为1秒，意味着在某些操作（比如等待客户端连接、接收数据等）如果超过1秒没有响应，可能会触发相应的超时处理逻辑，具体取决于服务器的实现
  srv.SetTimeout(1s);

  auto stream = srv.MakeStream();

  carla::streaming::low_level::Client<tcp::Client> c;
  c.Subscribe(io.service, stream.token(), [&](auto message) {
            // 每接收到一条消息，消息计数变量加1，通过原子操作保证计数的正确性
    ++message_count;
            // 断言接收到的消息大小与发送的消息文本大小相等，用于验证数据完整性，确保接收的数据长度正确
    ASSERT_EQ(message.size(), message_text.size());
            // 将接收到的消息（可能是某种缓冲区类型，取决于具体实现）转换为字符串类型，假设 as_string 函数是在 util::buffer 命名空间中定义用于这种转换的函数
    const std::string msg = as_string(message);
            // 断言接收到的消息内容与发送的消息内容一致，进一步验证数据的准确性
    ASSERT_EQ(msg, message_text);
  });
  
    // 创建一个基于TCP的底层流媒体客户端对象，用于连接到服务器并接收服务器发送的数据
  carla::Buffer Buf(boost::asio::buffer(message_text.c_str(), message_text.size()));
  carla::SharedBufferView BufView = carla::BufferView::CreateFrom(std::move(Buf));
  for (auto i = 0u; i < number_of_messages; ++i) {
            // 让当前线程暂停执行一小段时间（2毫秒），可能是为了模拟一定的发送间隔，避免过于频繁地发送数据，或者是为了让接收端有时间处理之前接收到的数据等原因
    std::this_thread::sleep_for(2ms);
            // 将共享缓冲区视图对象赋值给一个临时变量 View，这里的赋值操作可能涉及到一些引用计数等共享资源管理的逻辑，具体取决于 SharedBufferView 类型的实现
    carla::SharedBufferView View = BufView;
            // 通过流对象将缓冲区视图数据写入流中，这样服务器就会将这些数据发送给已订阅该流的客户端，具体的发送机制取决于流对象和服务器的底层实现
    stream.Write(View);
  }
    // 再让当前线程暂停执行一小段时间（2毫秒），可能是为了确保最后一批数据有足够时间被发送和接收处理
  std::this_thread::sleep_for(2ms);
  ASSERT_GE(message_count, number_of_messages - 3u);
  
    // 停止 io_context 的运行，结束服务器和客户端相关的异步操作，释放相关资源
  io.service.stop();
}
// 定义一个测试用例，测试名称为"streaming"，测试子项名称为"low_level_unsubscribing"，用于测试底层流媒体客户端取消订阅的相关功能
TEST(streaming, low_level_unsubscribing) {
      // 使用 util::buffer 命名空间，可能其中包含了与缓冲区操作相关的函数、类型等定义，具体取决于该命名空间的实际内容
  using namespace util::buffer;
      // 使用 carla::streaming 命名空间，包含了流媒体相关的通用功能、类型等定义，便于在测试函数中直接访问相关内容
  using namespace carla::streaming;
      // 使用 carla::streaming::detail 命名空间，涉及流媒体更详细、底层的实现相关的定义，这里使用可能是为了访问一些内部的辅助功能等
  using namespace carla::streaming::detail;
      // 使用 carla::streaming::low_level 命名空间，对应于流媒体底层相关的功能、类型等定义，针对更基础的操作进行测试
  using namespace carla::streaming::low_level;
  
    // 定义一个常量表达式，表示要发送的消息数量，这里设置为50条消息
  constexpr auto number_of_messages = 50u;
    // 定义一个字符串常量，表示要发送的消息内容，这里是"Hello client!"
  const std::string message_text = "Hello client!";
  
    // 创建一个 io_context_running 类型的对象，用于管理异步 I/O 上下文以及相关线程，默认会创建2个工作线程来运行 io_context 的事件循环
  io_context_running io;

  carla::streaming::low_level::Server<tcp::Server> srv(io.service, TESTING_PORT);
      // 为服务器设置超时时间为1秒，意味着在某些操作（比如等待客户端连接、接收数据等）如果超过1秒没有响应，可能会触发相应的超时处理逻辑，具体取决于服务器的实现
  srv.SetTimeout(1s);
  
    // 创建一个基于TCP的底层流媒体客户端对象，用于连接到服务器并接收服务器发送的数据
  carla::streaming::low_level::Client<tcp::Client> c;
      // 循环10次，每次创建一个流、订阅、发送数据、取消订阅，模拟多次不同流的订阅和取消操作场景
  for (auto n = 0u; n < 10u; ++n) {
            // 通过服务器对象创建一个流媒体流对象，这个流对象可以用于后续向客户端发送数据等操作，具体流的实现细节取决于服务器内部的逻辑
    auto stream = srv.MakeStream();
            // 定义一个原子类型的变量，用于统计接收到的消息数量，初始值为0，原子类型保证在多线程环境下对其的操作是原子性的，避免数据竞争导致计数错误
    std::atomic_size_t message_count{0u};
    

    c.Subscribe(io.service, stream.token(), [&](auto message) {
                  // 每接收到一条消息，消息计数变量加1，通过原子操作保证计数的正确性
      ++message_count;
                  // 断言接收到的消息大小与发送的消息文本大小相等，用于验证数据完整性，确保接收的数据长度正确
      ASSERT_EQ(message.size(), message_text.size());
                  // 将接收到的消息（可能是某种缓冲区类型，取决于具体实现）转换为字符串类型，假设 as_string 函数是在 util::buffer 命名空间中定义用于这种转换的函数
      const std::string msg = as_string(message);
                  // 断言接收到的消息内容与发送的消息内容一致，进一步验证数据的准确性
      ASSERT_EQ(msg, message_text);
    });
        // 创建一个 carla::Buffer 类型的对象，用于存储要发送的消息数据，通过 boost::asio::buffer 函数将消息文本的字符数组及其大小包装成一个缓冲区对象，方便后续对流媒体流进行写入操作
    carla::Buffer Buf(boost::asio::buffer(message_text.c_str(), message_text.size()));
    
    carla::SharedBufferView BufView = carla::BufferView::CreateFrom(std::move(Buf));
            // 循环发送指定数量的消息
    for (auto i = 0u; i < number_of_messages; ++i) {
                  // 让当前线程暂停执行一小段时间（4毫秒），可能是为了模拟一定的发送间隔，避免过于频繁地发送数据，或者是为了让接收端有时间处理之前接收到的数据等原因
      std::this_thread::sleep_for(4ms);
                  // 将共享缓冲区视图对象赋值给一个临时变量 View，这里的赋值操作可能涉及到一些引用计数等共享资源管理的逻辑，具体取决于 SharedBufferView 类型的实现
      carla::SharedBufferView View = BufView;
                  // 通过流对象将缓冲区视图数据写入流中，这样服务器就会将这些数据发送给已订阅该流的客户端，具体的发送机制取决于流对象和服务器的底层实现
      stream.Write(View);
    }
    
        // 让当前线程暂停执行一小段时间（4毫秒），可能是为了确保最后一批数据有足够时间被发送和接收处理
    std::this_thread::sleep_for(4ms);
            // 客户端调用 UnSubscribe 函数取消对指定流的订阅，这样后续服务器通过该流发送的数据客户端将不再接收，用于测试取消订阅功能是否正常
    c.UnSubscribe(stream.token());
    
        // 继续循环发送相同数量的消息，模拟取消订阅后服务器仍在发送数据的情况，验证客户端是否确实不再接收这些数据
    for (auto i = 0u; i < number_of_messages; ++i) {
                  // 让当前线程暂停执行一小段时间（2毫秒），可能是为了模拟一定的发送间隔，避免过于频繁地发送数据，或者是为了让接收端有时间处理之前接收到的数据等原因
      std::this_thread::sleep_for(2ms);
                  // 将共享缓冲区视图对象赋值给一个临时变量 View，这里的赋值操作可能涉及到一些引用计数等共享资源管理的逻辑，具体取决于 SharedBufferView 类型的实现
      carla::SharedBufferView View = BufView;
                  // 通过流对象将缓冲区视图数据写入流中，服务器会尝试发送这些数据，但客户端已取消订阅，不应再接收
      stream.Write(View);
    }

    ASSERT_GE(message_count, number_of_messages - 3u);
  }

  io.service.stop();
}

TEST(streaming, low_level_tcp_small_message) {
  using namespace carla::streaming;
  using namespace carla::streaming::detail;

  boost::asio::io_context io_context;
  tcp::Server::endpoint ep(boost::asio::ip::tcp::v4(), TESTING_PORT);

  tcp::Server srv(io_context, ep);
  srv.SetTimeout(1s);
  std::atomic_bool done{false};
  std::atomic_size_t message_count{0u};

  const std::string msg = "Hola!";

  srv.Listen([&](std::shared_ptr<tcp::ServerSession> session) {
    ASSERT_EQ(session->get_stream_id(), 1u);

    carla::Buffer Buf(boost::asio::buffer(msg.c_str(), msg.size()));
    carla::SharedBufferView BufView = carla::BufferView::CreateFrom(std::move(Buf));
    while (!done) {
      std::this_thread::sleep_for(1ns);
      carla::SharedBufferView View = BufView;
      session->Write(View);
    }
    std::cout << "done!\n";
  }, [](std::shared_ptr<tcp::ServerSession>) { std::cout << "session closed!\n"; });

  Dispatcher dispatcher{make_endpoint<tcp::Client::protocol_type>(srv.GetLocalEndpoint())};
  auto stream = dispatcher.MakeStream();
  auto c = std::make_shared<tcp::Client>(io_context, stream.token(), [&](carla::Buffer message) {
    ++message_count;
    ASSERT_FALSE(message.empty());
    ASSERT_EQ(message.size(), 5u);
    const std::string received = util::buffer::as_string(message);
    ASSERT_EQ(received, msg);
  });
  c->Connect();

  // We need at least two threads because this server loop consumes one.
  carla::ThreadGroup threads;
  threads.CreateThreads(
      std::max(2u, std::thread::hardware_concurrency()),
      [&]() { io_context.run(); });

  std::this_thread::sleep_for(2s);
  io_context.stop();
  done = true;
  std::cout << "client received " << message_count << " messages\n";
  ASSERT_GT(message_count, 10u);
  c->Stop();
}

struct DoneGuard {
  ~DoneGuard() { done = true; };
  std::atomic_bool &done;
};

TEST(streaming, stream_outlives_server) {
  using namespace carla::streaming;
  using namespace util::buffer;
  constexpr size_t iterations = 10u;
  std::atomic_bool done{false};
  const std::string message = "Hello client, how are you?";
  std::shared_ptr<Stream> stream;

  carla::ThreadGroup sender;
  DoneGuard g = {done};
  sender.CreateThread([&]() {

    carla::Buffer Buf(boost::asio::buffer(message.c_str(), message.size()));
    carla::SharedBufferView BufView = carla::BufferView::CreateFrom(std::move(Buf));
    while (!done) {
      std::this_thread::sleep_for(1ms);
      auto s = std::atomic_load_explicit(&stream, std::memory_order_relaxed);
      if (s != nullptr) {
        carla::SharedBufferView View = BufView;
        s->Write(View);
      }
    }
  });

  for (auto i = 0u; i < iterations; ++i) {
    Server srv(TESTING_PORT);
    srv.AsyncRun(2u);
    {
      auto s = std::make_shared<Stream>(srv.MakeStream());
      std::atomic_store_explicit(&stream, s, std::memory_order_relaxed);
    }
    std::atomic_size_t messages_received{0u};
    {
      Client c;
      c.AsyncRun(2u);
      c.Subscribe(stream->token(), [&](auto buffer) {
        const std::string result = as_string(buffer);
        ASSERT_EQ(result, message);
        ++messages_received;
      });
      std::this_thread::sleep_for(20ms);
    } // client dies here.
    ASSERT_GT(messages_received, 0u);
  } // server dies here.
  std::this_thread::sleep_for(20ms);
  done = true;
} // stream dies here.

TEST(streaming, multi_stream) {
  using namespace carla::streaming;
  using namespace util::buffer;
  constexpr size_t number_of_messages = 100u;
  constexpr size_t number_of_clients = 6u;
  constexpr size_t iterations = 10u;
  const std::string message = "Hi y'all!";

  Server srv(TESTING_PORT);
  srv.AsyncRun(number_of_clients);
  auto stream = srv.MakeStream();

  for (auto i = 0u; i < iterations; ++i) {
    std::vector<std::pair<std::atomic_size_t, std::unique_ptr<Client>>> v(number_of_clients);

    for (auto &pair : v) {
      pair.first = 0u;
      pair.second = std::make_unique<Client>();
      pair.second->AsyncRun(1u);
      pair.second->Subscribe(stream.token(), [&](auto buffer) {
        const std::string result = as_string(buffer);
        ASSERT_EQ(result, message);
        ++pair.first;
      });
    }

    carla::Buffer Buf(boost::asio::buffer(message.c_str(), message.size()));
    carla::SharedBufferView BufView = carla::BufferView::CreateFrom(std::move(Buf));
    std::this_thread::sleep_for(6ms);
    for (auto j = 0u; j < number_of_messages; ++j) {
      std::this_thread::sleep_for(6ms);
      carla::SharedBufferView View = BufView;
      stream.Write(View);
    }
    std::this_thread::sleep_for(6ms);

    for (auto &pair : v) {
      ASSERT_GE(pair.first, number_of_messages - 3u);
    }
  }
}
