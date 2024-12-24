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

// 这里需要级别低一些来在异常或者断言的情形下正确的停止线程
class io_context_running {
public:
    // 定义一个 boost::asio 库的 io_context 对象，用于异步 I/O 操作，例如网络通信中的异步读写等，它是整个异步操作的核心上下文环境
  boost::asio::io_context service;
    // 显式的构造函数，接收一个可选的参数threads，用于指定要创建的工作线程数量，默认值为2
  explicit io_context_running(size_t threads = 2u)
        // 创建一个 io_context::work 对象，它用于保持 io_context 处于运行状态，防止 io_context.run() 立即返回，只要这个对象存在且关联到 io_context，io_context 就会持续运行其事件循环
    : _work_to_do(service) {
      // 启动一个线程，在线程里面执行service.run
    _threads.CreateThreads(threads, [this]() { service.run(); });
  }

  ~io_context_running() {
    // 函数结束时，也顺便停止服务
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

  // 构造一个流对象
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
  // 断言message_count个数一定至少比message_count大3
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

// 这是一个测试用例，测试低级别TCP小消息流
TEST(streaming, low_level_tcp_small_message) {
  using namespace carla::streaming;
  using namespace carla::streaming::detail;

  boost::asio::io_context io_context;
  tcp::Server::endpoint ep(boost::asio::ip::tcp::v4(), TESTING_PORT);

  tcp::Server srv(io_context, ep);
    / 设置服务器超时时间
  srv.SetTimeout(1s);
    // 初始化一个原子布尔变量，表示任务是否完成
  std::atomic_bool done{false};
    // 初始化一个原子整数变量，表示接收到的消息数量
  std::atomic_size_t message_count{0u};

  const std::string msg = "Hola!";

    // 开始监听服务器会话
  srv.Listen([&](std::shared_ptr<tcp::ServerSession> session) {
    ASSERT_EQ(session->get_stream_id(), 1u);

      // 创建一个缓冲区对象，传入消息内容和大小
    carla::Buffer Buf(boost::asio::buffer(msg.c_str(), msg.size()));
       // 创建一个共享缓冲区视图对象
    carla::SharedBufferView BufView = carla::BufferView::CreateFrom(std::move(Buf));
      // 循环直到完成标志被设置
    while (!done) {
        // 线程休眠一段时间
      std::this_thread::sleep_for(1ns);
        // 写入共享缓冲区视图到会话
      carla::SharedBufferView View = BufView;
      session->Write(View);
    }
    std::cout << "done!\n";
  }, [](std::shared_ptr<tcp::ServerSession>) { std::cout << "session closed!\n"; });

     // 创建一个调度器对象，传入本地端点
  Dispatcher dispatcher{make_endpoint<tcp::Client::protocol_type>(srv.GetLocalEndpoint())};
  auto stream = dispatcher.MakeStream();
  auto c = std::make_shared<tcp::Client>(io_context, stream.token(), [&](carla::Buffer message) {
    ++message_count;
    ASSERT_FALSE(message.empty());
    ASSERT_EQ(message.size(), 5u);
    const std::string received = util::buffer::as_string(message);
      // 断言接收到的消息与原始消息相同
    ASSERT_EQ(received, msg);
  });
    // 连接客户端
  c->Connect();

  // 需要至少两个线程，因为这个服务循环要使用其中一个
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

// 测试流是否可以在服务器停止后继续存在。
// 这个测试用例主要用于验证流的生命周期相关特性，
// 即在服务器停止的情况下，流是否依然能够维持一定的功能或者存在状态。
TEST(streaming, stream_outlives_server) {
  using namespace carla::streaming;// 使用carla流命名空间。
  using namespace util::buffer;// 引入util::buffer命名空间，应该是用于操作缓冲区相关的功能
  constexpr size_t iterations = 10u;// 定义迭代次数的常量，这里设定为10次，用于控制循环执行的轮数等逻辑
  std::atomic_bool done{false};// 定义一个原子布尔类型的变量，用于标记某个操作是否完成，初始化为false
  const std::string message = "Hello client, how are you?";// 定义一个指向流对象的智能指针，用于后续对流的操作，初始时为空指针
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
    Server srv(TESTING_PORT);// 创建一个Server实例，使用TESTING_PORT作为端口号，具体端口值应该在别处定义
    srv.AsyncRun(2u);// 以异步方式启动服务器，参数2u可能用于配置相关运行参数，比如线程数量等（取决于Server类具体实现）
    {
      auto s = std::make_shared<Stream>(srv.MakeStream());// 通过服务器对象srv创建一个Stream智能指针s，这里的MakeStream函数应该是用于创建Stream实例
      std::atomic_store_explicit(&stream, s, std::memory_order_relaxed);// 使用原子操作将创建的Stream智能指针s存储到外部定义的stream变量中，采用宽松内存顺序
    }
    std::atomic_size_t messages_received{0u};
    {// 创建一个Client实例
      Client c;
        // 以异步方式启动客户端，参数2u同样可能涉及相关运行配置（和服务器端类似，取决于Client类实现）
      c.AsyncRun(2u);
        // 客户端进行订阅操作，传入从stream智能指针获取的token（标识）以及一个lambda表达式作为回调函数
      c.Subscribe(stream->token(), [&](auto buffer) {
        const std::string result = as_string(buffer);// 将接收到的buffer转换为字符串类型，as_string函数应该是自定义的转换函数
        ASSERT_EQ(result, message);// 使用断言验证转换后的结果是否和期望的message相等，message应该是外部定义的期望消息内容
        ++messages_received;// 如果消息验证通过，将统计接收消息数量的变量messages_received自增1
      });
      std::this_thread::sleep_for(20ms);
    } // client dies here.
    ASSERT_GT(messages_received, 0u);
  } // server dies here.
  std::this_thread::sleep_for(20ms);
  done = true;
} // stream dies here.

// 测试多个客户端订阅同一个流的情况
TEST(streaming, multi_stream) {
  using namespace carla::streaming;// 使用carla流命名空间。
  using namespace util::buffer;// 使用缓冲区工具命名空间。
  constexpr size_t number_of_messages = 100u;// 消息数量。
  constexpr size_t number_of_clients = 6u;// 客户端数量。
  constexpr size_t iterations = 10u; // 迭代次数。
  const std::string message = "Hi y'all!";

  Server srv(TESTING_PORT);
    // 创建服务器。
  srv.AsyncRun(number_of_clients);// 异步运行服务器。
  auto stream = srv.MakeStream();
// 创建流。
    
  for (auto i = 0u; i < iterations; ++i) {
    std::vector<std::pair<std::atomic_size_t, std::unique_ptr<Client>>> v(number_of_clients);
    // 创建客户端向量。

    for (auto &pair : v) {
        // 遍历客户端向量。
      pair.first = 0u; // 初始化接收到的消息数。
      pair.second = std::make_unique<Client>();
      pair.second->AsyncRun(1u);// 异步运行客户端。
      pair.second->Subscribe(stream.token(), [&](auto buffer) {
        const std::string result = as_string(buffer);
        ASSERT_EQ(result, message);// 断言结果等于发送的消息。
        ++pair.first;// 增加接收到的消息数。
      });
    }

    carla::Buffer Buf(boost::asio::buffer(message.c_str(), message.size()));// 创建缓冲区。
    carla::SharedBufferView BufView = carla::BufferView::CreateFrom(std::move(Buf));// 创建缓冲区视图
    std::this_thread::sleep_for(6ms);
    for (auto j = 0u; j < number_of_messages; ++j) {
      std::this_thread::sleep_for(6ms);
      carla::SharedBufferView View = BufView;// 创建缓冲区视图。
      stream.Write(View);
    }
    std::this_thread::sleep_for(6ms);

    for (auto &pair : v) {
        // 遍历客户端向量。
      ASSERT_GE(pair.first, number_of_messages - 3u);
        // 断言接收到的消息数至少为发送消息数减3。
    }
  }
}
