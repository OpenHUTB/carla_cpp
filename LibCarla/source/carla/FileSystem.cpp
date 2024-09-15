// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// ���� carla ���е��ļ�ϵͳ��ص�ͷ�ļ�
#include "carla/FileSystem.h"

// ���� carla ���е��쳣����ͷ�ļ�
#include "carla/Exception.h"
// ���� carla ���е��ַ���������ͷ�ļ�
#include "carla/StringUtil.h"

// ���� boost �ļ�ϵͳ��Ĳ������ͷ�ļ�
#include <boost/filesystem/operations.hpp>

// ���������ռ� carla
namespace carla {

// ���� boost �ļ�ϵͳ�������ռ䣬������� fs
namespace fs = boost::filesystem;

// ������ValidateFilePath
// ���ã���֤�ļ�·����ȷ��·������չ����������·���ĸ�Ŀ¼�������Ҫ��
  void FileSystem::ValidateFilePath(std::string &filepath, const std::string &ext) {
    fs::path path(filepath);
    if (path.extension().empty() && !ext.empty()) {
      if (ext[0] != '.') {
        path += '.';
      }
      path += ext;
    }
    auto parent = path.parent_path();
    if (!parent.empty()) {
      fs::create_directories(parent);
    }
    filepath = path.string();
  }
// ������ListFolder
// ���ã��г�ָ���ļ����·����ض�ͨ���ģʽ���ļ��б�
  std::vector<std::string> FileSystem::ListFolder(
      const std::string &folder_path,
      const std::string &wildcard_pattern) {
      	// ��������ļ���·��ת��Ϊ boost::filesystem::path ���͵Ķ���
    fs::path root(folder_path);
    if (!fs::exists(root) || !fs::is_directory(root)) {
      throw_exception(std::invalid_argument(folder_path + ": no such folder"));
    }

    std::vector<std::string> results;
    fs::directory_iterator end;
    for (fs::directory_iterator it(root); it != end; ++it) {
      if (fs::is_regular_file(*it)) {
        const std::string filename = it->path().filename().string();
        if (StringUtil::Match(filename, wildcard_pattern)) {
          results.emplace_back(filename);
        }
      }
    }
    return results;
  }

} // namespace carla
