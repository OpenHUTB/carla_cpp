//==============================================================================
// CARLA客户端程序
// 功能：创建CARLA模拟器客户端，生成随机车辆并进行基本控制
//==============================================================================

//------------------------------------------------------------------------------
// 标准库头文件
//------------------------------------------------------------------------------
#include <iostream>    // 提供输入输出流功能
#include <random>      // 提供随机数生成器功能
#include <sstream>     // 提供字符串流功能
#include <stdexcept>   // 提供异常处理功能
#include <string>      // 提供字符串类型功能
#include <thread>      // 提供多线程功能
#include <tuple>       // 提供元组类型和操作功能

//------------------------------------------------------------------------------
// CARLA客户端库头文件
//------------------------------------------------------------------------------
// 核心功能头文件
#include <carla/client/ActorBlueprint.h>       // 角色蓝图：用于生成或配置车辆、行人等模拟实体的属性
#include <carla/client/BlueprintLibrary.h>     // 蓝图库：提供访问车辆、建筑、传感器等各种对象的蓝图
#include <carla/client/Client.h>               // 客户端：用于连接模拟器服务器，管理会话，获取世界对象
#include <carla/client/Map.h>                  // 地图：提供对地图的访问和操作功能
#include <carla/client/World.h>                // 世界：管理模拟世界中的所有实体

// 传感器相关头文件
#include <carla/client/Sensor.h>               // 传感器：创建和配置相机、激光雷达、GPS等
#include <carla/sensor/data/Image.h>           // 图像数据：表示传感器拍摄的图像数据

// 工具类头文件
#include <carla/client/TimeoutException.h>     // 超时异常：处理客户端操作超时情况
#include <carla/geom/Transform.h>              // 几何变换：处理位置、旋转、缩放等
#include <carla/image/ImageIO.h>               // 图像IO：处理图像数据的输入输出
#include <carla/image/ImageView.h>             // 图像视图：访问和操作图像数据

//------------------------------------------------------------------------------
// 命名空间设置
//------------------------------------------------------------------------------
// CARLA命名空间别名
namespace cc = carla::client;          // 客户端功能
namespace cg = carla::geom;            // 几何变换
namespace csd = carla::sensor::data;   // 传感器数据

// 标准库功能引用
using namespace std::chrono_literals;   // 支持时间字面量（如 40s）
using namespace std::string_literals;   // 支持字符串字面量（如 "text"s）

//------------------------------------------------------------------------------
// 工具宏和函数
//------------------------------------------------------------------------------
// 运行时条件检查宏
#define EXPECT_TRUE(pred) if (!(pred)) { throw std::runtime_error(#pred); }

/// 从给定范围中随机选择元素的模板函数
/// @tparam RangeT 可迭代范围类型（如数组、向量）
/// @tparam RNG 随机数生成器类型
/// @param range 输入范围
/// @param generator 随机数生成器
/// @return 随机选择的元素引用
template <typename RangeT, typename RNG>
static auto &RandomChoice(const RangeT &range, RNG &&generator) {
    EXPECT_TRUE(range.size() > 0u);  // 确保范围非空
    std::uniform_int_distribution<size_t> dist{0u, range.size() - 1u};
    return range[dist(std::forward<RNG>(generator))];
}

//------------------------------------------------------------------------------
// 图像处理功能（暂未启用）
//------------------------------------------------------------------------------
/*
/// 将语义分割图像保存到磁盘
/// @param image 要保存的图像数据
static void SaveSemSegImageToDisk(const csd::Image &image) {
    using namespace carla::image;

    // 生成带帧号的文件名
    char buffer[9u];
    std::snprintf(buffer, sizeof(buffer), "%08zu", image.GetFrame());
    auto filename = "_images/"s + buffer + ".png";

    // 转换为CityScapes格式并保存
    auto view = ImageView::MakeColorConvertedView(
            ImageView::MakeView(image),
            ColorConverter::CityScapesPalette());
    ImageIO::WriteView(filename, view);
}
*/

//------------------------------------------------------------------------------
// 参数解析
//------------------------------------------------------------------------------
/// 解析命令行参数获取连接信息
/// @param argc 参数数量
/// @param argv 参数数组
/// @return {主机名, 端口号}的元组
static auto ParseArguments(int argc, const char *argv[]) {
    EXPECT_TRUE((argc == 1u) || (argc == 3u));  // 验证参数数量
    using ResultType = std::tuple<std::string, uint16_t>;
    // 返回连接信息：使用指定值或默认值
    return argc == 3u ?
           ResultType{argv[1u], std::stoi(argv[2u])} :
           ResultType{"localhost", 2000u};
}

//------------------------------------------------------------------------------
// 主程序
//------------------------------------------------------------------------------
int main(int argc, const char *argv[]) {
    try {
        //----------------------------------------------------------------------
        // 初始化
        //----------------------------------------------------------------------
        // 解析命令行参数
        std::string host;
        uint16_t port;
        std::tie(host, port) = ParseArguments(argc, argv);

        // 初始化随机数生成器
        std::mt19937_64 rng((std::random_device())());

        //----------------------------------------------------------------------
        // 建立CARLA连接
        //----------------------------------------------------------------------
        // 创建客户端并设置超时
        auto client = cc::Client(host, port);
        client.SetTimeout(40s);

        // 输出版本信息
        std::cout << "Client API version : " << client.GetClientVersion() << '\n';
        std::cout << "Server API version : " << client.GetServerVersion() << '\n';

        //----------------------------------------------------------------------
        // 世界设置
        //----------------------------------------------------------------------
        // 加载随机地图
        auto town_name = RandomChoice(client.GetAvailableMaps(), rng);
        std::cout << "Loading world: " << town_name << std::endl;
        auto world = client.LoadWorld(town_name);

        //----------------------------------------------------------------------
        // 车辆配置
        //----------------------------------------------------------------------
        // 获取车辆蓝图
        auto blueprint_library = world.GetBlueprintLibrary();
        auto vehicles = blueprint_library->Filter("vehicle");
        auto blueprint = RandomChoice(*vehicles, rng);

        // 配置车辆颜色
        if (blueprint.ContainsAttribute("color")) {
            auto &attribute = blueprint.GetAttribute("color");
            blueprint.SetAttribute(
                    "color",
                    RandomChoice(attribute.GetRecommendedValues(), rng));
        }

        // 选择生成点
        auto map = world.GetMap();
        auto transform = RandomChoice(map->GetRecommendedSpawnPoints(), rng);

        //----------------------------------------------------------------------
        // 车辆生成与控制
        //----------------------------------------------------------------------
        // 生成车辆
        auto actor = world.SpawnActor(blueprint, transform);
        std::cout << "Spawned " << actor->GetDisplayId() << '\n';
        auto vehicle = boost::static_pointer_cast<cc::Vehicle>(actor);

        // 设置车辆控制
        cc::Vehicle::Control control;
        control.throttle = 1.0f;
        vehicle->ApplyControl(control);

        //----------------------------------------------------------------------
        // 视角调整
        //----------------------------------------------------------------------
        auto spectator = world.GetSpectator();
        transform.location += 32.0f * transform.GetForwardVector();
        transform.location.z += 2.0f;
        transform.rotation.yaw += 180.0f;
        transform.rotation.pitch = -15.0f;
        spectator->SetTransform(transform);

        //----------------------------------------------------------------------
        // 相机系统（暂未启用）
        //----------------------------------------------------------------------
        /*
        // 配置相机
        auto camera_bp = blueprint_library->Find("sensor.camera.semantic_segmentation");
        EXPECT_TRUE(camera_bp != nullptr);

        // 生成相机
        auto camera_transform = cg::Transform{
                cg::Location{-5.5f, 0.0f, 2.8f},    // x, y, z
                cg::Rotation{-15.0f, 0.0f, 0.0f}};  // pitch, yaw, roll
        auto cam_actor = world.SpawnActor(*camera_bp, camera_transform, actor.get());
        auto camera = boost::static_pointer_cast<cc::Sensor>(cam_actor);

        // 设置图像处理
        camera->Listen([](auto data) {
            auto image = boost::static_pointer_cast<csd::Image>(data);
            EXPECT_TRUE(image != nullptr);
            SaveSemSegImageToDisk(*image);
        });

        std::this_thread::sleep_for(10s);

        // 清理相机
        camera->Destroy();
        */

        //----------------------------------------------------------------------
        // 清理
        //----------------------------------------------------------------------
        vehicle->Destroy();
        std::cout << "Actors destroyed." << std::endl;

    } 
    //--------------------------------------------------------------------------
    // 异常处理
    //--------------------------------------------------------------------------
    catch (const cc::TimeoutException &e) {
        // 处理连接超时
        std::cout << '\n' << e.what() << std::endl;
        return 1;
    } 
    catch (const std::exception &e) {
        // 处理其他异常
        std::cout << "\nException: " << e.what() << std::endl;
        return 2;
    }
}
