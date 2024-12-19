// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/detail/ActorState.h"
// 引入自定义的头文件 "carla/client/detail/ActorState.h"，推测这个头文件中包含了 ActorState 类的前置声明或者完整定义，以及相关依赖的类型、函数等的声明，是当前代码实现所依赖的基础。
#include <string>
#include <iterator>
// 引入C++标准库中的 <string> 和 <iterator> 头文件。
// <string> 头文件用于支持字符串相关的操作，比如创建、操作和转换字符串等，在代码中用于拼接显示用的字符串以及将整数转换为字符串等操作（通过 std::to_string 函数）。
// <iterator> 头文件提供了各种迭代器相关的定义和工具，在这里用于构造基于范围的迭代器来初始化 _attributes 成员变量（从描述信息中的属性范围进行初始化）。
namespace carla {
namespace client {
namespace detail {
// 定义了 carla::client::detail 嵌套命名空间，用于更细致地组织和封装与客户端相关的、可能属于内部实现细节的代码逻辑，将下面定义的 ActorState 类及其成员函数放在这个相对具体的命名空间下，避免与其他模块中的同名实体发生命名冲突，也使得代码结构更清晰，便于维护和理解。
  ActorState::ActorState(
      rpc::Actor description,
      EpisodeProxy episode)
    : _description(std::move(description)),
      _episode(std::move(episode)),
      _display_id([](const auto &desc) {
        using namespace std::string_literals;
        return
            "Actor "s +
            std::to_string(desc.id) +
            " (" + desc.description.id + ')';
      }(_description)),
      _attributes(_description.description.attributes.begin(), _description.description.attributes.end())
  {}// 这是 ActorState 类的构造函数定义，用于创建 ActorState 类的对象并进行初始化操作。
  // 它接受两个参数：
  //  - rpc::Actor 类型的 description，可能是一个包含了 Actor 详细描述信息的结构体或类对象（rpc 通常表示远程过程调用相关的类型，这里具体含义依赖于其定义所在的上下文，大概率是用于描述从远程获取的 Actor 相关信息），通过 std::move 进行右值引用传递，避免不必要的拷贝，将所有权转移给成员变量。
  //  - EpisodeProxy 类型的 episode，推测是与某个场景、情节相关的代理对象（用于在客户端与服务器端进行交互，获取或操作该情节下的相关资源等，具体功能取决于其自身的定义），同样通过 std::move 传递，转移所有权。
  // 在构造函数初始化列表中：
  //  - 将传入的 description 参数通过 std::move 移动赋值给私有成员变量 _description，用于保存 Actor 的详细描述信息。
  //  - 将传入的 episode 参数通过 std::move 移动赋值给私有成员变量 _episode，用于后续与该情节相关的操作（比如获取该情节下的其他资源等）。
  //  - 通过一个 lambda 表达式初始化私有成员变量 _display_id，这个 lambda 表达式接受一个参数 desc（类型由编译器自动推导，实际传入的是 _description），在表达式内部：
  //    - 首先使用 std::string_literals 命名空间（用于方便地创建字符串字面量，特别是在拼接字符串时更简洁，不需要额外的引号等处理）来创建一个以 "Actor " 开头的字符串字面量。
  //    - 然后使用 std::to_string 函数将 desc.id（即传入的描述信息中的 Actor 的 ID）转换为字符串，并与前面的字符串拼接起来。
  //    - 接着再拼接一个括号以及 desc.description.id（可能是描述信息中另一种形式的 ID 或者标识信息，具体依赖于 rpc::Actor 类型的定义），最终形成一个完整的用于显示的字符串，这个字符串用于标识该 Actor，方便在调试、日志记录或者界面展示等场景中使用。
  //  - 通过范围构造方式初始化私有成员变量 _attributes，使用 _description.description.attributes.begin() 和 _description.description.attributes.end() 作为迭代器范围，将描述信息中包含的属性信息（可能是一个属性列表、集合等形式，具体取决于其类型定义）提取出来，存储到 _attributes 中，以便后续对 Actor 的属性进行查询、操作等。

  SharedPtr<Actor> ActorState::GetParent() const {
    auto parent_id = GetParentId();
    return parent_id != 0u ? GetWorld().GetActor(parent_id) : nullptr;
  }
// 这是 ActorState 类的一个常成员函数 GetParent，用于获取当前 Actor 的父 Actor（如果存在的话），返回值是一个指向 Actor 类型对象的智能指针（SharedPtr<Actor>），通过智能指针管理对象的生命周期，避免内存泄漏等问题。
  // 函数内部首先调用 GetParentId 函数（该函数应该是在 ActorState 类或者其相关类中定义的，用于获取当前 Actor 的父 Actor 的 ID，返回值类型推测为一个无符号整数类型，比如 size_t 或者 unsigned int 等）获取父 Actor 的 ID，将其存储在 parent_id 变量中。
  // 然后通过一个三元表达式进行判断：如果 parent_id 不等于 0（表示存在有效的父 Actor ID），则调用 GetWorld().GetActor(parent_id) 尝试获取父 Actor 对象（GetWorld 函数应该是用于获取当前所处的世界、场景等相关上下文对象，然后调用其 GetActor 函数，传入父 Actor 的 ID，来获取对应的父 Actor 对象，返回的就是指向该父 Actor 的智能指针），并将获取到的智能指针返回；如果 parent_id 为 0，则返回 nullptr，表示当前 Actor 没有父 Actor。
} // namespace detail
} // namespace client
} // namespace carla
