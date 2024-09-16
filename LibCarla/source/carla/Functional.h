// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // ��ֹͷ�ļ�����ΰ���

#include <utility>  // ������׼�������ڴ�������ת����ͷ�ļ�

namespace carla {

  class Functional {
  public:

      /// ����һ���ݹ���ö��󣬽�������Ϊ��һ���������ݸ� @a func��
      /// �÷�ʾ���������ݹ� lambda
    template <typename FuncT>
    static auto MakeRecursive(FuncT &&func) {
      return Recursive<FuncT>(std::forward<FuncT>(func));
    }

    /// ����һ�������ص��ö��󡱣���һ�������ɵ��ö�����ɣ�
    /// ÿ���ɵ��ö��󽫹���һ�� operator() �����ء�
    /// ����������� lambda ��ϳ�һ�� lambda��
    template <typename... FuncTs>
    static auto MakeOverload(FuncTs &&... fs) {
      return Overload<FuncTs...>(std::forward<FuncTs>(fs)...);
    }

    /// �����ݹ����ص��ö������ MakeRecursive �� MakeOverload �Ĺ��ܡ�
    /// @see MakeRecursive �� MakeOverload��
    template <typename... FuncTs>
    static auto MakeRecursiveOverload(FuncTs &&... fs) {
      return MakeRecursive(MakeOverload(std::forward<FuncTs>(fs)...));
    }

  private:

    // ����һ��ģ��ṹ�� Overload��������϶���ɵ��ö���
    template <typename... Ts>
    struct Overload;

    template <typename T, typename... Ts>
    struct Overload<T, Ts...> : T, Overload<Ts...> {
      Overload(T &&func, Ts &&... rest)
        : T(std::forward<T>(func)),
          Overload<Ts...>(std::forward<Ts>(rest)...) {}

      using T::operator();

      using Overload<Ts...>::operator();
    };

    template <typename T>
    struct Overload<T> : T {
      Overload(T &&func) : T(std::forward<T>(func)) {}

      using T::operator();
    };

    template<typename T>
    struct Recursive {

      explicit Recursive(T &&func) : _func(std::forward<T>(func)) {}

      template<typename... Ts>
      auto operator()(Ts &&... arguments) const {
        return _func(*this, std::forward<Ts>(arguments)...);
      }

    private:

      T _func;
    };

  };

} // namespace carla
