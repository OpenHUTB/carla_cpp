// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.
 
#pragma once
 
#include "carla/AtomicSharedPtr.h"
#include "carla/NonCopyable.h"
 
#include <algorithm>
#include <mutex>
#include <vector>
 
namespace carla {
namespace client {
namespace detail {
 
  /// ����һ��ָ���б��ԭ��ָ�롣
  ///
  /// @warning �� Load ������ԭ�ӵģ����б���޸��ɻ�����������
  template <typename T>
  class AtomicList : private NonCopyable {
    using ListT = std::vector<T>;// �б����Ͷ���Ϊ std::vector<T>���洢Ԫ�� T
  public:
 
    AtomicList() : _list(std::make_shared<ListT>()) {}// ��ʼ���б�Ϊһ���յĹ���ָ��
 
    template <typename ValueT>
    void Push(ValueT &&value) {
      std::lock_guard<std::mutex> lock(_mutex);// �����������Ա�֤�̰߳�ȫ
      auto new_list = std::make_shared<ListT>(*Load());// ���Ƶ�ǰ�б�����һ�����б�
      new_list->emplace_back(std::forward<ValueT>(value)); // ����ֵ��ӵ����б��ĩβ
      _list = new_list;// ����ԭ��ָ��ָ�����б�
    }
 
    void DeleteByIndex(size_t index) {
      std::lock_guard<std::mutex> lock(_mutex);// �����������Ա�֤�̰߳�ȫ
      auto new_list = std::make_shared<ListT>(*Load());// ���Ƶ�ǰ�б�����һ�����б�
      auto begin = new_list->begin();// ��ȡ�б����ʼ������
      std::advance(begin, index);
      new_list->erase(begin);
      _list = new_list;// ����ԭ��ָ��ָ�����б�
    }
 
    template <typename ValueT>
    void DeleteByValue(const ValueT &value) {
      std::lock_guard<std::mutex> lock(_mutex);
      auto new_list = std::make_shared<ListT>(*Load());  // ʹ�� std::remove �ƶ����е��� value ��Ԫ�ص��б�ĩβ��Ȼ����� erase ɾ����ЩԪ��
      new_list->erase(std::remove(new_list->begin(), new_list->end(), value), new_list->end());
      _list = new_list;
    }
 
    void Clear() {
      std::lock_guard<std::mutex> lock(_mutex);
      _list = std::make_shared<ListT>();
    }
 
      /// ����ָ���б��ָ�롣
    std::shared_ptr<const ListT> Load() const {
      return _list.load();
    }
 
  private:
 
    std::mutex _mutex;// �����������ڱ������б���޸�
 
    AtomicSharedPtr<const ListT> _list;// ԭ�ӹ���ָ�룬ָ��ǰ�б�
  };
 
} // namespace detail
} // namespace client
} // namespace carla
