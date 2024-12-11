#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>

// 引入CARLA客户端库的头文件
#include <carla/client/ActorBlueprint.h>
#include <carla/client/BlueprintLibrary.h>
#include <carla/client/Client.h>
#include <carla/client/Map.h>
#include <carla/client/Sensor.h>
#include <carla/client/TimeoutException.h>
#include <carla/client/World.h>
#include <carla/geom/Transform.h>
#include <carla/image/ImageIO.h>
#include <carla/image/ImageView.h>
#include <carla/sensor/data/Image.h>

// 为CARLA命名空间创建别名，简化代码
namespace cc = carla::client;
namespace cg = carla::geom;
namespace csd = carla::sensor::data;

// 使用std命名空间中的chrono和string字面量
using namespace std::chrono_literals;
using namespace std::string_literals;

// 定义一个宏，用于断言表达式为真，否则抛出运行时错误
#define EXPECT_TRUE(pred) if (!(pred)) { throw std::runtime_error(#pred); }

/// 从给定范围中随机选择一个元素
/// Pick a random element from @a range.
template <typename RangeT, typename RNG>
static auto &RandomChoice(const RangeT &range, RNG &&generator) {
  EXPECT_TRUE(range.size() > 0u);  // 断言值一定大于0，否则就抛出异常
  std::uniform_int_distribution<size_t> dist{0u, range.size() - 1u};
  return range[dist(std::forward<RNG>(generator))];
}

/// Save a semantic segmentation image to disk converting to CityScapes palette.
/*
static void SaveSemSegImageToDisk(const csd::Image &image) {
  using namespace carla::image;

  char buffer[9u];
  std::snprintf(buffer, sizeof(buffer), "%08zu", image.GetFrame());
  auto filename = "_images/"s + buffer + ".png";

  auto view = ImageView::MakeColorConvertedView(
      ImageView::MakeView(image),
      ColorConverter::CityScapesPalette());
  ImageIO::WriteView(filename, view);
}
*/

// 解析命令行参数，返回主机名和端口号
static auto ParseArguments(int argc, const char *argv[]) {
  EXPECT_TRUE((argc == 1u) || (argc == 3u));  // 断言argc的值要么是1要么是3，否则就抛出异常
  using ResultType = std::tuple<std::string, uint16_t>;
  // 根据参数数量返回不同的结果
  return argc == 3u ?
      ResultType{argv[1u], std::stoi(argv[2u])} :
      ResultType{"localhost", 2000u};
}

int main(int argc, const char *argv[]) {
  try {
    // 解析命令行参数
    std::string host;
    uint16_t port;
    std::tie(host, port) = ParseArguments(argc, argv);
    
    // 初始化随机数生成器
    std::mt19937_64 rng((std::random_device())());

    // 创建CARLA客户端
    auto client = cc::Client(host, port);
    client.SetTimeout(40s);
    
    // 打印客户端和服务器API版本
    std::cout << "Client API version : " << client.GetClientVersion() << '\n';
    std::cout << "Server API version : " << client.GetServerVersion() << '\n';

    // 随机加载一个城镇位置
    auto town_name = RandomChoice(client.GetAvailableMaps(), rng);
    std::cout << "Loading world: " << town_name << std::endl;
    auto world = client.LoadWorld(town_name);

    // 随机获取一个交通工具蓝图
    auto blueprint_library = world.GetBlueprintLibrary();
    auto vehicles = blueprint_library->Filter("vehicle");
    auto blueprint = RandomChoice(*vehicles, rng);

    // 随机选取一个蓝图
    if (blueprint.ContainsAttribute("color")) {
      auto &attribute = blueprint.GetAttribute("color");
      blueprint.SetAttribute(
          "color",
          RandomChoice(attribute.GetRecommendedValues(), rng));
    }

    // 随机选择一个推荐的出生点
    auto map = world.GetMap();
    auto transform = RandomChoice(map->GetRecommendedSpawnPoints(), rng);

    // 在世界中生成车辆
    auto actor = world.SpawnActor(blueprint, transform);
    std::cout << "Spawned " << actor->GetDisplayId() << '\n';
    auto vehicle = boost::static_pointer_cast<cc::Vehicle>(actor);

    // 应用控制到车辆上，例如设置油门
    cc::Vehicle::Control control;
    control.throttle = 1.0f;
    vehicle->ApplyControl(control);

    // 调整观察者位置以便在模拟器窗口中看到车辆
    auto spectator = world.GetSpectator();
    transform.location += 32.0f * transform.GetForwardVector();
    transform.location.z += 2.0f;
    transform.rotation.yaw += 180.0f;
    transform.rotation.pitch = -15.0f;
    spectator->SetTransform(transform);

/*
    // Find a camera blueprint.
    auto camera_bp = blueprint_library->Find("sensor.camera.semantic_segmentation");
    EXPECT_TRUE(camera_bp != nullptr);

    // Spawn a camera attached to the vehicle.
    auto camera_transform = cg::Transform{
        cg::Location{-5.5f, 0.0f, 2.8f},   // x, y, z.
        cg::Rotation{-15.0f, 0.0f, 0.0f}}; // pitch, yaw, roll.
    auto cam_actor = world.SpawnActor(*camera_bp, camera_transform, actor.get());
    auto camera = boost::static_pointer_cast<cc::Sensor>(cam_actor);

    // Register a callback to save images to disk.
    camera->Listen([](auto data) {
        auto image = boost::static_pointer_cast<csd::Image>(data);
        EXPECT_TRUE(image != nullptr);
        SaveSemSegImageToDisk(*image);
    });

    std::this_thread::sleep_for(10s);

    // Remove actors from the simulation.
    camera->Destroy();
*/
    // 销毁车辆
    vehicle->Destroy();
    std::cout << "Actors destroyed." << std::endl;

  } catch (const cc::TimeoutException &e) {
    // 捕获超时异常并打印
    std::cout << '\n' << e.what() << std::endl;
    return 1;
  } catch (const std::exception &e) {
    // 捕获其他异常并打印
    std::cout << "\nException: " << e.what() << std::endl;
    return 2;
  }
}
