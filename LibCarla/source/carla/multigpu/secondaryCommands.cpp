// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// #include "carla/Logging.h"
#include "carla/multigpu/secondaryCommands.h"
// #include "carla/streaming/detail/tcp/Message.h"

namespace carla {
namespace multigpu {

void SecondaryCommands::set_secondary(std::shared_ptr<Secondary> secondary) {
  _secondary = secondary;  
}

void SecondaryCommands::set_callback(callback_type callback) {
  _callback = callback;
}

void SecondaryCommands::process_command(Buffer buffer) {
  // 获取标题
  CommandHeader *header;
  header = reinterpret_cast<CommandHeader *>(buffer.data());
  
   //只向回调发送数据
  Buffer data(buffer.data() + sizeof(CommandHeader), header->size);
  _callback(header->id, std::move(data));

  // log_info("Secondary got a command to process");
}


} //名称空间multigpu
}//命名空间Carla
