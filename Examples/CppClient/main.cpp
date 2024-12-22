//==============================================================================
// 头文件包含
//==============================================================================

// C++ 标准库头文件
#include <iostream>     // 提供输入输出流功能
#include <random>      // 提供随机数生成器功能
#include <sstream>     // 提供字符串流功能
#include <stdexcept>   // 提供异常处理功能
#include <string>      // 提供字符串类型功能
#include <thread>      // 提供多线程功能
#include <tuple>       // 提供元组类型和操作功能

// CARLA客户端库头文件
// 这些头文件用于与CARLA模拟器进行交互，包含获取蓝图、世界、传感器等相关功能的接口
#include <carla/client/ActorBlueprint.h>      // 角色蓝图：用于生成或配置车辆、行人等模拟实体
#include <carla/client/BlueprintLibrary.h>    // 蓝图库：提供访问车辆、建筑、传感器等对象的蓝图
#include <carla/client/Client.h>              // 客户端：定义与模拟器服务器的通信接口
#include <carla/client/Map.h>                 // 地图：提供对模拟器地图的访问和操作
#include <carla/client/Sensor.h>              // 传感器：用于创建和配置相机、激光雷达、GPS等
#include <carla/client/TimeoutException.h>    // 超时异常：处理客户端操作超时情况
#include <carla/client/World.h>               // 世界：提供对模拟世界中所有实体的访问和管理
#include <carla/geom/Transform.h>             // 转换：用于表示位置、旋转、缩放等几何变换
#include <carla/image/ImageIO.h>              // 图像IO：提供图像数据的输入输出功能
#include <carla/image/ImageView.h>            // 图像视图：用于访问和操作图像数据
#include <carla/sensor/data/Image.h>          // 图像数据：表示传感器拍摄的图像数据

//==============================================================================
// 命名空间别名和使用声明
//==============================================================================

// 创建命名空间别名，简化代码书写
namespace cc = carla::client;      // 客户端相关功能
namespace cg = carla::geom;        // 几何变换相关功能
namespace csd = carla::sensor::data;// 传感器数据相关功能

// 使用std命名空间中的特定功能
using namespace std::chrono_literals;   // 支持时间字面量，如 40s
using namespace std::string_literals;   // 支持字符串字面量，如 "text"s

//==============================================================================
// 宏定义
//==============================================================================

// 定义断言宏，用于运行时条件检查
#define EXPECT_TRUE(pred) if (!(pred)) { throw std::runtime_error(#pred); }

//==============================================================================
// 工具函数
//==============================================================================

/// 从给定范围中随机选择一个元素
/// @tparam RangeT 可迭代范围类型（如数组、向量等）
/// @tparam RNG 随机数生成器类型
/// @param range 输入的可迭代范围
/// @param generator 随机数生成器实例
/// @return 返回随机选择的元素引用
template <typename RangeT, typename RNG>
static auto &RandomChoice(const RangeT &range, RNG &&generator) {
    EXPECT_TRUE(range.size() > 0u);
    std::uniform_int_distribution<size_t> dist{0u, range.size() - 1u};
    return range[dist(std::forward<RNG>(generator))];
}

/*
// 保存语义分割图像到磁盘（暂未启用）
static void SaveSemSegImageToDisk(const csd::Image &image) {
    using namespace carla::image;
    
    // 生成文件名（8位数字帧号）
    char buffer[9u];
    std::snprintf(buffer, sizeof(buffer), "%08zu", image.GetFrame());
    auto filename = "_images/"s + buffer + ".png";
    
    // 转换为CityScapes调色板格式并保存
    auto view = ImageView::MakeColorConvertedView(
            ImageView::MakeView(image),
            ColorConverter::CityScapesPalette());
    ImageIO::WriteView(filename, view);
}
*/

/// 解析命令行参数
/// @param argc 参数数量
/// @param argv 参数数组
/// @return 返回包含主机名和端口号的元组
static auto ParseArguments(int argc, const char *argv[]) {
    EXPECT_TRUE((argc == 1u) || (argc == 3u));
    using ResultType = std::tuple<std::string, uint16_t>;
    return argc == 3u ?
           ResultType{argv[1u], std::stoi(argv[2u])} :
           ResultType{"localhost", 2000u};
}

//==============================================================================
// 主函数
//==============================================================================

int main(int argc, const char *argv[]) {
    try {
        // 解析命令行参数
        std::string host;
        uint16_t port;
        std::tie(host, port) = ParseArguments(argc, argv);

        // 初始化随机数生成器
        std::mt19937_64 rng((std::random_device())());

        //----------------------------------------------------------------------
        // 初始化CARLA客户端
        //----------------------------------------------------------------------
        
        // 创建客户端并设置超时
        auto client = cc::Client(host, port);
        client.SetTimeout(40s);

        // 输出版本信息
        std::cout << "Client API version : " << client.GetClientVersion() << '\n';
        std::cout << "Server API version : " << client.GetServerVersion() << '\n';

        //----------------------------------------------------------------------
        // 配置模拟环境
        //----------------------------------------------------------------------

        // 随机加载一个城镇
        auto town_name = RandomChoice(client.GetAvailableMaps(), rng);
        std::cout << "Loading world: " << town_name << std::endl;
        auto world = client.LoadWorld(town_name);

        // 获取并配置车辆蓝图
        auto blueprint_library = world.GetBlueprintLibrary();
        auto vehicles = blueprint_library->Filter("vehicle");
        auto blueprint = RandomChoice(*vehicles, rng);

        // 配置车辆颜色（如果支持）
        if (blueprint.ContainsAttribute("color")) {
            auto &attribute = blueprint.GetAttribute("color");
            blueprint.SetAttribute(
                    "color",
                    RandomChoice(attribute.GetRecommendedValues(), rng));
        }

        //----------------------------------------------------------------------
        // 生成和控制车辆
        //----------------------------------------------------------------------

        // 选择生成点并生成车辆
        auto map = world.GetMap();
        auto transform = RandomChoice(map->GetRecommendedSpawnPoints(), rng);
        auto actor = world.SpawnActor(blueprint, transform);
        std::cout << "Spawned " << actor->GetDisplayId() << '\n';
        auto vehicle = boost::static_pointer_cast<cc::Vehicle>(actor);

        // 设置车辆控制
        cc::Vehicle::Control control;
        control.throttle = 1.0f;
        vehicle->ApplyControl(control);

        // 调整观察者视角
        auto spectator = world.GetSpectator();
        transform.location += 32.0f * transform.GetForwardVector();
        transform.location.z += 2.0f;
        transform.rotation.yaw += 180.0f;
        transform.rotation.pitch = -15.0f;
        spectator->SetTransform(transform);

        /*
        //----------------------------------------------------------------------
        // 相机传感器配置（暂未启用）
        //----------------------------------------------------------------------

        // 配置相机蓝图
        auto camera_bp = blueprint_library->Find("sensor.camera.semantic_segmentation");
        EXPECT_TRUE(camera_bp != nullptr);

        // 生成相机
        auto camera_transform = cg::Transform{
                cg::Location{-5.5f, 0.0f, 2.8f},     // x, y, z
                cg::Rotation{-15.0f, 0.0f, 0.0f}};   // pitch, yaw, roll
        auto cam_actor = world.SpawnActor(*camera_bp, camera_transform, actor.get());
        auto camera = boost::static_pointer_cast<cc::Sensor>(cam_actor);

        // 设置图像处理回调
        camera->Listen([](auto data) {
            auto image = boost::static_pointer_cast<csd::Image>(data);
            EXPECT_TRUE(image != nullptr);
            SaveSemSegImageToDisk(*image);
        });

        std::this_thread::sleep_for(10s);

        // 清理相机
        camera->Destroy();
        */

        // 清理车辆
        vehicle->Destroy();
        std::cout << "Actors destroyed." << std::endl;

    } 
    //--------------------------------------------------------------------------
    // 异常处理
    //--------------------------------------------------------------------------
    catch (const cc::TimeoutException &e) {
        std::cout << '\n' << e.what() << std::endl;
        return 1;
    } 
    catch (const std::exception &e) {
        std::cout << "\nException: " << e.what() << std::endl;
        return 2;
    }
}
