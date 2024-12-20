// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Exception.h"//包含 Carla 项目中定义的异常类的头文件
#include "carla/MsgPack.h"//包含 Carla 项目中用于消息打包（可能是序列化）的头文件

#include <boost/optional.hpp>//包含 Boost.Optional 库的头文件

#ifdef _MSC_VER//这是一个条件编译指令，用于检查是否定义了宏 _MSC_VER
#pragma warning(push)//将当前的警告设置保存到一个堆栈中
#pragma warning(disable:4583)
#pragma warning(disable:4582)
#include <boost/variant2/variant.hpp>//包含 Boost.Variant2 库的头文件
#pragma warning(pop)//恢复之前保存的警告设置
#else
#include <boost/variant2/variant.hpp>
#endif

#include <tuple>

namespace clmdep_msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

  // ===========================================================================
  // -- Adaptors for boost::optional -------------------------------------------
  // ===========================================================================

  // 为 boost::optional<T> 定义的转换特化
  template<typename T>
  struct convert<boost::optional<T>> {
  
    // 重载函数调用运算符，将 MsgPack 对象转换为 boost::optional<T>
    const clmdep_msgpack::object &operator()(
        const clmdep_msgpack::object &o,
        boost::optional<T> &v) const {
      // 确保 MsgPack 对象是一个数组
      if (o.type != clmdep_msgpack::type::ARRAY) {
        ::carla::throw_exception(clmdep_msgpack::type_error());
      }
      
      // 检查数组大小以决定如何设置 optional
      if (o.via.array.size == 1) {
        // 如果大小为 1，表示没有值
        v.reset();
      } else if (o.via.array.size == 2) {
        // 如果大小为 2，从第二个元素中获取值
        v.reset(o.via.array.ptr[1].as<T>());
      } else {
        // 如果大小不为 1 或 2，抛出类型错误
        ::carla::throw_exception(clmdep_msgpack::type_error());
      }
      return o;
    }
  };
  // 为 boost::optional<T> 定义的打包特化
  template<typename T>
  struct pack<boost::optional<T>> {
    template <typename Stream>
    packer<Stream> &operator()(
        clmdep_msgpack::packer<Stream> &o,
        const boost::optional<T> &v) const {
      if (v.has_value()) {
        // 如果 optional 有值，将其打包为大小为 2 的数组
        o.pack_array(2);
        o.pack(true);// 第一个元素表示存在
        o.pack(*v);// 第二个元素是实际值
      } else {
        // 如果 optional 为空，将其打包为大小为 1 的数组
        o.pack_array(1);
        o.pack(false);// 表示值不存在
      }
      return o;
    }
  };
  // 为 boost::optional<T> 定义的带区域的对象特化
  template<typename T>
  struct object_with_zone<boost::optional<T>> {
    void operator()(
        clmdep_msgpack::object::with_zone &o,
        const boost::optional<T> &v) const {
      o.type = type::ARRAY; // 设置类型为数组
      if (v.has_value()) {
        // 如果 optional 有值，设置大小为 2 并分配内存
        o.via.array.size = 2;
        o.via.array.ptr = static_cast<clmdep_msgpack::object*>(o.zone.allocate_align(
            sizeof(clmdep_msgpack::object) * o.via.array.size,
            MSGPACK_ZONE_ALIGNOF(clmdep_msgpack::object)));
        // 第一个元素：true（表示存在）
        o.via.array.ptr[0] = clmdep_msgpack::object(true, o.zone);
         // 第二个元素：实际值
        o.via.array.ptr[1] = clmdep_msgpack::object(*v, o.zone);
      } else {
        // 如果 optional 为空，设置大小为 1 并分配内存
        o.via.array.size = 1;
        o.via.array.ptr = static_cast<clmdep_msgpack::object*>(o.zone.allocate_align(
            sizeof(clmdep_msgpack::object) * o.via.array.size,
            MSGPACK_ZONE_ALIGNOF(clmdep_msgpack::object)));
        // 第一个元素：false（表示不存在）
        o.via.array.ptr[0] = clmdep_msgpack::object(false, o.zone);
      }
    }
  };

  // ===========================================================================
  // -- Adaptors for boost::variant2::variant ----------------------------------
  // ===========================================================================

  template<typename... Ts>
  struct convert<boost::variant2::variant<Ts...>> {

    const clmdep_msgpack::object &operator()(
        const clmdep_msgpack::object &o,
        boost::variant2::variant<Ts...> &v) const {
      // 检查对象类型是否为数组
      if (o.type != clmdep_msgpack::type::ARRAY) {
        ::carla::throw_exception(clmdep_msgpack::type_error());
      }
      // 检查数组大小是否为 2
      if (o.via.array.size != 2) {
        ::carla::throw_exception(clmdep_msgpack::type_error());
      }
      // 获取索引
      const auto index = o.via.array.ptr[0].as<uint64_t>();
      copy_to_variant(index, o, v, std::make_index_sequence<sizeof...(Ts)>());
      return o;
    }

  private:

    // 从对象中复制到变体的实现
    template <uint64_t I>
    static void copy_to_variant_impl(
        const clmdep_msgpack::object &o,
        boost::variant2::variant<Ts...> &v) {
      /// @todo 找到类型的工作环绕。
      auto dummy = std::get<I>(std::tuple<Ts...>{});
      using T = decltype(dummy);
      v = o.via.array.ptr[1].as<T>();// 从对象中获取并赋值给变体
    }
    // 复制到变体的主函数
    template <uint64_t... Is>
    static void copy_to_variant(
        const uint64_t index,
        const clmdep_msgpack::object &o,
        boost::variant2::variant<Ts...> &v,
        std::index_sequence<Is...>) {
      std::initializer_list<int> ({
        (index == Is ? copy_to_variant_impl<Is>(o, v), 0 : 0)...
      });
    }
  };

  template<typename... Ts>
  struct pack<boost::variant2::variant<Ts...>> {
    template <typename Stream>
    packer<Stream> &operator()(
        clmdep_msgpack::packer<Stream> &o,
        const boost::variant2::variant<Ts...> &v) const {
      o.pack_array(2);// 打包数组大小
      o.pack(static_cast<uint64_t>(v.index()));
      // 使用访问器打包变体的值
      boost::variant2::visit([&](const auto &value) { o.pack(value); }, v);
      return o;
    }
  };

  template<typename... Ts>
  struct object_with_zone<boost::variant2::variant<Ts...>> {
    void operator()(
        clmdep_msgpack::object::with_zone &o,
        const boost::variant2::variant<Ts...> &v) const {
      o.type = type::ARRAY;
      o.via.array.size = 2;
      o.via.array.ptr = static_cast<clmdep_msgpack::object*>(o.zone.allocate_align(
          sizeof(clmdep_msgpack::object) * o.via.array.size,
          MSGPACK_ZONE_ALIGNOF(clmdep_msgpack::object)));
      // 设置数组的第一个元素为索引
      o.via.array.ptr[0] = clmdep_msgpack::object(static_cast<uint64_t>(v.index()), o.zone);
      boost::variant2::visit([&](const auto &value) {
        o.via.array.ptr[1] = clmdep_msgpack::object(value, o.zone);
      }, v);
    }
  };

} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
