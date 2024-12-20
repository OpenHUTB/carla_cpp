:: 将生成的html文档部署到OpenHUTB页面上

xcopy Doxygen %TEMP%  /q /e /r /S /Y

git checkout main

:: 将%TEMP%文件夹中的html文件拷贝到当前目录中

git add .
#git 添加
git commit -m update
#git 提交-m 更新
git push
#git 推进
