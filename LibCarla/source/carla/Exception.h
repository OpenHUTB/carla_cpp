// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#ifdef LIBCARLA_NO_EXCEPTIONS

namespace std {

  class exception; // å£°æ˜ std å‘½åç©ºé—´ä¸­çš„ exception ç±»

} // namespace std

namespace carla {

       /// ÓÃ»§×Ô¶¨ÒåµÄº¯Êı£¬ÀàËÆÓÚ Boost µÄ throw_exception¡£
      ///
      /// @important Boost Òì³£Ò²»áÂ·ÓÉµ½´Ëº¯Êı¡£
      ///
      /// µ±Ê¹ÓÃ LIBCARLA_NO_EXCEPTIONS ±àÒëÊ±£¬´Ëº¯ÊıÔÚ LibCarla ÖĞÎ´¶¨Òå£¬
      /// Ê¹ÓÃ LibCarla µÄÄ£¿éĞèÒªÌá¹©ºÏÊÊµÄ¶¨Òå¡£µ÷ÓÃ throw_exception µÄ´úÂë
      /// ¿ÉÒÔ¼ÙÉè´Ëº¯Êı²»»á·µ»Ø£»Òò´Ë£¬Èç¹ûÓÃ»§¶¨ÒåµÄ throw_exception ·µ»Ø£¬
      /// ĞĞÎªÊÇÎ´¶¨ÒåµÄ¡£
  [[ noreturn ]] void throw_exception(const std::exception &e);

} // namespace carla

#else

namespace carla {

  template <typename T>
  [[ noreturn ]] void throw_exception(const T &e) { // æ¨¡æ¿å‡½æ•°å£°æ˜ï¼Œæ¥å—ä»»ä½•ç±»å‹çš„å¼‚å¸¸
    throw e;
  }

} // namespace carla

#endif // LIBCARLA_NO_EXCEPTIONS
