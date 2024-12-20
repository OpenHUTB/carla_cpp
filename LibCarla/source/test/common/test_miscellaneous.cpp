// 版权所有 （c） 2017 Universitat Autonoma 计算机视觉中心 （CVC）
// 巴塞罗那 （UAB）。
//
// 本作品根据 MIT 许可证的条款进行许可。
// 有关副本，请参阅 <https://opensource.org/licenses/MIT>。

#include "test.h"

#include <carla/Version.h>

TEST(miscellaneous, version) {
  std::cout << "LibCarla " << carla::version() << std::endl;
}
