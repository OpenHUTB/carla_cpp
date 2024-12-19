:: 将生成的html文档部署到OpenHUTB页面上

xcopy Doxygen %TEMP%  /q /e /r /S /Y

git checkout main

:: 将%TEMP%文件夹中的html文件拷贝到当前目录中

git add .
git commit -m update
git push#将本地仓库的提交推送到远程仓库
