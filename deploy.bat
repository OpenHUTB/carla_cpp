:: 将生成的html文档部署到OpenHUTB页面上

xcopy Doxygen %TEMP%  /q /e /r /S /Y

git checkout main

:: 将%TEMP%文件夹中的html文件拷贝到当前目录中

git add .:: 这个命令用于将当前目录（. 表示当前目录）及其子目录中的所有更改添加到 Git 的暂存区
git commit -m update:: 这个命令用于将暂存区中的更改提交到本地 Git 仓库
git push:: 这个命令用于将本地仓库中的提交推送到远程仓库
