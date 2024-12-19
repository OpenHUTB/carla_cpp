// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/Version.h>

#定义了一个名为TEST的函数，接受两个参数miscllaneous，version
TEST(miscellaneous, version) {
  std::cout << "LibCarla " << carla::version() << std::endl;
}
