// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/ListView.h>

#include <array>
#include <cstring>
#include <list>
#include <set>
#include <string>
#include <vector>

using carla::MakeListView;// 使用命名空间，简化对 Carla 中的 MakeListView 函数的调用

template <typename Iterator>
static void TestSequence(carla::ListView<Iterator> view) {
	
  int count = 0;
 // 遍历 ListView 中的元素
  for (auto &&x : view) {
  
    ASSERT_EQ(x, count);// 断言当前元素与计数器的值相等
   
    ++count;// 增加计数器的值
  }
  
  ASSERT_EQ(count, 6);
}

TEST(listview, sequence) {
  int array[] = {0, 1, 2, 3, 4, 5};
  
  TestSequence(MakeListView(array));// 使用 MakeListView 创建一个 ListView 对象，并传入整数数组进行测试
  
  std::array<int, 6u> std_array = {0, 1, 2, 3, 4, 5};
  
  TestSequence(MakeListView(std_array)); // 使用 MakeListView 创建一个 ListView 对象，并传入 std::array 容器进行测试
  
  std::vector<int> vector = {0, 1, 2, 3, 4, 5};
  
  TestSequence(MakeListView(vector));// 使用 MakeListView 创建一个 ListView 对象，并传入 std::vector 容器进行测试
  std::list<int> list = {0, 1, 2, 3, 4, 5};
  TestSequence(MakeListView(list));
  std::set<int> set = {0, 1, 2, 3, 4, 5};
  TestSequence(MakeListView(set));
}

TEST(listview, string) {
  std::string str = "Hello list view!";
  
  std::string result;
  
  
  for (char c : MakeListView(str)) {
    result += c;
  }
  ASSERT_EQ(result, str);
  char hello[6u] = {0};
  auto begin = std::begin(hello);
  // 遍历由字符串的一部分创建的 ListView 对象，将字符逐个复制到字符数组中
  for (char c : MakeListView(str.begin(), str.begin() + 5u)) {
    *begin = c;
    
    ++begin;
  }
  ASSERT_EQ(std::strcmp(hello, "Hello"), 0); // 断言字符数组中的内容与 "Hello" 相等
}
