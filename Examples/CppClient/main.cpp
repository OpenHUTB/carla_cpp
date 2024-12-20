// 引入 C++ 标准库头文件
#include <iostream>    // 提供输入输出流功能
#include <random>      // 提供随机数生成器功能
#include <sstream>     // 提供字符串流功能
#include <stdexcept>   // 提供异常处理功能
#include <string>      // 提供字符串类型功能
#include <thread>      // 提供多线程功能
#include <tuple>       // 提供元组类型和操作功能

// 引入CARLA客户端库的头文件，用于与CARLA模拟器进行交互，包含获取蓝图、世界、传感器等相关功能的接口
#include <carla/client/ActorBlueprint.h>       // 用于获取和操作CARLA模拟器中的角色蓝图 (Actor Blueprint)，可以用于生成或配置车辆、行人等模拟实体的属性。
#include <carla/client/BlueprintLibrary.h>     // 提供访问CARLA模拟器中各种蓝图资源的功能，能够加载车辆、建筑、传感器等各种对象的蓝图。
#include <carla/client/Client.h>               // 定义与CARLA模拟器客户端通信的接口，主要用于连接到模拟器服务器，管理会话，获取世界对象等。
#include <carla/client/Map.h>                  // 提供对CARLA模拟器中的地图（包括地图的相关信息和操作）的访问，支持查询和操作地图。
#include <carla/client/Sensor.h>               // 用于创建和配置各种传感器（如相机、激光雷达、GPS等），并处理传感器的输出数据。
#include <carla/client/TimeoutException.h>     // 定义在CARLA模拟器客户端操作中可能遇到的超时异常，用于处理长时间未响应的情形。
#include <carla/client/World.h>               // 定义模拟器中的世界对象，提供对世界中的所有实体（如车辆、行人、道路、传感器等）的访问和管理。
#include <carla/geom/Transform.h>              // 定义转换（Transform）类，用于表示和操作位置、旋转、缩放等几何变换，常用于描述物体的位置和姿态。
#include <carla/image/ImageIO.h>              // 提供对图像数据输入输出的功能，可以读取和写入图像文件，常用于处理传感器生成的图像数据。
#include <carla/image/ImageView.h>            // 定义图像视图（ImageView）类，表示图像数据的访问和操作，通常用于访问传感器拍摄的图像内容。
#include <carla/sensor/data/Image.h>          // 定义图像数据类，用于表示传感器（如相机）拍摄到的图像数据，支持多种图像格式。

// 为CARLA命名空间创建别名，简化代码书写，后续使用cc、cg、csd来代表相应的carla命名空间，使代码更简洁易读
namespace cc = carla::client;
namespace cg = carla::geom;
namespace csd = carla::sensor::data;

// 使用std命名空间中的chrono和string字面量，方便在代码中直接使用时间字面量（如40s）以及更自然地进行字符串拼接操作
using namespace std::chrono_literals;
using namespace std::string_literals;

// 定义一个宏，用于断言表达式为真，否则抛出运行时错误。用于在代码逻辑中对一些关键条件进行检查，确保程序按预期执行
#define EXPECT_TRUE(pred) if (!(pred)) { throw std::runtime_error(#pred); }

/// 从给定范围中随机选择一个元素
/// 
/// @tparam RangeT 表示可迭代范围的类型，例如数组、向量等容器类型，其元素可通过下标访问
/// @tparam RNG 随机数生成器类型，用于生成随机的索引值来选取范围内的元素
/// @param range 输入的可迭代范围，从中选择元素
/// @param generator 随机数生成器实例，用于生成随机索引
/// @return 返回在给定范围中随机选择的元素的引用，确保range不为空，否则会抛出异常
template <typename RangeT, typename RNG>
static auto &RandomChoice(const RangeT &range, RNG &&generator) {
    EXPECT_TRUE(range.size() > 0u);  // 断言值一定大于0，确保可迭代范围中有元素可供选择，否则就抛出异常
    std::uniform_int_distribution<size_t> dist{0u, range.size() - 1u};
    return range[dist(std::forward<RNG>(generator))];
}

// 该函数的功能是将语义分割图像保存到磁盘，并将图像转换为CityScapes调色板格式。
// 目前该函数被注释掉，尚未启用，可能是因为没有相应的图像处理库或保存功能暂时不需要。
// 如果后续需要保存图像，可以取消注释并确保相关的依赖和功能已正确实现。
/*
static void SaveSemSegImageToDisk(const csd::Image &image) {
    using namespace carla::image;

    // 构造保存文件的文件名，文件名由图像的帧编号组成
    // 使用std::snprintf生成一个具有固定格式（8位数字）的字符串
    char buffer[9u];
    std::snprintf(buffer, sizeof(buffer), "%08zu", image.GetFrame());
    auto filename = "_images/"s + buffer + ".png";

    // 创建一个ColorConvertedView视图，将原始图像转换为CityScapes调色板格式
    // ImageView::MakeView(image) 创建一个基本的图像视图，
    // ColorConverter::CityScapesPalette() 将图像颜色转换为CityScapes调色板
    auto view = ImageView::MakeColorConvertedView(
            ImageView::MakeView(image),
            ColorConverter::CityScapesPalette());
    ImageIO::WriteView(filename, view);
}
*/

// 解析命令行参数，返回主机名和端口号
// 根据传入的命令行参数数量判断并解析出CARLA服务器对应的主机名和端口号信息
// 如果参数数量为1，默认使用localhost作为主机名，端口号为2000；如果参数数量为3，则使用传入的第2和第3个参数分别作为主机名和端口号
// 同时使用断言确保参数数量符合预期，避免因错误的参数输入导致程序异常
static auto ParseArguments(int argc, const char *argv[]) {
    EXPECT_TRUE((argc == 1u) || (argc == 3u));  // 断言argc的值要么是1要么是3，否则就抛出异常，保证参数数量正确
    using ResultType = std::tuple<std::string, uint16_t>;
    // 根据参数数量返回不同的结果，符合参数约定的情况下返回包含主机名和端口号的元组
    return argc == 3u?
           ResultType{argv[1u], std::stoi(argv[2u])} :
           ResultType{"localhost", 2000u};
}

int main(int argc, const char *argv[]) {
    try {
        // 解析命令行参数
        std::string host;
        uint16_t port;
        std::tie(host, port) = ParseArguments(argc, argv);

        // 初始化随机数生成器，使用硬件随机设备初始化一个基于梅森旋转算法的64位随机数生成器，用于后续生成各种随机选择的索引等操作
        std::mt19937_64 rng((std::random_device())());

        // 创建CARLA客户端，尝试连接到指定主机名和端口号对应的CARLA服务器，并设置连接超时时间为40秒
        auto client = cc::Client(host, port);
        client.SetTimeout(40s);

        // 打印客户端和服务器API版本，方便查看当前程序与服务器之间的接口版本信息，便于排查版本兼容性等问题
        std::cout << "Client API version : " << client.GetClientVersion() << '\n';
        std::cout << "Server API version : " << client.GetServerVersion() << '\n';

        // 随机加载一个城镇位置，从CARLA服务器获取可用的地图列表，然后使用随机数生成器随机选择一个城镇地图名称，接着加载对应的城镇世界到模拟环境中
        auto town_name = RandomChoice(client.GetAvailableMaps(), rng);
        std::cout << "Loading world: " << town_name << std::endl;
        auto world = client.LoadWorld(town_name);

        // 随机获取一个交通工具蓝图，先从加载的世界中获取蓝图库，然后筛选出代表交通工具的蓝图列表，最后通过随机数生成器从中随机选择一个交通工具蓝图
        auto blueprint_library = world.GetBlueprintLibrary();
        auto vehicles = blueprint_library->Filter("vehicle");
        auto blueprint = RandomChoice(*vehicles, rng);

        // 随机选取一个蓝图，如果该蓝图包含颜色属性，就从颜色属性的推荐值列表中随机选择一个颜色值，并设置为该蓝图的颜色属性，使得生成的车辆具有随机的外观颜色
        if (blueprint.ContainsAttribute("color")) {
            auto &attribute = blueprint.GetAttribute("color");
            blueprint.SetAttribute(
                    "color",
                    RandomChoice(attribute.GetRecommendedValues(), rng));
        }

        // 随机选择一个推荐的出生点，从当前世界的地图中获取推荐的车辆出生点列表，再利用随机数生成器随机选取一个出生点，该出生点将用于后续生成车辆
        auto map = world.GetMap();
        auto transform = RandomChoice(map->GetRecommendedSpawnPoints(), rng);

        // 在世界中生成车辆，根据前面选择的交通工具蓝图和出生点，在模拟世界中生成对应的车辆实例，并输出车辆的显示ID，方便后续识别和调试
        auto actor = world.SpawnActor(blueprint, transform);
        std::cout << "Spawned " << actor->GetDisplayId() << '\n';
        auto vehicle = boost::static_pointer_cast<cc::Vehicle>(actor);

        // 应用控制到车辆上，例如设置油门，创建一个车辆控制结构体实例，并将油门值设置为1.0，表示全油门加速，然后将该控制应用到生成的车辆上，使其开始运动
        cc::Vehicle::Control control;
        control.throttle = 1.0f;
        vehicle->ApplyControl(control);

        // 调整观察者位置以便在模拟器窗口中看到车辆，获取模拟世界中的观察者（相当于模拟窗口的视角），通过对其位置和旋转角度进行一系列计算和调整，改变视角位置和方向，使得可以在模拟窗口中看到生成并正在运动的车辆
        auto spectator = world.GetSpectator();
        transform.location += 32.0f * transform.GetForwardVector();
        transform.location.z += 2.0f;
        transform.rotation.yaw += 180.0f;
        transform.rotation.pitch = -15.0f;
        spectator->SetTransform(transform);

        /*
        // Find a camera blueprint.
        // 查找一个相机蓝图，在蓝图库中查找代表语义分割相机的蓝图，如果找不到则会触发后续的断言异常，用于后续在车辆上挂载相机传感器
        auto camera_bp = blueprint_library->Find("sensor.camera.semantic_segmentation");
        EXPECT_TRUE(camera_bp!= nullptr);

        // Spawn a camera attached to the vehicle.
        // 在车辆上生成一个相机传感器，根据指定的位置和旋转信息创建一个变换结构体，然后将相机传感器挂载到前面生成的车辆上，使得相机可以跟随车辆运动并采集数据
        auto camera_transform = cg::Transform{
                cg::Location{-5.5f, 0.0f, 2.8f},   // x, y, z.
                cg::Rotation{-15.0f, 0.0f, 0.0f}}; // pitch, yaw, roll.
        auto cam_actor = world.SpawnActor(*camera_bp, camera_transform, actor.get());
        auto camera = boost::static_pointer_cast<cc::Sensor>(cam_actor);

        // Register a callback to save images to disk.
        // 注册一个回调函数，用于当相机传感器采集到图像数据时进行处理，当前的处理逻辑是将接收到的图像数据转换为语义分割图像类型，然后调用保存图像到磁盘的函数（当前被注释掉未生效）
        camera->Listen([](auto data) {
            auto image = boost::static_pointer_cast<csd::Image>(data);
            EXPECT_TRUE(image!= nullptr);
            SaveSemSegImageToDisk(*image);
        });

        std::this_thread::sleep_for(10s);

        // Remove actors from the simulation.
        // 移除模拟环境中的相机传感器，释放相关资源，结束相机传感器的使用
        camera->Destroy();
        */

        // 销毁车辆，从模拟世界中移除生成的车辆实例，释放相关资源，完成本次模拟中车辆相关的操作
        vehicle->Destroy();
        std::cout << "Actors destroyed." << std::endl;

    } catch (const cc::TimeoutException &e) {
        // 捕获超时异常并打印异常信息，当连接CARLA服务器或者其他涉及超时的操作超过设置的时间限制时，会进入此异常处理分支，方便排查网络连接等相关问题
        std::cout << '\n' << e.what() << std::endl;
        return 1;
    } catch (const std::exception &e) {
        // 捕获其他异常并打印异常信息，用于处理除了超时异常之外的其他运行时异常情况，便于定位程序中其他可能出现的错误
        std::cout << "\nException: " << e.what() << std::endl;
        return 2;
    }
}
