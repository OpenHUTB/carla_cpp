// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/NonCopyable.h" // ����ǿ�����������ͷ�ļ�

#ifdef LIBCARLA_WITH_PYTHON_SUPPORT
#  if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wdeprecated-register"
#  endif
#    include <boost/python.hpp> // ����Boost Python���ͷ�ļ�
#  if defined(__clang__)
#    pragma clang diagnostic pop
#  endif
#endif // LIBCARLA_WITH_PYTHON_SUPPORT

namespace carla {

  class PythonUtil {
  public:

    static bool ThisThreadHasTheGIL() {
#ifdef LIBCARLA_WITH_PYTHON_SUPPORT
#  if PY_MAJOR_VERSION >= 3
      return PyGILState_Check(); // Python 3: ��鵱ǰ�߳��Ƿ����GIL
#  else
      PyThreadState *tstate = _PyThreadState_Current;
      return (tstate != nullptr) && (tstate == PyGILState_GetThisThreadState()); // Python 2: ��鵱ǰ�߳��Ƿ����GIL
#  endif // PYTHON3
#else
      return false; // ���û��Python֧�֣�����false
#endif // LIBCARLA_WITH_PYTHON_SUPPORT
    }

#ifdef LIBCARLA_WITH_PYTHON_SUPPORT

    /// ��ȡPython��ȫ�ֽ������� (GIL)������ȷ���̰߳�ȫ
    class AcquireGIL : private NonCopyable {
    public:

      AcquireGIL() : _state(PyGILState_Ensure()) {} // ���캯����ȷ����ǰ�̳߳���GIL

      ~AcquireGIL() {
        PyGILState_Release(_state); // �����������ͷ�GIL
      }

    private:

      PyGILState_STATE _state; // �洢GIL״̬
    };

    /// �ͷ�Python��ȫ�ֽ������� (GIL)���������������߳�ִ��Python����
    class ReleaseGIL : private NonCopyable {
    public:

      ReleaseGIL() : _state(PyEval_SaveThread()) {} // ���캯���б��浱ǰ�߳�״̬���ͷ�GIL

      ~ReleaseGIL() {
        PyEval_RestoreThread(_state); // ���������лָ��߳�״̬
      }

    private:

      PyThreadState *_state; // �洢�߳�״̬
    };

#else // LIBCARLA_WITH_PYTHON_SUPPORT

    class AcquireGIL : private NonCopyable {}; // û��Python֧��ʱ����Ϊ��
    class ReleaseGIL : private NonCopyable {}; // û��Python֧��ʱ����Ϊ��

#endif // LIBCARLA_WITH_PYTHON_SUPPORT

    /// ɾ��Python����ʱ���Զ�����GIL״̬
    class AcquireGILDeleter {
    public:

      template <typename T>
      void operator()(T *ptr) const {
#ifdef LIBCARLA_WITH_PYTHON_SUPPORT
        if (ptr != nullptr && !PythonUtil::ThisThreadHasTheGIL()) {
          AcquireGIL lock; // �����ǰ�߳�δ����GIL�����ȡGIL
          delete ptr; // ɾ������
        } else
#endif // LIBCARLA_WITH_PYTHON_SUPPORT
        delete ptr; // ɾ������
      }
    };

    /// ɾ��Python����ʱ���Զ�����GIL״̬
    class ReleaseGILDeleter {
    public:

      template <typename T>
      void operator()(T *ptr) const {
#ifdef LIBCARLA_WITH_PYTHON_SUPPORT
        if (ptr != nullptr && PythonUtil::ThisThreadHasTheGIL()) {
          ReleaseGIL lock; // �����ǰ�̳߳���GIL�����ͷ�GIL
          delete ptr; // ɾ������
        } else
#endif // LIBCARLA_WITH_PYTHON_SUPPORT
        delete ptr; // ɾ������
      }
    };
  };

} // namespace carla
