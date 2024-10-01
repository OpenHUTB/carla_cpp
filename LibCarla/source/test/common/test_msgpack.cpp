// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
// 引入测试所需的头文件
#include "test.h"
#include <carla/MsgPackAdaptors.h>
#include <carla/rpc/Actor.h>
#include <carla/rpc/Response.h>
// 引入线程相关的头文件，可能在测试中用于模拟并发场景
#include <thread>
// 使用 carla::rpc 命名空间
using namespace carla::rpc;
// 测试 MsgPack 对 Response 的序列化和反序列化功能
TEST(msgpack, response) {
  using mp = carla::MsgPack;// 使用 MsgPack 的别名 mp
  const std::string error = "Uh ah an error!";// 定义一个错误字符串
  Response<int> r = ResponseError(error);// 创建一个包含错误信息的 Response<int> 对象
  auto s = mp::UnPack<decltype(r)>(mp::Pack(r));
  ASSERT_TRUE(r.HasError());
  ASSERT_EQ(r.GetError().What(), error);// 断言 r 的错误信息与预设的一致
  ASSERT_TRUE(s.HasError());
  ASSERT_EQ(s.GetError().What(), error);// 断言 s 的错误信息与预设的一致
  r.Reset(42);// 将 r 重置为一个值为 42 的 Response
  s = mp::UnPack<decltype(r)>(mp::Pack(r)); // 再次序列化和反序列化得到 s
  ASSERT_FALSE(r.HasError());
  ASSERT_EQ(r.Get(), 42);
  ASSERT_FALSE(s.HasError());
  ASSERT_EQ(s.Get(), 42);
  r.SetError(error);// 再次设置 r 为错误状态
  s = mp::UnPack<decltype(r)>(mp::Pack(r));
  ASSERT_FALSE(r);// 断言 r 为 false，表示有错误
  ASSERT_EQ(r.GetError().What(), error);
  ASSERT_FALSE(s);
  ASSERT_EQ(s.GetError().What(), error);
  Response<std::vector<float>> rv; // 创建一个 Response<std::vector<float>> 对象
  auto sv = mp::UnPack<decltype(rv)>(mp::Pack(rv)); // 序列化和反序列化得到 sv
  // 断言 rv 和 sv 都有错误
  ASSERT_TRUE(rv.HasError());
  ASSERT_TRUE(sv.HasError());
  Response<void> er;// 创建一个 Response<void> 对象 er
  ASSERT_TRUE(er.HasError());
  er = Response<void>::Success(); // 将 er 设置为成功状态
  auto es = mp::UnPack<decltype(er)>(mp::Pack(er));// 序列化和反序列化得到 es
   // 断言 er 和 es 都没有错误
  ASSERT_FALSE(er.HasError());
  ASSERT_FALSE(es.HasError());
  er.SetError(error);
  es = mp::UnPack<decltype(er)>(mp::Pack(er));
  // 断言 er 和 es 都为 false，表示有错误
  ASSERT_FALSE(er);
  ASSERT_EQ(er.GetError().What(), error);
  ASSERT_FALSE(es);
  ASSERT_EQ(es.GetError().What(), error);
}
// 测试 MsgPack 对 Actor 的序列化和反序列化功能
TEST(msgpack, actor) {
  namespace c = carla;
  namespace cg = carla::geom;
   // 创建一个 Actor 对象
  Actor actor;
  // 设置 Actor 的各种属性
  actor.id = 42u;
  actor.description.uid = 2u;
  actor.description.id = "actor.random.whatever";
  actor.bounding_box = cg::BoundingBox{cg::Vector3D{1.0f, 2.0f, 3.0f}};
// 序列化 actor，然后反序列化得到 result
  auto buffer = c::MsgPack::Pack(actor);
  auto result = c::MsgPack::UnPack<Actor>(buffer);
// 断言 result 的属性与 actor 的属性一致
  ASSERT_EQ(result.id, actor.id);
  ASSERT_EQ(result.description.uid, actor.description.uid);
  ASSERT_EQ(result.description.id, actor.description.id);
  ASSERT_EQ(result.bounding_box, actor.bounding_box);
}
// 测试 MsgPack 对 boost::variant 的序列化和反序列化功能
TEST(msgpack, variant) {
  using mp = carla::MsgPack;
// 创建一个 boost::variant2::variant<bool, float, std::string> 对象
  boost::variant2::variant<bool, float, std::string> var;
// 设置 var 为 bool 类型的值 true
  var = true;
  auto result = mp::UnPack<decltype(var)>(mp::Pack(var)); // 序列化和反序列化得到 result
  ASSERT_EQ(result.index(), 0); // 断言 result 的索引为 0，表示 bool 类型
  ASSERT_EQ(boost::variant2::get<bool>(result), true);// 断言 result 的 bool 值为 true

  var = 42.0f;
  result = mp::UnPack<decltype(var)>(mp::Pack(var));
  ASSERT_EQ(result.index(), 1); // 断言 result 的索引为 1，表示 float 类型
  ASSERT_EQ(boost::variant2::get<float>(result), 42.0f); // 断言 result 的 float 值为 42.0f
// 设置 var 为 std::string 类型的值 "hola!"
  var = std::string("hola!");
  result = mp::UnPack<decltype(var)>(mp::Pack(var));
  ASSERT_EQ(result.index(), 2);// 断言 result 的索引为 2，表示 std::string 类型
  ASSERT_EQ(boost::variant2::get<std::string>(result), "hola!");// 断言 result 的 std::string 值为 "hola!"
}
// 测试 MsgPack 对 boost::optional 的序列化和反序列化功能
TEST(msgpack, optional) {
  using mp = carla::MsgPack;
// 创建一个 boost::optional<float> 对象
  boost::optional<float> var;
// 序列化和反序列化得到 result
  auto result = mp::UnPack<decltype(var)>(mp::Pack(var));
  // 断言 result 没有值
  ASSERT_FALSE(result.has_value());
  // 设置 var 为 42.0f
  var = 42.0f;
  result = mp::UnPack<decltype(var)>(mp::Pack(var));
   // 断言 result 有值且值为 42.0f
  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(*result, 42.0f);
}
