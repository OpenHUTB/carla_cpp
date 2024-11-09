// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
/**
 * @file
 * @brief 包含Carla流处理模块中TCP通信相关类的头文件依赖。
 *
 * 此文件汇集了Carla流处理模块中TCP通信功能所需的头文件，包括不可复制类、时间处理、类型特征、
 * 性能分析、流处理细节类型、TCP消息类以及Boost.Asio的网络编程库。
 */

 /**
  * @brief 引入Carla的不可复制类定义。
  *
  * 此类用于确保对象不能被复制，通常用于管理唯一资源或状态的对象。
  */
#include "carla/NonCopyable.h"
  /**
   * @brief 引入Carla的时间处理类定义。
   *
   * 此类提供了时间点的表示、时间间隔的计算以及时间相关的功能。
   */
#include "carla/Time.h"
   /**
    * @brief 引入Carla的类型特征库。
    *
    * 该库提供了类型检查和转换的功能，用于在编译时进行类型安全性和一致性的检查。
    */
#include "carla/TypeTraits.h"
    /**
     * @brief 引入Carla的性能分析类定义。
     *
     * 此类用于分析对象的生命周期性能，帮助开发者识别性能瓶颈。
     */
#include "carla/profiler/LifetimeProfiled.h"
     /**
      * @brief 引入Carla流处理模块中的底层细节类型定义。
      *
      * 此文件定义了流处理模块中使用的底层类型，如流ID和消息大小类型。
      */
#include "carla/streaming/detail/Types.h"
      /**
       * @brief 引入Carla流处理模块中TCP消息类的定义。
       *
       * 此类用于表示TCP通信中传输的消息，包括消息头和消息体。
       */
#include "carla/streaming/detail/tcp/Message.h"
       /**
        * @brief Clang编译器的警告控制区域开始。
        *
        * 如果使用Clang编译器，则忽略"-Wshadow"警告，该警告会在变量名遮蔽时触发。
        */
#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wshadow"
#endif
        /**
         * @brief 引入Boost.Asio的deadline_timer类。
         *
         * 该类用于在指定的时间点或经过指定的时间间隔后触发一个回调。
         */
#include <boost/asio/deadline_timer.hpp>
         /**
          * @brief 引入Boost.Asio的io_context类。
          *
          * 该类提供了I/O服务的核心功能，包括异步操作的调度和执行。
          */
#include <boost/asio/io_context.hpp>
          /**
           * @brief 引入Boost.Asio的TCP协议类。
           *
           * 该类提供了TCP协议的网络编程接口，包括套接字和端点的管理。
           */
#include <boost/asio/ip/tcp.hpp>
           /**
            * @brief 引入Boost.Asio的strand类。
            *
            * 该类用于确保在单个I/O上下文（io_context）中异步操作的顺序执行。
            */
#include <boost/asio/strand.hpp>
            /**
             * @brief Clang编译器的警告控制区域结束。
             *
             * 如果使用Clang编译器，则恢复之前的警告设置。
             */
#if defined(__clang__)
#  pragma clang diagnostic pop
#endif
             /**
              * @brief 引入C++标准库中的functional头文件。
              *
              * 该头文件提供了函数对象、函数包装器以及标准函数适配器等功能。
              */
#include <functional>
              /**
               * @brief 引入C++标准库中的memory头文件。
               *
               * 该头文件提供了智能指针、动态内存分配和对象生命周期管理等功能。
               */
#include <memory>

namespace carla {
namespace streaming {
namespace detail {
namespace tcp {

  class Server;

  /// A TCP server session. When a session opens, it reads from the socket a
  /// stream id object and passes itself to the callback functor. The session
  /// closes itself after @a timeout of inactivity is met.
  class ServerSession
    : public std::enable_shared_from_this<ServerSession>,
      private profiler::LifetimeProfiled,
      private NonCopyable {
  public:

    using socket_type = boost::asio::ip::tcp::socket;
    using callback_function_type = std::function<void(std::shared_ptr<ServerSession>)>;

    explicit ServerSession(
        boost::asio::io_context &io_context,
        time_duration timeout,
        Server &server);

    /// Starts the session and calls @a on_opened after successfully reading the
    /// stream id, and @a on_closed once the session is closed.
    void Open(
        callback_function_type on_opened,
        callback_function_type on_closed);

    /// @warning This function should only be called after the session is
    /// opened. It is safe to call this function from within the @a callback.
    stream_id_type get_stream_id() const {
      return _stream_id;
    }

    template <typename... Buffers>
    static auto MakeMessage(Buffers... buffers) {
      static_assert(
          are_same<SharedBufferView, Buffers...>::value,
          "This function only accepts arguments of type BufferView.");
      return std::make_shared<const Message>(buffers...);
    }

    /// Writes some data to the socket.
    void Write(std::shared_ptr<const Message> message);

    /// Writes some data to the socket.
    template <typename... Buffers>
    void Write(Buffers... buffers) {
      Write(MakeMessage(buffers...));
    }

    /// Post a job to close the session.
    void Close();

  private:

    void StartTimer();

    void CloseNow(boost::system::error_code ec = boost::system::error_code());

    friend class Server;

    Server &_server;

    const size_t _session_id;

    stream_id_type _stream_id = 0u;

    socket_type _socket;

    time_duration _timeout;

    boost::asio::deadline_timer _deadline;

    boost::asio::io_context::strand _strand;

    callback_function_type _on_closed;

    bool _is_writing = false;
  };

} // namespace tcp
} // namespace detail
} // namespace streaming
} // namespace carla
