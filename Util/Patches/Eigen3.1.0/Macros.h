// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2008-2010 Gael Guennebaud <gael.guennebaud@inria.fr>
// Copyright (C) 2006-2008 Benoit Jacob <jacob.benoit.1@gmail.com>
//
// Eigen is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// Alternatively, you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Eigen is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License or the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License and a copy of the GNU General Public License along with
// Eigen. If not, see <http://www.gnu.org/licenses/>.

#ifndef EIGEN_MACROS_H
#define EIGEN_MACROS_H

// 定义 Eigen 库的主版本号
#define EIGEN_WORLD_VERSION 3
// 定义 Eigen 库的次版本号
#define EIGEN_MAJOR_VERSION 1
// 定义 Eigen 库的修订版本号
#define EIGEN_MINOR_VERSION 0

// 检查 Eigen 库的版本是否至少为 (x, y, z)
#define EIGEN_VERSION_AT_LEAST(x,y,z) (EIGEN_WORLD_VERSION>x || (EIGEN_WORLD_VERSION>=x && \
                                      (EIGEN_MAJOR_VERSION>y || (EIGEN_MAJOR_VERSION>=y && \
                                                                 EIGEN_MINOR_VERSION>=z)))

// 检查是否使用的是 GCC 编译器，并且版本是否至少为 (x, y)
#ifdef __GNUC__
  #define EIGEN_GNUC_AT_LEAST(x,y) ((__GNUC__==x && __GNUC_MINOR__>=y) || __GNUC__>x)
#else
  #define EIGEN_GNUC_AT_LEAST(x,y) 0
#endif

// 检查是否使用的是 GCC 编译器，并且版本是否最多为 (x, y)
#ifdef __GNUC__
  #define EIGEN_GNUC_AT_MOST(x,y) ((__GNUC__==x && __GNUC_MINOR__<=y) || __GNUC__<x)
#else
  #define EIGEN_GNUC_AT_MOST(x,y) 0
#endif

// 根据 GCC 版本决定是否可以安全使用标准的断言宏
#if EIGEN_GNUC_AT_MOST(4,3) &&!defined(__clang__)
  // 参考 bug 89
  #define EIGEN_SAFE_TO_USE_STANDARD_ASSERT_MACRO 0
#else
  #define EIGEN_SAFE_TO_USE_STANDARD_ASSERT_MACRO 1
#endif

// 判断是否使用的是 GCC 3 或更旧的版本
#if defined(__GNUC__) && (__GNUC__ <= 3)
#define EIGEN_GCC3_OR_OLDER 1
#else
#define EIGEN_GCC3_OR_OLDER 0
#endif

// 16 字节对齐仅对向量化有用，根据编译器和架构决定是否启用 16 字节对齐
// 仅静态对齐可能会带来问题，因此在某些平台上会禁用它
#if defined(__GNUC__) &&!(defined(__i386__) || defined(__x86_64__) || defined(__powerpc__) || defined(__ppc__) || defined(__ia64__))
#define EIGEN_GCC_AND_ARCH_DOESNT_WANT_STACK_ALIGNMENT 1
#else
#define EIGEN_GCC_AND_ARCH_DOESNT_WANT_STACK_ALIGNMENT 0
#endif

// 根据架构和用户选择决定是否需要堆栈对齐
#if!EIGEN_GCC_AND_ARCH_DOESNT_WANT_STACK_ALIGNMENT \
 &&!EIGEN_GCC3_OR_OLDER \
 &&!defined(__SUNPRO_CC) \
 &&!defined(__QNXNTO__)
  #define EIGEN_ARCH_WANTS_STACK_ALIGNMENT 1
#else
  #define EIGEN_ARCH_WANTS_STACK_ALIGNMENT 0
#endif

// 根据用户选择是否禁用对齐，以及架构的需求，决定是否对齐
#ifdef EIGEN_DONT_ALIGN
  #ifndef EIGEN_DONT_ALIGN_STATICALLY
    #define EIGEN_DONT_ALIGN_STATICALLY
  #endif
  #define EIGEN_ALIGN 0
#else
  #define EIGEN_ALIGN 1
#endif

// EIGEN_ALIGN_STATICALLY 是最终决定是否在堆栈上对齐数组的标志，考虑了用户选择和架构需求
#if EIGEN_ARCH_WANTS_STACK_ALIGNMENT &&!defined(EIGEN_DONT_ALIGN_STATICALLY)
  #define EIGEN_ALIGN_STATICALLY 1
#else
  #define EIGEN_ALIGN_STATICALLY 0
  #ifndef EIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT
    #define EIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT
  #endif
#endif

// 根据用户选择或默认设置，决定矩阵的存储顺序
#ifdef EIGEN_DEFAULT_TO_ROW_MAJOR
#define EIGEN_DEFAULT_MATRIX_STORAGE_ORDER_OPTION RowMajor
#else
#define EIGEN_DEFAULT_MATRIX_STORAGE_ORDER_OPTION ColMajor
#endif

// 如果未定义，使用 std::ptrdiff_t 作为默认的稠密索引类型
#ifndef EIGEN_DEFAULT_DENSE_INDEX_TYPE
#define EIGEN_DEFAULT_DENSE_INDEX_TYPE std::ptrdiff_t
#endif

// 允许禁用一些可能影响结果精度的优化，默认启用，设置 EIGEN_FAST_MATH 为 0 可禁用
#ifndef EIGEN_FAST_MATH
#define EIGEN_FAST_MATH 1
#endif

// 调试时输出变量的值
#define EIGEN_DEBUG_VAR(x) std::cerr << #x << " = " << x << std::endl;

// 拼接两个标记
#define EIGEN_CAT2(a,b) a ## b
#define EIGEN_CAT(a,b) EIGEN_CAT2(a,b)

// 将标记转换为字符串
#define EIGEN_MAKESTRING2(a) #a
#define EIGEN_MAKESTRING(a) EIGEN_MAKESTRING2(a)

// 根据 GCC 版本决定是否使用 __attribute__((flatten)) 属性
#if EIGEN_GNUC_AT_LEAST(4,1) &&!defined(__clang__) &&!defined(__INTEL_COMPILER)
#define EIGEN_FLATTEN_ATTRIB __attribute__((flatten))
#else
#define EIGEN_FLATTEN_ATTRIB
#endif

// 更强的内联宏，在 MSVC 上使用 __forceinline，在其他编译器上使用 inline
#if (defined _MSC_VER) || (defined __INTEL_COMPILER)
#define EIGEN_STRONG_INLINE __forceinline
#else
#define EIGEN_STRONG_INLINE inline
#endif

// 最强的内联宏，使用 __attribute__((always_inline))，但在某些 GCC 版本中可能导致编译问题
#if EIGEN_GNUC_AT_LEAST(4,0)
#define EIGEN_ALWAYS_INLINE __attribute__((always_inline)) inline
#else
#define EIGEN_ALWAYS_INLINE EIGEN_STRONG_INLINE
#endif

// 不内联的宏，根据编译器不同而不同
#if (defined __GNUC__)
#define EIGEN_DONT_INLINE __attribute__((noinline))
#elif (defined _MSC_VER)
#define EIGEN_DONT_INLINE __declspec(noinline)
#else
#define EIGEN_DONT_INLINE
#endif

// 允许函数有多个定义，避免链接错误
#define EIGEN_DECLARE_FUNCTION_ALLOWING_MULTIPLE_DEFINITIONS
#define EIGEN_DEFINE_FUNCTION_ALLOWING_MULTIPLE_DEFINITIONS inline

// 根据是否定义了 NDEBUG，决定是否启用调试
#ifdef NDEBUG
# ifndef EIGEN_NO_DEBUG
#  define EIGEN_NO_DEBUG
# endif
#endif

// eigen_plain_assert 实现了对 GCC <= 4.3 中 assert 错误的解决方法
#ifdef EIGEN_NO_DEBUG
  #define eigen_plain_assert(x)
#else
  #if EIGEN_SAFE_TO_USE_STANDARD_ASSERT_MACRO
    namespace Eigen {
    namespace internal {
    // 复制布尔值的内联函数
    inline bool copy_bool(bool b) { return b; }
    }
    }
    #define eigen_plain_assert(x) assert(x)
  #else
    // 解决 bug 89 的问题
    #include <cstdlib>   // for abort
    #include <iostream>  // for std::cerr

    namespace Eigen {
    namespace internal {
    // 不内联的复制布尔值函数，参考 bug 89
    namespace {
    EIGEN_DONT_INLINE bool copy_bool(bool b) { return b; }
    }
    // 断言失败时输出错误信息并终止程序
    inline void assert_fail(const char *condition, const char *function, const char *file, int line)
    {
      std::cerr << "assertion failed: " << condition << " in function " << function << " at " << file << ":" << line << std::endl;
      abort();
    }
    }
    }
    #define eigen_plain_assert(x) \
      do { \
        if(!Eigen::internal::copy_bool(x)) \
          Eigen::internal::assert_fail(EIGEN_MAKESTRING(x), __PRETTY_FUNCTION__, __FILE__, __LINE__); \
      } while(false)
  #endif
#endif

// eigen_assert 可被重写，默认为 eigen_plain_assert
#ifndef eigen_assert
#define eigen_assert(x) eigen_plain_assert(x)
#endif

// 内部调试断言，可根据 EIGEN_INTERNAL_DEBUGGING 开关
#ifdef EIGEN_INTERNAL_DEBUGGING
#define eigen_internal_assert(x) eigen_assert(x)
#else
#define eigen_internal_assert(x)
#endif

// 根据是否启用调试，决定是否仅用于调试
#ifdef EIGEN_NO_DEBUG
#define EIGEN_ONLY_USED_FOR_DEBUG(x) (void)x
#else
#define EIGEN_ONLY_USED_FOR_DEBUG(x)
#endif

// 标记函数或变量已过时的宏
#ifndef EIGEN_NO_DEPRECATED_WARNING
  #if (defined __GNUC__)
    #define EIGEN_DEPRECATED __attribute__((deprecated))
  #elif (defined _MSC_VER)
    #define EIGEN_DEPRECATED __declspec(deprecated)
  #else
    #define EIGEN_DEPRECATED
  #endif
#else
  #define EIGEN_DEPRECATED
#endif

// 标记未使用的变量，避免编译器警告
#if (defined __GNUC__)
#define EIGEN_UNUSED __attribute__((unused))
#else
#define EIGEN_UNUSED
#endif

// 抑制未使用变量的警告
#define EIGEN_UNUSED_VARIABLE(var) (void)var;

// 编译器相关的汇编注释宏
#if!defined(EIGEN_ASM_COMMENT) && (defined __GNUC__)
#define EIGEN_ASM_COMMENT(X)  asm("#" X)
#else
#define EIGEN_ASM_COMMENT(X)
#endif

// 强制数据按 n 字节对齐，用于满足 SIMD 要求
#if (defined __GNUC__) || (defined __PGI) || (defined __IBMCPP__) || (defined __ARMCC_VERSION)
  #define EIGEN_ALIGN_TO_BOUNDARY(n) __attribute__((aligned(n)))
#elif (defined _MSC_VER)
  #define EIGEN_ALIGN_TO_BOUNDARY(n) __declspec(align(n))
#elif (defined __SUNPRO_CC)
  // 不确定此情况
  #define EIGEN_ALIGN_TO_BOUNDARY(n) __attribute__((aligned(n)))
#else
  #error Please tell me what is the equivalent of __attribute__((aligned(n))) for your compiler
#endif

#define EIGEN_ALIGN16 EIGEN_ALIGN_TO_BOUNDARY(16)

// 根据是否启用静态对齐，决定用户对齐的宏
#if EIGEN_ALIGN_STATICALLY
#define EIGEN_USER_ALIGN_TO_BOUNDARY(n) EIGEN_ALIGN_TO_BOUNDARY(n)
#define EIGEN_USER_ALIGN16 EIGEN_ALIGN16
#else
#define EIGEN_USER_ALIGN_TO_BOUNDARY(n)
#define EIGEN_USER_ALIGN16
#endif

// 处理 restrict 关键字，根据是否禁用使用不同的宏
#ifdef EIGEN_DONT_USE_RESTRICT_KEYWORD
  #define EIGEN_RESTRICT
#endif
#ifndef EIGEN_RESTRICT
  #define EIGEN_RESTRICT __restrict
#endif

// 堆栈分配的限制大小
#ifndef EIGEN_STACK_ALLOCATION_LIMIT
#define EIGEN_STACK_ALLOCATION_LIMIT 20000
#endif

// 默认的输入输出格式
#ifndef EIGEN_DEFAULT_IO_FORMAT
#ifdef EIGEN_MAKING_DOCS
// 文档中使用的格式
// 由于在 CMake 中添加定义时转义字符的问题，需要在此定义
#define EIGEN_DEFAULT_IO_FORMAT Eigen::IOFormat(3, 0, " ", "\n", "", "")
#else
#define EIGEN_DEFAULT_IO_FORMAT Eigen::IOFormat()
#endif
#endif

// 空宏
#define EIGEN_EMPTY

// 根据编译器版本决定如何继承赋值运算符
#if defined(_MSC_VER) && (_MSC_VER < 1900) && (!defined(__INTEL_COMPILER))
#define EIGEN_INHERIT_ASSIGNMENT_EQUAL_OPERATOR(Derived) \
  using Base::operator =;
#else
#define EIGEN_INHERIT_ASSIGNMENT_EQUAL_OPERATOR(Derived) \
  using Base::operator =; \
  EIGEN_STRONG_INLINE Derived& operator=(const Derived& other) \
  { \
    Base::operator=(other); \
    return *this; \
  }
#endif

#define EIGEN_INHERIT_ASSIGNMENT_OPERATORS(Derived) \
  EIGEN_INHERIT_ASSIGNMENT_EQUAL_OPERATOR(Derived)

// 通用的公共接口宏，包含一些常见的类型和枚举定义
#define EIGEN_GENERIC_PUBLIC_INTERFACE(Derived) \
  typedef typename Eigen::internal::traits<Derived>::Scalar Scalar; /*!< \brief Numeric type, e.g. float, double, int or std::complex<float>. */ \
  typedef typename Eigen::NumTraits<Scalar>::Real RealScalar; /*!< \brief The underlying numeric type for composed scalar types. \details In cases where Scalar is e.g. std::complex<T>, T were corresponding to RealScalar. */ \
  typedef typename Base::CoeffReturnType CoeffReturnType; /*!< \brief The return type for coefficient access. \details Depending on whether the object allows direct coefficient access (e.g. for a MatrixXd), this type is either 'const Scalar&' or simply 'Scalar' for objects that do not allow direct coefficient access. */ \
  typedef typename Eigen::internal::nested<Derived>::type Nested; \
  typedef typename Eigen::internal::traits<Derived>::StorageKind StorageKind; \
  typedef typename Eigen::internal::traits<Derived>::Index Index; \
  enum { RowsAtCompileTime = Eigen::internal::traits<Derived>::RowsAtCompileTime, \
        ColsAtCompileTime = Eigen::internal::traits<Derived>::ColsAtCompileTime, \
        Flags = Eigen::internal::traits<Derived>::Flags, \
        CoeffReadCost = Eigen::internal::traits<Derived>::CoeffReadCost, \
        SizeAtCompileTime = Base::SizeAtCompileTime, \
        MaxSizeAtCompileTime = Base::MaxSizeAtCompileTime, \
        IsVectorAtCompileTime = Base::IsVectorAtCompileTime };


// 稠密矩阵的公共接口宏，包含一些额外的类型和枚举定义
#define EIGEN_DENSE_PUBLIC_INTERFACE(Derived) \
  typedef typename Eigen::internal::traits<Derived>::Scalar Scalar; /*!< \brief Numeric type, e.g. float, double, int or std::complex<float>. */ \
  typedef typename Eigen::NumTraits<Scalar>::Real RealScalar; /*!< \brief The underlying numeric type for composed scalar types. \details In cases where Scalar is e.g. std::complex<T>, T were corresponding to RealScalar. */ \
  typedef typename Base::PacketScalar PacketScalar; \
  typedef typename Base::CoeffReturnType CoeffReturnType; /*!< \brief The return type for coefficient access. \details Depending on whether the object allows direct coefficient access (e.g. for a MatrixXd), this type is either 'const Scalar&' or simply 'Scalar' for objects that do not allow direct coefficient access. */ \
  typedef typename Eigen::internal::nested<Derived>::type Nested; \
  typedef typename Eigen::internal::traits<Derived>::StorageKind StorageKind; \
  typedef typename Eigen::internal::traits<Derived>::Index Index; \
  enum { RowsAtCompileTime = Eigen::internal::traits<Derived>::RowsAtCompileTime, \
        ColsAtCompileTime = Eigen::internal::traits<Derived>::ColsAtCompileTime, \
        MaxRowsAtCompileTime = Eigen::internal::traits<Derived>::MaxRowsAtCompileTime, \
        MaxColsAtCompileTime = Eigen::internal::traits<Derived>::MaxColsAtCompileTime, \
        Flags = Eigen::internal::traits<Derived>::Flags, \
        CoeffReadCost = Eigen::internal::traits<Derived>::CoeffReadCost, \
        SizeAtCompileTime = Base::SizeAtCompileTime, \
        MaxSizeAtCompileTime = Base::MaxSizeAtCompileTime, \
        IsVectorAtCompileTime = Base::IsVectorAtCompileTime }; \
  using Base::derived; \
  using Base::const_cast_derived;


// 计算两个枚举值的最小值，优先考虑 0 和 1
#define EIGEN_PLAIN_ENUM_MIN(a,b) (((int)a <= (int)b)? (int)a : (int)b)
#define EIGEN_PLAIN_ENUM_MAX(a,b) (((int)a >= (int)b)? (int)a : (int)b)

// 计算两个大小的最小值，优先考虑 0 和 1，动态大小优先于其他有限大小
#define EIGEN_SIZE_MIN_PREFER_DYNAMIC(a,b) (((int)a == 0 || (int)b == 0)? 0 \
                           : ((int)a == 1 || (int)b == 1)? 1 \
                           : ((int)a == Dynamic || (int)b == Dynamic)? Dynamic \
                           : ((int)a <= (int)b)? (int)a : (int)b)

// 计算两个最大大小的最小值，有限大小优先于动态大小
#define EIGEN_SIZE_MIN_PREFER_FIXED(a,b)  (((int)a == 0 || (int)b == 0)? 0 \
                           : ((int)a == 1 || (int)b == 1)? 1 \
                           : ((int)a == Dynamic && (int)b == Dynamic)? Dynamic \
                           : ((int)a == Dynamic)? (int)b \
                           : ((int)b == Dynamic)? (int)a \
                           : ((int)a <= (int)b)? (int)a : (int)b)

// 计算两个大小的最大值，动态大小优先
#define EIGEN_SIZE_MAX(a,b) (((int)a == Dynamic || (int)b == Dynamic)? Dynamic \
                           : ((int)a >= (int)b)? (int)a : (int)b)

// 逻辑异或操作
#define EIGEN_LOGICAL_XOR(a,b) (((a) || (b)) &&!((a) && (b)))

// 逻辑蕴含操作
#define EIGEN_IMPLIES(a,b) (!(a) || (b))

// 定义 Cwise 二元操作的宏
#define EIGEN_MAKE_CWISE_BINARY_OP(METHOD,FUNCTOR) \
  template<typename OtherDerived> \
  EIGEN_STRONG_INLINE const CwiseBinaryOp<FUNCTOR<Scalar>, const Derived, const OtherDerived> \
  (METHOD)(const EIGEN_CURRENT_STORAGE_BASE_CLASS<OtherDerived> &other) const \
  { \
    return CwiseBinaryOp<FUNCTOR<Scalar>, const Derived, const OtherDerived>(derived(), other.derived()); \
  }

// Cwise 乘积的表达式类型
#define EIGEN_CWISE_PRODUCT_RETURN_TYPE(LHS,RHS) \
    CwiseBinaryOp< \
      internal::scalar_product_op< \
          typename internal::traits<LHS>::Scalar, \
          typename internal::traits<RHS>::Scalar \
      >, \
      const LHS, \
      const RHS \
    >

#endif // EIGEN_MACROS_H
