// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/PythonUtil.h"
#include "carla/client/Client.h"
#include "carla/client/World.h"
#include "carla/Logging.h"
#include "carla/rpc/ActorId.h"
#include "carla/trafficmanager/TrafficManager.h"

#include <thread>

#include <boost/python/stl_iterator.hpp>

//包含了 Carla 客户端相关的各种头文件，如PythonUtil.h用于处理 Python 相关的工具函数，可能涉及到与 Python 解释器交互时的全局解释器锁（GIL）等操作；client/Client.h和client/World.h分别定义了客户端和世界相关的类，用于与 Carla 模拟器进行连接、获取世界信息等操作；Logging.h用于日志记录；rpc/ActorId.h涉及到处理演员（在 Carla 场景中可以理解为各种实体对象）的唯一标识；trafficmanager/TrafficManager.h与交通管理相关，可能用于控制场景中的交通流量、车辆行为等。
//<thread>头文件用于支持多线程操作，在代码中用于并行处理批量命令等场景。
//boost/python/stl_iterator.hpp是 Boost.Python 库中的文件，用于在 Python 和 C++ 之间方便地处理标准库容器（如vector等）类型的数据绑定，使得在 Python 中可以像操作原生容器一样操作 C++ 的容器类型数据。

namespace ctm = carla::traffic_manager;

//定义了一个别名ctm，用于简化carla::traffic_manager命名空间的引用，方便在后续代码中使用，提高代码的可读性。

static void SetTimeout(carla::client::Client &client, double seconds) {
  client.SetTimeout(TimeDurationFromSeconds(seconds));
}

//这是一个静态函数，用于设置carla::client::Client对象的超时时间。它接受一个客户端对象引用和一个表示秒数的双精度浮点数作为参数，通过调用客户端对象的SetTimeout方法，并将秒数转换为相应的时间持续类型（可能是通过TimeDurationFromSeconds函数进行转换，该函数应该在其他地方定义）来设置超时时间。
static auto GetAvailableMaps(const carla::client::Client &self) {
  boost::python::list result;
  std::vector<std::string> maps;
  {
    carla::PythonUtil::ReleaseGIL unlock;
    maps = self.GetAvailableMaps();
  }
  for (const auto &str : maps) {
    result.append(str);
  }
  return result;
}
//该静态函数用于获取客户端可用的地图列表。首先创建一个boost::python::list类型的对象result用于存储最终要返回给 Python 的结果，以及一个std::vector<std::string>类型的maps用于临时存储从客户端获取到的地图名称字符串向量。
//在获取地图名称列表时，先通过carla::PythonUtil::ReleaseGIL释放全局解释器锁（GIL），这是为了在执行可能耗时的self.GetAvailableMaps()操作时，允许其他 Python 线程继续执行，避免阻塞整个 Python 解释器。然后将获取到的地图名称逐个添加到result列表中，最后返回这个列表，以便在 Python 环境中可以方便地访问可用地图的名称。
static auto GetRequiredFiles(const carla::client::Client &self, const std::string &folder, const bool download) {
  boost::python::list result;
  for (const auto &str : self.GetRequiredFiles(folder, download)) {
    result.append(str);
  }
  return result;
}
//此函数用于获取客户端在指定文件夹下所需的文件列表。同样先创建一个boost::python::list类型的result对象用于存储结果。
//它接受客户端对象引用、一个表示文件夹路径的字符串以及一个表示是否下载的布尔值作为参数。通过遍历客户端对象的GetRequiredFiles方法返回的文件名称字符串向量，将每个文件名称添加到result列表中，最后返回该列表，使得在 Python 环境中可以获取到这些文件信息。
static void ApplyBatchCommands(
    const carla::client::Client &self,
    const boost::python::object &commands,
    bool do_tick) {
  using CommandType = carla::rpc::Command;
  std::vector<CommandType> cmds{
    boost::python::stl_input_iterator<CommandType>(commands),
        boost::python::stl_input_iterator<CommandType>()};
  self.ApplyBatch(std::move(cmds), do_tick);
}
//这个函数用于向客户端应用一批命令。它接受客户端对象引用、一个boost::python::object类型的commands对象（应该是包含一系列命令的可迭代对象，具体类型可能通过后续的迭代器转换确定）以及一个表示是否执行tick操作的布尔值作为参数。
在函数内部，首先定义了一个CommandType类型别名（等同于carla::rpc::Command），然后通过boost::python::stl_input_iterator将commands对象转换为CommandType类型的向量cmds。最后调用客户端对象的ApplyBatch方法，将转换后的命令向量传递进去，并根据do_tick的值决定是否执行相关的tick操作。
static auto ApplyBatchCommandsSync(
    const carla::client::Client &self,
    const boost::python::object &commands,
    bool do_tick) {

  using CommandType = carla::rpc::Command;
  std::vector<CommandType> cmds {
    boost::python::stl_input_iterator<CommandType>(commands),
    boost::python::stl_input_iterator<CommandType>()
  };

  boost::python::list result;
  auto responses = self.ApplyBatchSync(cmds, do_tick);
  for (auto &response : responses) {
    result.append(std::move(response));
  }

  // check for autopilot command
  std::vector<carla::traffic_manager::ActorPtr> vehicles_to_enable(cmds.size(), nullptr);
  std::vector<carla::traffic_manager::ActorPtr> vehicles_to_disable(cmds.size(), nullptr);
  carla::client::World world = self.GetWorld();
  uint16_t tm_port = 8000;

  std::atomic<size_t> vehicles_to_enable_index;
  std::atomic<size_t> vehicles_to_disable_index;

  vehicles_to_enable_index.store(0);
  vehicles_to_disable_index.store(0);

  auto ProcessCommand = [&](size_t min_index, size_t max_index) {
    for (size_t i = min_index; i < max_index; ++i) {
      if (!responses[i].HasError()) {

        bool isAutopilot = false;
        bool autopilotValue = false;

        CommandType::CommandType& cmd_type = cmds[i].command;

        // check SpawnActor command
        if (const auto *maybe_spawn_actor_cmd = boost::variant2::get_if<carla::rpc::Command::SpawnActor>(&cmd_type)) {
          // check inside 'do_after'
          for (auto &cmd : maybe_spawn_actor_cmd->do_after) {
            if (const auto *maybe_set_autopilot_command = boost::variant2::get_if<carla::rpc::Command::SetAutopilot>(&cmd.command)) {
              tm_port = maybe_set_autopilot_command->tm_port;
              autopilotValue = maybe_set_autopilot_command->enabled;
              isAutopilot = true;
            }
          }
        }
        // check SetAutopilot command
        else if (const auto *maybe_set_autopilot_command = boost::variant2::get_if<carla::rpc::Command::SetAutopilot>(&cmd_type)) {
          tm_port = maybe_set_autopilot_command->tm_port;
          autopilotValue = maybe_set_autopilot_command->enabled;
          isAutopilot = true;
        }

        // check if found any SetAutopilot command
        if (isAutopilot) {
          // get the id
          carla::rpc::ActorId id = static_cast<carla::rpc::ActorId>(responses[i].Get());

          // get all actors
          carla::SharedPtr<carla::client::Actor> actor;
          actor = world.GetActor(id);

          // check to enable or disable
          if (actor) {
            if (autopilotValue) {
              size_t index = vehicles_to_enable_index.fetch_add(1);
              vehicles_to_enable[index] = actor;
            } else {
              size_t index = vehicles_to_disable_index.fetch_add(1);
              vehicles_to_disable[index] = actor;
            }
          }
        }
      }
    }
  };

  const size_t TaskLimit = 50;
  size_t num_commands = cmds.size();
  size_t num_batches = num_commands / TaskLimit;

  std::vector<std::thread*> t(num_batches+1);

  for(size_t n = 0; n < num_batches; n++) {
    t[n] = new std::thread(ProcessCommand, n * TaskLimit, (n+1) * TaskLimit);
  }
  t[num_batches] = new std::thread(ProcessCommand, num_batches * TaskLimit, num_commands);

  for(size_t n = 0; n <= num_batches; n++) {
    if(t[n]->joinable()){
      t[n]->join();
    }
    delete t[n];
  }

  // Fix vector size
  vehicles_to_enable.resize(vehicles_to_enable_index.load());
  vehicles_to_disable.resize(vehicles_to_disable_index.load());
  // Release memory
  vehicles_to_enable.shrink_to_fit();
  vehicles_to_disable.shrink_to_fit();

  // Ensure the TM always receives the same vector by sorting the elements
  std::vector<carla::traffic_manager::ActorPtr> sorted_vehicle_to_enable = vehicles_to_enable;
  std::sort(sorted_vehicle_to_enable.begin(), sorted_vehicle_to_enable.end(), [](carla::traffic_manager::ActorPtr &a, carla::traffic_manager::ActorPtr &b) {return a->GetId() < b->GetId(); });

  std::vector<carla::traffic_manager::ActorPtr> sorted_vehicle_to_disable = vehicles_to_disable;
  std::sort(sorted_vehicle_to_disable.begin(), sorted_vehicle_to_disable.end(), [](carla::traffic_manager::ActorPtr &a, carla::traffic_manager::ActorPtr &b) {return a->GetId() < b->GetId(); });

  // check if any autopilot command was sent
  if (sorted_vehicle_to_enable.size() || sorted_vehicle_to_disable.size()) {
    self.GetInstanceTM(tm_port).RegisterVehicles(sorted_vehicle_to_enable);
    self.GetInstanceTM(tm_port).UnregisterVehicles(sorted_vehicle_to_disable);
  }

  return result;
}
/*此函数与ApplyBatchCommands类似，但它是同步执行批量命令并进行一些额外的处理。
首先同样将boost::python::object类型的commands对象转换为CommandType类型的向量cmds，然后调用客户端的ApplyBatchSync方法获取命令执行的响应结果，并将这些结果逐个添加到boost::python::list类型的result对象中。
接下来，主要进行了与自动驾驶相关命令的处理：
创建了两个std::vector<carla::traffic_manager::ActorPtr>类型的向量vehicles_to_enable和vehicles_to_disable，用于分别存储要启用和禁用自动驾驶的车辆指针，初始大小设置为命令数量，并将所有元素初始化为nullptr。
通过定义一个ProcessCommand的 lambda 函数来处理每个命令的响应，在这个函数内部：
检查响应是否有错误，如果没有错误则进一步检查命令是否是SpawnActor或SetAutopilot类型且包含自动驾驶相关设置。
如果是相关命令且设置了自动驾驶，获取响应中的演员 ID，通过客户端获取对应的演员对象，然后根据自动驾驶的值（启用或禁用）将演员指针添加到相应的vehicles_to_enable或vehicles_to_disable向量中。
为了并行处理这些命令检查，将命令分成多个批次，每个批次最多处理TaskLimit个命令，创建相应数量的线程来并行执行ProcessCommand函数处理每个批次的命令。
在所有线程执行完毕后，根据实际添加到vehicles_to_enable和vehicles_to_disable向量中的元素数量调整向量大小，并进行内存释放操作（通过shrink_to_fit）。
最后，对要启用和禁用自动驾驶的车辆指针向量进行排序，确保按照演员 ID 从小到大的顺序排列，然后如果这两个向量中有元素，就通过客户端获取交通管理器实例，并分别注册要启用自动驾驶的车辆和注销要禁用自动驾驶的车辆。*/
void export_client() {
  using namespace boost::python;
  namespace cc = carla::client;
  namespace rpc = carla::rpc;

  class_<rpc::OpendriveGenerationParameters>("OpendriveGenerationParameters",
      init<double, double, double, double, bool, bool, bool>((arg("vertex_distance")=2.0, arg("max_road_length")=50.0, arg("wall_height")=1.0, arg("additional_width")=0.6, arg("smooth_junctions")=true, arg("enable_mesh_visibility")=true, arg("enable_pedestrian_navigation")=true)))
    .def_readwrite("vertex_distance", &rpc::OpendriveGenerationParameters::vertex_distance)
    .def_readwrite("max_road_length", &rpc::OpendriveGenerationParameters::max_road_length)
    .def_readwrite("wall_height", &rpc::OpendriveGenerationParameters::wall_height)
    .def_readwrite("additional_width", &rpc::OpendriveGenerationParameters::additional_width)
    .def_readwrite("smooth_junctions", &rpc::OpendriveGenerationParameters::smooth_junctions)
    .def_readwrite("enable_mesh_visibility", &rpc::OpendriveGenerationParameters::enable_mesh_visibility)
    .def_readwrite("enable_pedestrian_navigation", &rpc::OpendriveGenerationParameters::enable_pedestrian_navigation)
  ;

  class_<cc::Client>("Client",
      init<std::string, uint16_t, size_t>((arg("host")="127.0.0.1", arg("port")=2000, arg("worker_threads")=0u)))
    .def("set_timeout", &::SetTimeout, (arg("seconds")))
    .def("get_client_version", &cc::Client::GetClientVersion)
    .def("get_server_version", CONST_CALL_WITHOUT_GIL(cc::Client, GetServerVersion))
    .def("get_world", &cc::Client::GetWorld)
    .def("get_available_maps", &GetAvailableMaps)
    .def("set_files_base_folder", &cc::Client::SetFilesBaseFolder, (arg("path")))
    .def("get_required_files", &GetRequiredFiles, (arg("folder")="", arg("download")=true))
    .def("request_file", &cc::Client::RequestFile, (arg("name")))
    .def("reload_world", CONST_CALL_WITHOUT_GIL_1(cc::Client, ReloadWorld, bool), (arg("reset_settings")=true))
    .def("load_world", CONST_CALL_WITHOUT_GIL_3(cc::Client, LoadWorld, std::string, bool, rpc::MapLayer), (arg("map_name"), arg("reset_settings")=true, arg("map_layers")=rpc::MapLayer::All))
    .def("load_world_if_different", &cc::Client::LoadWorldIfDifferent, (arg("map_name"), arg("reset_settings")=true, arg("map_layers")=rpc::MapLayer::All))
    .def("generate_opendrive_world", CONST_CALL_WITHOUT_GIL_3(cc::Client, GenerateOpenDriveWorld, std::string,
        rpc::OpendriveGenerationParameters, bool), (arg("opendrive"), arg("parameters")=rpc::OpendriveGenerationParameters(),
        arg("reset_settings")=true))
    .def("start_recorder", CALL_WITHOUT_GIL_2(cc::Client, StartRecorder, std::string, bool), (arg("name"), arg("additional_data")=false))
    .def("stop_recorder", &cc::Client::StopRecorder)
    .def("show_recorder_file_info", CALL_WITHOUT_GIL_2(cc::Client, ShowRecorderFileInfo, std::string, bool), (arg("name"), arg("show_all")))
    .def("show_recorder_collisions", CALL_WITHOUT_GIL_3(cc::Client, ShowRecorderCollisions, std::string, char, char), (arg("name"), arg("type1"), arg("type2")))
    .def("show_recorder_actors_blocked", CALL_WITHOUT_GIL_3(cc::Client, ShowRecorderActorsBlocked, std::string, double, double), (arg("name"), arg("min_time"), arg("min_distance")))
    .def("replay_file", CALL_WITHOUT_GIL_5(cc::Client, ReplayFile, std::string, double, double, uint32_t, bool), (arg("name"), arg("time_start"), arg("duration"), arg("follow_id"), arg("replay_sensors")=false))
    .def("stop_replayer", &cc::Client::StopReplayer, (arg("keep_actors")))
    .def("set_replayer_time_factor", &cc::Client::SetReplayerTimeFactor, (arg("time_factor")))
    .def("set_replayer_ignore_hero", &cc::Client::SetReplayerIgnoreHero, (arg("ignore_hero")))
    .def("set_replayer_ignore_spectator", &cc::Client::SetReplayerIgnoreSpectator, (arg("ignore_spectator")))
    .def("apply_batch", &ApplyBatchCommands, (arg("commands"), arg("do_tick")=false))
    .def("apply_batch_sync", &ApplyBatchCommandsSync, (arg("commands"), arg("do_tick")=false))
    .def("get_trafficmanager", CONST_CALL_WITHOUT_GIL_1(cc::Client, GetInstanceTM, uint16_t), (arg("port")=ctm::TM_DEFAULT_PORT))
  ;
}
