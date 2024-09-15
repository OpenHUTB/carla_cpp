// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once  // ȷ��ͷ�ļ�ֻ������һ�Σ������ظ���������

#if defined(_MSC_VER)  // ����������� Microsoft Visual C++
#  define LIBCARLA_FORCEINLINE __forceinline  // ���� LIBCARLA_FORCEINLINE Ϊ __forceinline��ǿ������
#  define LIBCARLA_NOINLINE __declspec(noinline)  // ���� LIBCARLA_NOINLINE Ϊ __declspec(noinline)����ֹ����
#elif defined(__clang__) || defined(__GNUC__)  // ����������� Clang �� GCC
#  if defined(NDEBUG)  // ����ڷ���ģʽ�£�NDEBUG �Ѷ��壩
#    define LIBCARLA_FORCEINLINE inline __attribute__((always_inline))  // ���� LIBCARLA_FORCEINLINE Ϊ inline �� __attribute__((always_inline))��ǿ������
#  else
#    define LIBCARLA_FORCEINLINE inline  // ���� LIBCARLA_FORCEINLINE Ϊ inline���ڵ���ģʽ�£���ǿ��������
#  endif // NDEBUG
#  define LIBCARLA_NOINLINE __attribute__((noinline))  // ���� LIBCARLA_NOINLINE Ϊ __attribute__((noinline))����ֹ����
#else  // �������������֧��
#  warning Compiler not supported.  // �������棬��ʾ����������֧��
#  define LIBCARLA_NOINLINE  // ���� LIBCARLA_NOINLINE Ϊ�գ��޲��������Ա���δ������Ϊ
#endif

