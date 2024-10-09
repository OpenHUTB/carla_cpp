// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

//#include "carla/client/Client.h"


#include "carla/Sockets.h"
#include "carla/client/detail/Simulator.h"

#include "carla/trafficmanager/Constants.h"
#include "carla/trafficmanager/TrafficManager.h"
#include "carla/trafficmanager/TrafficManagerBase.h"
#include "carla/trafficmanager/TrafficManagerLocal.h"
#include "carla/trafficmanager/TrafficManagerRemote.h"

#define DEBUG_PRINT_TM  0
#define IP_DATA_BUFFER_SIZE     80

namespace carla {
namespace traffic_manager {

// 使用速度阈值和PID常量
using namespace constants::SpeedThreshold;
using namespace constants::PID;

// 定义一个映射，存储TrafficManagerBase指针
std::map<uint16_t, TrafficManagerBase*> TrafficManager::_tm_map;
// 定义互斥锁，用于多线程安全
std::mutex TrafficManager::_mutex;

// TrafficManager构造函数
TrafficManager::TrafficManager(
    carla::client::detail::EpisodeProxy episode_proxy,
    uint16_t port)
  : _port(port) {

  // 检查是否能获取当前端口的交通管理器
  if(!GetTM(_port)){
    // 如果没有获取到，尝试连接已存在的TM服务器
    if(!CreateTrafficManagerClient(episode_proxy, port)) {
      // 如果TM服务器没有运行，创建一个新的TM服务器
      CreateTrafficManagerServer(episode_proxy, port);
    }
  }
}

// 释放资源
void TrafficManager::Release() {
  std::lock_guard<std::mutex> lock(_mutex); // 锁定互斥锁以保证线程安全
  for(auto& tm : _tm_map) { // 遍历所有交通管理器
    tm.second->Release(); // 调用释放方法
    TrafficManagerBase *base_ptr = tm.second; // 获取指针
    delete base_ptr; // 删除指针，释放内存
  }
  _tm_map.clear(); // 清空映射
}

// 重置所有交通管理器
void TrafficManager::Reset() {
  std::lock_guard<std::mutex> lock(_mutex); // 锁定互斥锁
  for(auto& tm : _tm_map) { // 遍历所有交通管理器
    tm.second->Reset(); // 调用重置方法
  }
}

// 执行每个交通管理器的Tick操作
void TrafficManager::Tick() {
  std::lock_guard<std::mutex> lock(_mutex); // 锁定互斥锁
  for(auto& tm : _tm_map) { // 遍历所有交通管理器
    tm.second->SynchronousTick(); // 调用同步Tick方法
  }
}

// 关闭交通管理器
void TrafficManager::ShutDown() {
  TrafficManagerBase* tm_ptr = GetTM(_port); // 获取指定端口的交通管理器
  std::lock_guard<std::mutex> lock(_mutex); // 锁定互斥锁
  auto it = _tm_map.find(_port); // 查找端口对应的交通管理器
  if (it != _tm_map.end()) {
    _tm_map.erase(it); // 删除映射中的交通管理器
  }
  if(tm_ptr != nullptr) {
    tm_ptr->ShutDown(); // 调用关闭方法
    delete tm_ptr; // 删除指针，释放内存
  }
}

// 创建交通管理器服务器
void TrafficManager::CreateTrafficManagerServer(
    carla::client::detail::EpisodeProxy episode_proxy,
    uint16_t port) {

  // 获取本地IP地址
  auto GetLocalIP = [=](const uint16_t sport)-> std::pair<std::string, uint16_t> {
    std::pair<std::string, uint16_t> localIP; // 存储本地IP和端口对
    int sock = socket(AF_INET, SOCK_DGRAM, 0); // 创建UDP socket
    if(sock == SOCK_INVALID_INDEX) {
      #if DEBUG_PRINT_TM
      std::cout << "Error number 1: " << errno << std::endl; // 打印错误号
      std::cout << "Error message: " << strerror(errno) << std::endl; // 打印错误信息
      #endif
    } else {
      int err;
      sockaddr_in loopback; // 定义回环地址结构
      std::memset(&loopback, 0, sizeof(loopback)); // 清零结构体
      loopback.sin_family = AF_INET; // 设置地址族为IPv4
      loopback.sin_addr.s_addr = INADDR_LOOPBACK; // 设置为回环地址
      loopback.sin_port = htons(9); // 设置端口为9（任意端口）
      err = connect(sock, reinterpret_cast<sockaddr*>(&loopback), sizeof(loopback)); // 尝试连接
      if(err == SOCK_INVALID_INDEX) {
        #if DEBUG_PRINT_TM
        std::cout << "Error number 2: " << errno << std::endl; // 打印连接错误号
        std::cout << "Error message: " << strerror(errno) << std::endl; // 打印连接错误信息
        #endif
      } else {
        socklen_t addrlen = sizeof(loopback);
        err = getsockname(sock, reinterpret_cast<struct sockaddr*> (&loopback), &addrlen); // 获取socket名称
        if(err == SOCK_INVALID_INDEX) {
          #if DEBUG_PRINT_TM
          std::cout << "Error number 3: " << errno << std::endl; // 打印获取socket名称错误号
          std::cout << "Error message: " << strerror(errno) << std::endl; // 打印获取socket名称错误信息
          #endif
        } else {
          char buffer[IP_DATA_BUFFER_SIZE]; // 定义缓冲区存储IP
          const char* p = inet_ntop(AF_INET, &loopback.sin_addr, buffer, IP_DATA_BUFFER_SIZE); // 转换IP地址
          if(p != NULL) {
            localIP = std::pair<std::string, uint16_t>(std::string(buffer), sport); // 保存本地IP和端口
          } else {
            #if DEBUG_PRINT_TM
            std::cout << "Error number 4: " << errno << std::endl; // 打印转换IP地址错误号
            std::cout << "Error message: " << strerror(errno) << std::endl; // 打印转换IP地址错误信息
            #endif
          }
        }
      }
     #ifdef _WIN32
        closesocket(sock); // 在Windows上关闭socket
#else
        close(sock); // 在非Windows系统上关闭socket
#endif
    }
    return localIP; // 返回本地IP和端口的对
  };

  /// 定义局部常量
  const std::vector<float> longitudinal_param = LONGITUDIAL_PARAM; // 纵向参数
  const std::vector<float> longitudinal_highway_param = LONGITUDIAL_HIGHWAY_PARAM; // 纵向高速公路参数
  const std::vector<float> lateral_param = LATERAL_PARAM; // 横向参数
  const std::vector<float> lateral_highway_param = LATERAL_HIGHWAY_PARAM; // 横向高速公路参数
  const float perc_difference_from_limit = INITIAL_PERCENTAGE_SPEED_DIFFERENCE; // 初始速度差百分比

  std::pair<std::string, uint16_t> serverTM; // 存储服务器IP和端口的对

  /// 创建交通管理器的本地实例
  TrafficManagerLocal* tm_ptr = new TrafficManagerLocal(
    longitudinal_param, // 传入纵向参数
    longitudinal_highway_param, // 传入纵向高速公路参数
    lateral_param, // 传入横向参数
    lateral_highway_param, // 传入横向高速公路参数
    perc_difference_from_limit, // 传入初始速度差百分比
    episode_proxy, // 传入剧集代理
    port); // 传入端口

  /// 获取TM服务器信息（本地IP和端口）
  serverTM = GetLocalIP(port);

  /// 将此客户端设置为TM服务器
  episode_proxy.Lock()->AddTrafficManagerRunning(serverTM); // 向剧集代理添加正在运行的交通管理器

  #if DEBUG_PRINT_TM
  /// 打印状态信息
  std::cout << "NEW@: Registered TM at "
        << serverTM.first  << ":"
        << serverTM.second << " ..... SUCCESS." // 打印成功注册的消息
        << std::endl;
  #endif

  /// 设置实例指针
  _tm_map.insert(std::make_pair(port, tm_ptr)); // 将交通管理器的指针存入映射

}

bool TrafficManager::CreateTrafficManagerClient(
    carla::client::detail::EpisodeProxy episode_proxy,
    uint16_t port) {

  bool result = false; // 初始化结果为false

  if(episode_proxy.Lock()->IsTrafficManagerRunning(port)) { // 检查指定端口的交通管理器是否正在运行

    /// 获取TM服务器信息（远程IP和端口）
    std::pair<std::string, uint16_t> serverTM =
      episode_proxy.Lock()->GetTrafficManagerRunning(port);

    /// 设置远程TM服务器的IP和端口
    TrafficManagerRemote* tm_ptr = new(std::nothrow)
      TrafficManagerRemote(serverTM, episode_proxy); // 创建远程交通管理器实例

    /// 尝试连接到远程TM服务器
    try {

      /// 检查内存是否分配成功
      if(tm_ptr != nullptr) {

        #if DEBUG_PRINT_TM
        // 测试打印
        std::cout << "OLD@: Registered TM at "
              << serverTM.first  << ":"
              << serverTM.second << " ..... TRY " // 打印尝试连接的消息
              << std::endl;
        #endif
        /// 尝试重置所有交通信号灯
        tm_ptr->HealthCheckRemoteTM(); // 进行健康检查

        /// 设置实例指针
        _tm_map.insert(std::make_pair(port, tm_ptr)); // 将远程交通管理器的指针存入映射

        result = true; // 更新结果为true，表示连接成功
      }
    }

    /// 如果发生连接错误
    catch (...) {

      /// 清除之前分配的内存
      delete tm_ptr; // 删除远程交通管理器实例

      #if DEBUG_PRINT_TM
      /// 测试打印
      std::cout 	<< "OLD@: Registered TM at "
            << serverTM.first  << ":"
            << serverTM.second << " ..... FAILED " // 打印连接失败的消息
            << std::endl;
      #endif
    }

  }

  return result; // 返回连接结果
}

} // namespace traffic_manager
} // namespace carla
