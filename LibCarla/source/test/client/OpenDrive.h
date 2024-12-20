// 版权所有 （c） 2019 Universitat Autonoma 计算机视觉中心 （CVC）
//巴塞罗那 （UAB）。
//
//本作品根据 MIT 许可证的条款进行许可。
// 有关副本，请参阅 <https://opensource.org/licenses/MIT>。

#pragma once

#include <string>
#include <vector>

namespace util {

  /// 加载OpenDrive目录下文件的工具类.
  class OpenDrive {
  public:

    static std::vector<std::string> GetAvailableFiles();

    static std::string Load(const std::string &filename);
  };

} // 命名空间实用程序
