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

using carla::MakeListView;// ʹ�������ռ䣬�򻯶� Carla �е� MakeListView �����ĵ���

template <typename Iterator>// ����һ��ģ�庯�������ڲ��Բ�ͬ���������͵�����
static void TestSequence(carla::ListView<Iterator> view) {
	
  int count = 0;
 // ���� ListView �е�Ԫ��
  for (auto &&x : view) {
  // ���Ե�ǰԪ�����������ֵ���
    ASSERT_EQ(x, count);{
   
    ++count;// ���Ӽ�������ֵ
  }
  
  ASSERT_EQ(count, 6);//���Լ�������ֵΪ 6��ȷ�� ListView ���� 6 ��Ԫ��
}
TEST(listview, sequence) {
  int array[] = {0, 1, 2, 3, 4, 5};
  
  TestSequence(MakeListView(array));// ʹ�� MakeListView ����һ�� ListView ���󣬲���������������в���
  
  std::array<int, 6u> std_array = {0, 1, 2, 3, 4, 5};
  
  TestSequence(MakeListView(std_array)); // ʹ�� MakeListView ����һ�� ListView ���󣬲����� std::array �������в���
  
  std::vector<int> vector = {0, 1, 2, 3, 4, 5};
  
  TestSequence(MakeListView(vector));// ʹ�� MakeListView ����һ�� ListView ���󣬲����� std::vector �������в���
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
  // ���Խ���ַ�����ԭʼ�ַ������ 
  ASSERT_EQ(result, str); 
  char hello[6u] = {0};
  auto begin = std::begin(hello);
  // �������ַ�����һ���ִ����� ListView ���󣬽��ַ�������Ƶ��ַ�������
  for (char c : MakeListView(str.begin(), str.begin() + 5u)) {
    *begin = c;
    
    ++begin;
  }
  ASSERT_EQ(std::strcmp(hello, "Hello"), 0); // �����ַ������е������� "Hello" ���
}
