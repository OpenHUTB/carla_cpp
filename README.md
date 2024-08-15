


# Carla 的 C++ 文档
## 生成文档
1.在项目主目录下运行以下命令，根据 C++ 代码生成对应的 html 文档：
```shell script
doxygen
```
生成的文档位于`carla_cpp\Doxygen\html`目录下，包括的文档：
[LibCarla/source/carla](https://openhutb.github.io/carla_cpp/dir_b14cdd661f9a7048a44f1771cd402401.html) 、[Unreal/CarlaUE4/Plugins/Carla/Source/Carla](https://openhutb.github.io/carla_cpp/dir_8fc34afb5f07a67966c78bf5319f94ae.html) 、[Unreal/CarlaUE4/Source](https://openhutb.github.io/carla_cpp/dir_8fc34afb5f07a67966c78bf5319f94ae.html) 。

2.文档部署（可选）：
```shell script
deploy.bat
```

## 新建开发分支
直接创建一个空白的分支，而是像普通的分支一样把原分支的内容拷贝过来：
```shell
git checkout --orphan dev
```
还需要手动删除孤立分支中的内容：
```shell
git rm -rf .
```