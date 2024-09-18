// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/ListView.h>
// �����׼�������ڲ�ͬ�������͵�ͷ�ļ�
#include <array>
#include <cstring>
#include <list>
#include <set>
#include <string>
#include <vector>
// ʹ�������ռ䣬�򻯶� Carla ���� MakeListView �����ĵ���
using carla::MakeListView;
// ����һ��ģ�庯�������ڲ��Բ�ͬ���������͵���������
template <typename Iterator>
static void TestSequence(carla::ListView<Iterator> view) {
	// ��ʼ��������
  int count = 0;
  // ���� ListView �е�Ԫ��
  for (auto &&x : view) {
  // ���ԣ���鵱ǰԪ�ص�ֵ�Ƿ����������ֵ���
    ASSERT_EQ(x, count);
    // ���Ӽ�������ֵ
    ++count;
  }
  // ���ԣ����������Ƿ���� 6����ȷ�� ListView ���� 6 ��Ԫ��
  ASSERT_EQ(count, 6);
}
// ����һ���������������ڲ��� ListView �벻ͬ���������Ľ���
TEST(listview, sequence) {
  int array[] = {0, 1, 2, 3, 4, 5};// ����һ���������飬����ʼ������ 0 �� 5 ������
  TestSequence(MakeListView(array));// ʹ�� MakeListView ����һ�� ListView ���󣬲���������������в���
  std::array<int, 6u> std_array = {0, 1, 2, 3, 4, 5};// ����һ�� std::array ����������ʼ������ 0 �� 5 ������
  TestSequence(MakeListView(std_array));// ʹ�� MakeListView ����һ�� ListView ���󣬲����� std::array �������в���
  std::vector<int> vector = {0, 1, 2, 3, 4, 5};// ����һ�� std::vector ����������ʼ������ 0 �� 5 ������
  TestSequence(MakeListView(vector));// ʹ�� MakeListView ����һ�� ListView ���󣬲����� std::vector �������в���
  std::list<int> list = {0, 1, 2, 3, 4, 5};
  TestSequence(MakeListView(list));
  std::set<int> set = {0, 1, 2, 3, 4, 5};
  TestSequence(MakeListView(set));
}
// ������һ���������������ڲ��� ListView ���ַ����Ľ���
TEST(listview, string) {
  std::string str = "Hello list view!";// ����һ�� std::string ���󲢳�ʼ��
  std::string result;// ����һ���յ� std::string �������ڴ洢���
  // ���� ListView �����е��ַ���ListView �������ַ�������
  for (char c : MakeListView(str)) {
    result += c;
  }
  ASSERT_EQ(result, str);
  char hello[6u] = {0};
  auto begin = std::begin(hello);
  // ���� ListView �����е��ַ���ListView �������ַ�����һ���ִ���
  for (char c : MakeListView(str.begin(), str.begin() + 5u)) {
    *begin = c;// ���ַ����Ƶ��ַ�������
    ++begin;// �ƶ��ַ�����ĵ�����
  }
  ASSERT_EQ(std::strcmp(hello, "Hello"), 0);// ���ԣ�����ַ������Ƿ���� "Hello"
}
