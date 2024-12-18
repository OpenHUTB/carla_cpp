// 版权所有 (c) 2017 巴塞罗那自治大学 (UAB) 计算机视觉中心 (CVC)。
//
// 本作品依据 MIT 许可证条款授权使用。
// 如需副本，请访问 https://opensource.org/licenses/MIT。

// 引入 carla 库中的文件系统相关的头文件
#include "carla/FileSystem.h"

// 引入 carla 库中的异常处理头文件
#include "carla/Exception.h"
// 引入 carla 库中的字符串处理工具头文件
#include "carla/StringUtil.h"

// 引入 boost 文件系统库的操作相关头文件
#include <boost/filesystem/operations.hpp>

// 定义命名空间 carla
namespace carla {

// 引入 boost 文件系统库命名空间，并起别名 fs
namespace fs = boost::filesystem;

// 函数：ValidateFilePath
// 作用：验证文件路径，确保路径有扩展名，并创建路径的父目录（如果需要）
  void FileSystem::ValidateFilePath(std::string &filepath, const std::string &ext) {
    // 使用文件系统库中的path类来处理文件路径
    fs::path path(filepath);
    // 检查路径是否有扩展名，并且提供的扩展名不为空
    if (path.extension().empty() && !ext.empty()) {
      // 如果提供的扩展名不以'.'开头，则先添加一个'.'
      if (ext[0] != '.') {
        path += '.';
      }
      // 将扩展名添加到路径中
      path += ext;
    }
    // 获取路径的父目录
    auto parent = path.parent_path();
    // 如果父目录不为空（即路径不是根目录或类似的情况）
    if (!parent.empty()) {
      // 创建父目录及其所有不存在的上级目录
      fs::create_directories(parent);
    }
    // 更新传入的文件路径字符串，以反映可能的更改（如添加了扩展名）
    filepath = path.string();
  }
// 函数：ListFolder
// 作用：列出指定文件夹下符合特定通配符模式的文件列表
  std::vector<std::string> FileSystem::ListFolder(
      const std::string &folder_path, // 输入参数：要列出的文件夹路径
      const std::string &wildcard_pattern // 输入参数：用于匹配文件名的通配符模式
  ) {
      	// 将输入的文件夹路径转换为 boost::filesystem::path 类型的对象
    fs::path root(folder_path);

    // 检查路径是否存在且是否为一个目录
    if (!fs::exists(root) || !fs::is_directory(root)) {
      // 如果不是，则抛出一个异常，指出没有这样的文件夹
      throw_exception(std::invalid_argument(folder_path + ": no such folder"));
    }

    // 用于存储匹配的文件名列表的向量
    std::vector<std::string> results;
    // 定义目录迭代器的结束标志
    fs::directory_iterator end;
    // 遍历指定文件夹下的所有文件和子目录
    for (fs::directory_iterator it(root); it != end; ++it) {
      // 如果当前项是一个常规文件（不是目录、链接等）
      if (fs::is_regular_file(*it)) {
        // 获取文件的名称
        const std::string filename = it->path().filename().string();
        // 使用 StringUtil::Match 函数检查文件名是否符合通配符模式
        if (StringUtil::Match(filename, wildcard_pattern)) {
          // 如果匹配，则将文件名添加到结果列表中
          results.emplace_back(filename);
        }
      }
    }
    // 返回匹配的文件名列表
    return results;
  }

} // namespace carla
