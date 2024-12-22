// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/multigpu/router.h"
#include "carla/multigpu/listener.h"
#include "carla/streaming/EndPoint.h"

namespace carla {
namespace multigpu {

// Router类的默认构造函数，初始化成员变量_next为0，可能用于后续标识下一个要处理的相关元素（比如连接等情况）
Router::Router(void) :
  _next(0) { }

// Router类的析构函数，用于在对象销毁时进行资源清理等操作
// 调用Stop函数来停止相关的监听、释放资源等操作
Router::~Router() {
  Stop();
}

// 停止路由器（Router）相关操作的函数，主要执行以下清理和停止操作：
// 1. 调用ClearSessions函数清除所有活动的会话。
// 2. 调用_listener的Stop函数停止监听器，防止接受新连接。
// 3. 通过_reset释放_listener对象的内存，避免内存泄漏等问题。
// 4. 调用_pool的Stop函数停止相关的线程池以释放其所占用的资源。
void Router::Stop() {
  ClearSessions();    // 清除所有活动的会话。
  _listener->Stop();  // 停止监听器，防止接受新连接
  _listener.reset();  // 释放监听器对象的内存。
  _pool.Stop();       // 停止相关的线程池以释放资源。
}

// Router类的构造函数，接受一个端口号（port）作为参数，用于初始化监听的端口相关设置等
// 参数port: 要监听的网络端口号，用于接收外部连接
// 首先初始化成员变量_next为0，然后创建一个TCP端点（_endpoint），使其监听所有网络接口（0.0.0.0）上的指定端口，
// 接着初始化_listener为指向Listener对象的共享指针，该Listener对象用于处理传入的连接，传入线程池的I/O上下文和创建好的端点信息。
Router::Router(uint16_t port) :
  _next(0) {

  // 创建一个TCP端点，监听所有网络接口（0.0.0.0）上的指定端口
  _endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string("0.0.0.0"), port);
  // 初始化_listener为指向Listener对象的共享指针，用于处理传入连接。
  _listener = std::make_shared<carla::multigpu::Listener>(_pool.io_context(), _endpoint);
}

// 设置路由器（Router）回调函数的函数，主要用于设置与连接相关的回调逻辑，包括连接打开、关闭以及收到响应时的处理逻辑等
// 具体操作如下：
// 1. 首先将当前Router对象的弱指针（weak）保存起来，方便在后续的回调函数中判断对象是否仍然有效（避免悬空指针等问题）。
// 2. 设置连接打开（on_open）回调函数，当有新的连接建立时（对应的是与Primary类型的会话连接），会调用ConnectSession函数来处理该新连接。
// 3. 设置连接关闭（on_close）回调函数，当会话连接关闭时，会调用DisconnectSession函数进行相应的清理等操作。
// 4. 设置收到响应（on_response）回调函数，当从辅助服务器收到数据时，会根据是否有对应的承诺（promise）来进行不同的处理，
//    如果有承诺，则将收到的数据设置到对应的承诺中（表示异步操作完成并返回结果），然后从承诺列表中移除该承诺；如果没有承诺，则仅记录收到数据的日志信息。
// 5. 将自身（Router对象）设置到_commander对象中（可能是用于后续的命令相关操作，通过共享指针共享自身给_commander使用）。
// 6. 调用_listener的Listen函数，传入上述设置好的回调函数，开始监听连接相关的事件，并记录开始监听的日志信息（包含监听的端点信息）。
void Router::SetCallbacks() {
  // 准备服务器
  std::weak_ptr<Router> weak = shared_from_this();

  carla::multigpu::Listener::callback_function_type on_open = [=](std::shared_ptr<carla::multigpu::Primary> session) {
    auto self = weak.lock();
    if (!self) return;
    self->ConnectSession(session);
  };

  carla::multigpu::Listener::callback_function_type on_close = [=](std::shared_ptr<carla::multigpu::Primary> session) {
    auto self = weak.lock();
    if (!self) return;
    self->DisconnectSession(session);
  };

  carla::multigpu::Listener::callback_function_type_response on_response =
    [=](std::shared_ptr<carla::multigpu::Primary> session, carla::Buffer buffer) {
      auto self = weak.lock();
      if (!self) return;
      std::lock_guard<std::mutex> lock(self->_mutex);
      auto prom =self-> _promises.find(session.get());
      if (prom!= self->_promises.end()) {
        log_info("Got data from secondary (with promise): ", buffer.size());
        prom->second->set_value({session, std::move(buffer)});
        self->_promises.erase(prom);
      } else {
        log_info("Got data from secondary (without promise): ", buffer.size());
      }
    };

  _commander.set_router(shared_from_this());

  _listener->Listen(on_open, on_close, on_response);
  log_info("Listening at ", _endpoint);
}

// 设置新连接回调函数的函数，外部可以传入一个函数对象（std::function<void(void)>类型），该函数会在有新连接建立时被调用
// 参数func: 外部传入的函数对象，无参数，无返回值，用于定义新连接建立时的自定义操作逻辑
void Router::SetNewConnectionCallback(std::function<void(void)> func)
{
  _callback = func;
}

// 异步运行路由器（Router）相关操作的函数，启动线程池以异步处理相关任务
// 参数worker_threads: 指定线程池中的工作线程数量，用于控制并发处理能力
void Router::AsyncRun(size_t worker_threads) {
  _pool.AsyncRun(worker_threads);
}

// 获取路由器（Router）本地监听端点信息的函数，返回其监听的TCP端点对象（包含IP地址和端口等信息）
boost::asio::ip::tcp::endpoint Router::GetLocalEndpoint() const {
  return _endpoint;
}

// 处理新连接建立的函数，将新的会话（Primary类型的共享指针）添加到活动会话列表（_sessions）中，并记录相关日志信息
// 参数session: 指向新建立的Primary类型会话的共享指针，表示新连接的会话对象
// 首先通过DEBUG_ASSERT断言传入的会话指针不为空，然后使用互斥锁（_mutex）保护共享资源（_sessions列表），
// 将传入的会话添加到_sessions列表末尾，记录当前连接的辅助服务器数量的日志信息，
// 最后如果设置了新连接的外部回调函数（_callback），则调用该回调函数执行自定义的新连接操作逻辑。
void Router::ConnectSession(std::shared_ptr<Primary> session) {
  DEBUG_ASSERT(session!= nullptr);
  std::lock_guard<std::mutex> lock(_mutex);
  _sessions.emplace_back(std::move(session));
  log_info("Connected secondary servers:", _sessions.size());
  // 对新连接运行外部回调
  if (_callback)
    _callback();
}

// 处理会话连接断开的函数，从活动会话列表（_sessions）中移除指定的会话，并记录相关日志信息
// 参数session: 指向要断开的Primary类型会话的共享指针，表示要移除的会话对象
// 同样先通过DEBUG_ASSERT断言传入的会话指针不为空，然后使用互斥锁（_mutex）保护共享资源（_sessions列表），
// 判断如果活动会话列表不为空，则通过std::remove和erase结合的方式从列表中移除指定的会话对象，
// 最后记录当前连接的辅助服务器数量的日志信息。
void Router::DisconnectSession(std::shared_ptr<Primary> session) {
  DEBUG_ASSERT(session!= nullptr);
  std::lock_guard<std::mutex> lock(_mutex);
  if (_sessions.size() == 0) return;
  _sessions.erase(
      std::remove(_sessions.begin(), _sessions.end(), session),
      _sessions.end());
  log_info("Connected secondary servers:", _sessions.size());
}

// 清除所有活动会话的函数，通过互斥锁（_mutex）保护共享资源（_sessions列表），直接清空_sessions列表，
// 并记录断开所有辅助服务器连接的日志信息，表示所有活动会话都已被清理。
void Router::ClearSessions() {
  std::lock_guard<std::mutex> lock(_mutex);
  _sessions.clear();
  log_info("Disconnecting all secondary servers");
}

// 向所有活动会话（辅助服务器）广播写入消息的函数，消息包含命令头和数据缓冲区两部分内容
// 参数id: 表示要发送的MultiGPUCommand类型的命令ID，用于标识消息的类型或用途。
// 参数buffer: 要发送的数据缓冲区（使用右值引用，避免不必要的拷贝），包含实际要发送的数据内容。
// 具体操作如下：
// 1. 首先创建一个命令头（CommandHeader）结构体对象header，设置其命令ID（id）和数据大小（buffer.size()），
//    然后将该命令头结构体转换为Buffer类型（buf_header），方便后续处理。
// 2. 通过BufferView将命令头和数据缓冲区分别创建视图（view_header和view_data），
//    再利用Primary的MakeMessage函数将这两个视图组合成一个完整的消息（message）。
// 3. 使用互斥锁（_mutex）保护共享资源（_sessions列表），遍历所有活动会话（_sessions），
//    对于每个不为空的会话对象，调用其Write函数将消息发送出去，实现向所有辅助服务器广播消息的功能。
void Router::Write(MultiGPUCommand id, Buffer &&buffer) {
  // 定义命令头
  CommandHeader header;
  header.id = id;
  header.size = buffer.size();
  Buffer buf_header((uint8_t *) &header, sizeof(header));

  auto view_header = carla::BufferView::CreateFrom(std::move(buf_header));
  auto view_data = carla::BufferView::CreateFrom(std::move(buffer));
  auto message = Primary::MakeMessage(view_header, view_data);

  // 写入多个服务器
  std::lock_guard<std::mutex> lock(_mutex);
  for (auto &s : _sessions) {
    if (s!= nullptr) {
      s->Write(message);
    }
  }
}

// 向特定的下一个活动会话（辅助服务器）写入消息，并返回一个表示异步操作结果的未来对象（std::future），用于获取后续的响应信息
// 参数id: 表示要发送的MultiGPUCommand类型的命令ID，用于标识消息的类型或用途。
// 参数buffer: 要发送的数据缓冲区（使用右值引用，避免不必要的拷贝），包含实际要发送的数据内容。
// 具体操作如下：
// 1. 类似Write函数，先创建命令头结构体对象header，设置相关属性后转换为Buffer类型（buf_header），
//    再通过BufferView创建命令头和数据的视图（view_header和view_data），并组合成完整消息（message）。
// 2. 创建一个共享指针指向的std::promise对象（response），用于后续异步操作完成时设置返回值（实现类似回调的功能）。
// 3. 使用互斥锁（_mutex）保护共享资源（_sessions列表），通过_next变量确定要发送的下一个会话索引，
//    如果索引超出会话列表大小，则重置为0；如果索引合法，则获取对应的会话对象（s），
//    如果会话对象不为空，则将当前创建的承诺（response）添加到_promises映射中（以会话对象的指针为键，方便后续根据会话查找对应的承诺），
//    然后调用该会话对象的Write函数发送消息，并递增_next变量指向下一个可能的会话索引。
// 4. 最后返回response的未来对象（response->get_future()），外部可以通过该未来对象获取异步操作的最终结果（比如等待响应并获取返回的数据等）。
std::future<SessionInfo> Router::WriteToNext(MultiGPUCommand id, Buffer &&buffer) {
  // 定义命令头
  CommandHeader header;
  header.id = id;
  header.size = buffer.size();
  Buffer buf_header((uint8_t *) &header, sizeof(header));

  auto view_header = carla::BufferView::CreateFrom(std::move(buf_header));
  auto view_data = carla::BufferView::CreateFrom(std::move(buffer));
  auto message = Primary::MakeMessage(view_header, view_data);

  // 为可能的答案创建承诺自动响应
  auto response = std::make_shared<std::promise<SessionInfo>>();

  // 只写特定服务器
  std::lock_guard<std::mutex> lock(_mutex);
  if (_next >= _sessions.size()) {
    _next = 0;
  }
  if (_next < _sessions.size()) {
    // std::cout << "Sending to session " << _next << std::endl;
    auto s = _sessions[_next];
    if (s!= nullptr) {
      _promises[s.get()] = response;
      std::cout << "Updated promise into map: " << _promises.size() << std::endl;
      s->Write(message);
    }
  }
  ++_next;
  return response->get_future();
}

// 向指定的活动会话（辅助服务器）写入消息，并返回一个表示异步操作结果的未来对象（std::future），用于获取后续的响应信息
// 参数server: 指向要发送消息的Primary类型会话的弱指针（std::weak_ptr），通过lock操作可获取对应的强指针来访问会话对象。
// 参数id: 表示要发送的MultiGPUCommand类型的命令ID，用于标识消息的类型或用途。
// 参数buffer: 要发送的数据缓冲区（使用右值引用，避免不必要的拷贝），包含实际要发送的数据内容。
// 具体操作如下：
// 1. 同样先创建命令头相关内容并组合成消息（与前面类似的步骤）。
// 2. 创建一个共享指针指向的std::promise对象（response）用于后续设置返回值。
// 3. 使用互斥锁（_mutex）保护共享资源，通过server弱指针获取对应的强指针（s），如果获取成功（表示会话对象有效），
//    则将当前创建的承诺（response）添加到_promises映射中，并调用该会话对象的Write函数发送消息。
// 4. 最后返回response的未来对象，供外部获取异步操作的结果。
std::future<SessionInfo> Router::WriteToOne(std::weak_ptr<Primary> server, MultiGPUCommand id, Buffer &&buffer) {
  // 定义命令头
  CommandHeader header;
  header.id = id;
  header.size = buffer.size();
  Buffer buf_header((uint8_t *) &header, sizeof(header));

  auto view_header = carla::BufferView::CreateFrom(std::move(buf_header));
  auto view_data = carla::BufferView::CreateFrom(std::move(buffer));
  auto message = Primary::MakeMessage(view_header, view_data);

  // 为可能的答案创建承诺自动响应

  auto response = std::make_shared<std::promise<SessionInfo>>();

  // 只写特定服务器

  std::lock_guard<std::mutex> lock(_mutex);
  auto s = server.lock();
  if (s) {
    _promises[s.get()] = response;
    s->Write(message);
  }
  return response->get_future();
}

// 获取下一个活动会话（辅助服务器）的弱指针（std::weak_ptr）的函数，通过_next变量来确定下一个要返回的会话索引，
// 如果索引超出会话列表大小，则重置为0；如果索引合法，则返回对应会话的弱指针，方便外部以弱引用的方式访问会话对象，
// 如果索引不合法（会话列表为空等情况），则返回一个空的弱指针。
std::weak_ptr<Primary> Router::GetNextServer() {
  std::lock_guard<std::mutex> lock(_mutex);
  if (_next >= _sessions.size()) {
    _next = 0;
  }
  if (_next < _sessions.size()) {
    return std::weak_ptr<Primary>(_sessions[_next]);
  } else {
    return std::weak_ptr<Primary>();
  }
}

} // 名称空间 multigpu
} // 名称空间 carla
