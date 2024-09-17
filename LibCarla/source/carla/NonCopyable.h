// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

<<<<<<< HEAD
#pragma once // 确保该头文件只会被编译一次

namespace carla { // 定义命名空间 carla，以避免命名冲突

  /// 这个类用于禁止拷贝和移动构造函数及赋值操作
  class NonCopyable {
  public:

    NonCopyable() = default; // 默认构造函数

    NonCopyable(const NonCopyable &) = delete; // 禁用拷贝构造函数
    NonCopyable &operator=(const NonCopyable &) = delete; // 禁用拷贝赋值操作符

    NonCopyable(NonCopyable &&) = delete; // 禁用移动构造函数
    NonCopyable &operator=(NonCopyable &&) = delete; // 禁用移动赋值操作符
  };

  /// 这个类用于禁止拷贝构造函数和赋值操作，但允许移动构造函数和赋值操作
  class MovableNonCopyable {
  public:

    MovableNonCopyable() = default; // 默认构造函数

    MovableNonCopyable(const MovableNonCopyable &) = delete; // 禁用拷贝构造函数
    MovableNonCopyable &operator=(const MovableNonCopyable &) = delete; // 禁用拷贝赋值操作符

    MovableNonCopyable(MovableNonCopyable &&) = default; // 允许默认的移动构造函数
    MovableNonCopyable &operator=(MovableNonCopyable &&) = default; // 允许默认的移动赋值操作符
  };

} // 结束命名空间 carla
=======
#pragma once // 纭繚璇ュご鏂囦欢鍙細琚紪璇戜竴娆�

namespace carla { // 瀹氫箟鍛藉悕绌洪棿 carla锛屼互閬垮厤鍛藉悕鍐茬獊

  /// 杩欎釜绫荤敤浜庣姝㈡嫹璐濆拰绉诲姩鏋勯�犲嚱鏁板強璧嬪�兼搷浣�
  class NonCopyable {
  public:

    NonCopyable() = default; // 榛樿鏋勯�犲嚱鏁�

    NonCopyable(const NonCopyable &) = delete; // 绂佺敤鎷疯礉鏋勯�犲嚱鏁�
    NonCopyable &operator=(const NonCopyable &) = delete; // 绂佺敤鎷疯礉璧嬪�兼搷浣滅

    NonCopyable(NonCopyable &&) = delete; // 绂佺敤绉诲姩鏋勯�犲嚱鏁�
    NonCopyable &operator=(NonCopyable &&) = delete; // 绂佺敤绉诲姩璧嬪�兼搷浣滅
  };

  /// 杩欎釜绫荤敤浜庣姝㈡嫹璐濇瀯閫犲嚱鏁板拰璧嬪�兼搷浣滐紝浣嗗厑璁哥Щ鍔ㄦ瀯閫犲嚱鏁板拰璧嬪�兼搷浣�
  class MovableNonCopyable {
  public:

    MovableNonCopyable() = default; // 榛樿鏋勯�犲嚱鏁�

    MovableNonCopyable(const MovableNonCopyable &) = delete; // 绂佺敤鎷疯礉鏋勯�犲嚱鏁�
    MovableNonCopyable &operator=(const MovableNonCopyable &) = delete; // 绂佺敤鎷疯礉璧嬪�兼搷浣滅

    MovableNonCopyable(MovableNonCopyable &&) = default; // 鍏佽榛樿鐨勭Щ鍔ㄦ瀯閫犲嚱鏁�
    MovableNonCopyable &operator=(MovableNonCopyable &&) = default; // 鍏佽榛樿鐨勭Щ鍔ㄨ祴鍊兼搷浣滅
  };

} // 缁撴潫鍛藉悕绌洪棿 carla
>>>>>>> 4abc25176ac5d5930e03168b4e3d67d59cc132e7
