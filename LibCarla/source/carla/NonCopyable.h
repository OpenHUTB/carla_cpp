// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

<<<<<<< HEAD
#pragma once // È·±£¸ÃÍ·ÎÄ¼þÖ»»á±»±àÒëÒ»´Î

namespace carla { // ¶¨ÒåÃüÃû¿Õ¼ä carla£¬ÒÔ±ÜÃâÃüÃû³åÍ»

  /// Õâ¸öÀàÓÃÓÚ½ûÖ¹¿½±´ºÍÒÆ¶¯¹¹Ôìº¯Êý¼°¸³Öµ²Ù×÷
  class NonCopyable {
  public:

    NonCopyable() = default; // Ä¬ÈÏ¹¹Ôìº¯Êý

    NonCopyable(const NonCopyable &) = delete; // ½ûÓÃ¿½±´¹¹Ôìº¯Êý
    NonCopyable &operator=(const NonCopyable &) = delete; // ½ûÓÃ¿½±´¸³Öµ²Ù×÷·û

    NonCopyable(NonCopyable &&) = delete; // ½ûÓÃÒÆ¶¯¹¹Ôìº¯Êý
    NonCopyable &operator=(NonCopyable &&) = delete; // ½ûÓÃÒÆ¶¯¸³Öµ²Ù×÷·û
  };

  /// Õâ¸öÀàÓÃÓÚ½ûÖ¹¿½±´¹¹Ôìº¯ÊýºÍ¸³Öµ²Ù×÷£¬µ«ÔÊÐíÒÆ¶¯¹¹Ôìº¯ÊýºÍ¸³Öµ²Ù×÷
  class MovableNonCopyable {
  public:

    MovableNonCopyable() = default; // Ä¬ÈÏ¹¹Ôìº¯Êý

    MovableNonCopyable(const MovableNonCopyable &) = delete; // ½ûÓÃ¿½±´¹¹Ôìº¯Êý
    MovableNonCopyable &operator=(const MovableNonCopyable &) = delete; // ½ûÓÃ¿½±´¸³Öµ²Ù×÷·û

    MovableNonCopyable(MovableNonCopyable &&) = default; // ÔÊÐíÄ¬ÈÏµÄÒÆ¶¯¹¹Ôìº¯Êý
    MovableNonCopyable &operator=(MovableNonCopyable &&) = default; // ÔÊÐíÄ¬ÈÏµÄÒÆ¶¯¸³Öµ²Ù×÷·û
  };

} // ½áÊøÃüÃû¿Õ¼ä carla
=======
#pragma once // ç¡®ä¿è¯¥å¤´æ–‡ä»¶åªä¼šè¢«ç¼–è¯‘ä¸€æ¬¡

namespace carla { // å®šä¹‰å‘½åç©ºé—´ carlaï¼Œä»¥é¿å…å‘½åå†²çª

  /// è¿™ä¸ªç±»ç”¨äºŽç¦æ­¢æ‹·è´å’Œç§»åŠ¨æž„é€ å‡½æ•°åŠèµ‹å€¼æ“ä½œ
  class NonCopyable {
  public:

    NonCopyable() = default; // é»˜è®¤æž„é€ å‡½æ•°

    NonCopyable(const NonCopyable &) = delete; // ç¦ç”¨æ‹·è´æž„é€ å‡½æ•°
    NonCopyable &operator=(const NonCopyable &) = delete; // ç¦ç”¨æ‹·è´èµ‹å€¼æ“ä½œç¬¦

    NonCopyable(NonCopyable &&) = delete; // ç¦ç”¨ç§»åŠ¨æž„é€ å‡½æ•°
    NonCopyable &operator=(NonCopyable &&) = delete; // ç¦ç”¨ç§»åŠ¨èµ‹å€¼æ“ä½œç¬¦
  };

  /// è¿™ä¸ªç±»ç”¨äºŽç¦æ­¢æ‹·è´æž„é€ å‡½æ•°å’Œèµ‹å€¼æ“ä½œï¼Œä½†å…è®¸ç§»åŠ¨æž„é€ å‡½æ•°å’Œèµ‹å€¼æ“ä½œ
  class MovableNonCopyable {
  public:

    MovableNonCopyable() = default; // é»˜è®¤æž„é€ å‡½æ•°

    MovableNonCopyable(const MovableNonCopyable &) = delete; // ç¦ç”¨æ‹·è´æž„é€ å‡½æ•°
    MovableNonCopyable &operator=(const MovableNonCopyable &) = delete; // ç¦ç”¨æ‹·è´èµ‹å€¼æ“ä½œç¬¦

    MovableNonCopyable(MovableNonCopyable &&) = default; // å…è®¸é»˜è®¤çš„ç§»åŠ¨æž„é€ å‡½æ•°
    MovableNonCopyable &operator=(MovableNonCopyable &&) = default; // å…è®¸é»˜è®¤çš„ç§»åŠ¨èµ‹å€¼æ“ä½œç¬¦
  };

} // ç»“æŸå‘½åç©ºé—´ carla
>>>>>>> 4abc25176ac5d5930e03168b4e3d67d59cc132e7
