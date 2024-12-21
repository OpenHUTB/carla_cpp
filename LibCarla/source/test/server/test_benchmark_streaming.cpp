// 版权所有 （c） 2017 Universitat Autonoma 计算机视觉中心 （CVC）
// 巴塞罗那 （UAB）。
//
// 本作品根据 MIT 许可证的条款进行许可。
// 有关副本，请参阅 <https://opensource.org/licenses/MIT>。

#include "test.h"

#include <carla/Buffer.h>
#include <carla/BufferView.h>
#include <carla/streaming/Client.h>
#include <carla/streaming/Server.h>

#include <boost/asio/post.hpp>

#include <algorithm>

using namespace carla::streaming;
using namespace std::chrono_literals;

// 创建特定大小的消息
static auto make_special_message(size_t size) { // 创建一个包含42的向量，大小为size
  std::vector<uint32_t> v(size/sizeof(uint32_t), 42u);
  carla::Buffer msg(v);
  EXPECT_EQ(msg.size(), size);
  carla::SharedBufferView BufView = carla::BufferView::CreateFrom(std::move(msg));

  return BufView;
}

// 基准测试类
class Benchmark {
public:

  Benchmark(uint16_t port, size_t message_size, double success_ratio)
    : _server(port),
      _client(),
      _message(make_special_message(message_size)),
      _client_callback(),
      _work_to_do(_client_callback),
      _success_ratio(success_ratio) {}// 初始化成功率

  // 添加一个流
  void AddStream() {
    Stream stream = _server.MakeStream();// 从服务器创建一个流

    // 客户端订阅流，并定义消息处理回调
    _client.Subscribe(stream.token(), [this](carla::Buffer msg) {
      carla::SharedBufferView BufView = carla::BufferView::CreateFrom(std::move(msg));
      DEBUG_ASSERT_EQ(BufView->size(), _message->size());
      boost::asio::post(_client_callback, [this]() {
        CARLA_PROFILE_FPS(client, listen_callback);
        ++_number_of_messages_received;// 增加接收到的消息计数
      });
    });

    _streams.push_back(stream);// 将流添加到流列表中
  }

  // 添加多个流
  void AddStreams(size_t count) {
    for (auto i = 0u; i < count; ++i) {
      AddStream();
    }
  }

  // 运行基准测试
  void Run(size_t number_of_messages) {
    _threads.CreateThread([this]() { _client_callback.run(); });
    _server.AsyncRun(_streams.size());
    _client.AsyncRun(_streams.size());

    std::this_thread::sleep_for(1s); // 等待客户端准备好，以确保接收所有消息

     // 对每个流进行消息发送
    for (auto &&stream : _streams) {
      _threads.CreateThread([=]() mutable {
        for (auto i = 0u; i < number_of_messages; ++i) {
          std::this_thread::sleep_for(11ms); // 约90帧
          {
            CARLA_PROFILE_SCOPE(game, write_to_stream);// 记录写入流的性能
            stream.Write(_message);// 向流写入消息
          }
        }
      });
    }

    // 计算预期接收到的消息数量
    const auto expected_number_of_messages = _streams.size() * number_of_messages;
    const auto threshold =
        static_cast<size_t>(_success_ratio * static_cast<double>(expected_number_of_messages));

    // 等待消息接收完成
    for (auto i = 0u; i < 10; ++i) {
      std::cout << "received " << _number_of_messages_received
                << " of " << expected_number_of_messages
                << " messages,";
      if (_number_of_messages_received >= expected_number_of_messages) {
        break;// 如果接收到的消息数量达到预期，则退出
      }
      std::cout << " waiting..." << std::endl;
      std::this_thread::sleep_for(1s);
    }

    _client_callback.stop();
    _threads.JoinAll();
    std::cout << " done." << std::endl;

#ifdef NDEBUG
    ASSERT_GE(_number_of_messages_received, threshold); // 断言接收到的消息数大于等于阈值
#else
    
    // 在调试模式下，如果未达到阈值，记录警告
    if (_number_of_messages_received < threshold) {
      carla::log_warning("threshold unmet:", _number_of_messages_received, '/', threshold);
    }
#endif // NDEBUG
  }

private:

  carla::ThreadGroup _threads;

  Server _server;

  Client _client;

  const carla::SharedBufferView _message;

  boost::asio::io_context _client_callback;

  boost::asio::io_context::work _work_to_do;

  const double _success_ratio;

  std::vector<Stream> _streams;

  std::atomic_size_t _number_of_messages_received{0u};
};

// 获取最大并发数
static size_t get_max_concurrency() {
  size_t concurrency = std::thread::hardware_concurrency() / 2u;
  return std::max((size_t) 2u, concurrency);// 返回至少为2的最大并发数
}

// 基准测试图像流
static void benchmark_image(
    const size_t dimensions,
    const size_t number_of_streams = 1u,
    const double success_ratio = 1.0) {
  constexpr auto number_of_messages = 100u;// 定义消息数量
  carla::logging::log("Benchmark:", number_of_streams, "streams at 90FPS.");// 输出基准测试信息
  Benchmark benchmark(TESTING_PORT, 4u * dimensions, success_ratio);
  benchmark.AddStreams(number_of_streams);
  benchmark.Run(number_of_messages);
}

TEST(benchmark_streaming, image_200x200) {
  benchmark_image(200u * 200u);
}

TEST(benchmark_streaming, image_800x600) {
  benchmark_image(800u * 600u, 1u, 0.9);
}

TEST(benchmark_streaming, image_1920x1080) {
  benchmark_image(1920u * 1080u, 1u, 0.9);
}

TEST(benchmark_streaming, image_200x200_mt) {
  benchmark_image(200u * 200u, get_max_concurrency());
}

TEST(benchmark_streaming, image_800x600_mt) {
  benchmark_image(800u * 600u, get_max_concurrency(), 0.9);
}

TEST(benchmark_streaming, image_1920x1080_mt) {
  benchmark_image(1920u * 1080u, get_max_concurrency(), 0.9);
}
