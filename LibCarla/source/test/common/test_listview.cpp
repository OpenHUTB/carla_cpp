// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/ListView.h>
// 引入标准库中用于不同容器类型的头文件
#include <array>
#include <cstring>
#include <list>
#include <set>
#include <string>
#include <vector>
// 使用命名空间，简化对 Carla 库中 MakeListView 函数的调用
using carla::MakeListView;
// 定义一个模板函数，用于测试不同迭代器类型的序列容器
template <typename Iterator>
static void TestSequence(carla::ListView<Iterator> view) {
	// 初始化计数器
  int count = 0;
  // 遍历 ListView 中的元素
  for (auto &&x : view) {
  // 断言，检查当前元素的值是否与计数器的值相等
    ASSERT_EQ(x, count);
    // 增加计数器的值
    ++count;
  }
  // 断言，检查计数器是否等于 6，以确保 ListView 中有 6 个元素
  ASSERT_EQ(count, 6);
}
// 定义一个测试用例，用于测试 ListView 与不同序列容器的交互
TEST(listview, sequence) {
  int array[] = {0, 1, 2, 3, 4, 5};// 定义一个整数数组，并初始化包含 0 到 5 的整数
  TestSequence(MakeListView(array));// 使用 MakeListView 创建一个 ListView 对象，并传入整数数组进行测试
  std::array<int, 6u> std_array = {0, 1, 2, 3, 4, 5};// 定义一个 std::array 容器，并初始化包含 0 到 5 的整数
  TestSequence(MakeListView(std_array));// 使用 MakeListView 创建一个 ListView 对象，并传入 std::array 容器进行测试
  std::vector<int> vector = {0, 1, 2, 3, 4, 5};// 定义一个 std::vector 容器，并初始化包含 0 到 5 的整数
  TestSequence(MakeListView(vector));// 使用 MakeListView 创建一个 ListView 对象，并传入 std::vector 容器进行测试
  std::list<int> list = {0, 1, 2, 3, 4, 5};
  TestSequence(MakeListView(list));
  std::set<int> set = {0, 1, 2, 3, 4, 5};
  TestSequence(MakeListView(set));
}
// 定义另一个测试用例，用于测试 ListView 与字符串的交互
TEST(listview, string) {
  std::string str = "Hello list view!";// 定义一个 std::string 对象并初始化
  std::string result;// 定义一个空的 std::string 对象用于存储结果
  // 遍历 ListView 对象中的字符，ListView 对象由字符串创建
  for (char c : MakeListView(str)) {
    result += c;
  }
  ASSERT_EQ(result, str);
  char hello[6u] = {0};
  auto begin = std::begin(hello);
  // 遍历 ListView 对象中的字符，ListView 对象由字符串的一部分创建
  for (char c : MakeListView(str.begin(), str.begin() + 5u)) {
    *begin = c;// 将字符复制到字符数组中
    ++begin;// 移动字符数组的迭代器
  }
  ASSERT_EQ(std::strcmp(hello, "Hello"), 0);// 断言，检查字符数组是否等于 "Hello"
}
