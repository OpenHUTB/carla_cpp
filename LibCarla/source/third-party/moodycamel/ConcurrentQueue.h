// 提供多生产者、多消费者无锁队列的 C++ 11实现。
// 这里提供了一个概述，包括基准测试结果:
//     http://moodycamel.com/blog/2014/a-fast-general-purpose-lock-free-queue-for-c++
// 完整的设计也有详细的描述：
//    http://moodycamel.com/blog/2014/detailed-design-of-a-lock-free-queue

//简化的 BSD 许可证：
// 版权所有 （c） 2013-2016，Cameron Desrochers。
// 保留所有权利。
//
// 允许源代码和二进制形式的再分发和使用，无论是否经过修改，只要满足以下条件：
//
// 源代码的再分发必须保留上述版权声明、条件列表和免责声明。
// 二进制形式的再分发必须在文档和/或其他材料中再现上述版权声明、条件列表和免责声明。
//
//该软件由版权持有者和贡献者“按原样”提供，没有任何明示或暗示的担保，包括但不限于适销性和特定用途适用性的暗示担保
//在任何情况下，版权持有者或贡献者不对因使用该软件而产生的任何直接、间接、附带、特别、惩罚性或结果性的损害负责，
//包括但不限于采购替代商品或服务、使用、数据或利润的损失或业务中断，是否基于合同、严格责任或侵权（包括疏忽或其他）理论，即使已经被告知可能发生这样的损害。
// 注意：这个文件为了被 CARLA 使用做了略微的修改。

#pragma once

#if defined(__GNUC__)
// 禁用 -Wconversion 警告（当 Traits::size_t 和 Traits::index_t 设置为小于 32 位时，整数提升可能引发这些警告
// 在赋值计算值时会出现警告）
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"

#ifdef MCDBGQ_USE_RELACY
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#endif
#endif

#if defined(__APPLE__)
#include "TargetConditionals.h"
#endif

#ifdef MCDBGQ_USE_RELACY
#include "relacy/relacy_std.hpp"
#include "relacy_shims.h"
// 我们只使用 malloc/free，因此 delete 宏会干扰 `= delete` 方法声明。
// 我们将自己覆盖默认的 trait malloc，而不使用宏。
#undef new
#undef delete
#undef malloc
#undef free
#else
#include <atomic>    // Requires C++11. Sorry VS2010.
#include <cassert>
#endif
#include <cstddef>              // for max_align_t
#include <cstdint>
#include <cstdlib>
#include <type_traits>
#include <algorithm>
#include <utility>
#include <limits>
#include <climits>    // for CHAR_BIT
#include <array>
#include <thread>    // 部分用于 __WINPTHREADS_VERSION如果在 MinGW-w64 上带有 POSIX 线程

// 平台特定的数字线程 ID 类型和无效值定义
namespace moodycamel { namespace details {
  template<typename thread_id_t> struct thread_id_converter {
    typedef thread_id_t thread_id_numeric_size_t;
    typedef thread_id_t thread_id_hash_t;
    static thread_id_hash_t prehash(thread_id_t const& x) { return x; }
  };
} }
#if defined(MCDBGQ_USE_RELACY)
namespace moodycamel { namespace details {
  typedef std::uint32_t thread_id_t;
  static const thread_id_t invalid_thread_id  = 0xFFFFFFFFU;
  static const thread_id_t invalid_thread_id2 = 0xFFFFFFFEU;
  static inline thread_id_t thread_id() { return rl::thread_index(); }
} }
#elif defined(_WIN32) || defined(__WINDOWS__) || defined(__WIN32__)
// 在头文件中引入 windows.h 没有意义，我们将手动声明所用的函数
// 并依赖向后兼容性确保这不会破坏
extern "C" __declspec(dllimport) unsigned long __stdcall GetCurrentThreadId(void);
namespace moodycamel { namespace details {
  static_assert(sizeof(unsigned long) == sizeof(std::uint32_t), "Expected size of unsigned long to be 32 bits on Windows");
  typedef std::uint32_t thread_id_t;
  static const thread_id_t invalid_thread_id  = 0;      //查看 http://blogs.msdn.com/b/oldnewthing/archive/2004/02/23/78395.aspx
  static const thread_id_t invalid_thread_id2 = 0xFFFFFFFFU;  // 在技术上不能保证无效，但在实践中从未使用过。请注意，所有 Win32 线程 ID 目前都是 4 的倍数。
  static inline thread_id_t thread_id() { return static_cast<thread_id_t>(::GetCurrentThreadId()); }
} }
#elif defined(__arm__) || defined(_M_ARM) || defined(__aarch64__) || (defined(__APPLE__) && TARGET_OS_IPHONE)
namespace moodycamel { namespace details {
  static_assert(sizeof(std::thread::id) == 4 || sizeof(std::thread::id) == 8, "std::thread::id is expected to be either 4 or 8 bytes");

  typedef std::thread::id thread_id_t;
  static const thread_id_t invalid_thread_id;         // 默认 ctor 创建无效 ID

  // 请注意，我们不定义 invalid_thread_id2，因为 std::thread::id 没有无效值；它
  // 仅在 MOODYCAMEL_CPP11_THREAD_LOCAL_SUPPORTED 定义时才会使用，但实际上不会定义它。
  static inline thread_id_t thread_id() { return std::this_thread::get_id(); }

  template<std::size_t> struct thread_id_size { };
  template<> struct thread_id_size<4> { typedef std::uint32_t numeric_t; };
  template<> struct thread_id_size<8> { typedef std::uint64_t numeric_t; };

  template<> struct thread_id_converter<thread_id_t> {
    typedef thread_id_size<sizeof(thread_id_t)>::numeric_t thread_id_numeric_size_t;
#ifndef __APPLE__
    typedef std::size_t thread_id_hash_t;
#else
    typedef thread_id_numeric_size_t thread_id_hash_t;
#endif

    static thread_id_hash_t prehash(thread_id_t const& x)
    {
#ifndef __APPLE__
      return std::hash<std::thread::id>()(x);
#else
      return *reinterpret_cast<thread_id_hash_t const*>(&x);
#endif
    }
  };
} }
#else
// 使用这个答案中的巧妙方法：http://stackoverflow.com/a/8438730/21475
// 为了以平台无关的方式获取数字线程 ID，我们使用线程局部静态变量的地址作为线程标识符 :-) :-)
#if defined(__GNUC__) || defined(__INTEL_COMPILER)
#define MOODYCAMEL_THREADLOCAL __thread
#elif defined(_MSC_VER)
#define MOODYCAMEL_THREADLOCAL __declspec(thread)
#else
//假设编译器符合 C++11 标准
#define MOODYCAMEL_THREADLOCAL thread_local
#endif
namespace moodycamel { namespace details {
  typedef std::uintptr_t thread_id_t;
  static const thread_id_t invalid_thread_id  = 0;    //地址不能为 nullptr
  static const thread_id_t invalid_thread_id2 = 1;    // 对 null 指针的成员访问通常也是无效的。另外，它没有对齐。
  static inline thread_id_t thread_id() { static MOODYCAMEL_THREADLOCAL int x; return reinterpret_cast<thread_id_t>(&x); }
} }
#endif

//异常
#ifndef MOODYCAMEL_EXCEPTIONS_ENABLED
#if (defined(_MSC_VER) && defined(_CPPUNWIND)) || (defined(__GNUC__) && defined(__EXCEPTIONS)) || (!defined(_MSC_VER) && !defined(__GNUC__))
#define MOODYCAMEL_EXCEPTIONS_ENABLED
#endif
#endif

// ~~~ @begin 为 CARLA 所做的修改 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include <carla/Exception.h>

#if (defined(LIBCARLA_NO_EXCEPTIONS) && defined(MOODYCAMEL_EXCEPTIONS_ENABLED))
#  undef MOODYCAMEL_EXCEPTIONS_ENABLED
#endif

#ifdef MOODYCAMEL_EXCEPTIONS_ENABLED
#define MOODYCAMEL_TRY try
#define MOODYCAMEL_CATCH(...) catch(__VA_ARGS__)
#define MOODYCAMEL_RETHROW throw
#define MOODYCAMEL_THROW(expr) ::carla::throw_exception(expr)
#else
#define MOODYCAMEL_TRY if (true)
#define MOODYCAMEL_CATCH(...) else if (false)
#define MOODYCAMEL_RETHROW
#define MOODYCAMEL_THROW(expr) ::carla::throw_exception(expr)
#endif

// ~~~ @end 为 CARLA 所做的修改 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef MOODYCAMEL_NOEXCEPT
#if !defined(MOODYCAMEL_EXCEPTIONS_ENABLED)
#define MOODYCAMEL_NOEXCEPT
#define MOODYCAMEL_NOEXCEPT_CTOR(type, valueType, expr) true
#define MOODYCAMEL_NOEXCEPT_ASSIGN(type, valueType, expr) true
#elif defined(_MSC_VER) && defined(_NOEXCEPT) && _MSC_VER < 1800
// VS2012 的 std::is_nothrow_[move_]constructible 存在问题，返回 true 时不应如此 :-(
// 我们必须假设 VS2012 上的所有非平凡构造函数可能会抛出异常！
#define MOODYCAMEL_NOEXCEPT _NOEXCEPT
#define MOODYCAMEL_NOEXCEPT_CTOR(type, valueType, expr) (std::is_rvalue_reference<valueType>::value && std::is_move_constructible<type>::value ? std::is_trivially_move_constructible<type>::value : std::is_trivially_copy_constructible<type>::value)
#define MOODYCAMEL_NOEXCEPT_ASSIGN(type, valueType, expr) ((std::is_rvalue_reference<valueType>::value && std::is_move_assignable<type>::value ? std::is_trivially_move_assignable<type>::value || std::is_nothrow_move_assignable<type>::value : std::is_trivially_copy_assignable<type>::value || std::is_nothrow_copy_assignable<type>::value) && MOODYCAMEL_NOEXCEPT_CTOR(type, valueType, expr))
#elif defined(_MSC_VER) && defined(_NOEXCEPT) && _MSC_VER < 1900
#define MOODYCAMEL_NOEXCEPT _NOEXCEPT
#define MOODYCAMEL_NOEXCEPT_CTOR(type, valueType, expr) (std::is_rvalue_reference<valueType>::value && std::is_move_constructible<type>::value ? std::is_trivially_move_constructible<type>::value || std::is_nothrow_move_constructible<type>::value : std::is_trivially_copy_constructible<type>::value || std::is_nothrow_copy_constructible<type>::value)
#define MOODYCAMEL_NOEXCEPT_ASSIGN(type, valueType, expr) ((std::is_rvalue_reference<valueType>::value && std::is_move_assignable<type>::value ? std::is_trivially_move_assignable<type>::value || std::is_nothrow_move_assignable<type>::value : std::is_trivially_copy_assignable<type>::value || std::is_nothrow_copy_assignable<type>::value) && MOODYCAMEL_NOEXCEPT_CTOR(type, valueType, expr))
#else
#define MOODYCAMEL_NOEXCEPT noexcept
#define MOODYCAMEL_NOEXCEPT_CTOR(type, valueType, expr) noexcept(expr)
#define MOODYCAMEL_NOEXCEPT_ASSIGN(type, valueType, expr) noexcept(expr)
#endif
#endif

#ifndef MOODYCAMEL_CPP11_THREAD_LOCAL_SUPPORTED
#ifdef MCDBGQ_USE_RELACY
#define MOODYCAMEL_CPP11_THREAD_LOCAL_SUPPORTED
#else
// VS2013 不支持 `thread_local`，而 MinGW-w64 与 POSIX 线程的组合存在一个严重的 bug： http://sourceforge.net/p/mingw-w64/bugs/445
// g++ 版本 <=4.7 也不支持 `thread_local`。
// 最后，iOS/ARM 不支持 `thread_local`，虽然 g++/ARM 允许编译，但尚未确认是否实际有效
#if (!defined(_MSC_VER) || _MSC_VER >= 1900) && (!defined(__MINGW32__) && !defined(__MINGW64__) || !defined(__WINPTHREADS_VERSION)) && (!defined(__GNUC__) || __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8)) && (!defined(__APPLE__) || !TARGET_OS_IPHONE) && !defined(__arm__) && !defined(_M_ARM) && !defined(__aarch64__)
// 假设所有其他 C++11 编译器/平台都完全支持 `thread_local`
//#define MOODYCAMEL_CPP11_THREAD_LOCAL_SUPPORTED    // 由于多个用户报告存在问题，因此目前总是禁用
#endif
#endif
#endif

// VS2012 不支持已删除的函数。
// 在这种情况下，我们正常声明函数但不定义它。如果调用该函数，会生成链接错误。
#ifndef MOODYCAMEL_DELETE_FUNCTION
#if defined(_MSC_VER) && _MSC_VER < 1800
#define MOODYCAMEL_DELETE_FUNCTION
#else
#define MOODYCAMEL_DELETE_FUNCTION = delete
#endif
#endif

// 编译器特定的 likely/unlikely 提示
namespace moodycamel { namespace details {
#if defined(__GNUC__)
  static inline bool (likely)(bool x) { return __builtin_expect((x), true); }
  static inline bool (unlikely)(bool x) { return __builtin_expect((x), false); }
#else
  static inline bool (likely)(bool x) { return x; }
  static inline bool (unlikely)(bool x) { return x; }
#endif
} }

#ifdef MOODYCAMEL_QUEUE_INTERNAL_DEBUG
#include "internal/concurrentqueue_internal_debug.h"
#endif

namespace moodycamel {
namespace details {
  template<typename T>
  struct const_numeric_max {
    static_assert(std::is_integral<T>::value, "const_numeric_max can only be used with integers");
    static const T value = std::numeric_limits<T>::is_signed
      ? (static_cast<T>(1) << (sizeof(T) * CHAR_BIT - 1)) - static_cast<T>(1)
      : static_cast<T>(-1);
  };

#if defined(__GLIBCXX__)
  typedef ::max_align_t std_max_align_t;      // libstdc++ 一段时间内忘记将其添加到 std:: 中
#else
  typedef std::max_align_t std_max_align_t;   // 其他编译器（例如 MSVC）坚持认为它只能通过 std:: 访问


  // 一些平台错误地将 max_align_t 设置为一个对齐小于 8 字节的类型，即便它支持 8 字节对齐的标量值（*咳* 32 位 iOS）。
  //用我们自己的联合体解决这个问题。参见问题 #64
  typedef union {
    std_max_align_t x;
    long long y;
    void* z;
  } max_align_t;
}

  // ConcurrentQueue 的默认特性。
  // 要改变一些特性而无需重新实现所有特性，可以从这个结构体继承并覆盖你希望不同的声明；
  // 由于这些特性作为模板类型参数使用，覆盖的声明将在定义的地方使用，其他地方则使用默认值。
struct ConcurrentQueueDefaultTraits
{
  // 通用大小类型。强烈推荐使用 std::size_t。
  typedef std::size_t size_t;

  // 用于入队和出队索引的类型。必须至少与 size_t 一样大。
  // 应该比你预期一次性容纳的元素数量大得多，特别是当你有高周转率时；
  // 例如，在 32 位 x86 上，如果你预期有超过一亿个元素或在非常短的时间内处理几百万个元素，
  // 使用 32 位类型 *可能* 会触发竞争条件。在这种情况下，推荐使用 64 位整数类型，
  // 实际上将防止竞争条件，无论队列的使用情况如何。
  // 请注意，队列是否在使用 64 位整数类型时无锁，取决于 std::atomic<std::uint64_t> 是否无锁，这具有平台特性。

  typedef std::size_t index_t;

  // 内部所有元素都从多元素块中入队和出队；这是最小的可控单位。
  // 如果你预计元素较少但生产者较多，应选择较小的块大小。
  // 对于生产者较少和/或元素较多的情况，建议选择较大的块大小。
  // 提供了一个合理的默认值。块大小必须是 2 的幂。

  static const size_t BLOCK_SIZE = 32;

  // 对于显式生产者（即使用生产者令牌时），通过迭代每个元素的标志列表来检查块是否为空。
  // 对于较大的块大小，这种方法效率过低，改为基于原子计数器的方法更快。
  // 当块大小严格大于此阈值时，会切换到这种方法。

  static const size_t EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD = 32;

  // 单个显式生产者可以预期多少个完整块？这个值应反映该数量的最大值以获得最佳性能。
  // 必须是 2 的幂。
  static const size_t EXPLICIT_INITIAL_INDEX_SIZE = 32;

  // 单个隐式 producer 可以预期有多少个完整块？这应该
  // 反映该数字的最大值以获得最佳性能。必须是 2 的幂。
  static const size_t IMPLICIT_INITIAL_INDEX_SIZE = 32;

  // 线程 ID 到隐式生产者的哈希表的初始大小。
  // 注意，每当哈希表填充到一半时，会进行调整。
  // 必须是 2 的幂，并且为 0 或至少为 1。如果为 0，则禁用隐式生产（使用不带显式生产者令牌的入队方法）。

  static const size_t INITIAL_IMPLICIT_PRODUCER_HASH_SIZE = 32;

  // 控制显式消费者（即带令牌的消费者）在导致所有消费者旋转并转到下一个内部队列之前
  // 必须消费的项目数量。
  static const std::uint32_t EXPLICIT_CONSUMER_CONSUMPTION_QUOTA_BEFORE_ROTATE = 256;

  // 子队列中最多可以排队的元素数量（包括）。如果入队操作会超过此限制，则操作将失败。
  // 请注意，这个限制在块级别强制执行（为了性能原因），即它会被四舍五入到最接近的块大小。
static const size_t MAX_SUBQUEUE_SIZE = details::const_numeric_max<size_t>::value;

#ifndef MCDBGQ_USE_RELACY
  // 如果需要，可以自定义内存分配。
  // malloc 应该在失败时返回 nullptr，并处理对齐问题，就像 std::malloc 一样。
#if defined(malloc) || defined(free)
  // 哎，这已经是 2015 年了，停止定义破坏标准代码的宏吧！
  // 解决 malloc/free 是特殊宏的问题：
  static inline void* WORKAROUND_malloc(size_t size) { return malloc(size); }
  static inline void WORKAROUND_free(void* ptr) { return free(ptr); }
  static inline void* (malloc)(size_t size) { return WORKAROUND_malloc(size); }
  static inline void (free)(void* ptr) { return WORKAROUND_free(ptr); }
#endif
#endif

  static const size_t MAX_SUBQUEUE_SIZE = details::const_numeric_max<size_t>::value;


#ifndef MCDBGQ_USE_RELACY
  // 如果需要，可以自定义内存分配。
  // malloc 应在失败时返回 nullptr，并像 std：：malloc 一样处理对齐。
#if defined(malloc) || defined(free)
  // 噢，现在是 2015 年，别再定义违反标准代码的宏了！
  // 解决 malloc/free 作为特殊宏的问题：
  static inline void* WORKAROUND_malloc(size_t size) { return malloc(size); }
  static inline void WORKAROUND_free(void* ptr) { return free(ptr); }
  static inline void* (malloc)(size_t size) { return WORKAROUND_malloc(size); }
  static inline void (free)(void* ptr) { return WORKAROUND_free(ptr); }
#else
  static inline void* malloc(size_t size) { return std::malloc(size); }
  static inline void free(void* ptr) { return std::free(ptr); }
#endif
#else
  // 在使用 Relacy 竞态检测器运行时的调试版本（在用户代码中忽略这些）

  static inline void* malloc(size_t size) { return rl::rl_malloc(size, $); }
  static inline void free(void* ptr) { return rl::rl_free(ptr, $); }
#endif
};


// 当生产或消费大量元素时，最有效的方法是：
//    1) 使用队列的批量操作方法，并附带一个 token
//    2) 如果不能使用 token，使用没有 token 的批量操作方法
//    3) 如果仍然无法使用，创建一个 token，并使用它来调用单项方法
//    4) 如果以上方法都不可用，使用队列的单参数方法
// 需要注意的是，不要随意创建 tokens —— 理想情况下，每个线程（每种类型）应该最多只有一个 token。

struct ProducerToken;
struct ConsumerToken;

template<typename T, typename Traits> class ConcurrentQueue;
template<typename T, typename Traits> class BlockingConcurrentQueue;
class ConcurrentQueueTests;


namespace details
{
  struct ConcurrentQueueProducerTypelessBase
  {
    ConcurrentQueueProducerTypelessBase* next;
    std::atomic<bool> inactive;
    ProducerToken* token;

    ConcurrentQueueProducerTypelessBase()
      : next(nullptr), inactive(false), token(nullptr)
    {
    }
  };

  template<bool use32> struct _hash_32_or_64 {
    static inline std::uint32_t hash(std::uint32_t h)
    {
      
      // MurmurHash3 完成器 -- 参见 https://code.google.com/p/smhasher/source/browse/trunk/MurmurHash3.cpp
      // 由于线程 ID 已经是唯一的，我们真正要做的只是将这种唯一性均匀地传播到所有位上，
      // 这样我们可以使用这些位的子集，同时显著减少碰撞。
      h ^= h >> 16;
      h *= 0x85ebca6b;
      h ^= h >> 13;
      h *= 0xc2b2ae35;
      return h ^ (h >> 16);
    }
  };
  template<> struct _hash_32_or_64<1> {
    static inline std::uint64_t hash(std::uint64_t h)
    {
      h ^= h >> 33;
      h *= 0xff51afd7ed558ccd;
      h ^= h >> 33;
      h *= 0xc4ceb9fe1a85ec53;
      return h ^ (h >> 33);
    }
  };
  template<std::size_t size> struct hash_32_or_64 : public _hash_32_or_64<(size > 4)> {  };

  static inline size_t hash_thread_id(thread_id_t id)
  {
    static_assert(sizeof(thread_id_t) <= 8, "Expected a platform where thread IDs are at most 64-bit values");
    return static_cast<size_t>(hash_32_or_64<sizeof(thread_id_converter<thread_id_t>::thread_id_hash_t)>::hash(
      thread_id_converter<thread_id_t>::prehash(id)));
  }

  template<typename T>
  static inline bool circular_less_than(T a, T b)
  {
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4554)
#endif
    static_assert(std::is_integral<T>::value && !std::numeric_limits<T>::is_signed, "circular_less_than is intended to be used only with unsigned integer types");
    return static_cast<T>(a - b) > static_cast<T>(static_cast<T>(1) << static_cast<T>(sizeof(T) * CHAR_BIT - 1));
#ifdef _MSC_VER
#pragma warning(pop)
#endif
  }

  template<typename U>
  static inline char* align_for(char* ptr)
  {
    const std::size_t alignment = std::alignment_of<U>::value;
    return ptr + (alignment - (reinterpret_cast<std::uintptr_t>(ptr) % alignment)) % alignment;
  }

  template<typename T>
  static inline T ceil_to_pow_2(T x)
  {
    static_assert(std::is_integral<T>::value && !std::numeric_limits<T>::is_signed, "ceil_to_pow_2 is intended to be used only with unsigned integer types");

    // Adapted from http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    for (std::size_t i = 1; i < sizeof(T); i <<= 1) {
      x |= x >> (i << 3);
    }
    ++x;
    return x;
  }

  template<typename T>
  static inline void swap_relaxed(std::atomic<T>& left, std::atomic<T>& right)
  {
    T temp = std::move(left.load(std::memory_order_relaxed));
    left.store(std::move(right.load(std::memory_order_relaxed)), std::memory_order_relaxed);
    right.store(std::move(temp), std::memory_order_relaxed);
  }

  template<typename T>
  static inline T const& nomove(T const& x)
  {
    return x;
  }

  template<bool Enable>
  struct nomove_if
  {
    template<typename T>
    static inline T const& eval(T const& x)
    {
      return x;
    }
  };

  template<>
  struct nomove_if<false>
  {
    template<typename U>
    static inline auto eval(U&& x)
      -> decltype(std::forward<U>(x))
    {
      return std::forward<U>(x);
    }
  };

  template<typename It>
  static inline auto deref_noexcept(It& it) MOODYCAMEL_NOEXCEPT -> decltype(*it)
  {
    return *it;
  }

#if defined(__clang__) || !defined(__GNUC__) || __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8)
  template<typename T> struct is_trivially_destructible : std::is_trivially_destructible<T> { };
#else
  template<typename T> struct is_trivially_destructible : std::has_trivial_destructor<T> { };
#endif

#ifdef MOODYCAMEL_CPP11_THREAD_LOCAL_SUPPORTED
#ifdef MCDBGQ_USE_RELACY
  typedef RelacyThreadExitListener ThreadExitListener;
  typedef RelacyThreadExitNotifier ThreadExitNotifier;
#else
  struct ThreadExitListener
  {
    typedef void (*callback_t)(void*);
    callback_t callback;
    void* userData;

    ThreadExitListener* next;    // 保留供 ThreadExitNotifier 使用

  };


  class ThreadExitNotifier
  {
  public:

    // 将监听器添加到订阅者列表中
    static void subscribe(ThreadExitListener* listener)
    {
      auto& tlsInst = instance();
      listener->next = tlsInst.tail;
      tlsInst.tail = listener;
    }

    // 从订阅者列表中移除监听器
    static void unsubscribe(ThreadExitListener* listener)
    {
      auto& tlsInst = instance();
      ThreadExitListener** prev = &tlsInst.tail;
      for (auto ptr = tlsInst.tail; ptr != nullptr; ptr = ptr->next) {
        if (ptr == listener) {
          *prev = ptr->next;
          break;
        }
        prev = &ptr->next;// 更新前一个节点的指针到当前节点的 next
      }
    }

  private:
    // 私有构造函数，确保使用单例模式
    ThreadExitNotifier() : tail(nullptr) { }
    ThreadExitNotifier(ThreadExitNotifier const&) MOODYCAMEL_DELETE_FUNCTION;
    ThreadExitNotifier& operator=(ThreadExitNotifier const&) MOODYCAMEL_DELETE_FUNCTION;

    ~ThreadExitNotifier()
    {
      // 该线程即将退出，通知所有人！
      assert(this == &instance() && "If this assert fails, you likely have a buggy compiler! Change the preprocessor conditions such that MOODYCAMEL_CPP11_THREAD_LOCAL_SUPPORTED is no longer defined.");
      for (auto ptr = tail; ptr != nullptr; ptr = ptr->next) {
        ptr->callback(ptr->userData);
      }
    }

    // Thread-local
    static inline ThreadExitNotifier& instance()
    {
      static thread_local ThreadExitNotifier notifier;
      return notifier;
    }

  private:
    ThreadExitListener* tail;
  };
#endif
#endif

  template<typename T> struct static_is_lock_free_num { enum { value = 0 }; };
  template<> struct static_is_lock_free_num<signed char> { enum { value = ATOMIC_CHAR_LOCK_FREE }; };
  template<> struct static_is_lock_free_num<short> { enum { value = ATOMIC_SHORT_LOCK_FREE }; };
  template<> struct static_is_lock_free_num<int> { enum { value = ATOMIC_INT_LOCK_FREE }; };
  template<> struct static_is_lock_free_num<long> { enum { value = ATOMIC_LONG_LOCK_FREE }; };
  template<> struct static_is_lock_free_num<long long> { enum { value = ATOMIC_LLONG_LOCK_FREE }; };
  template<typename T> struct static_is_lock_free : static_is_lock_free_num<typename std::make_signed<T>::type> {  };
  template<> struct static_is_lock_free<bool> { enum { value = ATOMIC_BOOL_LOCK_FREE }; };
  template<typename U> struct static_is_lock_free<U*> { enum { value = ATOMIC_POINTER_LOCK_FREE }; };
}


struct ProducerToken
{
  template<typename T, typename Traits>
  explicit ProducerToken(ConcurrentQueue<T, Traits>& queue);

  template<typename T, typename Traits>
  explicit ProducerToken(BlockingConcurrentQueue<T, Traits>& queue);

  ProducerToken(ProducerToken&& other) MOODYCAMEL_NOEXCEPT
    : producer(other.producer)
  {
    other.producer = nullptr;
    if (producer != nullptr) {
      producer->token = this;
    }
  }

  inline ProducerToken& operator=(ProducerToken&& other) MOODYCAMEL_NOEXCEPT
  {
    swap(other);
    return *this;
  }

  void swap(ProducerToken& other) MOODYCAMEL_NOEXCEPT
  {
    std::swap(producer, other.producer);
    if (producer != nullptr) {
      producer->token = this;
    }
    if (other.producer != nullptr) {
      other.producer->token = &other;
    }
  }

  // 一个令牌通常是有效的，除非：
  //     1) 在构造过程中内存分配失败
  //     2) 通过移动构造函数移动了令牌
  //        （注意：赋值操作会进行交换，因此两个令牌都可能有效）
  //     3) 关联的队列被销毁
  // 注意，如果 valid() 返回 true，这仅表示令牌对于特定队列是有效的，
  // 但并不能确定是哪一个队列；这需要由用户自己跟踪。

  inline bool valid() const { return producer != nullptr; }

  ~ProducerToken()
  {
    if (producer != nullptr) {
      producer->token = nullptr;
      producer->inactive.store(true, std::memory_order_release);
    }
  }

  // 禁用复制和分配
  ProducerToken(ProducerToken const&) MOODYCAMEL_DELETE_FUNCTION;
  ProducerToken& operator=(ProducerToken const&) MOODYCAMEL_DELETE_FUNCTION;

private:
  template<typename T, typename Traits> friend class ConcurrentQueue;
  friend class ConcurrentQueueTests;

protected:
  details::ConcurrentQueueProducerTypelessBase* producer;
};


struct ConsumerToken
{
  template<typename T, typename Traits>
  explicit ConsumerToken(ConcurrentQueue<T, Traits>& q);

  template<typename T, typename Traits>
  explicit ConsumerToken(BlockingConcurrentQueue<T, Traits>& q);

  ConsumerToken(ConsumerToken&& other) MOODYCAMEL_NOEXCEPT
    : initialOffset(other.initialOffset), lastKnownGlobalOffset(other.lastKnownGlobalOffset), itemsConsumedFromCurrent(other.itemsConsumedFromCurrent), currentProducer(other.currentProducer), desiredProducer(other.desiredProducer)
  {
  }

  inline ConsumerToken& operator=(ConsumerToken&& other) MOODYCAMEL_NOEXCEPT
  {
    swap(other);
    return *this;
  }

  void swap(ConsumerToken& other) MOODYCAMEL_NOEXCEPT
  {
    std::swap(initialOffset, other.initialOffset);
    std::swap(lastKnownGlobalOffset, other.lastKnownGlobalOffset);
    std::swap(itemsConsumedFromCurrent, other.itemsConsumedFromCurrent);
    std::swap(currentProducer, other.currentProducer);
    std::swap(desiredProducer, other.desiredProducer);
  }

  // 禁用拷贝和赋值操作
  ConsumerToken(ConsumerToken const&) MOODYCAMEL_DELETE_FUNCTION;
  ConsumerToken& operator=(ConsumerToken const&) MOODYCAMEL_DELETE_FUNCTION;

private:
  template<typename T, typename Traits> friend class ConcurrentQueue;
  friend class ConcurrentQueueTests;

private: // 但与并发队列共享
  std::uint32_t initialOffset;
  std::uint32_t lastKnownGlobalOffset;
  std::uint32_t itemsConsumedFromCurrent;
  details::ConcurrentQueueProducerTypelessBase* currentProducer;
  details::ConcurrentQueueProducerTypelessBase* desiredProducer;
};

// 需要前向声明这个 swap，因为它在一个命名空间中。
// 参见 http://stackoverflow.com/questions/4492062/why-does-a-c-friend-class-need-a-forward-declaration-only-in-other-namespaces
template<typename T, typename Traits>
inline void swap(typename ConcurrentQueue<T, Traits>::ImplicitProducerKVP& a, typename ConcurrentQueue<T, Traits>::ImplicitProducerKVP& b) MOODYCAMEL_NOEXCEPT;


template<typename T, typename Traits = ConcurrentQueueDefaultTraits>
class ConcurrentQueue
{
public:
  typedef ::moodycamel::ProducerToken producer_token_t;
  typedef ::moodycamel::ConsumerToken consumer_token_t;

  typedef typename Traits::index_t index_t;
  typedef typename Traits::size_t size_t;

  static const size_t BLOCK_SIZE = static_cast<size_t>(Traits::BLOCK_SIZE);
  static const size_t EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD = static_cast<size_t>(Traits::EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD);
  static const size_t EXPLICIT_INITIAL_INDEX_SIZE = static_cast<size_t>(Traits::EXPLICIT_INITIAL_INDEX_SIZE);
  static const size_t IMPLICIT_INITIAL_INDEX_SIZE = static_cast<size_t>(Traits::IMPLICIT_INITIAL_INDEX_SIZE);
  static const size_t INITIAL_IMPLICIT_PRODUCER_HASH_SIZE = static_cast<size_t>(Traits::INITIAL_IMPLICIT_PRODUCER_HASH_SIZE);
  static const std::uint32_t EXPLICIT_CONSUMER_CONSUMPTION_QUOTA_BEFORE_ROTATE = static_cast<std::uint32_t>(Traits::EXPLICIT_CONSUMER_CONSUMPTION_QUOTA_BEFORE_ROTATE);
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4307)    //+ 整型常量溢出（这就是三元表达式的用途！
#pragma warning(disable: 4309)    // static_cast：常量值的截断
#endif
  static const size_t MAX_SUBQUEUE_SIZE = (details::const_numeric_max<size_t>::value - static_cast<size_t>(Traits::MAX_SUBQUEUE_SIZE) < BLOCK_SIZE) ? details::const_numeric_max<size_t>::value : ((static_cast<size_t>(Traits::MAX_SUBQUEUE_SIZE) + (BLOCK_SIZE - 1)) / BLOCK_SIZE * BLOCK_SIZE);
#ifdef _MSC_VER
#pragma warning(pop)
#endif

  static_assert(!std::numeric_limits<size_t>::is_signed && std::is_integral<size_t>::value, "Traits::size_t must be an unsigned integral type");
  static_assert(!std::numeric_limits<index_t>::is_signed && std::is_integral<index_t>::value, "Traits::index_t must be an unsigned integral type");
  static_assert(sizeof(index_t) >= sizeof(size_t), "Traits::index_t must be at least as wide as Traits::size_t");
  static_assert((BLOCK_SIZE > 1) && !(BLOCK_SIZE & (BLOCK_SIZE - 1)), "Traits::BLOCK_SIZE must be a power of 2 (and at least 2)");
  static_assert((EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD > 1) && !(EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD & (EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD - 1)), "Traits::EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD must be a power of 2 (and greater than 1)");
  static_assert((EXPLICIT_INITIAL_INDEX_SIZE > 1) && !(EXPLICIT_INITIAL_INDEX_SIZE & (EXPLICIT_INITIAL_INDEX_SIZE - 1)), "Traits::EXPLICIT_INITIAL_INDEX_SIZE must be a power of 2 (and greater than 1)");
  static_assert((IMPLICIT_INITIAL_INDEX_SIZE > 1) && !(IMPLICIT_INITIAL_INDEX_SIZE & (IMPLICIT_INITIAL_INDEX_SIZE - 1)), "Traits::IMPLICIT_INITIAL_INDEX_SIZE must be a power of 2 (and greater than 1)");
  static_assert((INITIAL_IMPLICIT_PRODUCER_HASH_SIZE == 0) || !(INITIAL_IMPLICIT_PRODUCER_HASH_SIZE & (INITIAL_IMPLICIT_PRODUCER_HASH_SIZE - 1)), "Traits::INITIAL_IMPLICIT_PRODUCER_HASH_SIZE must be a power of 2");
  static_assert(INITIAL_IMPLICIT_PRODUCER_HASH_SIZE == 0 || INITIAL_IMPLICIT_PRODUCER_HASH_SIZE >= 1, "Traits::INITIAL_IMPLICIT_PRODUCER_HASH_SIZE must be at least 1 (or 0 to disable implicit enqueueing)");

public:

  // 创建一个具有至少 `capacity` 元素槽的队列；注意实际能够插入的元素数量
  // 取决于生产者的数量和块大小（例如，如果块大小等于 `capacity`，则只会
  // 预先分配一个块，这意味着只有一个生产者能够在不进行额外分配的情况下
  // 将元素入队 —— 块在生产者之间不会共享）。这个方法不是线程安全的 ——
  // 用户需要确保队列在开始被其他线程使用之前已经完全构造（这包括
  // 使构造的内存效果可见，可能需要使用内存屏障）。
  explicit ConcurrentQueue(size_t capacity = 6 * BLOCK_SIZE)
    : producerListTail(nullptr),
    producerCount(0),
    initialBlockPoolIndex(0),
    nextExplicitConsumerId(0),
    globalExplicitConsumerOffset(0)
  {
    implicitProducerHashResizeInProgress.clear(std::memory_order_relaxed);
    populate_initial_implicit_producer_hash();
    populate_initial_block_list(capacity / BLOCK_SIZE + ((capacity & (BLOCK_SIZE - 1)) == 0 ? 0 : 1));

#ifdef MOODYCAMEL_QUEUE_INTERNAL_DEBUG
    // 使用完全解析的类型化列表跟踪所有生产者
    // 每一种;这使得可以从
    // 根队列对象（否则需要
    // 不要在 Debugger 的 Expression Evaluator 中编译）。
    explicitProducers.store(nullptr, std::memory_order_relaxed);
    implicitProducers.store(nullptr, std::memory_order_relaxed);
#endif
  }

  // 根据您希望在任何给定时间可用的最小元素数量和每种生产者的最大并发数量，
  // 计算适当的预分配块数量。

  ConcurrentQueue(size_t minCapacity, size_t maxExplicitProducers, size_t maxImplicitProducers)
    : producerListTail(nullptr),
    producerCount(0),
    initialBlockPoolIndex(0),
    nextExplicitConsumerId(0),
    globalExplicitConsumerOffset(0)
  {
    implicitProducerHashResizeInProgress.clear(std::memory_order_relaxed);
    populate_initial_implicit_producer_hash();
    size_t blocks = (((minCapacity + BLOCK_SIZE - 1) / BLOCK_SIZE) - 1) * (maxExplicitProducers + 1) + 2 * (maxExplicitProducers + maxImplicitProducers);
    populate_initial_block_list(blocks);

#ifdef MOODYCAMEL_QUEUE_INTERNAL_DEBUG
    explicitProducers.store(nullptr, std::memory_order_relaxed);
    implicitProducers.store(nullptr, std::memory_order_relaxed);
#endif
  }

  // 注意：在队列被删除时不应同时访问它。用户需要同步这一点。
  // 这个方法不是线程安全的。

  ~ConcurrentQueue()
  {
    // 销毁生产者
    auto ptr = producerListTail.load(std::memory_order_relaxed);
    while (ptr != nullptr) {
      auto next = ptr->next_prod();
      if (ptr->token != nullptr) {
        ptr->token->producer = nullptr;
      }
      destroy(ptr);
      ptr = next;
    }

    // 销毁隐式生产者哈希表
    if (INITIAL_IMPLICIT_PRODUCER_HASH_SIZE != 0) {
      auto hash = implicitProducerHash.load(std::memory_order_relaxed);
      while (hash != nullptr) {
        auto prev = hash->prev;
        if (prev != nullptr) {    // 最后一个哈希是此对象的一部分，不是动态分配的
          for (size_t i = 0; i != hash->capacity; ++i) {
            hash->entries[i].~ImplicitProducerKVP();
          }
          hash->~ImplicitProducerHash();
          (Traits::free)(hash);
        }
        hash = prev;
      }
    }

    // 销毁全局空闲列表
    auto block = freeList.head_unsafe();
    while (block != nullptr) {
      auto next = block->freeListNext.load(std::memory_order_relaxed);
      if (block->dynamicallyAllocated) {
        destroy(block);
      }
      block = next;
    }

    // 销毁初始空闲列表
    destroy_array(initialBlockPool, initialBlockPoolSize);
  }

  // 禁用复制构造函数和复制赋值运算符
  ConcurrentQueue(ConcurrentQueue const&) MOODYCAMEL_DELETE_FUNCTION;
  ConcurrentQueue& operator=(ConcurrentQueue const&) MOODYCAMEL_DELETE_FUNCTION;

  // 移动操作是支持的，但请注意它 *不是* 线程安全的操作。
  // 在队列被移动时，其他线程不能使用该队列，并且必须在其他线程可以使用它之前传播该移动的内存效果。
  // 注意：当队列被移动时，它的令牌仍然有效，但只能与目标队列一起使用（即语义上它们也被随队列一起移动）。

  ConcurrentQueue(ConcurrentQueue&& other) MOODYCAMEL_NOEXCEPT
    : producerListTail(other.producerListTail.load(std::memory_order_relaxed)),
    producerCount(other.producerCount.load(std::memory_order_relaxed)),
    initialBlockPoolIndex(other.initialBlockPoolIndex.load(std::memory_order_relaxed)),
    initialBlockPool(other.initialBlockPool),
    initialBlockPoolSize(other.initialBlockPoolSize),
    freeList(std::move(other.freeList)),
    nextExplicitConsumerId(other.nextExplicitConsumerId.load(std::memory_order_relaxed)),
    globalExplicitConsumerOffset(other.globalExplicitConsumerOffset.load(std::memory_order_relaxed))
  {
    // 将另一个队列移动到当前队列，并将另一个队列留为空队列
    implicitProducerHashResizeInProgress.clear(std::memory_order_relaxed);
    populate_initial_implicit_producer_hash();
    swap_implicit_producer_hashes(other);

    other.producerListTail.store(nullptr, std::memory_order_relaxed);
    other.producerCount.store(0, std::memory_order_relaxed);
    other.nextExplicitConsumerId.store(0, std::memory_order_relaxed);
    other.globalExplicitConsumerOffset.store(0, std::memory_order_relaxed);

#ifdef MOODYCAMEL_QUEUE_INTERNAL_DEBUG
    explicitProducers.store(other.explicitProducers.load(std::memory_order_relaxed), std::memory_order_relaxed);
    other.explicitProducers.store(nullptr, std::memory_order_relaxed);
    implicitProducers.store(other.implicitProducers.load(std::memory_order_relaxed), std::memory_order_relaxed);
    other.implicitProducers.store(nullptr, std::memory_order_relaxed);
#endif

    other.initialBlockPoolIndex.store(0, std::memory_order_relaxed);
    other.initialBlockPoolSize = 0;
    other.initialBlockPool = nullptr;

    reown_producers();
  }

  inline ConcurrentQueue& operator=(ConcurrentQueue&& other) MOODYCAMEL_NOEXCEPT
  {
    return swap_internal(other);
  }

  // 交换当前队列的状态与另一个队列的状态。此操作不是线程安全的。
  // 交换两个队列不会使它们的令牌失效，然而
  // 为一个队列创建的令牌必须只与交换后的队列一起使用（即，令牌与
  // 队列的可移动状态相关联，而不是对象本身）。

  inline void swap(ConcurrentQueue& other) MOODYCAMEL_NOEXCEPT
  {
    swap_internal(other);
  }

private:
  ConcurrentQueue& swap_internal(ConcurrentQueue& other)
  {
    if (this == &other) {
      return *this;
    }

    details::swap_relaxed(producerListTail, other.producerListTail);
    details::swap_relaxed(producerCount, other.producerCount);
    details::swap_relaxed(initialBlockPoolIndex, other.initialBlockPoolIndex);
    std::swap(initialBlockPool, other.initialBlockPool);
    std::swap(initialBlockPoolSize, other.initialBlockPoolSize);
    freeList.swap(other.freeList);
    details::swap_relaxed(nextExplicitConsumerId, other.nextExplicitConsumerId);
    details::swap_relaxed(globalExplicitConsumerOffset, other.globalExplicitConsumerOffset);

    swap_implicit_producer_hashes(other);

    reown_producers();
    other.reown_producers();

#ifdef MOODYCAMEL_QUEUE_INTERNAL_DEBUG
    details::swap_relaxed(explicitProducers, other.explicitProducers);
    details::swap_relaxed(implicitProducers, other.implicitProducers);
#endif

    return *this;
  }

public:

  // 将单个项目（通过复制）入队。
  // 如有必要，分配内存。仅在内存分配失败时（或隐式
  // 生产被禁用，因为 Traits::INITIAL_IMPLICIT_PRODUCER_HASH_SIZE 为 0，
  // 或 Traits::MAX_SUBQUEUE_SIZE 已定义并可能被超越）才会失败。
  // 线程安全。
  inline bool enqueue(T const& item)
  {
    if (INITIAL_IMPLICIT_PRODUCER_HASH_SIZE == 0) return false;
    return inner_enqueue<CanAlloc>(item);
  }

  // 将单个项目（如果可能，通过移动）入队。
  // 如有必要，分配内存。仅在内存分配失败时（或隐式
  // 生产被禁用，因为 Traits::INITIAL_IMPLICIT_PRODUCER_HASH_SIZE 为 0，
  // 或 Traits::MAX_SUBQUEUE_SIZE 已定义并可能被超越）才会失败。
  // 线程安全。
  inline bool enqueue(T&& item)
  {
    if (INITIAL_IMPLICIT_PRODUCER_HASH_SIZE == 0) return false;
    return inner_enqueue<CanAlloc>(std::move(item));
  }

  // 使用显式生产者令牌将单个项目（通过复制）入队。
  // 如有必要，分配内存。仅在内存分配失败时（或
  // Traits::MAX_SUBQUEUE_SIZE 已定义并可能被超越）才会失败。
  // 线程安全。
  inline bool enqueue(producer_token_t const& token, T const& item)
  {
    return inner_enqueue<CanAlloc>(token, item);
  }

  // 使用显式生产者令牌将单个项目（如果可能，通过移动）入队。
  // 如有必要，分配内存。仅在内存分配失败时（或
  // Traits::MAX_SUBQUEUE_SIZE 已定义并可能被超越）才会失败。
  // 线程安全。
  inline bool enqueue(producer_token_t const& token, T&& item)
  {
    return inner_enqueue<CanAlloc>(token, std::move(item));
  }

  // 将多个项目入队。
  // 如有必要，分配内存。仅在内存分配失败时（或隐式生产被禁用，因为 Traits::INITIAL_IMPLICIT_PRODUCER_HASH_SIZE 为 0，
  // 或 Traits::MAX_SUBQUEUE_SIZE 已定义并可能被超越）才会失败。
  // 注意：如果要移动而非复制元素，请使用 std::make_move_iterator。
  // Thread-safe.
  template<typename It>
  bool enqueue_bulk(It itemFirst, size_t count)
  {
    if (INITIAL_IMPLICIT_PRODUCER_HASH_SIZE == 0) return false;
    return inner_enqueue_bulk<CanAlloc>(itemFirst, count);
  }

  // 使用显式生产者令牌将多个项目入队。
  // Allocates memory if required. Only fails if memory allocation fails
  // (or Traits::MAX_SUBQUEUE_SIZE has been defined and would be surpassed).
  // Note: Use std::make_move_iterator if the elements should be moved
  // instead of copied.
  // Thread-safe.
  template<typename It>
  bool enqueue_bulk(producer_token_t const& token, It itemFirst, size_t count)
  {
    return inner_enqueue_bulk<CanAlloc>(token, itemFirst, count);
  }

  // 将单个项目（通过复制）入队。
  // Does not allocate memory. Fails if not enough room to enqueue (or implicit
  // production is disabled because Traits::INITIAL_IMPLICIT_PRODUCER_HASH_SIZE
  // is 0).
  // Thread-safe.
  inline bool try_enqueue(T const& item)
  {
    if (INITIAL_IMPLICIT_PRODUCER_HASH_SIZE == 0) return false;
    return inner_enqueue<CannotAlloc>(item);
  }

  // 将单个项目入队（如果可能，通过移动）。
  // Does not allocate memory (except for one-time implicit producer).
  // Fails if not enough room to enqueue (or implicit production is
  // disabled because Traits::INITIAL_IMPLICIT_PRODUCER_HASH_SIZE is 0).
  // Thread-safe.
  inline bool try_enqueue(T&& item)
  {
    if (INITIAL_IMPLICIT_PRODUCER_HASH_SIZE == 0) return false;
    return inner_enqueue<CannotAlloc>(std::move(item));
  }

  // 使用显式生产者令牌将单个项目入队（通过复制）。
  // Does not allocate memory. Fails if not enough room to enqueue.
  // Thread-safe.
  inline bool try_enqueue(producer_token_t const& token, T const& item)
  {
    return inner_enqueue<CannotAlloc>(token, item);
  }

  // Enqueues a single item (by moving it, if possible) using an explicit producer token.
  // Does not allocate memory. Fails if not enough room to enqueue.
  // Thread-safe.
  inline bool try_enqueue(producer_token_t const& token, T&& item)
  {
    return inner_enqueue<CannotAlloc>(token, std::move(item));
  }

  // 批量入队多个项目。
  // 不会分配内存（除了一个一次性的隐式生产者）。
  // Fails if not enough room to enqueue (or implicit production is
  // disabled because Traits::INITIAL_IMPLICIT_PRODUCER_HASH_SIZE is 0).
  // Note: Use std::make_move_iterator if the elements should be moved
  // instead of copied.
  // Thread-safe.
  template<typename It>
  bool try_enqueue_bulk(It itemFirst, size_t count)
  {
    if (INITIAL_IMPLICIT_PRODUCER_HASH_SIZE == 0) return false;
    return inner_enqueue_bulk<CannotAlloc>(itemFirst, count);
  }

  // 使用显式生产者令牌批量入队多个项目。
  // Does not allocate memory. Fails if not enough room to enqueue.
  // Note: Use std::make_move_iterator if the elements should be moved
  // instead of copied.
  // Thread-safe.
  template<typename It>
  bool try_enqueue_bulk(producer_token_t const& token, It itemFirst, size_t count)
  {
    return inner_enqueue_bulk<CannotAlloc>(token, itemFirst, count);
  }



  // 尝试从队列中出队。
  // 如果在检查时所有生产者流都为空，则返回 false（因此队列可能为空，但不保证为空）。
  // 从不进行内存分配。线程安全。
  template<typename U>
  bool try_dequeue(U& item)
  {
    
    // 我们不是简单地依次尝试每个生产者（这可能会导致第一个生产者出现不必要的竞争），
    // 而是通过启发式方法对它们进行评分。
    size_t nonEmptyCount = 0;
    ProducerBase* best = nullptr;
    size_t bestSize = 0;
    for (auto ptr = producerListTail.load(std::memory_order_acquire); nonEmptyCount < 3 && ptr != nullptr; ptr = ptr->next_prod()) {
      auto size = ptr->size_approx();
      if (size > 0) {
        if (size > bestSize) {
          bestSize = size;
          best = ptr;
        }
        ++nonEmptyCount;
      }
    }

    // 如果至少有一个非空队列，但在尝试从中出队时它似乎为空，我们需要确保每个队列都已被尝试过。
    if (nonEmptyCount > 0) {
      if ((details::likely)(best->dequeue(item))) {
        return true;
      }
      for (auto ptr = producerListTail.load(std::memory_order_acquire); ptr != nullptr; ptr = ptr->next_prod()) {
        if (ptr != best && ptr->dequeue(item)) {
          return true;
        }
      }
    }
    return false;
  }

  // 尝试从队列中出队。
  // 如果在检查时所有生产者流都为空，则返回 false（因此队列可能为空，但不保证为空）。
  // 与 try_dequeue(item) 方法不同的是，这个方法不会通过交错生产者流的出队顺序来减少竞争。
  // 因此，在竞争情况下使用这个方法可能会降低整体吞吐量，但在单线程消费者场景下会提供更可预测的结果。
  // 这主要适用于内部单元测试。
  // 从不进行内存分配。Thread-safe.
  template<typename U>
  bool try_dequeue_non_interleaved(U& item)
  {
    for (auto ptr = producerListTail.load(std::memory_order_acquire); ptr != nullptr; ptr = ptr->next_prod()) {
      if (ptr->dequeue(item)) {
        return true;
      }
    }
    return false;
  }

  // Attempts to dequeue from the queue using an explicit consumer token.
  // Returns false if all producer streams appeared empty at the time they
  // were checked (so, the queue is likely but not guaranteed to be empty).
  // Never allocates. Thread-safe.
  template<typename U>
  bool try_dequeue(consumer_token_t& token, U& item)
  {
    
    // 大致思想如下：
    // 每处理来自一个生产者的 256 个项目，就让所有生产者进行轮换（增加全局偏移量）——这意味着效率最高的消费者在一定程度上决定了其他消费者的轮换速度。
    // 如果看到全局偏移量发生了变化，你必须重置你的消费计数器并移动到指定的位置。
    // 如果你所在的位置没有项目，继续移动直到找到一个有项目的生产者。
    // 如果全局偏移量没有变化，但你已经没有更多项目可以消费，继续从当前位置移动，直到找到一个有项目的生产者。
    if (token.desiredProducer == nullptr || token.lastKnownGlobalOffset != globalExplicitConsumerOffset.load(std::memory_order_relaxed)) {
      if (!update_current_producer_after_rotation(token)) {
        return false;
      }
    }

    // 如果至少有一个非空队列，但在尝试从中取出元素时它却显得为空，我们需要确保每个队列都已经被尝试过
    if (static_cast<ProducerBase*>(token.currentProducer)->dequeue(item)) {
      if (++token.itemsConsumedFromCurrent == EXPLICIT_CONSUMER_CONSUMPTION_QUOTA_BEFORE_ROTATE) {
        globalExplicitConsumerOffset.fetch_add(1, std::memory_order_relaxed);
      }
      return true;
    }

    auto tail = producerListTail.load(std::memory_order_acquire);
    auto ptr = static_cast<ProducerBase*>(token.currentProducer)->next_prod();
    if (ptr == nullptr) {
      ptr = tail;
    }
    while (ptr != static_cast<ProducerBase*>(token.currentProducer)) {
      if (ptr->dequeue(item)) {
        token.currentProducer = ptr;
        token.itemsConsumedFromCurrent = 1;
        return true;
      }
      ptr = ptr->next_prod();
      if (ptr == nullptr) {
        ptr = tail;
      }
    }
    return false;
  }

  // 尝试从队列中取出多个元素。
  // 返回实际取出的元素数量。
  // 如果所有生产者流在检查时都显得为空（因此，队列可能但不一定为空），则返回0。
  // Never allocates. Thread-safe.
  template<typename It>
  size_t try_dequeue_bulk(It itemFirst, size_t max)
  {
    size_t count = 0;
    for (auto ptr = producerListTail.load(std::memory_order_acquire); ptr != nullptr; ptr = ptr->next_prod()) {
      count += ptr->dequeue_bulk(itemFirst, max - count);
      if (count == max) {
        break;
      }
    }
    return count;
  }

  // 尝试使用显式消费者令牌从队列中取出多个元素。
  // Returns the number of items actually dequeued.
  // Returns 0 if all producer streams appeared empty at the time they
  // were checked (so, the queue is likely but not guaranteed to be empty).
  // Never allocates. Thread-safe.
  template<typename It>
  size_t try_dequeue_bulk(consumer_token_t& token, It itemFirst, size_t max)
  {
    if (token.desiredProducer == nullptr || token.lastKnownGlobalOffset != globalExplicitConsumerOffset.load(std::memory_order_relaxed)) {
      if (!update_current_producer_after_rotation(token)) {
        return 0;
      }
    }

    size_t count = static_cast<ProducerBase*>(token.currentProducer)->dequeue_bulk(itemFirst, max);
    if (count == max) {
      if ((token.itemsConsumedFromCurrent += static_cast<std::uint32_t>(max)) >= EXPLICIT_CONSUMER_CONSUMPTION_QUOTA_BEFORE_ROTATE) {
        globalExplicitConsumerOffset.fetch_add(1, std::memory_order_relaxed);
      }
      return max;
    }
    token.itemsConsumedFromCurrent += static_cast<std::uint32_t>(count);
    max -= count;

    auto tail = producerListTail.load(std::memory_order_acquire);
    auto ptr = static_cast<ProducerBase*>(token.currentProducer)->next_prod();
    if (ptr == nullptr) {
      ptr = tail;
    }
    while (ptr != static_cast<ProducerBase*>(token.currentProducer)) {
      auto dequeued = ptr->dequeue_bulk(itemFirst, max);
      count += dequeued;
      if (dequeued != 0) {
        token.currentProducer = ptr;
        token.itemsConsumedFromCurrent = static_cast<std::uint32_t>(dequeued);
      }
      if (dequeued == max) {
        break;
      }
      max -= dequeued;
      ptr = ptr->next_prod();
      if (ptr == nullptr) {
        ptr = tail;
      }
    }
    return count;
  }



  // Attempts to dequeue from a specific producer's inner queue.
  // If you happen to know which producer you want to dequeue from, this
  // is significantly faster than using the general-case try_dequeue methods.
  // Returns false if the producer's queue appeared empty at the time it
  // was checked (so, the queue is likely but not guaranteed to be empty).
  // Never allocates. Thread-safe.
  template<typename U>
  inline bool try_dequeue_from_producer(producer_token_t const& producer, U& item)
  {
    return static_cast<ExplicitProducer*>(producer.producer)->dequeue(item);
  }

  // Attempts to dequeue several elements from a specific producer's inner queue.
  // Returns the number of items actually dequeued.
  // If you happen to know which producer you want to dequeue from, this
  // is significantly faster than using the general-case try_dequeue methods.
  // Returns 0 if the producer's queue appeared empty at the time it
  // was checked (so, the queue is likely but not guaranteed to be empty).
  // Never allocates. Thread-safe.
  template<typename It>
  inline size_t try_dequeue_bulk_from_producer(producer_token_t const& producer, It itemFirst, size_t max)
  {
    return static_cast<ExplicitProducer*>(producer.producer)->dequeue_bulk(itemFirst, max);
  }


  // Returns an estimate of the total number of elements currently in the queue. This
  // estimate is only accurate if the queue has completely stabilized before it is called
  // (i.e. all enqueue and dequeue operations have completed and their memory effects are
  // visible on the calling thread, and no further operations start while this method is
  // being called).
  // Thread-safe.
  size_t size_approx() const
  {
    size_t size = 0;
    for (auto ptr = producerListTail.load(std::memory_order_acquire); ptr != nullptr; ptr = ptr->next_prod()) {
      size += ptr->size_approx();
    }
    return size;
  }


  // Returns true if the underlying atomic variables used by
  // the queue are lock-free (they should be on most platforms).
  // Thread-safe.
  static bool is_lock_free()
  {
    return
      details::static_is_lock_free<bool>::value == 2 &&
      details::static_is_lock_free<size_t>::value == 2 &&
      details::static_is_lock_free<std::uint32_t>::value == 2 &&
      details::static_is_lock_free<index_t>::value == 2 &&
      details::static_is_lock_free<void*>::value == 2 &&
      details::static_is_lock_free<typename details::thread_id_converter<details::thread_id_t>::thread_id_numeric_size_t>::value == 2;
  }


private:
  friend struct ProducerToken;
  friend struct ConsumerToken;
  struct ExplicitProducer;
  friend struct ExplicitProducer;
  struct ImplicitProducer;
  friend struct ImplicitProducer;
  friend class ConcurrentQueueTests;

  enum AllocationMode { CanAlloc, CannotAlloc };


  ///////////////////////////////
  // Queue methods
  ///////////////////////////////

  template<AllocationMode canAlloc, typename U>
  inline bool inner_enqueue(producer_token_t const& token, U&& element)
  {
    return static_cast<ExplicitProducer*>(token.producer)->ConcurrentQueue::ExplicitProducer::template enqueue<canAlloc>(std::forward<U>(element));
  }

  template<AllocationMode canAlloc, typename U>
  inline bool inner_enqueue(U&& element)
  {
    auto producer = get_or_add_implicit_producer();
    return producer == nullptr ? false : producer->ConcurrentQueue::ImplicitProducer::template enqueue<canAlloc>(std::forward<U>(element));
  }

  template<AllocationMode canAlloc, typename It>
  inline bool inner_enqueue_bulk(producer_token_t const& token, It itemFirst, size_t count)
  {
    return static_cast<ExplicitProducer*>(token.producer)->ConcurrentQueue::ExplicitProducer::template enqueue_bulk<canAlloc>(itemFirst, count);
  }

  template<AllocationMode canAlloc, typename It>
  inline bool inner_enqueue_bulk(It itemFirst, size_t count)
  {
    auto producer = get_or_add_implicit_producer();
    return producer == nullptr ? false : producer->ConcurrentQueue::ImplicitProducer::template enqueue_bulk<canAlloc>(itemFirst, count);
  }

  inline bool update_current_producer_after_rotation(consumer_token_t& token)
  {
    // 发生了轮换，找出我们应该处于的位置！
    auto tail = producerListTail.load(std::memory_order_acquire);
    if (token.desiredProducer == nullptr && tail == nullptr) {
      return false;
    }
    auto prodCount = producerCount.load(std::memory_order_relaxed);
    auto globalOffset = globalExplicitConsumerOffset.load(std::memory_order_relaxed);
    if ((details::unlikely)(token.desiredProducer == nullptr)) {
      // 我们第一次从队列中取出任何东西。
      // 确定我们的本地位置。
      // 注意：偏移量是从开始处计算的，而我们是从末尾遍历的——因此先从计数中减去偏移量。

      std::uint32_t offset = prodCount - 1 - (token.initialOffset % prodCount);
      token.desiredProducer = tail;
      for (std::uint32_t i = 0; i != offset; ++i) {
        token.desiredProducer = static_cast<ProducerBase*>(token.desiredProducer)->next_prod();
        if (token.desiredProducer == nullptr) {
          token.desiredProducer = tail;
        }
      }
    }

    std::uint32_t delta = globalOffset - token.lastKnownGlobalOffset;
    if (delta >= prodCount) {
      delta = delta % prodCount;
    }
    for (std::uint32_t i = 0; i != delta; ++i) {
      token.desiredProducer = static_cast<ProducerBase*>(token.desiredProducer)->next_prod();
      if (token.desiredProducer == nullptr) {
        token.desiredProducer = tail;
      }
    }

    token.lastKnownGlobalOffset = globalOffset;
    token.currentProducer = token.desiredProducer;
    token.itemsConsumedFromCurrent = 0;
    return true;
  }


  ///////////////////////////
  // Free list
  ///////////////////////////

  template <typename N>
  struct FreeListNode
  {
    FreeListNode() : freeListRefs(0), freeListNext(nullptr) { }

    std::atomic<std::uint32_t> freeListRefs;
    std::atomic<N*> freeListNext;
  };

  // 一个基于CAS的简单无锁空闲列表。
  // 在高争用的情况下不是最快的，但它简单且正确（假设节点在空闲列表销毁之前不会被释放）。
  // 在低争用的情况下相当快速。
  template<typename N>   // N 必须继承自 FreeListNode 或具有相同的字段（以及它们的初始化）
  struct FreeList
  {
    FreeList() : freeListHead(nullptr) { }
    FreeList(FreeList&& other) : freeListHead(other.freeListHead.load(std::memory_order_relaxed)) { other.freeListHead.store(nullptr, std::memory_order_relaxed); }
    void swap(FreeList& other) { details::swap_relaxed(freeListHead, other.freeListHead); }

    FreeList(FreeList const&) MOODYCAMEL_DELETE_FUNCTION;
    FreeList& operator=(FreeList const&) MOODYCAMEL_DELETE_FUNCTION;

    inline void add(N* node)
    {
#if MCDBGQ_NOLOCKFREE_FREELIST
      debug::DebugLock lock(mutex);
#endif
      
      // 我们知道此时应在自由列表上的标志位为 0，因此可以安全地使用 fetch_add 来设置它。
      if (node->freeListRefs.fetch_add(SHOULD_BE_ON_FREELIST, std::memory_order_acq_rel) == 0) {
        
        // 我们是最后一个引用这个节点的，我们知道我们想将它添加到自由列表中，所以就这样做吧！
         add_knowing_refcount_is_zero(node);
      }
    }

    inline N* try_get()
    {
#if MCDBGQ_NOLOCKFREE_FREELIST
      debug::DebugLock lock(mutex);
#endif
      auto head = freeListHead.load(std::memory_order_acquire);
      while (head != nullptr) {
        auto prevHead = head;
        auto refs = head->freeListRefs.load(std::memory_order_relaxed);
        if ((refs & REFS_MASK) == 0 || !head->freeListRefs.compare_exchange_strong(refs, refs + 1, std::memory_order_acquire, std::memory_order_relaxed)) {
          head = freeListHead.load(std::memory_order_acquire);
          continue;
        }

        // 好的，引用计数已被递增（它之前不为零），这意味着我们可以安全地读取下一个值，而不必担心在现在和执行 CAS 操作之间它会发生变化。
        auto next = head->freeListNext.load(std::memory_order_relaxed);
        if (freeListHead.compare_exchange_strong(head, next, std::memory_order_acquire, std::memory_order_relaxed)) {
          // 太好了，拿到了节点。这意味着节点原本在列表上，这也表明 shouldBeOnFreeList 必须是假的，无论 refcount 的值如何。
          // 这是因为在节点被取下之前，没有其他人知道它已经被取下，所以它不能被放回列表中。
          assert((head->freeListRefs.load(std::memory_order_relaxed) & SHOULD_BE_ON_FREELIST) == 0);

          // Decrease refcount twice, once for our ref, and once for the list's ref
          head->freeListRefs.fetch_sub(2, std::memory_order_release);
          return head;
        }

        // 好的，头指针必须已经发生变化，但我们仍然需要减少我们之前增加的 refcount。
        // 请注意，我们不需要释放任何内存效果，但我们需要确保 refcount 的递减操作在对头指针的 CAS 操作之后发生。
        refs = prevHead->freeListRefs.fetch_sub(1, std::memory_order_acq_rel);
        if (refs == SHOULD_BE_ON_FREELIST + 1) {
          add_knowing_refcount_is_zero(prevHead);
        }
      }

      return nullptr;
    }

    // 在没有争用的情况下遍历列表时很有用（例如，销毁剩余的节点）
    N* head_unsafe() const { return freeListHead.load(std::memory_order_relaxed); }

  private:
    inline void add_knowing_refcount_is_zero(N* node)
    {
      // 因为引用计数为零，并且一旦它为零，除了我们之外没有其他线程可以增加它（因为我们
      // 每次处理一个节点时只有一个线程在运行，即单线程情况），所以我们可以安全地改变
      // 节点的 next 指针。然而，一旦引用计数回到零以上，其他线程可能会增加它（这种情况
      // 发生在重度争用的情况下，当引用计数在 load 和 try_get 的节点的引用计数增加之间变为零，
      // 然后又回到非零值时，其他线程会完成引用计数的增加）——因此，如果将节点添加到实际
      // 列表的 CAS 操作失败，则减少引用计数，并将添加操作留给下一个将引用计数恢复到零的线程
      // （这可能是我们自己，因此循环）。
      auto head = freeListHead.load(std::memory_order_relaxed);
      while (true) {
        node->freeListNext.store(head, std::memory_order_relaxed);
        node->freeListRefs.store(1, std::memory_order_release);
        if (!freeListHead.compare_exchange_strong(head, node, std::memory_order_release, std::memory_order_relaxed)) {
          // 嗯，添加操作失败了，但我们只能在引用计数回到零时再尝试一次
          if (node->freeListRefs.fetch_add(SHOULD_BE_ON_FREELIST - 1, std::memory_order_release) == 1) {
            continue;
          }
        }
        return;
      }
    }

  private:
    // 实现方式类似于栈，但节点的顺序不重要（在争用情况下，节点可能会无序插入）
    std::atomic<N*> freeListHead;

  static const std::uint32_t REFS_MASK = 0x7FFFFFFF;
  static const std::uint32_t SHOULD_BE_ON_FREELIST = 0x80000000;

#if MCDBGQ_NOLOCKFREE_FREELIST
    debug::DebugMutex mutex;
#endif
  };


  ///////////////////////////
  // Block
  ///////////////////////////

  enum InnerQueueContext { implicit_context = 0, explicit_context = 1 };

  struct Block
  {
    Block()
      : next(nullptr), elementsCompletelyDequeued(0), freeListRefs(0), freeListNext(nullptr), shouldBeOnFreeList(false), dynamicallyAllocated(true)
    {
#if MCDBGQ_TRACKMEM
      owner = nullptr;
#endif
    }

    template<InnerQueueContext context>
    inline bool is_empty() const
    {
      if (context == explicit_context && BLOCK_SIZE <= EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD) {
        // 检查标志
        for (size_t i = 0; i < BLOCK_SIZE; ++i) {
          if (!emptyFlags[i].load(std::memory_order_relaxed)) {
            return false;
          }
        }

        // 啊，空的；确保我们在设置空标志之前完成了所有其他的内存操作
        std::atomic_thread_fence(std::memory_order_acquire);
        return true;
      }
      else {
        // Check counter
        if (elementsCompletelyDequeued.load(std::memory_order_relaxed) == BLOCK_SIZE) {
          std::atomic_thread_fence(std::memory_order_acquire);
          return true;
        }
        assert(elementsCompletelyDequeued.load(std::memory_order_relaxed) <= BLOCK_SIZE);
        return false;
      }
    }

    // 如果块现在为空，则返回 true（在显式上下文中不适用）
    template<InnerQueueContext context>
    inline bool set_empty(index_t i)
    {
      if (context == explicit_context && BLOCK_SIZE <= EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD) {
        // Set flag
        assert(!emptyFlags[BLOCK_SIZE - 1 - static_cast<size_t>(i & static_cast<index_t>(BLOCK_SIZE - 1))].load(std::memory_order_relaxed));
        emptyFlags[BLOCK_SIZE - 1 - static_cast<size_t>(i & static_cast<index_t>(BLOCK_SIZE - 1))].store(true, std::memory_order_release);
        return false;
      }
      else {
        // Increment counter
        auto prevVal = elementsCompletelyDequeued.fetch_add(1, std::memory_order_release);
        assert(prevVal < BLOCK_SIZE);
        return prevVal == BLOCK_SIZE - 1;
      }
    }

    // 将多个连续的项状态设置为“空”（假设没有环绕，并且计数 > 0）。
    // 如果块现在为空，则返回 true（在显式上下文中不适用）。
    template<InnerQueueContext context>
    inline bool set_many_empty(index_t i, size_t count)
    {
      if (context == explicit_context && BLOCK_SIZE <= EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD) {
        // Set flags
        std::atomic_thread_fence(std::memory_order_release);
        i = BLOCK_SIZE - 1 - static_cast<size_t>(i & static_cast<index_t>(BLOCK_SIZE - 1)) - count + 1;
        for (size_t j = 0; j != count; ++j) {
          assert(!emptyFlags[i + j].load(std::memory_order_relaxed));
          emptyFlags[i + j].store(true, std::memory_order_relaxed);
        }
        return false;
      }
      else {
        // Increment counter
        auto prevVal = elementsCompletelyDequeued.fetch_add(count, std::memory_order_release);
        assert(prevVal + count <= BLOCK_SIZE);
        return prevVal + count == BLOCK_SIZE;
      }
    }

    template<InnerQueueContext context>
    inline void set_all_empty()
    {
      if (context == explicit_context && BLOCK_SIZE <= EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD) {
        // Set all flags
        for (size_t i = 0; i != BLOCK_SIZE; ++i) {
          emptyFlags[i].store(true, std::memory_order_relaxed);
        }
      }
      else {
        // Reset counter
        elementsCompletelyDequeued.store(BLOCK_SIZE, std::memory_order_relaxed);
      }
    }

    template<InnerQueueContext context>
    inline void reset_empty()
    {
      if (context == explicit_context && BLOCK_SIZE <= EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD) {
        // Reset flags
        for (size_t i = 0; i != BLOCK_SIZE; ++i) {
          emptyFlags[i].store(false, std::memory_order_relaxed);
        }
      }
      else {
        // Reset counter
        elementsCompletelyDequeued.store(0, std::memory_order_relaxed);
      }
    }

    inline T* operator[](index_t idx) MOODYCAMEL_NOEXCEPT { return static_cast<T*>(static_cast<void*>(elements)) + static_cast<size_t>(idx & static_cast<index_t>(BLOCK_SIZE - 1)); }
    inline T const* operator[](index_t idx) const MOODYCAMEL_NOEXCEPT { return static_cast<T const*>(static_cast<void const*>(elements)) + static_cast<size_t>(idx & static_cast<index_t>(BLOCK_SIZE - 1)); }

  private:
    // 重要提示：这必须是 Block 中的第一个成员，以确保如果 T 依赖于 malloc 返回地址的对齐方式，
    // 该对齐方式将被保留。显然，clang 在某些情况下为 AVX 指令生成的代码会利用这一假设。
    // 理想情况下，我们还应该将 Block 对齐到 T 的对齐方式，以防 T 的对齐要求高于 malloc 的 16 字节对齐，
    // 但在跨平台中很难做到这一点。对此情况进行断言：
    static_assert(std::alignment_of<T>::value <= std::alignment_of<details::max_align_t>::value, "The queue does not support super-aligned types at this time");
    // 此外，我们还需要确保 Block 自身的对齐方式是 max_align_t 的倍数，否则在 Block 的末尾将不会添加适当的填充，
    // 从而使 Block 数组中的所有元素都能正确对齐（而不仅仅是第一个）。我们使用一个联合体来强制实现这一点。
    union {
      char elements[sizeof(T) * BLOCK_SIZE];
      details::max_align_t dummy;
    };
  public:
    Block* next;
    std::atomic<size_t> elementsCompletelyDequeued;
    std::atomic<bool> emptyFlags[BLOCK_SIZE <= EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD ? BLOCK_SIZE : 1];
  public:
    std::atomic<std::uint32_t> freeListRefs;
    std::atomic<Block*> freeListNext;
    std::atomic<bool> shouldBeOnFreeList;
    bool dynamicallyAllocated;     // 这个名字可能更好：'isNotPartOfInitialBlockPool'

#if MCDBGQ_TRACKMEM
    void* owner;
#endif
  };
  static_assert(std::alignment_of<Block>::value >= std::alignment_of<details::max_align_t>::value, "Internal error: Blocks must be at least as aligned as the type they are wrapping");


#if MCDBGQ_TRACKMEM
public:
  struct MemStats;
private:
#endif

  ///////////////////////////
  // Producer base
  ///////////////////////////

  struct ProducerBase : public details::ConcurrentQueueProducerTypelessBase
  {
    ProducerBase(ConcurrentQueue* parent_, bool isExplicit_) :
      tailIndex(0),
      headIndex(0),
      dequeueOptimisticCount(0),
      dequeueOvercommit(0),
      tailBlock(nullptr),
      isExplicit(isExplicit_),
      parent(parent_)
    {
    }

    virtual ~ProducerBase() { };

    template<typename U>
    inline bool dequeue(U& element)
    {
      if (isExplicit) {
        return static_cast<ExplicitProducer*>(this)->dequeue(element);
      }
      else {
        return static_cast<ImplicitProducer*>(this)->dequeue(element);
      }
    }

    template<typename It>
    inline size_t dequeue_bulk(It& itemFirst, size_t max)
    {
      if (isExplicit) {
        return static_cast<ExplicitProducer*>(this)->dequeue_bulk(itemFirst, max);
      }
      else {
        return static_cast<ImplicitProducer*>(this)->dequeue_bulk(itemFirst, max);
      }
    }

    inline ProducerBase* next_prod() const { return static_cast<ProducerBase*>(next); }

    inline size_t size_approx() const
    {
      auto tail = tailIndex.load(std::memory_order_relaxed);
      auto head = headIndex.load(std::memory_order_relaxed);
      return details::circular_less_than(head, tail) ? static_cast<size_t>(tail - head) : 0;
    }

    inline index_t getTail() const { return tailIndex.load(std::memory_order_relaxed); }
  protected:
    std::atomic<index_t> tailIndex;    // Where to enqueue to next
    std::atomic<index_t> headIndex;    // Where to dequeue from next

    std::atomic<index_t> dequeueOptimisticCount;
    std::atomic<index_t> dequeueOvercommit;

    Block* tailBlock;

  public:
    bool isExplicit;
    ConcurrentQueue* parent;

  protected:
#if MCDBGQ_TRACKMEM
    friend struct MemStats;
#endif
  };


  ///////////////////////////
  // Explicit queue
  ///////////////////////////

  struct ExplicitProducer : public ProducerBase
  {
    explicit ExplicitProducer(ConcurrentQueue* parent) :
      ProducerBase(parent, true),
      blockIndex(nullptr),
      pr_blockIndexSlotsUsed(0),
      pr_blockIndexSize(EXPLICIT_INITIAL_INDEX_SIZE >> 1),
      pr_blockIndexFront(0),
      pr_blockIndexEntries(nullptr),
      pr_blockIndexRaw(nullptr)
    {
      size_t poolBasedIndexSize = details::ceil_to_pow_2(parent->initialBlockPoolSize) >> 1;
      if (poolBasedIndexSize > pr_blockIndexSize) {
        pr_blockIndexSize = poolBasedIndexSize;
      }

      new_block_index(0);    // 这将创建一个具有当前条目数量两倍的索引，即 EXPLICIT_INITIAL_INDEX_SIZE

    }

    ~ExplicitProducer()
    {
      // 析构任何尚未出队的元素。
      // 由于我们在析构函数中，我们可以假设所有元素
      // 要么完全出队，要么完全未出队（没有半途而废的情况）。
      // 如果 tailBlock 不是空指针，则必须存在一个块索引
      // 首先找到部分出队的块（如果有的话）

      if (this->tailBlock != nullptr) {    // Note this means there must be a block index too
        // First find the block that's partially dequeued, if any
        Block* halfDequeuedBlock = nullptr;
        if ((this->headIndex.load(std::memory_order_relaxed) & static_cast<index_t>(BLOCK_SIZE - 1)) != 0) {
          // 头部不在块边界上，意味着某个块部分出队
          // （或者头块是尾块且已完全出队，但头部/尾部仍未在边界上）
          size_t i = (pr_blockIndexFront - pr_blockIndexSlotsUsed) & (pr_blockIndexSize - 1);
          while (details::circular_less_than<index_t>(pr_blockIndexEntries[i].base + BLOCK_SIZE, this->headIndex.load(std::memory_order_relaxed))) {
            i = (i + 1) & (pr_blockIndexSize - 1);
          }
          assert(details::circular_less_than<index_t>(pr_blockIndexEntries[i].base, this->headIndex.load(std::memory_order_relaxed)));
          halfDequeuedBlock = pr_blockIndexEntries[i].block;
        }

        // 从头块开始（注意第一次迭代时，循环的第一行给我们提供了头部）
        auto block = this->tailBlock;
        do {
          block = block->next;
          if (block->ConcurrentQueue::Block::template is_empty<explicit_context>()) {
            continue;
          }

          size_t i = 0;  // 块中的偏移
          if (block == halfDequeuedBlock) {
            i = static_cast<size_t>(this->headIndex.load(std::memory_order_relaxed) & static_cast<index_t>(BLOCK_SIZE - 1));
          }

          // 遍历块中的所有项目；如果这是尾块，当达到尾索引时需要停止
          auto lastValidIndex = (this->tailIndex.load(std::memory_order_relaxed) & static_cast<index_t>(BLOCK_SIZE - 1)) == 0 ? BLOCK_SIZE : static_cast<size_t>(this->tailIndex.load(std::memory_order_relaxed) & static_cast<index_t>(BLOCK_SIZE - 1));
          while (i != BLOCK_SIZE && (block != this->tailBlock || i != lastValidIndex)) {
            (*block)[i++]->~T();
          }
        } while (block != this->tailBlock);
      }

      // 销毁所有我们拥有的块
      if (this->tailBlock != nullptr) {
        auto block = this->tailBlock;
        do {
          auto nextBlock = block->next;
          if (block->dynamicallyAllocated) {
            destroy(block);
          }
          else {
            this->parent->add_block_to_free_list(block);
          }
          block = nextBlock;
        } while (block != this->tailBlock);
      }

      // 销毁块索引
      auto header = static_cast<BlockIndexHeader*>(pr_blockIndexRaw);
      while (header != nullptr) {
        auto prev = static_cast<BlockIndexHeader*>(header->prev);
        header->~BlockIndexHeader();
        (Traits::free)(header);
        header = prev;
      }
    }

    template<AllocationMode allocMode, typename U>
    inline bool enqueue(U&& element)
    {
      index_t currentTailIndex = this->tailIndex.load(std::memory_order_relaxed);
      index_t newTailIndex = 1 + currentTailIndex;
      if ((currentTailIndex & static_cast<index_t>(BLOCK_SIZE - 1)) == 0) {
        // 我们到达了一个块的末尾，开始一个新的块
        auto startBlock = this->tailBlock;
        auto originalBlockIndexSlotsUsed = pr_blockIndexSlotsUsed;
        if (this->tailBlock != nullptr && this->tailBlock->next->ConcurrentQueue::Block::template is_empty<explicit_context>()) {
          // 我们可以重用下一个块，它是空的！
          this->tailBlock = this->tailBlock->next;
          this->tailBlock->ConcurrentQueue::Block::template reset_empty<explicit_context>();

          // 我们将把块放到块索引中（由于我们先从中移除最后一个块，因此可以保证有空间 —— 除了移除再添加，我们可以直接覆盖）。
          // 请注意，这里必须有一个有效的块索引，因为即使在构造函数中分配失败，它也会在将第一个块添加到队列时重新尝试；由于存在这样的块，因此块索引必须成功分配。
        }
        else {
          // 我们看到的头部值大于或等于我们在此处看到的最后一个值（相对而言），并且小于等于其当前值。由于我们有最新的尾部，头部必须小于等于尾部。
          auto head = this->headIndex.load(std::memory_order_relaxed);
          assert(!details::circular_less_than<index_t>(currentTailIndex, head));
          if (!details::circular_less_than<index_t>(head, currentTailIndex + BLOCK_SIZE)
            || (MAX_SUBQUEUE_SIZE != details::const_numeric_max<size_t>::value && (MAX_SUBQUEUE_SIZE == 0 || MAX_SUBQUEUE_SIZE - BLOCK_SIZE < currentTailIndex - head))) {
             // 我们不能在另一个块中入队，因为没有足够的余地 —— 尾部可能在块填满之前超越头部！ （或者，如果第二部分条件为真，我们将超过大小限制。）
        return false;
            return false;
          }
           // 我们需要一个新的块；检查块索引是否有空间
          if (pr_blockIndexRaw == nullptr || pr_blockIndexSlotsUsed == pr_blockIndexSize) {
            // 嗯，圆形块索引已经满了 —— 我们需要分配一个新的索引。请注意，pr_blockIndexRaw 只能为 nullptr，如果初始分配在构造函数中失败的话。
            if (allocMode == CannotAlloc || !new_block_index(pr_blockIndexSlotsUsed)) {
              return false;
            }
          }

           // 在圆形链表中插入一个新块
          auto newBlock = this->parent->ConcurrentQueue::template requisition_block<allocMode>();
          if (newBlock == nullptr) {
            return false;
          }
#if MCDBGQ_TRACKMEM
          newBlock->owner = this;
#endif
          newBlock->ConcurrentQueue::Block::template reset_empty<explicit_context>();
          if (this->tailBlock == nullptr) {
            newBlock->next = newBlock;
          }
          else {
            newBlock->next = this->tailBlock->next;
            this->tailBlock->next = newBlock;
          }
          this->tailBlock = newBlock;
          ++pr_blockIndexSlotsUsed;
        }

        if (!MOODYCAMEL_NOEXCEPT_CTOR(T, U, new (nullptr) T(std::forward<U>(element)))) {
           // 构造函数可能抛出异常。在这种情况下，我们希望元素不出现在队列中（而不会损坏队列）：
          MOODYCAMEL_TRY {
            new ((*this->tailBlock)[currentTailIndex]) T(std::forward<U>(element));
          }
          MOODYCAMEL_CATCH (...) {
            // 撤销对当前块的更改，但保留新块以供下次使用
            pr_blockIndexSlotsUsed = originalBlockIndexSlotsUsed;
            this->tailBlock = startBlock == nullptr ? this->tailBlock : startBlock;
            MOODYCAMEL_RETHROW;
          }
        }
        else {
          (void)startBlock;
          (void)originalBlockIndexSlotsUsed;
        }

        // 将块添加到块索引中
        auto& entry = blockIndex.load(std::memory_order_relaxed)->entries[pr_blockIndexFront];
        entry.base = currentTailIndex;
        entry.block = this->tailBlock;
        blockIndex.load(std::memory_order_relaxed)->front.store(pr_blockIndexFront, std::memory_order_release);
        pr_blockIndexFront = (pr_blockIndexFront + 1) & (pr_blockIndexSize - 1);

        if (!MOODYCAMEL_NOEXCEPT_CTOR(T, U, new (nullptr) T(std::forward<U>(element)))) {
          this->tailIndex.store(newTailIndex, std::memory_order_release);
          return true;
        }
      }

      // 入队
      new ((*this->tailBlock)[currentTailIndex]) T(std::forward<U>(element));

      this->tailIndex.store(newTailIndex, std::memory_order_release);
      return true;
    }

    template<typename U>
    bool dequeue(U& element)
    {
      auto tail = this->tailIndex.load(std::memory_order_relaxed);
      auto overcommit = this->dequeueOvercommit.load(std::memory_order_relaxed);
      if (details::circular_less_than<index_t>(this->dequeueOptimisticCount.load(std::memory_order_relaxed) - overcommit, tail)) {
         // 可能有东西要出队，让我们试试看

        // 注意，这个if仅仅是为了提高性能，在队列为空且值最终一致的常见情况下
        // 我们可能会错误地进入这里。

        // 注意，无论overcommit和tail的值如何，它们都不会改变（除非我们改变它们）
        // 并且在这里的if内部时，它们的值必须与if条件被评估时相同。

        // 在此处插入一个获取屏障，以与下面增加dequeueOvercommit的释放同步。
        // 这确保了无论我们加载到overcommit中的值是什么，下面fetch_add中
        // 加载的dequeueOptimisticCount的值至少是那个值的最新值（因此至少一样大）。
        // 注意，我相信此处的编译器（信号）屏障是足够的，因为fetch_add的性质
        // （所有读-修改-写操作都保证在修改顺序中对最新值起作用），但不幸的是，
        // 仅使用C++11标准无法证明这一点是正确的。
        // 参见 http://stackoverflow.com/questions/18223161/what-are-the-c11-memory-ordering-guarantees-in-this-corner-case
        std::atomic_thread_fence(std::memory_order_acquire);

        // 增加乐观计数器，然后检查它是否超出了边界
        auto myDequeueCount = this->dequeueOptimisticCount.fetch_add(1, std::memory_order_relaxed);

        // 注意，由于dequeueOvercommit必须小于等于dequeueOptimisticCount（因为dequeueOvercommit只会在
        // dequeueOptimisticCount之后增加——这是在下面的`else`块中强制执行的），并且由于我们现在有一个
        // 至少与overcommit一样新的dequeueOptimisticCount版本（由于增加dequeueOvercommit的释放以及
        // 与其同步的获取），所以overcommit <= myDequeueCount。
        // 但是我们不能断言这一点，因为dequeueOptimisticCount和dequeueOvercommit都可能（独立地）溢出；
        // 在这种情况下，逻辑仍然成立，因为两者之间的差异得以保持。

        // 注意我们在这里重新加载tail以防它发生了变化；它将与之前的值相同或更大，因为
        // 这个加载是排在（发生在）上面的先前加载之后的。这由读取-读取一致性支持
        // （如标准中定义的），详见：http://en.cppreference.com/w/cpp/atomic/memory_order
        tail = this->tailIndex.load(std::memory_order_acquire);
        if ((details::likely)(details::circular_less_than<index_t>(myDequeueCount - overcommit, tail))) {
          // 保证至少有一个元素要出队！

          // 获取索引。注意，由于保证至少有一个元素，这
          // 将永远不会超过tail。我们需要在这里做一个获取-释放屏障，
          // 因为可能导致我们到达此点的条件是先前入队的元素（我们已经看到它的内存效应），
          // 但到我们增加时可能有人已将其增加，我们需要看到*那个*元素的内存效应，
          // 在这种情况下，该元素的内存效应在以更当前的条件首先增加它的线程上是可见的
          // （他们必须获取一个至少与最近一样新的tail）。
          auto index = this->headIndex.fetch_add(1, std::memory_order_acq_rel);


          // 确定元素在哪个块中

          auto localBlockIndex = blockIndex.load(std::memory_order_acquire);
          auto localBlockIndexHead = localBlockIndex->front.load(std::memory_order_acquire);

          // 我们在这里需要小心减法和除法，因为索引的环绕。
          // 当索引环绕时，我们在将其除以块大小时需要保持偏移的符号
           // （以便在所有情况下获得正确的有符号块计数偏移）：
          auto headBase = localBlockIndex->entries[localBlockIndexHead].base;
          auto blockBaseIndex = index & ~static_cast<index_t>(BLOCK_SIZE - 1);
          auto offset = static_cast<size_t>(static_cast<typename std::make_signed<index_t>::type>(blockBaseIndex - headBase) / BLOCK_SIZE);
          auto block = localBlockIndex->entries[(localBlockIndexHead + offset) & (localBlockIndex->size - 1)].block;

          // Dequeue
          auto& el = *((*block)[index]);
          if (!MOODYCAMEL_NOEXCEPT_ASSIGN(T, T&&, element = std::move(el))) {
            // 确保即使赋值操作抛出异常，元素仍能完全出队并被销毁
            struct Guard {
              Block* block;
              index_t index;
              // 析构函数，当 Guard 离开作用域时调用
              ~Guard()
              {
                (*block)[index]->~T();
                block->ConcurrentQueue::Block::template set_empty<explicit_context>(index);
              }
            } guard = { block, index };

            element = std::move(el);
          }
          else {
            element = std::move(el);
            el.~T();
            block->ConcurrentQueue::Block::template set_empty<explicit_context>(index);
          }

          return true;
        }
        else {
          // 如果实际上没有元素可以出队，则更新出队过度计数，使其最终与实际一致
          this->dequeueOvercommit.fetch_add(1, std::memory_order_release);    // Release so that the fetch_add on dequeueOptimisticCount is guaranteed to happen before this write
        }
      }

      return false;
    }

    template<AllocationMode allocMode, typename It>
    bool enqueue_bulk(It itemFirst, size_t count)
    {
      // 首先，我们需要确保有足够的空间来入队所有元素；
      // 这意味着需要预先分配块，并将它们放入块索引中（前提是所有分配都成功）。
      index_t startTailIndex = this->tailIndex.load(std::memory_order_relaxed);
      auto startBlock = this->tailBlock;
      auto originalBlockIndexFront = pr_blockIndexFront;
      auto originalBlockIndexSlotsUsed = pr_blockIndexSlotsUsed;

      Block* firstAllocatedBlock = nullptr;

       // 计算需要分配多少块，并进行分配
      size_t blockBaseDiff = ((startTailIndex + count - 1) & ~static_cast<index_t>(BLOCK_SIZE - 1)) - ((startTailIndex - 1) & ~static_cast<index_t>(BLOCK_SIZE - 1));
      index_t currentTailIndex = (startTailIndex - 1) & ~static_cast<index_t>(BLOCK_SIZE - 1);
      if (blockBaseDiff > 0) {
        // 尽可能从现有的块链表中分配块
        while (blockBaseDiff > 0 && this->tailBlock != nullptr && this->tailBlock->next != firstAllocatedBlock && this->tailBlock->next->ConcurrentQueue::Block::template is_empty<explicit_context>()) {
          blockBaseDiff -= static_cast<index_t>(BLOCK_SIZE);
          currentTailIndex += static_cast<index_t>(BLOCK_SIZE);

          this->tailBlock = this->tailBlock->next;
          firstAllocatedBlock = firstAllocatedBlock == nullptr ? this->tailBlock : firstAllocatedBlock;

          auto& entry = blockIndex.load(std::memory_order_relaxed)->entries[pr_blockIndexFront];
          entry.base = currentTailIndex;
          entry.block = this->tailBlock;
          pr_blockIndexFront = (pr_blockIndexFront + 1) & (pr_blockIndexSize - 1);
        }

        // 如果需要，继续从块池中分配新的块
        while (blockBaseDiff > 0) {
          blockBaseDiff -= static_cast<index_t>(BLOCK_SIZE);
          currentTailIndex += static_cast<index_t>(BLOCK_SIZE);

          auto head = this->headIndex.load(std::memory_order_relaxed);
          assert(!details::circular_less_than<index_t>(currentTailIndex, head));
          bool full = !details::circular_less_than<index_t>(head, currentTailIndex + BLOCK_SIZE) || (MAX_SUBQUEUE_SIZE != details::const_numeric_max<size_t>::value && (MAX_SUBQUEUE_SIZE == 0 || MAX_SUBQUEUE_SIZE - BLOCK_SIZE < currentTailIndex - head));
          if (pr_blockIndexRaw == nullptr || pr_blockIndexSlotsUsed == pr_blockIndexSize || full) {
            if (allocMode == CannotAlloc || full || !new_block_index(originalBlockIndexSlotsUsed)) {
              // Failed to allocate, undo changes (but keep injected blocks)
              pr_blockIndexFront = originalBlockIndexFront;
              pr_blockIndexSlotsUsed = originalBlockIndexSlotsUsed;
              this->tailBlock = startBlock == nullptr ? firstAllocatedBlock : startBlock;
              return false;
            }
            // pr_blockIndexFront 在 new_block_index 内部被更新，因此我们也需要更新备用值（因为即使后来失败，我们仍然保留新的索引）
            originalBlockIndexFront = originalBlockIndexSlotsUsed;
          }

          // 在循环链表中插入新块
          auto newBlock = this->parent->ConcurrentQueue::template requisition_block<allocMode>();
          if (newBlock == nullptr) {
            pr_blockIndexFront = originalBlockIndexFront;
            pr_blockIndexSlotsUsed = originalBlockIndexSlotsUsed;
            this->tailBlock = startBlock == nullptr ? firstAllocatedBlock : startBlock;
            return false;
          }

#if MCDBGQ_TRACKMEM
          newBlock->owner = this;
#endif
          newBlock->ConcurrentQueue::Block::template set_all_empty<explicit_context>();
          if (this->tailBlock == nullptr) {
            newBlock->next = newBlock;
          }
          else {
            newBlock->next = this->tailBlock->next;
            this->tailBlock->next = newBlock;
          }
          this->tailBlock = newBlock;
          firstAllocatedBlock = firstAllocatedBlock == nullptr ? this->tailBlock : firstAllocatedBlock;

          ++pr_blockIndexSlotsUsed;

          auto& entry = blockIndex.load(std::memory_order_relaxed)->entries[pr_blockIndexFront];
          entry.base = currentTailIndex;
          entry.block = this->tailBlock;
          pr_blockIndexFront = (pr_blockIndexFront + 1) & (pr_blockIndexSize - 1);
        }

        // Excellent, all allocations succeeded. Reset each block's emptiness before we fill them up, and
        // publish the new block index front
        auto block = firstAllocatedBlock;
        while (true) {
          block->ConcurrentQueue::Block::template reset_empty<explicit_context>();
          if (block == this->tailBlock) {
            break;
          }
          block = block->next;
        }

        if (MOODYCAMEL_NOEXCEPT_CTOR(T, decltype(*itemFirst), new (nullptr) T(details::deref_noexcept(itemFirst)))) {
          blockIndex.load(std::memory_order_relaxed)->front.store((pr_blockIndexFront - 1) & (pr_blockIndexSize - 1), std::memory_order_release);
        }
      }

      // 一次入队一个块
      index_t newTailIndex = startTailIndex + static_cast<index_t>(count);
      currentTailIndex = startTailIndex;
      auto endBlock = this->tailBlock;
      this->tailBlock = startBlock;
      assert((startTailIndex & static_cast<index_t>(BLOCK_SIZE - 1)) != 0 || firstAllocatedBlock != nullptr || count == 0);
      if ((startTailIndex & static_cast<index_t>(BLOCK_SIZE - 1)) == 0 && firstAllocatedBlock != nullptr) {
        this->tailBlock = firstAllocatedBlock;
      }
      while (true) {
        auto stopIndex = (currentTailIndex & ~static_cast<index_t>(BLOCK_SIZE - 1)) + static_cast<index_t>(BLOCK_SIZE);
        if (details::circular_less_than<index_t>(newTailIndex, stopIndex)) {
          stopIndex = newTailIndex;
        }
        if (MOODYCAMEL_NOEXCEPT_CTOR(T, decltype(*itemFirst), new (nullptr) T(details::deref_noexcept(itemFirst)))) {
          while (currentTailIndex != stopIndex) {
            new ((*this->tailBlock)[currentTailIndex++]) T(*itemFirst++);
          }
        }
        else {
          MOODYCAMEL_TRY {
            while (currentTailIndex != stopIndex) {
              // 即使存在移动构造函数，也必须使用拷贝构造函数
              // 因为如果发生异常，我们可能需要进行恢复。
              // 对于下一行模板化代码很抱歉，但这是唯一能在编译时禁用移动构造的方式
              // 这很重要，因为一个类型可能只定义了一个（noexcept）移动构造函数，
              // 因此即使它们在一个永远不会被执行的 if 分支中，调用拷贝构造函数也无法编译。
              new ((*this->tailBlock)[currentTailIndex]) T(details::nomove_if<(bool)!MOODYCAMEL_NOEXCEPT_CTOR(T, decltype(*itemFirst), new (nullptr) T(details::deref_noexcept(itemFirst)))>::eval(*itemFirst));
              ++currentTailIndex;
              ++itemFirst;
            }
          }
          MOODYCAMEL_CATCH (...) {
            // 哎呀，抛出了异常——销毁已排队的元素
            // 并恢复整个批量操作（不过我们会保留任何已分配的块以备后用）。
            auto constructedStopIndex = currentTailIndex;
            auto lastBlockEnqueued = this->tailBlock;

            pr_blockIndexFront = originalBlockIndexFront;
            pr_blockIndexSlotsUsed = originalBlockIndexSlotsUsed;
            this->tailBlock = startBlock == nullptr ? firstAllocatedBlock : startBlock;

            if (!details::is_trivially_destructible<T>::value) {
              auto block = startBlock;
              if ((startTailIndex & static_cast<index_t>(BLOCK_SIZE - 1)) == 0) {
                block = firstAllocatedBlock;
              }
              currentTailIndex = startTailIndex;
              while (true) {
                stopIndex = (currentTailIndex & ~static_cast<index_t>(BLOCK_SIZE - 1)) + static_cast<index_t>(BLOCK_SIZE);
                if (details::circular_less_than<index_t>(constructedStopIndex, stopIndex)) {
                  stopIndex = constructedStopIndex;
                }
                while (currentTailIndex != stopIndex) {
                  (*block)[currentTailIndex++]->~T();
                }
                if (block == lastBlockEnqueued) {
                  break;
                }
                block = block->next;
              }
            }
            MOODYCAMEL_RETHROW;
          }
        }

        if (this->tailBlock == endBlock) {
          assert(currentTailIndex == newTailIndex);
          break;
        }
        this->tailBlock = this->tailBlock->next;
      }

      if (!MOODYCAMEL_NOEXCEPT_CTOR(T, decltype(*itemFirst), new (nullptr) T(details::deref_noexcept(itemFirst))) && firstAllocatedBlock != nullptr) {
        blockIndex.load(std::memory_order_relaxed)->front.store((pr_blockIndexFront - 1) & (pr_blockIndexSize - 1), std::memory_order_release);
      }

      this->tailIndex.store(newTailIndex, std::memory_order_release);
      return true;
    }

    template<typename It>
    size_t dequeue_bulk(It& itemFirst, size_t max)
    {
      auto tail = this->tailIndex.load(std::memory_order_relaxed);
      auto overcommit = this->dequeueOvercommit.load(std::memory_order_relaxed);
      auto desiredCount = static_cast<size_t>(tail - (this->dequeueOptimisticCount.load(std::memory_order_relaxed) - overcommit));
      if (details::circular_less_than<size_t>(0, desiredCount)) {
        desiredCount = desiredCount < max ? desiredCount : max;
        std::atomic_thread_fence(std::memory_order_acquire);

        auto myDequeueCount = this->dequeueOptimisticCount.fetch_add(desiredCount, std::memory_order_relaxed);;

        tail = this->tailIndex.load(std::memory_order_acquire);
        auto actualCount = static_cast<size_t>(tail - (myDequeueCount - overcommit));
        if (details::circular_less_than<size_t>(0, actualCount)) {
          actualCount = desiredCount < actualCount ? desiredCount : actualCount;
          if (actualCount < desiredCount) {
            this->dequeueOvercommit.fetch_add(desiredCount - actualCount, std::memory_order_release);
          }

          // 获取第一个索引。注意，由于保证至少有 actualCount 个元素，这
          // 不会超过 tail。
          auto firstIndex = this->headIndex.fetch_add(actualCount, std::memory_order_acq_rel);

          // 确定第一个元素所在的块
          auto localBlockIndex = blockIndex.load(std::memory_order_acquire);
          auto localBlockIndexHead = localBlockIndex->front.load(std::memory_order_acquire);

          auto headBase = localBlockIndex->entries[localBlockIndexHead].base;
          auto firstBlockBaseIndex = firstIndex & ~static_cast<index_t>(BLOCK_SIZE - 1);
          auto offset = static_cast<size_t>(static_cast<typename std::make_signed<index_t>::type>(firstBlockBaseIndex - headBase) / BLOCK_SIZE);
          auto indexIndex = (localBlockIndexHead + offset) & (localBlockIndex->size - 1);

          // Iterate the blocks and dequeue
          auto index = firstIndex;
          do {
            auto firstIndexInBlock = index;
            auto endIndex = (index & ~static_cast<index_t>(BLOCK_SIZE - 1)) + static_cast<index_t>(BLOCK_SIZE);
            endIndex = details::circular_less_than<index_t>(firstIndex + static_cast<index_t>(actualCount), endIndex) ? firstIndex + static_cast<index_t>(actualCount) : endIndex;
            auto block = localBlockIndex->entries[indexIndex].block;
            if (MOODYCAMEL_NOEXCEPT_ASSIGN(T, T&&, details::deref_noexcept(itemFirst) = std::move((*(*block)[index])))) {
              while (index != endIndex) {
                auto& el = *((*block)[index]);
                *itemFirst++ = std::move(el);
                el.~T();
                ++index;
              }
            }
            else {
              MOODYCAMEL_TRY {
                while (index != endIndex) {
                  auto& el = *((*block)[index]);
                  *itemFirst = std::move(el);
                  ++itemFirst;
                  el.~T();
                  ++index;
                }
              }
              MOODYCAMEL_CATCH (...) {
                // 由于已为时已晚，无法恢复出队操作，但我们可以确保所有已出队的对象
                // 被正确销毁，并且块索引（以及空闲计数）在传播异常之前被正确更新。
                do {
                  block = localBlockIndex->entries[indexIndex].block;
                  while (index != endIndex) {
                    (*block)[index++]->~T();
                  }
                  block->ConcurrentQueue::Block::template set_many_empty<explicit_context>(firstIndexInBlock, static_cast<size_t>(endIndex - firstIndexInBlock));
                  indexIndex = (indexIndex + 1) & (localBlockIndex->size - 1);

                  firstIndexInBlock = index;
                  endIndex = (index & ~static_cast<index_t>(BLOCK_SIZE - 1)) + static_cast<index_t>(BLOCK_SIZE);
                  endIndex = details::circular_less_than<index_t>(firstIndex + static_cast<index_t>(actualCount), endIndex) ? firstIndex + static_cast<index_t>(actualCount) : endIndex;
                } while (index != firstIndex + actualCount);

                MOODYCAMEL_RETHROW;
              }
            }
            block->ConcurrentQueue::Block::template set_many_empty<explicit_context>(firstIndexInBlock, static_cast<size_t>(endIndex - firstIndexInBlock));
            indexIndex = (indexIndex + 1) & (localBlockIndex->size - 1);
          } while (index != firstIndex + actualCount);

          return actualCount;
        }
        else {
          // 实际上没有任何东西可以出队；使有效的出队计数最终保持一致
          this->dequeueOvercommit.fetch_add(desiredCount, std::memory_order_release);
        }
      }

      return 0;
    }

  private:
    struct BlockIndexEntry
    {
      index_t base;
      Block* block;
    };

    struct BlockIndexHeader
    {
      size_t size;
      std::atomic<size_t> front;    // 当前槽（而不是下一个槽，如 pr_blockIndexFront）


      BlockIndexEntry* entries;
      void* prev;
    };


    bool new_block_index(size_t numberOfFilledSlotsToExpose)
    {
      auto prevBlockSizeMask = pr_blockIndexSize - 1;

      // Create the new block
      pr_blockIndexSize <<= 1;
      auto newRawPtr = static_cast<char*>((Traits::malloc)(sizeof(BlockIndexHeader) + std::alignment_of<BlockIndexEntry>::value - 1 + sizeof(BlockIndexEntry) * pr_blockIndexSize));
      if (newRawPtr == nullptr) {
        pr_blockIndexSize >>= 1;    / 重置以允许优雅地重试
        return false;
      }

      auto newBlockIndexEntries = reinterpret_cast<BlockIndexEntry*>(details::align_for<BlockIndexEntry>(newRawPtr + sizeof(BlockIndexHeader)));

      // 复制所有旧的索引，如果有的话
      size_t j = 0;
      if (pr_blockIndexSlotsUsed != 0) {
        auto i = (pr_blockIndexFront - pr_blockIndexSlotsUsed) & prevBlockSizeMask;
        do {
          newBlockIndexEntries[j++] = pr_blockIndexEntries[i];
          i = (i + 1) & prevBlockSizeMask;
        } while (i != pr_blockIndexFront);
      }

      // Update everything
      auto header = new (newRawPtr) BlockIndexHeader;
      header->size = pr_blockIndexSize;
      header->front.store(numberOfFilledSlotsToExpose - 1, std::memory_order_relaxed);
      header->entries = newBlockIndexEntries;
      header->prev = pr_blockIndexRaw;    // 将新块链接到旧块，以便后续可以释放旧块
      
      // 更新指向新块索引的指针
      pr_blockIndexFront = j;
      pr_blockIndexEntries = newBlockIndexEntries;
      pr_blockIndexRaw = newRawPtr;
      blockIndex.store(header, std::memory_order_release);

      return true;
    }

  private:
    std::atomic<BlockIndexHeader*> blockIndex;

    // 仅供生产者使用 —— 消费者必须使用由 blockIndex 引用的那些
    size_t pr_blockIndexSlotsUsed;
    size_t pr_blockIndexSize;
    size_t pr_blockIndexFront;    // 下一个槽位（而非当前槽位）
    BlockIndexEntry* pr_blockIndexEntries;
    void* pr_blockIndexRaw;

#ifdef MOODYCAMEL_QUEUE_INTERNAL_DEBUG
  public:
    ExplicitProducer* nextExplicitProducer;
  private:
#endif

#if MCDBGQ_TRACKMEM
    friend struct MemStats;
#endif
  };


  //////////////////////////////////
  // Implicit queue
  //////////////////////////////////

  struct ImplicitProducer : public ProducerBase
  {
    ImplicitProducer(ConcurrentQueue* parent) :
      ProducerBase(parent, false),
      nextBlockIndexCapacity(IMPLICIT_INITIAL_INDEX_SIZE),
      blockIndex(nullptr)
    {
      new_block_index();
    }

    ~ImplicitProducer()
    {
      // 请注意，由于我们在析构函数中，我们可以假设所有的入队/出队操作已经完成；
      // 这意味着所有未出队的元素都被连续地放置在相邻的块中，
      // 并且只有第一个和最后一个剩余的块可能是部分空的（所有其他剩余的块必须是完全满的）。

#ifdef MOODYCAMEL_CPP11_THREAD_LOCAL_SUPPORTED
      
      // 注销我们自己以便接收线程终止通知
      if (!this->inactive.load(std::memory_order_relaxed)) {
        details::ThreadExitNotifier::unsubscribe(&threadExitListener);
      }
#endif

      // 销毁所有剩余的元素！
      auto tail = this->tailIndex.load(std::memory_order_relaxed);
      auto index = this->headIndex.load(std::memory_order_relaxed);
      Block* block = nullptr;
      assert(index == tail || details::circular_less_than(index, tail));
      bool forceFreeLastBlock = index != tail;    // If we enter the loop, then the last (tail) block will not be freed
      while (index != tail) {
        if ((index & static_cast<index_t>(BLOCK_SIZE - 1)) == 0 || block == nullptr) {
          if (block != nullptr) {
            // Free the old block
            this->parent->add_block_to_free_list(block);
          }

          block = get_block_index_entry_for_index(index)->value.load(std::memory_order_relaxed);
        }

        ((*block)[index])->~T();
        ++index;
      }
      // 即使队列为空，仍然会有一个块不在空闲列表中
      // （除非头索引已经到达块的末尾，在这种情况下尾部将准备创建一个新的块）。
      if (this->tailBlock != nullptr && (forceFreeLastBlock || (tail & static_cast<index_t>(BLOCK_SIZE - 1)) != 0)) {
        this->parent->add_block_to_free_list(this->tailBlock);
      }

      //销毁块索引
      auto localBlockIndex = blockIndex.load(std::memory_order_relaxed);
      if (localBlockIndex != nullptr) {
        for (size_t i = 0; i != localBlockIndex->capacity; ++i) {
          localBlockIndex->index[i]->~BlockIndexEntry();
        }
        do {
          auto prev = localBlockIndex->prev;
          localBlockIndex->~BlockIndexHeader();
          (Traits::free)(localBlockIndex);
          localBlockIndex = prev;
        } while (localBlockIndex != nullptr);
      }
    }

    template<AllocationMode allocMode, typename U>
    inline bool enqueue(U&& element)
    {
      index_t currentTailIndex = this->tailIndex.load(std::memory_order_relaxed);
      index_t newTailIndex = 1 + currentTailIndex;
      if ((currentTailIndex & static_cast<index_t>(BLOCK_SIZE - 1)) == 0) {
        // 我们到达了一个块的末尾，开始一个新的块
        auto head = this->headIndex.load(std::memory_order_relaxed);
        assert(!details::circular_less_than<index_t>(currentTailIndex, head));
        if (!details::circular_less_than<index_t>(head, currentTailIndex + BLOCK_SIZE) || (MAX_SUBQUEUE_SIZE != details::const_numeric_max<size_t>::value && (MAX_SUBQUEUE_SIZE == 0 || MAX_SUBQUEUE_SIZE - BLOCK_SIZE < currentTailIndex - head))) {
          return false;
        }
#if MCDBGQ_NOLOCKFREE_IMPLICITPRODBLOCKINDEX
        debug::DebugLock lock(mutex);
#endif
        // Find out where we'll be inserting this block in the block index
        BlockIndexEntry* idxEntry;
        if (!insert_block_index_entry<allocMode>(idxEntry, currentTailIndex)) {
          return false;
        }

        // Get ahold of a new block
        auto newBlock = this->parent->ConcurrentQueue::template requisition_block<allocMode>();
        if (newBlock == nullptr) {
          rewind_block_index_tail();
          idxEntry->value.store(nullptr, std::memory_order_relaxed);
          return false;
        }
#if MCDBGQ_TRACKMEM
        newBlock->owner = this;
#endif
        newBlock->ConcurrentQueue::Block::template reset_empty<implicit_context>();

        if (!MOODYCAMEL_NOEXCEPT_CTOR(T, U, new (nullptr) T(std::forward<U>(element)))) {
          // 可能会抛出异常，尝试现在插入数据，先于我们发布新块的事实
          MOODYCAMEL_TRY {
            new ((*newBlock)[currentTailIndex]) T(std::forward<U>(element));
          }
          MOODYCAMEL_CATCH (...) {
            rewind_block_index_tail();
            idxEntry->value.store(nullptr, std::memory_order_relaxed);
            this->parent->add_block_to_free_list(newBlock);
            MOODYCAMEL_RETHROW;
          }
        }

        // Insert the new block into the index
        idxEntry->value.store(newBlock, std::memory_order_relaxed);

        this->tailBlock = newBlock;

        if (!MOODYCAMEL_NOEXCEPT_CTOR(T, U, new (nullptr) T(std::forward<U>(element)))) {
          this->tailIndex.store(newTailIndex, std::memory_order_release);
          return true;
        }
      }

      // Enqueue
      new ((*this->tailBlock)[currentTailIndex]) T(std::forward<U>(element));

      this->tailIndex.store(newTailIndex, std::memory_order_release);
      return true;
    }

    template<typename U>
    bool dequeue(U& element)
    {
      // 请参阅 ExplicitProducer::dequeue 以了解原因和解释
      index_t tail = this->tailIndex.load(std::memory_order_relaxed);
      index_t overcommit = this->dequeueOvercommit.load(std::memory_order_relaxed);
      if (details::circular_less_than<index_t>(this->dequeueOptimisticCount.load(std::memory_order_relaxed) - overcommit, tail)) {
        std::atomic_thread_fence(std::memory_order_acquire);

        index_t myDequeueCount = this->dequeueOptimisticCount.fetch_add(1, std::memory_order_relaxed);
        tail = this->tailIndex.load(std::memory_order_acquire);
        if ((details::likely)(details::circular_less_than<index_t>(myDequeueCount - overcommit, tail))) {
          index_t index = this->headIndex.fetch_add(1, std::memory_order_acq_rel);

          // 确定元素所在的块
          auto entry = get_block_index_entry_for_index(index);

          // Dequeue
          auto block = entry->value.load(std::memory_order_relaxed);
          auto& el = *((*block)[index]);

          if (!MOODYCAMEL_NOEXCEPT_ASSIGN(T, T&&, element = std::move(el))) {
#if MCDBGQ_NOLOCKFREE_IMPLICITPRODBLOCKINDEX
            // 注意：每次 dequeue 时都获取互斥锁，而不是仅在释放块时获取，这非常不理想，但毕竟这只是纯调试代码。
            debug::DebugLock lock(producer->mutex);
#endif
            struct Guard {
              Block* block;
              index_t index;
              BlockIndexEntry* entry;
              ConcurrentQueue* parent;

              ~Guard()
              {
                (*block)[index]->~T();
                if (block->ConcurrentQueue::Block::template set_empty<implicit_context>(index)) {
                  entry->value.store(nullptr, std::memory_order_relaxed);
                  parent->add_block_to_free_list(block);
                }
              }
            } guard = { block, index, entry, this->parent };

            element = std::move(el);
          }
          else {
            element = std::move(el);
            el.~T();

            if (block->ConcurrentQueue::Block::template set_empty<implicit_context>(index)) {
              {
#if MCDBGQ_NOLOCKFREE_IMPLICITPRODBLOCKINDEX
                debug::DebugLock lock(mutex);
#endif
                // Add the block back into the global free pool (and remove from block index)
                entry->value.store(nullptr, std::memory_order_relaxed);
              }
              this->parent->add_block_to_free_list(block);    // releases the above store
            }
          }

          return true;
        }
        else {
          this->dequeueOvercommit.fetch_add(1, std::memory_order_release);
        }
      }

      return false;
    }

    template<AllocationMode allocMode, typename It>
    bool enqueue_bulk(It itemFirst, size_t count)
    {
      // 首先，我们需要确保有足够的空间来入队所有元素；
      // 这意味着需要预先分配块，并将它们放入块索引中（但只有在所有分配成功的情况下才这么做）。

      // 请注意，我们开始时的 tailBlock 可能不再由我们拥有；
      // 这种情况发生在当它被填满至顶部（将 tailIndex 设置为尚未分配的下一个块的第一个索引），
      // 然后在我们再次入队之前被完全出队（将其放入空闲列表中）。

      index_t startTailIndex = this->tailIndex.load(std::memory_order_relaxed);
      auto startBlock = this->tailBlock;
      Block* firstAllocatedBlock = nullptr;
      auto endBlock = this->tailBlock;

      // 确定我们需要分配多少块，并进行分配
      size_t blockBaseDiff = ((startTailIndex + count - 1) & ~static_cast<index_t>(BLOCK_SIZE - 1)) - ((startTailIndex - 1) & ~static_cast<index_t>(BLOCK_SIZE - 1));
      index_t currentTailIndex = (startTailIndex - 1) & ~static_cast<index_t>(BLOCK_SIZE - 1);
      if (blockBaseDiff > 0) {
#if MCDBGQ_NOLOCKFREE_IMPLICITPRODBLOCKINDEX
        debug::DebugLock lock(mutex);
#endif
        do {
          blockBaseDiff -= static_cast<index_t>(BLOCK_SIZE);
          currentTailIndex += static_cast<index_t>(BLOCK_SIZE);

          // 确定我们将在块索引中插入此块的位置
          BlockIndexEntry* idxEntry = nullptr;  // 这里的初始化是不必要的，但编译器并不总能判断出来
          Block* newBlock;
          bool indexInserted = false;
          auto head = this->headIndex.load(std::memory_order_relaxed);
          assert(!details::circular_less_than<index_t>(currentTailIndex, head));
          bool full = !details::circular_less_than<index_t>(head, currentTailIndex + BLOCK_SIZE) || (MAX_SUBQUEUE_SIZE != details::const_numeric_max<size_t>::value && (MAX_SUBQUEUE_SIZE == 0 || MAX_SUBQUEUE_SIZE - BLOCK_SIZE < currentTailIndex - head));
          if (full || !(indexInserted = insert_block_index_entry<allocMode>(idxEntry, currentTailIndex)) || (newBlock = this->parent->ConcurrentQueue::template requisition_block<allocMode>()) == nullptr) {
            // 索引分配或块分配失败；撤销目前为止已完成的其他分配
            // 和索引插入操作
            if (indexInserted) {
              rewind_block_index_tail();
              idxEntry->value.store(nullptr, std::memory_order_relaxed);
            }
            currentTailIndex = (startTailIndex - 1) & ~static_cast<index_t>(BLOCK_SIZE - 1);
            for (auto block = firstAllocatedBlock; block != nullptr; block = block->next) {
              currentTailIndex += static_cast<index_t>(BLOCK_SIZE);
              idxEntry = get_block_index_entry_for_index(currentTailIndex);
              idxEntry->value.store(nullptr, std::memory_order_relaxed);
              rewind_block_index_tail();
            }
            this->parent->add_blocks_to_free_list(firstAllocatedBlock);
            this->tailBlock = startBlock;

            return false;
          }

#if MCDBGQ_TRACKMEM
          newBlock->owner = this;
#endif
          newBlock->ConcurrentQueue::Block::template reset_empty<implicit_context>();
          newBlock->next = nullptr;

          // 将新块插入到索引中
          idxEntry->value.store(newBlock, std::memory_order_relaxed);

          // 存储块链，以便在后续分配失败时可以撤销，
          // 并且在实际入队时可以找到这些块
          if ((startTailIndex & static_cast<index_t>(BLOCK_SIZE - 1)) != 0 || firstAllocatedBlock != nullptr) {
            assert(this->tailBlock != nullptr);
            this->tailBlock->next = newBlock;
          }
          this->tailBlock = newBlock;
          endBlock = newBlock;
          firstAllocatedBlock = firstAllocatedBlock == nullptr ? newBlock : firstAllocatedBlock;
        } while (blockBaseDiff > 0);
      }

      // Enqueue, one block at a time
      index_t newTailIndex = startTailIndex + static_cast<index_t>(count);
      currentTailIndex = startTailIndex;
      this->tailBlock = startBlock;
      assert((startTailIndex & static_cast<index_t>(BLOCK_SIZE - 1)) != 0 || firstAllocatedBlock != nullptr || count == 0);
      if ((startTailIndex & static_cast<index_t>(BLOCK_SIZE - 1)) == 0 && firstAllocatedBlock != nullptr) {
        this->tailBlock = firstAllocatedBlock;
      }
      while (true) {
        auto stopIndex = (currentTailIndex & ~static_cast<index_t>(BLOCK_SIZE - 1)) + static_cast<index_t>(BLOCK_SIZE);
        if (details::circular_less_than<index_t>(newTailIndex, stopIndex)) {
          stopIndex = newTailIndex;
        }
        if (MOODYCAMEL_NOEXCEPT_CTOR(T, decltype(*itemFirst), new (nullptr) T(details::deref_noexcept(itemFirst)))) {
          while (currentTailIndex != stopIndex) {
            new ((*this->tailBlock)[currentTailIndex++]) T(*itemFirst++);
          }
        }
        else {
          MOODYCAMEL_TRY {
            while (currentTailIndex != stopIndex) {
              new ((*this->tailBlock)[currentTailIndex]) T(details::nomove_if<(bool)!MOODYCAMEL_NOEXCEPT_CTOR(T, decltype(*itemFirst), new (nullptr) T(details::deref_noexcept(itemFirst)))>::eval(*itemFirst));
              ++currentTailIndex;
              ++itemFirst;
            }
          }
          MOODYCAMEL_CATCH (...) {
            auto constructedStopIndex = currentTailIndex;
            auto lastBlockEnqueued = this->tailBlock;

            if (!details::is_trivially_destructible<T>::value) {
              auto block = startBlock;
              if ((startTailIndex & static_cast<index_t>(BLOCK_SIZE - 1)) == 0) {
                block = firstAllocatedBlock;
              }
              currentTailIndex = startTailIndex;
              while (true) {
                stopIndex = (currentTailIndex & ~static_cast<index_t>(BLOCK_SIZE - 1)) + static_cast<index_t>(BLOCK_SIZE);
                if (details::circular_less_than<index_t>(constructedStopIndex, stopIndex)) {
                  stopIndex = constructedStopIndex;
                }
                while (currentTailIndex != stopIndex) {
                  (*block)[currentTailIndex++]->~T();
                }
                if (block == lastBlockEnqueued) {
                  break;
                }
                block = block->next;
              }
            }

            currentTailIndex = (startTailIndex - 1) & ~static_cast<index_t>(BLOCK_SIZE - 1);
            for (auto block = firstAllocatedBlock; block != nullptr; block = block->next) {
              currentTailIndex += static_cast<index_t>(BLOCK_SIZE);
              auto idxEntry = get_block_index_entry_for_index(currentTailIndex);
              idxEntry->value.store(nullptr, std::memory_order_relaxed);
              rewind_block_index_tail();
            }
            this->parent->add_blocks_to_free_list(firstAllocatedBlock);
            this->tailBlock = startBlock;
            MOODYCAMEL_RETHROW;
          }
        }

        if (this->tailBlock == endBlock) {
          assert(currentTailIndex == newTailIndex);
          break;
        }
        this->tailBlock = this->tailBlock->next;
      }
      this->tailIndex.store(newTailIndex, std::memory_order_release);
      return true;
    }

    template<typename It>
    size_t dequeue_bulk(It& itemFirst, size_t max)
    {
      auto tail = this->tailIndex.load(std::memory_order_relaxed);
      auto overcommit = this->dequeueOvercommit.load(std::memory_order_relaxed);
      auto desiredCount = static_cast<size_t>(tail - (this->dequeueOptimisticCount.load(std::memory_order_relaxed) - overcommit));
      if (details::circular_less_than<size_t>(0, desiredCount)) {
        desiredCount = desiredCount < max ? desiredCount : max;
        std::atomic_thread_fence(std::memory_order_acquire);

        auto myDequeueCount = this->dequeueOptimisticCount.fetch_add(desiredCount, std::memory_order_relaxed);

        tail = this->tailIndex.load(std::memory_order_acquire);
        auto actualCount = static_cast<size_t>(tail - (myDequeueCount - overcommit));
        if (details::circular_less_than<size_t>(0, actualCount)) {
          actualCount = desiredCount < actualCount ? desiredCount : actualCount;
          if (actualCount < desiredCount) {
            this->dequeueOvercommit.fetch_add(desiredCount - actualCount, std::memory_order_release);
          }

          // 获取第一个索引。请注意，由于保证至少有 actualCount 个元素，这个值永远不会超过 tail。

          auto firstIndex = this->headIndex.fetch_add(actualCount, std::memory_order_acq_rel);

          // Iterate the blocks and dequeue
          auto index = firstIndex;
          BlockIndexHeader* localBlockIndex;
          auto indexIndex = get_block_index_index_for_index(index, localBlockIndex);
          do {
            auto blockStartIndex = index;
            auto endIndex = (index & ~static_cast<index_t>(BLOCK_SIZE - 1)) + static_cast<index_t>(BLOCK_SIZE);
            endIndex = details::circular_less_than<index_t>(firstIndex + static_cast<index_t>(actualCount), endIndex) ? firstIndex + static_cast<index_t>(actualCount) : endIndex;

            auto entry = localBlockIndex->index[indexIndex];
            auto block = entry->value.load(std::memory_order_relaxed);
            if (MOODYCAMEL_NOEXCEPT_ASSIGN(T, T&&, details::deref_noexcept(itemFirst) = std::move((*(*block)[index])))) {
              while (index != endIndex) {
                auto& el = *((*block)[index]);
                *itemFirst++ = std::move(el);
                el.~T();
                ++index;
              }
            }
            else {
              MOODYCAMEL_TRY {
                while (index != endIndex) {
                  auto& el = *((*block)[index]);
                  *itemFirst = std::move(el);
                  ++itemFirst;
                  el.~T();
                  ++index;
                }
              }
              MOODYCAMEL_CATCH (...) {
                do {
                  entry = localBlockIndex->index[indexIndex];
                  block = entry->value.load(std::memory_order_relaxed);
                  while (index != endIndex) {
                    (*block)[index++]->~T();
                  }

                  if (block->ConcurrentQueue::Block::template set_many_empty<implicit_context>(blockStartIndex, static_cast<size_t>(endIndex - blockStartIndex))) {
#if MCDBGQ_NOLOCKFREE_IMPLICITPRODBLOCKINDEX
                    debug::DebugLock lock(mutex);
#endif
                    entry->value.store(nullptr, std::memory_order_relaxed);
                    this->parent->add_block_to_free_list(block);
                  }
                  indexIndex = (indexIndex + 1) & (localBlockIndex->capacity - 1);

                  blockStartIndex = index;
                  endIndex = (index & ~static_cast<index_t>(BLOCK_SIZE - 1)) + static_cast<index_t>(BLOCK_SIZE);
                  endIndex = details::circular_less_than<index_t>(firstIndex + static_cast<index_t>(actualCount), endIndex) ? firstIndex + static_cast<index_t>(actualCount) : endIndex;
                } while (index != firstIndex + actualCount);

                MOODYCAMEL_RETHROW;
              }
            }
            if (block->ConcurrentQueue::Block::template set_many_empty<implicit_context>(blockStartIndex, static_cast<size_t>(endIndex - blockStartIndex))) {
              {
#if MCDBGQ_NOLOCKFREE_IMPLICITPRODBLOCKINDEX
                debug::DebugLock lock(mutex);
#endif
                // 请注意，上面的 set_many_empty 执行了释放操作，这意味着任何获得我们即将释放的块的人
                // 都可以安全地使用它，因为我们的写操作（和读取操作！）在此之前已经完成。
                entry->value.store(nullptr, std::memory_order_relaxed);
              }
              this->parent->add_block_to_free_list(block);    // releases the above store
            }
            indexIndex = (indexIndex + 1) & (localBlockIndex->capacity - 1);
          } while (index != firstIndex + actualCount);

          return actualCount;
        }
        else {
          this->dequeueOvercommit.fetch_add(desiredCount, std::memory_order_release);
        }
      }

      return 0;
    }

  private:
    // 块大小必须大于 1，因此任何低位比特被设置的数字都是无效的块基索引
    static const index_t INVALID_BLOCK_BASE = 1;

    struct BlockIndexEntry
    {
      std::atomic<index_t> key;
      std::atomic<Block*> value;
    };

    struct BlockIndexHeader
    {
      size_t capacity;
      std::atomic<size_t> tail;
      BlockIndexEntry* entries;
      BlockIndexEntry** index;
      BlockIndexHeader* prev;
    };

    template<AllocationMode allocMode>
    inline bool insert_block_index_entry(BlockIndexEntry*& idxEntry, index_t blockStartIndex)
    {
      auto localBlockIndex = blockIndex.load(std::memory_order_relaxed);    // We're the only writer thread, relaxed is OK
      if (localBlockIndex == nullptr) {
        return false;  // this can happen if new_block_index failed in the constructor
      }
      auto newTail = (localBlockIndex->tail.load(std::memory_order_relaxed) + 1) & (localBlockIndex->capacity - 1);
      idxEntry = localBlockIndex->index[newTail];
      if (idxEntry->key.load(std::memory_order_relaxed) == INVALID_BLOCK_BASE ||
        idxEntry->value.load(std::memory_order_relaxed) == nullptr) {

        idxEntry->key.store(blockStartIndex, std::memory_order_relaxed);
        localBlockIndex->tail.store(newTail, std::memory_order_release);
        return true;
      }

      // 旧的块索引没有空间，尝试分配另一个块索引！
      if (allocMode == CannotAlloc || !new_block_index()) {
        return false;
      }
      localBlockIndex = blockIndex.load(std::memory_order_relaxed);
      newTail = (localBlockIndex->tail.load(std::memory_order_relaxed) + 1) & (localBlockIndex->capacity - 1);
      idxEntry = localBlockIndex->index[newTail];
      assert(idxEntry->key.load(std::memory_order_relaxed) == INVALID_BLOCK_BASE);
      idxEntry->key.store(blockStartIndex, std::memory_order_relaxed);
      localBlockIndex->tail.store(newTail, std::memory_order_release);
      return true;
    }

    inline void rewind_block_index_tail()
    {
      auto localBlockIndex = blockIndex.load(std::memory_order_relaxed);
      localBlockIndex->tail.store((localBlockIndex->tail.load(std::memory_order_relaxed) - 1) & (localBlockIndex->capacity - 1), std::memory_order_relaxed);
    }

    inline BlockIndexEntry* get_block_index_entry_for_index(index_t index) const
    {
      BlockIndexHeader* localBlockIndex;
      auto idx = get_block_index_index_for_index(index, localBlockIndex);
      return localBlockIndex->index[idx];
    }

    inline size_t get_block_index_index_for_index(index_t index, BlockIndexHeader*& localBlockIndex) const
    {
#if MCDBGQ_NOLOCKFREE_IMPLICITPRODBLOCKINDEX
      debug::DebugLock lock(mutex);
#endif
      index &= ~static_cast<index_t>(BLOCK_SIZE - 1);
      localBlockIndex = blockIndex.load(std::memory_order_acquire);
      auto tail = localBlockIndex->tail.load(std::memory_order_acquire);
      auto tailBase = localBlockIndex->index[tail]->key.load(std::memory_order_relaxed);
      assert(tailBase != INVALID_BLOCK_BASE);
      // 注意：必须使用除法而不是位移，因为索引可能会回绕，导致负偏移，我们希望保留这个负值
      auto offset = static_cast<size_t>(static_cast<typename std::make_signed<index_t>::type>(index - tailBase) / BLOCK_SIZE);
      size_t idx = (tail + offset) & (localBlockIndex->capacity - 1);
      assert(localBlockIndex->index[idx]->key.load(std::memory_order_relaxed) == index && localBlockIndex->index[idx]->value.load(std::memory_order_relaxed) != nullptr);
      return idx;
    }

    bool new_block_index()
    {
      auto prev = blockIndex.load(std::memory_order_relaxed);
      size_t prevCapacity = prev == nullptr ? 0 : prev->capacity;
      auto entryCount = prev == nullptr ? nextBlockIndexCapacity : prevCapacity;
      auto raw = static_cast<char*>((Traits::malloc)(
        sizeof(BlockIndexHeader) +
        std::alignment_of<BlockIndexEntry>::value - 1 + sizeof(BlockIndexEntry) * entryCount +
        std::alignment_of<BlockIndexEntry*>::value - 1 + sizeof(BlockIndexEntry*) * nextBlockIndexCapacity));
      if (raw == nullptr) {
        return false;
      }

      auto header = new (raw) BlockIndexHeader;
      auto entries = reinterpret_cast<BlockIndexEntry*>(details::align_for<BlockIndexEntry>(raw + sizeof(BlockIndexHeader)));
      auto index = reinterpret_cast<BlockIndexEntry**>(details::align_for<BlockIndexEntry*>(reinterpret_cast<char*>(entries) + sizeof(BlockIndexEntry) * entryCount));
      if (prev != nullptr) {
        auto prevTail = prev->tail.load(std::memory_order_relaxed);
        auto prevPos = prevTail;
        size_t i = 0;
        do {
          prevPos = (prevPos + 1) & (prev->capacity - 1);
          index[i++] = prev->index[prevPos];
        } while (prevPos != prevTail);
        assert(i == prevCapacity);
      }
      for (size_t i = 0; i != entryCount; ++i) {
        new (entries + i) BlockIndexEntry;
        entries[i].key.store(INVALID_BLOCK_BASE, std::memory_order_relaxed);
        index[prevCapacity + i] = entries + i;
      }
      header->prev = prev;
      header->entries = entries;
      header->index = index;
      header->capacity = nextBlockIndexCapacity;
      header->tail.store((prevCapacity - 1) & (nextBlockIndexCapacity - 1), std::memory_order_relaxed);

      blockIndex.store(header, std::memory_order_release);

      nextBlockIndexCapacity <<= 1;

      return true;
    }

  private:
    size_t nextBlockIndexCapacity;
    std::atomic<BlockIndexHeader*> blockIndex;

#ifdef MOODYCAMEL_CPP11_THREAD_LOCAL_SUPPORTED
  public:
    details::ThreadExitListener threadExitListener;
  private:
#endif

#ifdef MOODYCAMEL_QUEUE_INTERNAL_DEBUG
  public:
    ImplicitProducer* nextImplicitProducer;
  private:
#endif

#if MCDBGQ_NOLOCKFREE_IMPLICITPRODBLOCKINDEX
    mutable debug::DebugMutex mutex;
#endif
#if MCDBGQ_TRACKMEM
    friend struct MemStats;
#endif
  };


  //////////////////////////////////
  // Block pool manipulation
  //////////////////////////////////

  void populate_initial_block_list(size_t blockCount)
  {
    initialBlockPoolSize = blockCount;
    if (initialBlockPoolSize == 0) {
      initialBlockPool = nullptr;
      return;
    }

    initialBlockPool = create_array<Block>(blockCount);
    if (initialBlockPool == nullptr) {
      initialBlockPoolSize = 0;
    }
    for (size_t i = 0; i < initialBlockPoolSize; ++i) {
      initialBlockPool[i].dynamicallyAllocated = false;
    }
  }

  inline Block* try_get_block_from_initial_pool()
  {
    if (initialBlockPoolIndex.load(std::memory_order_relaxed) >= initialBlockPoolSize) {
      return nullptr;
    }

    auto index = initialBlockPoolIndex.fetch_add(1, std::memory_order_relaxed);

    return index < initialBlockPoolSize ? (initialBlockPool + index) : nullptr;
  }

  inline void add_block_to_free_list(Block* block)
  {
#if MCDBGQ_TRACKMEM
    block->owner = nullptr;
#endif
    freeList.add(block);
  }

  inline void add_blocks_to_free_list(Block* block)
  {
    while (block != nullptr) {
      auto next = block->next;
      add_block_to_free_list(block);
      block = next;
    }
  }

  inline Block* try_get_block_from_free_list()
  {
    return freeList.try_get();
  }

  // 从内存池中获取一个空闲块，或分配一个新块（如果适用）
  template<AllocationMode canAlloc>
  Block* requisition_block()
  {
    auto block = try_get_block_from_initial_pool();
    if (block != nullptr) {
      return block;
    }

    block = try_get_block_from_free_list();
    if (block != nullptr) {
      return block;
    }

    if (canAlloc == CanAlloc) {
      return create<Block>();
    }

    return nullptr;
  }


#if MCDBGQ_TRACKMEM
  public:
// 定义一个名为MemStats的结构体，用于统计内存相关的各种信息
    struct MemStats {
 // 已分配的块数量
      size_t allocatedBlocks;
 // 已分配的块数量
      size_t usedBlocks;
 // 空闲的块数量
      size_t freeBlocks;
 // 显式拥有的块数量
      size_t ownedBlocksExplicit;
 // 显式拥有的块数量
      size_t ownedBlocksImplicit;
// 隐式生产者的数量
      size_t implicitProducers;
// 隐式生产者的数量
      size_t explicitProducers;
// 入队元素的数量
      size_t elementsEnqueued;
 // 块类所占用的字节数
      size_t blockClassBytes;
 // 队列类所占用的字节数
      size_t queueClassBytes;
 // 隐式块索引所占用的字节数
      size_t implicitBlockIndexBytes;
// 显式块索引所占用的字节数
      size_t explicitBlockIndexBytes;
 // 声明ConcurrentQueue类为友元类，意味着ConcurrentQueue类可以访问MemStats的私有成员
      friend class ConcurrentQueue;

    private:
 // 静态成员函数，用于获取给定ConcurrentQueue对象的内存统计信息，参数为指向ConcurrentQueue的指针
      static MemStats getFor(ConcurrentQueue* q)
      {
// 创建一个MemStats结构体实例，并将所有成员初始化为0
        MemStats stats = { 0 };
 // 获取队列中大约的元素数量，并赋值给stats的elementsEnqueued成员，这里size_approx()应该是ConcurrentQueue类提供的用于估算队列元素个数的函数
        stats.elementsEnqueued = q->size_approx();
     // 获取队列空闲链表的头节点，这里假设freeList是ConcurrentQueue类中用于管理空闲块链表的数据成员，head_unsafe()用于获取头节点（可能是一种非线程安全的获取方式，具体取决于实现）
        auto block = q->freeList.head_unsafe();
 // 循环遍历空闲链表，直到遍历到链表末尾（节点为nullptr表示链表结束）
        while (block != nullptr) {
 // 已分配块数量加1，因为当前遍历到的是一个已分配的空闲块
          ++stats.allocatedBlocks;
 // 空闲块数量加1，当前块处于空闲状态
          ++stats.freeBlocks;
 // 获取下一个空闲块节点，通过原子加载操作（memory_order_relaxed表示一种较宽松的内存顺序要求，常用于性能优先的场景）获取下一个节点指针
          block = block->freeListNext.load(std::memory_order_relaxed);
        }
  // 加载队列生产者链表的尾节点，使用memory_order_acquire内存顺序保证获取到的是其他线程已完成写入的最新值，用于后续遍历生产者链表
        for (auto ptr = q->producerListTail.load(std::memory_order_acquire); ptr != nullptr; ptr = ptr->next_prod()) {
                // 通过动态类型转换判断当前生产者指针指向的是否是隐式生产者（ImplicitProducer类型），如果转换成功（不为nullptr）则表示是隐式生产者
          bool implicit = dynamic_cast<ImplicitProducer*>(ptr) != nullptr;
// 如果是隐式生产者，隐式生产者数量加1
          stats.implicitProducers += implicit ? 1 : 0;
 // 如果不是隐式生产者（即显式生产者），显式生产者数量加1
          stats.explicitProducers += implicit ? 0 : 1;
 // 如果是隐式生产者，进入以下逻辑进行相关统计信息的更新
       if (implicit) {
 // 将ptr指针转换为ImplicitProducer*类型，以便后续访问ImplicitProducer类相关的成员变量和函数
            auto prod = static_cast<ImplicitProducer*>(ptr);
// 累加ImplicitProducer类型对象所占用的字节数到queueClassBytes成员，用于统计队列类相关的内存占用情况
            stats.queueClassBytes += sizeof(ImplicitProducer);
// 原子加载隐式生产者的头索引，同样使用memory_order_relaxed内存顺序
            auto head = prod->headIndex.load(std::memory_order_relaxed);
 // 原子加载隐式生产者的尾索引
            auto tail = prod->tailIndex.load(std::memory_order_relaxed);
// 原子加载隐式生产者的块索引（这里假设是一个指向某种数据结构用于管理块索引的指针）
            auto hash = prod->blockIndex.load(std::memory_order_relaxed);
 // 如果块索引指针不为nullptr，说明存在块索引相关的数据结构，进入以下循环处理逻辑
            if (hash != nullptr) {
// 循环遍历块索引数据结构中每个索引位置（假设index是一个数组或者类似可遍历的数据结构）
              for (size_t i = 0; i != hash->capacity; ++i) {
                 // 检查当前索引位置对应的块索引条目的键是否不等于无效块基值（这里INVALID_BLOCK_BASE应该是ImplicitProducer类中定义的表示无效块的一个常量之类的），并且对应的值指针不为nullptr，表示该块是有效的已分配块
                if (hash->index[i]->key.load(std::memory_order_relaxed) != ImplicitProducer::INVALID_BLOCK_BASE && hash->index[i]->value.load(std::memory_order_relaxed) != nullptr) {
 // 已分配块数量加1，因为找到了一个有效的已分配块
                  ++stats.allocatedBlocks;
 // 隐式拥有的块数量加1，因为这是隐式生产者拥有的有效块
                  ++stats.ownedBlocksImplicit;
                }
              }
 // 累加隐式块索引所占用的字节数，计算方式为索引容量乘以每个索引条目的字节大小（这里假设BlockIndexEntry是用于表示块索引条目的结构体之类的）
              stats.implicitBlockIndexBytes += hash->capacity * sizeof(typename ImplicitProducer::BlockIndexEntry);
   // 循环遍历块索引数据结构的链表（假设通过prev指针连接），用于统计整个链表结构所占用的字节数，包括头部和每个节点相关的字节数
              for (; hash != nullptr; hash = hash->prev) {
                stats.implicitBlockIndexBytes += sizeof(typename ImplicitProducer::BlockIndexHeader) + hash->capacity * sizeof(typename ImplicitProducer::BlockIndexEntry*);
              }
            }
// 根据头索引和尾索引循环处理已使用块的统计，这里假设circular_less_than是用于比较循环索引大小的函数，BLOCK_SIZE是块大小相关的常量之类的
            for (; details::circular_less_than<index_t>(head, tail); head += BLOCK_SIZE) {
// 已使用块数量加1，说明对应位置的块正在被使用
              //auto block = prod->get_block_index_entry_for_index(head);
              ++stats.usedBlocks;
            }
          }
 // 如果不是隐式生产者（即显式生产者），进入以下逻辑进行相关统计信息的更新
          else {
            auto prod = static_cast<ExplicitProducer*>(ptr);
            stats.queueClassBytes += sizeof(ExplicitProducer);
            auto tailBlock = prod->tailBlock;
            bool wasNonEmpty = false;
            if (tailBlock != nullptr) {
              auto block = tailBlock;
              do {
                ++stats.allocatedBlocks;
                if (!block->ConcurrentQueue::Block::template is_empty<explicit_context>() || wasNonEmpty) {
                  ++stats.usedBlocks;
                  wasNonEmpty = wasNonEmpty || block != tailBlock;
                }
                ++stats.ownedBlocksExplicit;
                block = block->next;
              } while (block != tailBlock);
            }
            auto index = prod->blockIndex.load(std::memory_order_relaxed);
            while (index != nullptr) {
              stats.explicitBlockIndexBytes += sizeof(typename ExplicitProducer::BlockIndexHeader) + index->size * sizeof(typename ExplicitProducer::BlockIndexEntry);
              index = static_cast<typename ExplicitProducer::BlockIndexHeader*>(index->prev);
            }
          }
        }

        auto freeOnInitialPool = q->initialBlockPoolIndex.load(std::memory_order_relaxed) >= q->initialBlockPoolSize ? 0 : q->initialBlockPoolSize - q->initialBlockPoolIndex.load(std::memory_order_relaxed);
        stats.allocatedBlocks += freeOnInitialPool;
        stats.freeBlocks += freeOnInitialPool;

        stats.blockClassBytes = sizeof(Block) * stats.allocatedBlocks;
        stats.queueClassBytes += sizeof(ConcurrentQueue);

        return stats;
      }
    };

    // 仅用于调试。不是线程安全的。
    MemStats getMemStats()
    {
      return MemStats::getFor(this);
    }
  private:
    friend struct MemStats;
#endif


  //////////////////////////////////
  // 生产者列表操作
  //////////////////////////////////

  ProducerBase* recycle_or_create_producer(bool isExplicit)
  {
    bool recycled;
    return recycle_or_create_producer(isExplicit, recycled);
  }

  ProducerBase* recycle_or_create_producer(bool isExplicit, bool& recycled)
  {
#if MCDBGQ_NOLOCKFREE_IMPLICITPRODHASH
    debug::DebugLock lock(implicitProdMutex);
#endif
    // 先尝试重用一个
    for (auto ptr = producerListTail.load(std::memory_order_acquire); ptr != nullptr; ptr = ptr->next_prod()) {
      if (ptr->inactive.load(std::memory_order_relaxed) && ptr->isExplicit == isExplicit) {
        bool expected = true;
        if (ptr->inactive.compare_exchange_strong(expected, /* desired */ false, std::memory_order_acquire, std::memory_order_relaxed)) {
          // 我们抓到一个了！它已被标记为激活，调用者可以使用它
          recycled = true;
          return ptr;
        }
      }
    }

    recycled = false;
    return add_producer(isExplicit ? static_cast<ProducerBase*>(create<ExplicitProducer>(this)) : create<ImplicitProducer>(this));
  }

  ProducerBase* add_producer(ProducerBase* producer)
  {
    // 处理内存分配失败
    if (producer == nullptr) {
      return nullptr;
    }

    producerCount.fetch_add(1, std::memory_order_relaxed);

    // 将其添加到无锁列表中
    auto prevTail = producerListTail.load(std::memory_order_relaxed);
    do {
      producer->next = prevTail;
    } while (!producerListTail.compare_exchange_weak(prevTail, producer, std::memory_order_release, std::memory_order_relaxed));

#ifdef MOODYCAMEL_QUEUE_INTERNAL_DEBUG
    if (producer->isExplicit) {
      auto prevTailExplicit = explicitProducers.load(std::memory_order_relaxed);
      do {
        static_cast<ExplicitProducer*>(producer)->nextExplicitProducer = prevTailExplicit;
      } while (!explicitProducers.compare_exchange_weak(prevTailExplicit, static_cast<ExplicitProducer*>(producer), std::memory_order_release, std::memory_order_relaxed));
    }
    else {
      auto prevTailImplicit = implicitProducers.load(std::memory_order_relaxed);
      do {
        static_cast<ImplicitProducer*>(producer)->nextImplicitProducer = prevTailImplicit;
      } while (!implicitProducers.compare_exchange_weak(prevTailImplicit, static_cast<ImplicitProducer*>(producer), std::memory_order_release, std::memory_order_relaxed));
    }
#endif

    return producer;
  }

  void reown_producers()
  {
    // 在另一个实例移动到/与此实例交换之后，我们偷来的所有生产者仍然认为它们的父队列是另一个队列。
    // 所以需要修复它们！
    for (auto ptr = producerListTail.load(std::memory_order_relaxed); ptr != nullptr; ptr = ptr->next_prod()) {
      ptr->parent = this;
    }
  }


  //////////////////////////////////
  // 隐式生产者哈希
  //////////////////////////////////

  struct ImplicitProducerKVP
  {
    std::atomic<details::thread_id_t> key;
    ImplicitProducer* value;    // 由于只有设置它的线程会读取它，因此不需要原子性

    ImplicitProducerKVP() : value(nullptr) { }

    ImplicitProducerKVP(ImplicitProducerKVP&& other) MOODYCAMEL_NOEXCEPT
    {
      key.store(other.key.load(std::memory_order_relaxed), std::memory_order_relaxed);
      value = other.value;
    }

    inline ImplicitProducerKVP& operator=(ImplicitProducerKVP&& other) MOODYCAMEL_NOEXCEPT
    {
      swap(other);
      return *this;
    }

    inline void swap(ImplicitProducerKVP& other) MOODYCAMEL_NOEXCEPT
    {
      if (this != &other) {
        details::swap_relaxed(key, other.key);
        std::swap(value, other.value);
      }
    }
  };

  template<typename XT, typename XTraits>
  friend void moodycamel::swap(typename ConcurrentQueue<XT, XTraits>::ImplicitProducerKVP&, typename ConcurrentQueue<XT, XTraits>::ImplicitProducerKVP&) MOODYCAMEL_NOEXCEPT;

  struct ImplicitProducerHash
  {
    size_t capacity;
    ImplicitProducerKVP* entries;
    ImplicitProducerHash* prev;
  };

  inline void populate_initial_implicit_producer_hash()
  {
    if (INITIAL_IMPLICIT_PRODUCER_HASH_SIZE == 0) return;

    implicitProducerHashCount.store(0, std::memory_order_relaxed);
    auto hash = &initialImplicitProducerHash;
    hash->capacity = INITIAL_IMPLICIT_PRODUCER_HASH_SIZE;
    hash->entries = &initialImplicitProducerHashEntries[0];
    for (size_t i = 0; i != INITIAL_IMPLICIT_PRODUCER_HASH_SIZE; ++i) {
      initialImplicitProducerHashEntries[i].key.store(details::invalid_thread_id, std::memory_order_relaxed);
    }
    hash->prev = nullptr;
    implicitProducerHash.store(hash, std::memory_order_relaxed);
  }

  void swap_implicit_producer_hashes(ConcurrentQueue& other)
  {
    if (INITIAL_IMPLICIT_PRODUCER_HASH_SIZE == 0) return;

    // 交换（假设我们的隐式生产者哈希已初始化）
    initialImplicitProducerHashEntries.swap(other.initialImplicitProducerHashEntries);
    initialImplicitProducerHash.entries = &initialImplicitProducerHashEntries[0];
    other.initialImplicitProducerHash.entries = &other.initialImplicitProducerHashEntries[0];

    details::swap_relaxed(implicitProducerHashCount, other.implicitProducerHashCount);

    details::swap_relaxed(implicitProducerHash, other.implicitProducerHash);
    if (implicitProducerHash.load(std::memory_order_relaxed) == &other.initialImplicitProducerHash) {
      implicitProducerHash.store(&initialImplicitProducerHash, std::memory_order_relaxed);
    }
    else {
      ImplicitProducerHash* hash;
      for (hash = implicitProducerHash.load(std::memory_order_relaxed); hash->prev != &other.initialImplicitProducerHash; hash = hash->prev) {
        continue;
      }
      hash->prev = &initialImplicitProducerHash;
    }
    if (other.implicitProducerHash.load(std::memory_order_relaxed) == &initialImplicitProducerHash) {
      other.implicitProducerHash.store(&other.initialImplicitProducerHash, std::memory_order_relaxed);
    }
    else {
      ImplicitProducerHash* hash;
      for (hash = other.implicitProducerHash.load(std::memory_order_relaxed); hash->prev != &initialImplicitProducerHash; hash = hash->prev) {
        continue;
      }
      hash->prev = &other.initialImplicitProducerHash;
    }
  }

  // 仅在内存分配失败时才会失败（返回 nullptr）
  ImplicitProducer* get_or_add_implicit_producer()
  {
    // 注意，由于数据本质上是线程本地的（键是线程 ID），
    // 所以对内存屏障的需求减少（每个线程的内存顺序本身已一致），
    // 除了当前的表本身外。

    // 首先在当前表和所有先前的哈希表中查找线程 ID。
    // 如果未找到，它一定不在其中，因为这个线程之前会将其
    // 添加到我们遍历过的表中的某一个表里。

    // 代码和算法改编自 http://preshing.com/20130605/the-worlds-simplest-lock-free-hash-table

#if MCDBGQ_NOLOCKFREE_IMPLICITPRODHASH
    debug::DebugLock lock(implicitProdMutex);
#endif

    auto id = details::thread_id();
    auto hashedId = details::hash_thread_id(id);

    auto mainHash = implicitProducerHash.load(std::memory_order_acquire);
    for (auto hash = mainHash; hash != nullptr; hash = hash->prev) {
      // 在这个哈希表中查找 ID
      auto index = hashedId;
      while (true) {    // 不是无限循环，因为哈希表中至少有一个槽位是空闲的
        index &= hash->capacity - 1;

        auto probedKey = hash->entries[index].key.load(std::memory_order_relaxed);
        if (probedKey == id) {
          // 找到了！不过，如果我们不得不在多个哈希表中进行深度搜索，我们应该懒惰地将其添加到
          // 当前的主哈希表中，以避免下次的扩展搜索。
          // 注意，当前的哈希表中保证有空间，因为每个后续的哈希表隐式地为所有先前的表保留了空间
          // （只有一个 implicitProducerHashCount）。
          auto value = hash->entries[index].value;
          if (hash != mainHash) {
            index = hashedId;
            while (true) {
              index &= mainHash->capacity - 1;
              probedKey = mainHash->entries[index].key.load(std::memory_order_relaxed);
              auto empty = details::invalid_thread_id;
#ifdef MOODYCAMEL_CPP11_THREAD_LOCAL_SUPPORTED
              auto reusable = details::invalid_thread_id2;
              if ((probedKey == empty    && mainHash->entries[index].key.compare_exchange_strong(empty,    id, std::memory_order_relaxed, std::memory_order_relaxed)) ||
                (probedKey == reusable && mainHash->entries[index].key.compare_exchange_strong(reusable, id, std::memory_order_acquire, std::memory_order_acquire))) {
#else
              if ((probedKey == empty    && mainHash->entries[index].key.compare_exchange_strong(empty,    id, std::memory_order_relaxed, std::memory_order_relaxed))) {
#endif
                mainHash->entries[index].value = value;
                break;
              }
              ++index;
            }
          }

          return value;
        }
        if (probedKey == details::invalid_thread_id) {
          break;    // 不在这个哈希表中
        }
        ++index;
      }
    }

    // Insert!
    auto newCount = 1 + implicitProducerHashCount.fetch_add(1, std::memory_order_relaxed);
    while (true) {
      if (newCount >= (mainHash->capacity >> 1) && !implicitProducerHashResizeInProgress.test_and_set(std::memory_order_acquire)) {
        // 我们已获得了调整大小的锁，尝试分配一个更大的哈希表。
        // 注意，获取屏障与此块末尾的释放屏障同步，因此当我们重新加载 implicitProducerHash 时，它
        // 必须是最新版本（它只在这个锁定的块内被更改）。
        mainHash = implicitProducerHash.load(std::memory_order_acquire);
        if (newCount >= (mainHash->capacity >> 1)) {
          auto newCapacity = mainHash->capacity << 1;
          while (newCount >= (newCapacity >> 1)) {
            newCapacity <<= 1;
          }
          auto raw = static_cast<char*>((Traits::malloc)(sizeof(ImplicitProducerHash) + std::alignment_of<ImplicitProducerKVP>::value - 1 + sizeof(ImplicitProducerKVP) * newCapacity));
          if (raw == nullptr) {
            // 分配失败
            implicitProducerHashCount.fetch_sub(1, std::memory_order_relaxed);
            implicitProducerHashResizeInProgress.clear(std::memory_order_relaxed);
            return nullptr;
          }

          auto newHash = new (raw) ImplicitProducerHash;
          newHash->capacity = newCapacity;
          newHash->entries = reinterpret_cast<ImplicitProducerKVP*>(details::align_for<ImplicitProducerKVP>(raw + sizeof(ImplicitProducerHash)));
          for (size_t i = 0; i != newCapacity; ++i) {
            new (newHash->entries + i) ImplicitProducerKVP;
            newHash->entries[i].key.store(details::invalid_thread_id, std::memory_order_relaxed);
          }
          newHash->prev = mainHash;
          implicitProducerHash.store(newHash, std::memory_order_release);
          implicitProducerHashResizeInProgress.clear(std::memory_order_release);
          mainHash = newHash;
        }
        else {
          implicitProducerHashResizeInProgress.clear(std::memory_order_release);
        }
      }

      // 如果当前表的填充度低于三分之四，即使如此也将其添加到旧表中，以避免等待下一个表
      // 被另一个线程分配（如果我们刚刚完成了上面的分配，条件将总是为真）。
      if (newCount < (mainHash->capacity >> 1) + (mainHash->capacity >> 2)) {
        bool recycled;
        auto producer = static_cast<ImplicitProducer*>(recycle_or_create_producer(false, recycled));
        if (producer == nullptr) {
          implicitProducerHashCount.fetch_sub(1, std::memory_order_relaxed);
          return nullptr;
        }
        if (recycled) {
          implicitProducerHashCount.fetch_sub(1, std::memory_order_relaxed);
        }

#ifdef MOODYCAMEL_CPP11_THREAD_LOCAL_SUPPORTED
        producer->threadExitListener.callback = &ConcurrentQueue::implicit_producer_thread_exited_callback;
        producer->threadExitListener.userData = producer;
        details::ThreadExitNotifier::subscribe(&producer->threadExitListener);
#endif

        auto index = hashedId;
        while (true) {
          index &= mainHash->capacity - 1;
          auto probedKey = mainHash->entries[index].key.load(std::memory_order_relaxed);

          auto empty = details::invalid_thread_id;
#ifdef MOODYCAMEL_CPP11_THREAD_LOCAL_SUPPORTED
          auto reusable = details::invalid_thread_id2;
          if ((probedKey == empty    && mainHash->entries[index].key.compare_exchange_strong(empty,    id, std::memory_order_relaxed, std::memory_order_relaxed)) ||
            (probedKey == reusable && mainHash->entries[index].key.compare_exchange_strong(reusable, id, std::memory_order_acquire, std::memory_order_acquire))) {
#else
          if ((probedKey == empty    && mainHash->entries[index].key.compare_exchange_strong(empty,    id, std::memory_order_relaxed, std::memory_order_relaxed))) {
#endif
            mainHash->entries[index].value = producer;
            break;
          }
          ++index;
        }
        return producer;
      }

      // 嗯，旧的哈希表已经很满了，而其他线程正在忙于分配一个新的哈希表。
      // 我们需要等待正在分配的新表的线程完成（如果分配成功，我们添加到新表中；如果不成功，
      // 我们自己尝试分配）。
      mainHash = implicitProducerHash.load(std::memory_order_acquire);
    }
  }

#ifdef MOODYCAMEL_CPP11_THREAD_LOCAL_SUPPORTED
  void implicit_producer_thread_exited(ImplicitProducer* producer)
  {
    // 从线程退出监听器中移除
    details::ThreadExitNotifier::unsubscribe(&producer->threadExitListener);

    // 从哈希表中移除
#if MCDBGQ_NOLOCKFREE_IMPLICITPRODHASH
    debug::DebugLock lock(implicitProdMutex);
#endif
    auto hash = implicitProducerHash.load(std::memory_order_acquire);
    assert(hash != nullptr);    // 线程退出监听器仅在我们最初被添加到哈希表时注册

    auto id = details::thread_id();
    auto hashedId = details::hash_thread_id(id);
    details::thread_id_t probedKey;

    // 我们需要遍历所有的哈希表，以防其他线程还没有在当前哈希表上，
    // 并且它们正在尝试添加一个条目，认为还有空位（因为它们重用了一个生产者）
    for (; hash != nullptr; hash = hash->prev) {
      auto index = hashedId;
      do {
        index &= hash->capacity - 1;
        probedKey = hash->entries[index].key.load(std::memory_order_relaxed);
        if (probedKey == id) {
          hash->entries[index].key.store(details::invalid_thread_id2, std::memory_order_release);
          break;
        }
        ++index;
      } while (probedKey != details::invalid_thread_id);    // 可能发生在哈希表已改变但我们尚未被重新添加，或者我们最初根本没有被添加到这个哈希表

    }

    // 将队列标记为可回收
    producer->inactive.store(true, std::memory_order_release);
  }

  static void implicit_producer_thread_exited_callback(void* userData)
  {
    auto producer = static_cast<ImplicitProducer*>(userData);
    auto queue = producer->parent;
    queue->implicit_producer_thread_exited(producer);
  }
#endif

  //////////////////////////////////
  // 工具函数
  //////////////////////////////////

  template<typename U>
  static inline U* create_array(size_t count)
  {
    assert(count > 0);
    auto p = static_cast<U*>((Traits::malloc)(sizeof(U) * count));
    if (p == nullptr) {
      return nullptr;
    }

    for (size_t i = 0; i != count; ++i) {
      new (p + i) U();
    }
    return p;
  }

  template<typename U>
  static inline void destroy_array(U* p, size_t count)
  {
    if (p != nullptr) {
      assert(count > 0);
      for (size_t i = count; i != 0; ) {
        (p + --i)->~U();
      }
      (Traits::free)(p);
    }
  }

  template<typename U>
  static inline U* create()
  {
    auto p = (Traits::malloc)(sizeof(U));
    return p != nullptr ? new (p) U : nullptr;
  }

  template<typename U, typename A1>
  static inline U* create(A1&& a1)
  {
    auto p = (Traits::malloc)(sizeof(U));
    return p != nullptr ? new (p) U(std::forward<A1>(a1)) : nullptr;
  }

  template<typename U>
  static inline void destroy(U* p)
  {
    if (p != nullptr) {
      p->~U();
    }
    (Traits::free)(p);
  }

private:
  std::atomic<ProducerBase*> producerListTail;
  std::atomic<std::uint32_t> producerCount;

  std::atomic<size_t> initialBlockPoolIndex;
  Block* initialBlockPool;
  size_t initialBlockPoolSize;

#if !MCDBGQ_USEDEBUGFREELIST
  FreeList<Block> freeList;
#else
  debug::DebugFreeList<Block> freeList;
#endif

  std::atomic<ImplicitProducerHash*> implicitProducerHash;
  std::atomic<size_t> implicitProducerHashCount;    // Number of slots logically used
  ImplicitProducerHash initialImplicitProducerHash;
  std::array<ImplicitProducerKVP, INITIAL_IMPLICIT_PRODUCER_HASH_SIZE> initialImplicitProducerHashEntries;
  std::atomic_flag implicitProducerHashResizeInProgress;

  std::atomic<std::uint32_t> nextExplicitConsumerId;
  std::atomic<std::uint32_t> globalExplicitConsumerOffset;

#if MCDBGQ_NOLOCKFREE_IMPLICITPRODHASH
  debug::DebugMutex implicitProdMutex;
#endif

#ifdef MOODYCAMEL_QUEUE_INTERNAL_DEBUG
  std::atomic<ExplicitProducer*> explicitProducers;
  std::atomic<ImplicitProducer*> implicitProducers;
#endif
};


template<typename T, typename Traits>
ProducerToken::ProducerToken(ConcurrentQueue<T, Traits>& queue)
  : producer(queue.recycle_or_create_producer(true))
{
  if (producer != nullptr) {
    producer->token = this;
  }
}

template<typename T, typename Traits>
ProducerToken::ProducerToken(BlockingConcurrentQueue<T, Traits>& queue)
  : producer(reinterpret_cast<ConcurrentQueue<T, Traits>*>(&queue)->recycle_or_create_producer(true))
{
  if (producer != nullptr) {
    producer->token = this;
  }
}

template<typename T, typename Traits>
ConsumerToken::ConsumerToken(ConcurrentQueue<T, Traits>& queue)
  : itemsConsumedFromCurrent(0), currentProducer(nullptr), desiredProducer(nullptr)
{
  initialOffset = queue.nextExplicitConsumerId.fetch_add(1, std::memory_order_release);
  lastKnownGlobalOffset = -1;
}

template<typename T, typename Traits>
ConsumerToken::ConsumerToken(BlockingConcurrentQueue<T, Traits>& queue)
  : itemsConsumedFromCurrent(0), currentProducer(nullptr), desiredProducer(nullptr)
{
  initialOffset = reinterpret_cast<ConcurrentQueue<T, Traits>*>(&queue)->nextExplicitConsumerId.fetch_add(1, std::memory_order_release);
  lastKnownGlobalOffset = -1;
}

template<typename T, typename Traits>
inline void swap(ConcurrentQueue<T, Traits>& a, ConcurrentQueue<T, Traits>& b) MOODYCAMEL_NOEXCEPT
{
  a.swap(b);
}

inline void swap(ProducerToken& a, ProducerToken& b) MOODYCAMEL_NOEXCEPT
{
  a.swap(b);
}

inline void swap(ConsumerToken& a, ConsumerToken& b) MOODYCAMEL_NOEXCEPT
{
  a.swap(b);
}

template<typename T, typename Traits>
inline void swap(typename ConcurrentQueue<T, Traits>::ImplicitProducerKVP& a, typename ConcurrentQueue<T, Traits>::ImplicitProducerKVP& b) MOODYCAMEL_NOEXCEPT
{
  a.swap(b);
}

}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
