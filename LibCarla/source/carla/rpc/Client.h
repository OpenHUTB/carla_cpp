// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// 这段注释表明代码的版权归属，即归属于巴塞罗那自治大学的计算机视觉中心（CVC），
// 同时说明该代码依据MIT许可协议进行授权使用，若要查看具体协议内容可通过给定网址访问。
// For a copy, see <https://opensource.org/licenses/MIT>.

// 头文件保护指令，可确保该头文件在同一个编译单元内只会被包含一次，
// 防止因重复包含而出现诸如类型重复定义等编译时错误。
#pragma once

// 包含 "carla/rpc/Metadata.h" 头文件，推测这个头文件中定义了和元数据（Metadata）相关的类型、函数等，
// 可能在后续的远程过程调用（RPC）操作中用于传递额外的描述信息、控制调用行为等。
#include "carla/rpc/Metadata.h"

// 包含 <rpc/client.h> 头文件，应该是引入了一个基础的RPC客户端相关的库，
// 提供了诸如建立连接、发送请求、接收响应等与远程服务交互的底层功能。
#include <rpc/client.h>

// 定义了名为carla的命名空间，用于将整个Carla项目相关的代码在逻辑上进行统一组织，
// 这样可以有效避免代码中的命名冲突，使代码结构更清晰，便于代码的维护和扩展。
namespace carla {
    // 在carla命名空间内，进一步定义了rpc子命名空间，
// 通常用来存放与远程过程调用（RPC）相关的各类自定义的类型、类定义以及函数等内容。
    namespace rpc {

        // 定义了一个名为Client的类，从类的功能来看，它主要是作为与远程服务进行交互的客户端角色，
        // 对底层的RPC客户端功能进行了一定的封装，以便在Carla项目中更方便地使用。
        class Client {
        public:
            // 这是一个模板构造函数，接受任意数量的参数（Args &&... args），
            // 通过使用 std::forward<Args>(args) 进行完美转发，将传入的参数原封不动地传递给成员变量 _client 的构造函数，
            // 避免了不必要的拷贝操作，保证了参数的右值引用特性得以保留（如果传入的是右值的话），从而提高效率。
            // 它的作用是初始化这个Client类的实例，同时初始化其内部的 _client 成员变量。
            template <typename... Args>
            explicit Client(Args &&... args)
                : _client(std::forward<Args>(args)...) {}

            // 该方法用于设置客户端与远程服务交互时的超时时间，
            // 它调用了底层的 ::rpc::client 对象（即成员变量 _client）的 set_timeout 方法来实现具体的设置操作，
            // 参数 value 就是要设置的超时时间值（类型为int64_t）。
            void set_timeout(int64_t value) {
                _client.set_timeout(value);
            }

            // 这个方法用于获取当前客户端与远程服务交互时设置的超时时间，
            // 通过调用底层 ::rpc::client 对象（_client）的 get_timeout 方法来获取超时时间值，
            // 并将获取到的值返回给调用者，返回值类型由底层的 get_timeout 方法决定。
            auto get_timeout() const {
                return _client.get_timeout();
            }

            // 这是一个模板函数，用于发起一个同步的远程过程调用（RPC）。
            // 它接受一个函数名（function，类型为const std::string &，表示对远程服务中某个具体函数的调用），
            // 以及任意数量的其他参数（Args &&... args），这些参数会被转发给底层的RPC调用机制。
            // 在调用底层 _client 的 call 方法时，除了传入函数名和转发的参数外，还传入了 Metadata::MakeSync()，
            // 推测是告知底层此次调用是同步的，并传递一些相关的元数据信息用于控制调用过程，
            // 最后返回远程调用的结果，结果类型由底层的RPC调用返回类型决定。
            template <typename... Args>
            auto call(const std::string &function, Args &&... args) {
                return _client.call(function, Metadata::MakeSync(), std::forward<Args>(args)...);
            }

            // 此方法用于发起一个异步的远程过程调用（RPC）。
            // 同样接受一个函数名（function，类型为const std::string &）以及任意数量的其他参数（Args &&... args），
            // 调用底层 _client 的 async_call 方法时，传入函数名、Metadata::MakeAsync()（推测是用于告知底层此次调用是异步的，同时传递相关元数据）
            // 和转发的参数，以此实现异步调用的发起，而该方法本身无返回值，因为异步调用结果通常需要通过其他方式（比如回调函数等）来获取。
            template <typename... Args>
            void async_call(const std::string &function, Args &&... args) {
                _client.async_call(function, Metadata::MakeAsync(), std::forward<Args>(args)...);
            }

        private:
            // 定义了一个私有成员变量 _client，类型为 ::rpc::client，
            // 它是底层实际用于和远程服务进行交互的RPC客户端对象，
            // 本类（Client）中的各种方法基本都是围绕对这个底层客户端对象的操作来实现与远程服务通信功能的。
            ::rpc::client _client;
        };

    } // namespace rpc
} // namespace carla

// 以下注释说明了这种客户端类设计的优势，它将底层RPC通信相关的复杂操作进行了封装，
// 使得客户端和远程服务之间的通信变得简单且高效，特别适合应用在像Carla这样的自动驾驶模拟器中，
// 方便模拟器不同模块之间通过RPC机制进行交互通信，例如控制模拟车辆、获取环境数据等场景下使用。
// This design makes the communication between the client and the remote service simple and efficient,
// and is suitable for use in an autonomous driving simulator like Carla.
