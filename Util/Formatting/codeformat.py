#!/usr/bin/python

#
# Copyright (c) 2017-2020 Intel Corporation
#
# Helper script for code formatting using clang-format-3.9 and autopep

import argparse  # 用于解析命令行参数的标准库模块
import filecmp  # 用于比较文件内容的模块，可帮助进行文件差异对比等操作
import os  # 提供了与操作系统交互的函数，例如文件和目录操作相关功能
import re  # 正则表达式模块，用于处理文本匹配、查找等操作
import sets  # （注：在Python 3中，sets模块已被弃用，通常使用内置的set类型来替代它，此处可能需根据Python版本确认使用情况）用于处理集合相关操作
import subprocess  # 允许在Python程序中启动新进程，执行外部命令
import sys  # 提供了对Python解释器相关的变量和函数的访问，比如获取命令行参数等
from termcolor import cprint  # 从termcolor库中导入cprint函数，用于给终端输出添加颜色

# 定义脚本的版本号，这里版本号为 "1.3"，可以用于版本展示、兼容性判断等场景
SCRIPT_VERSION = "1.3"


# 定义CodeFormatter类，该类大概率是用于代码格式化相关功能的封装
class CodeFormatter:
    # 类的构造函数（初始化方法），用于初始化CodeFormatter类的实例对象的各个属性
    def __init__(self, command, expectedVersion, formatCommandArguments,
                 verifyCommandArguments, verifyOutputIsDiff, fileEndings,
                 fileDescription, installCommand):
        # 用于执行代码格式化操作的命令，比如可能是类似"black"（Python代码格式化工具）这样的命令字符串，
        # 具体值会在实例化类时传入，决定了实际调用哪个工具来进行格式化
        self.command = command
        # 期望的代码格式化工具的版本号，后续可能会通过某种方式（比如执行命令查看版本输出并对比）来验证实际使用的工具版本是否符合预期，
        # 以此确保格式化的效果符合要求
        self.expectedVersion = expectedVersion
        # 这是一个列表或者元组类型（通常是这样），包含了执行代码格式化命令时需要传递的额外参数，
        # 例如格式化工具可能有一些特定的配置选项（如控制缩进格式、换行规则等），通过这些参数传递给格式化命令
        self.formatCommandArguments = formatCommandArguments
        # 同样是一个列表或者元组类型，存放执行验证操作（验证代码格式化后的结果是否正确等情况）时要传递给相关验证命令的参数，
        # 不同的验证场景可能有不同的参数需求
        self.verifyCommandArguments = verifyCommandArguments
        # 一个布尔值，表示验证输出的结果是否应该呈现为差异形式（比如对比格式化前后文件内容的差异情况），
        # 如果为True，可能意味着后续会以展示差异的方式来体现验证结果
        self.verifyOutputIsDiff = verifyOutputIsDiff
        # 是一个列表，存放了需要进行代码格式化操作的文件的后缀名，例如 [".py", ".cpp"]，
        # 通过后缀名来筛选出符合要求的文件进行格式化处理
        self.fileEndings = fileEndings
        # 对要进行格式化的文件的描述信息字符串，比如 "Python source files"（表示Python源文件），
        # 可以让使用者更清晰地了解要处理文件的类型和性质
        self.fileDescription = fileDescription
        # 用于安装相应代码格式化工具的命令，在需要安装格式化工具但尚未安装的情况下，可以通过执行这个命令来安装，
        # 确保能够正常使用格式化功能
        self.installCommand = installCommand

    #试图通过verifyformatterversion函数运行来获取格式化工具的版本信息
    def verifyFormatterVersion(self):
        try:
             # 使用 subprocess.check_output 执行命令并捕获输出，去除末尾的换行符
            versionOutput = subprocess.check_output([self.command, "--version"]).rstrip('\r\n')
            if self.expectedVersion != "":
                 # 如果设置了预期版本，检查实际版本是否符合预期
                if versionOutput.startswith(self.expectedVersion):
                    print("[OK] Found formatter '" + versionOutput + "'")
                    return
                else:
                     # 版本不匹配时，打印错误信息
                    cprint("[NOT OK] Found '" + versionOutput + "'", "red")
                    cprint("[NOT OK] Version string does not start with '" + self.expectedVersion + "'", "red")
            else:
                # 如果没有设置预期版本，则直接返回
                return
        except:
             # 捕获所有异常，包括但不限于找不到命令、权限问题等，并打印错误信息
            cprint("[ERROR] Could not run " + self.command, "red")
            cprint("[INFO] Please check if correct version is installed or install with '" +
                   self.installCommand + "'", "blue")
              # 如果出现任何问题，退出程序，状态码为1表示有错误发生
        sys.exit(1)

    def printInputFiles(self):
        if len(self.inputFiles) > 0:
             # 打印找到的文件数量和类型描述
            print("Found " + self.fileDescription + " files:")
            for fileName in self.inputFiles:
                # 逐个打印文件名
                print(fileName)
            print("")
            # 打印空行以分隔输出内容

    def formatFile(self, fileName):
        commandList = [self.command]
        commandList.extend(self.formatCommandArguments)
        commandList.append(fileName)
        try:
            subprocess.check_output(commandList, stderr=subprocess.STDOUT)
            print("[OK] " + fileName)
        except subprocess.CalledProcessError as e:
            cprint("[ERROR] " + fileName + " (" + e.output.rstrip('\r\n') + ")", "red")
            return True
        return False

    def performGitDiff(self, fileName, verifyOutput):
        try:
            diffProcess = subprocess.Popen(
                ["git", "diff", "--color=always", "--exit-code", "--no-index", "--", fileName, "-"],
                                           stdin=subprocess.PIPE,
                                           stdout=subprocess.PIPE,
                                           stderr=subprocess.PIPE)
            diffOutput, _ = diffProcess.communicate(verifyOutput)
            if diffProcess.returncode == 0:
                diffOutput = ""
        except OSError:
            cprint("[ERROR] Failed to run git diff on " + fileName, "red")
            return (True, "")
        return (False, diffOutput)

    def verifyFile(self, fileName, printDiff):## 创建一个命令列表，以self.command开头，添加self.verifyCommandArguments中的元素，最后添加fileName
        commandList = [self.command]
        commandList.extend(self.verifyCommandArguments)
        commandList.append(fileName)
        try:
            verifyOutput = subprocess.check_output(commandList, stderr=subprocess.STDOUT)# 使用subprocess.check_output执行命令列表中的命令，并获取输出
        except subprocess.CalledProcessError as e:
            cprint("[ERROR] " + fileName + " (" + e.output.rstrip('\r\n') + ")", "red")
            return True

        diffOutput = ""
        if self.verifyOutputIsDiff:
            diffOutput = verifyOutput
        else:
            status, diffOutput = self.performGitDiff(fileName, verifyOutput)
            if status:
                return True

        if diffOutput != "":
            cprint("[NOT OK] " + fileName, "red")
            if printDiff:
                print(diffOutput.rstrip('\r\n'))
            return True

        print("[OK] " + fileName)
        return False


class CodeFormatterClang(CodeFormatter):
    CLANG_FORMAT_FILE = ".clang-format"
    CHECKED_IN_CLANG_FORMAT_FILE = "clang-format"
    CODE_FORMAT_IGNORE_FILE = ".codeformatignore"

    def __init__(self):# 调用父类CodeFormatter的__init__方法进行初始化
        CodeFormatter.__init__(self,
                               command="clang-format-3.9",
                               expectedVersion="clang-format version 3.9",
                               formatCommandArguments=["-style=file", "-fallback-style=none", "-i"],
                               verifyCommandArguments=["-style=file", "-fallback-style=none"],
                               verifyOutputIsDiff=False,
                               fileEndings=["cpp", "hpp", "c", "h", "cc"],
                               fileDescription="source and header",
                               installCommand="sudo apt-get install clang-format-3.9")
        self.scriptPath = os.path.dirname(os.path.abspath(__file__))#
        self.checkedInClangFormatFile = os.path.join(self.scriptPath, CodeFormatterClang.CHECKED_IN_CLANG_FORMAT_FILE)

    def verifyFormatterVersion(self):
        CodeFormatter.verifyFormatterVersion(self)
        self.verifyClangFormatFileExistsAndMatchesCheckedIn()

    def verifyCheckedInClangFormatFileExists(self):
        if os.path.exists(self.checkedInClangFormatFile):
            print("[OK] Found " + CodeFormatterClang.CHECKED_IN_CLANG_FORMAT_FILE + " file (the one that should be in a repository) " +
                  self.checkedInClangFormatFile)
        else:
            cprint("[WARN] Not found " + CodeFormatterClang.CHECKED_IN_CLANG_FORMAT_FILE + " file " +
                   self.checkedInClangFormatFile, "yellow")
            self.confirmWithUserClangFormatFileCantBeVerified()

    def confirmWithUserClangFormatFileCantBeVerified(self):
        if not self.args.yes:
            answer = raw_input("Are you sure your .clang-format file is up-to-date and you want to continue? (y/N)")
            if answer != "y":
                sys.exit(1)

    def verifyClangFormatFileExistsAndMatchesCheckedIn(self):
        self.verifyCheckedInClangFormatFileExists()
        foundClangFormatFiles = sets.Set()
        for fileName in self.inputFiles:
            dirName = os.path.dirname(os.path.abspath(fileName))
            if not self.findClangFormatFileStartingFrom(dirName, fileName, foundClangFormatFiles):
                sys.exit(1)

    def findClangFormatFileStartingFrom(self, dirName, fileName, foundClangFormatFiles):
        clangFormatFile = os.path.join(dirName, CodeFormatterClang.CLANG_FORMAT_FILE)
        if os.path.exists(clangFormatFile):
            if clangFormatFile not in foundClangFormatFiles:
                foundClangFormatFiles.add(clangFormatFile)
                if os.path.exists(self.checkedInClangFormatFile) and \
                   not filecmp.cmp(self.checkedInClangFormatFile, clangFormatFile):
                    cprint("[WARN] " + clangFormatFile + " does not match " + self.checkedInClangFormatFile, "yellow")
                    self.confirmWithUserClangFormatFileCantBeVerified()
                else:
                    print("[OK] Found " + CodeFormatterClang.CLANG_FORMAT_FILE +
                          " file (used by the formatter) " + clangFormatFile)
            return True
        else:
            dirNameOneLevelUp = os.path.dirname(dirName)
            if dirNameOneLevelUp == dirName:
                # dirName was already root folder -> clang-format file not found
                cprint("[ERROR] Not found " + CodeFormatterClang.CLANG_FORMAT_FILE + " for " +
                       fileName + " in same directory or in any parent directory", "red")
                return False
            else:
                return self.findClangFormatFileStartingFrom(dirNameOneLevelUp, fileName, foundClangFormatFiles)


class CodeFormatterAutopep(CodeFormatter):

    def __init__(self):
        CodeFormatter.__init__(self,
                               command="autopep8",
                               expectedVersion="",
                               formatCommandArguments=["--in-place", "--max-line-length=119"],
                               verifyCommandArguments=["--diff", "--max-line-length=119"],
                               verifyOutputIsDiff=True,
                               fileEndings=["py"],
                               fileDescription="python",
                               installCommand="sudo apt-get install python-pep8 python-autopep8")


class CodeFormat:

    def __init__(self):
        self.failure = False
        self.codeFormatterInstances = []
        return

    def parseCommandLine(self):
        parser = argparse.ArgumentParser(
            description="Helper script for code formatting.")
        parser.add_argument("input", nargs="+",
                            help="files or directories to process")
        parser.add_argument("-v", "--verify", action="store_true",
                            help="do not change file, but only verify the format is correct")
        parser.add_argument("-d", "--diff", action="store_true",
                            help="show diff, implies verify mode")
        parser.add_argument("-e", "--exclude", nargs="+", metavar="exclude",
                            help="exclude files or directories containing words from the exclude list in their names")
        parser.add_argument("-y", "--yes", action="store_true",
                            help="do not ask for confirmation before formatting more than one file")
        parser.add_argument("--version", action="version", version="%(prog)s " + SCRIPT_VERSION)
        self.args = parser.parse_args()
        if self.args.diff:
            self.args.verify = True

    def addCodeFormatter(self, codeFormatterInstance):
        self.codeFormatterInstances.append(codeFormatterInstance)

    def scanForInputFiles(self):
        for formatterInstance in self.codeFormatterInstances:
            filePattern = re.compile("^[^.].*\.(" + "|".join(formatterInstance.fileEndings) + ")$")
            formatterInstance.inputFiles = []
            for fileOrDirectory in self.args.input:
                if os.path.exists(fileOrDirectory):
                    formatterInstance.inputFiles.extend(self.scanFileOrDirectory(fileOrDirectory, filePattern))
                else:
                    cprint("[WARN] Cannot find '" + fileOrDirectory + "'", "yellow")

    def scanFileOrDirectory(self, fileOrDirectory, filePattern):
        fileList = []
        if os.path.isdir(fileOrDirectory):
            for root, directories, fileNames in os.walk(fileOrDirectory):
                directories[:] = self.filterDirectories(root, directories)
                for filename in filter(lambda name: filePattern.match(name), fileNames):
                    fullFilename = os.path.join(root, filename)
                    if self.isFileNotExcluded(fullFilename):
                        fileList.append(fullFilename)
        else:
            if self.isFileNotExcluded(fileOrDirectory) and (filePattern.match(os.path.basename(fileOrDirectory)) is not None):
                fileList.append(fileOrDirectory)
        return fileList

    import os  # 导入os模块，用于处理文件和目录路径
import sys  # 导入sys模块，用于访问与Python解释器紧密相关的变量和函数
from some_module import cprint  # 假设cprint是从某个模块导入的，用于打印彩色文本（需要替换为实际模块）

class CodeFormatter:  # 假设这些方法属于一个名为CodeFormatter的类
    # 类的其他属性和方法...
    # 假设CODE_FORMAT_IGNORE_FILE是一个类属性，存储了用于标识忽略格式化的文件名的字符串
    CODE_FORMAT_IGNORE_FILE = ".codeformatignore"  # 示例文件名，表示该目录下的文件将被忽略格式化

    # 定义一个方法，用于过滤掉隐藏目录和包含忽略文件的目录
    def filterDirectories(self, root, directories):
        """
        过滤掉隐藏目录（以'.'开头的目录）和包含CODE_FORMAT_IGNORE_FILE文件的目录。

        参数:
            root (str): 根目录的路径。
            directories (list of str): 要过滤的目录名列表。

        返回:
            list of str: 过滤后的目录名列表。
        """
        # 使用列表推导式过滤目录
        directories[:] = [directory for directory in directories if
                          not directory.startswith(".") and  # 排除隐藏目录
                          not os.path.exists(os.path.join(root, directory, self.CODE_FORMAT_IGNORE_FILE))]  # 排除包含忽略文件的目录
        return directories

    # 定义一个方法，用于检查文件是否不被排除
    def isFileNotExcluded(self, fileName):
        """
        检查文件是否不被排除。如果文件名包含在任何排除项中，或者文件是符号链接，则返回False。

        参数:
            fileName (str): 要检查的文件名。

        返回:
            bool: 如果文件不被排除，则返回True；否则返回False。
        """
        # 检查文件名是否包含在任何排除项中
        if self.args.exclude is not None:
            for excluded in self.args.exclude:
                if excluded in fileName:
                    return False
        # 检查文件是否是符号链接
        if os.path.islink(fileName):
            return False

        return True

    # 定义一个方法，用于确认用户是否要格式化不在Git仓库中的文件
    def confirmWithUserFileIsOutsideGit(self, fileName):
        """
        打印警告信息，并确认用户是否要格式化不在Git仓库中的文件。

        参数:
            fileName (str): 要格式化的文件名。
        """
        # 打印警告信息
        cprint("[WARN] File is not in a Git repo: " + fileName, "yellow")
        # 询问用户是否确定要格式化文件
        answer = raw_input("Are you sure to code format it anyway? (y/Q)")  # 注意：在Python 3中，应使用input()而不是raw_input()
        # 如果用户回答不是'y'，则退出程序
        if answer != "y":
            sys.exit(1)

    # 定义一个方法，用于确认用户是否要格式化Git仓库中未跟踪的文件
    def confirmWithUserFileIsUntracked(self, fileName):
        """
        打印警告信息，并确认用户是否要格式化Git仓库中未跟踪的文件。

        参数:
            fileName (str): 要格式化的文件名。
        """
        # 打印警告信息
        cprint("[WARN] File is untracked in Git: " + fileName, "yellow")
        # 询问用户是否确定要格式化文件
        answer = raw_input("Are you sure to code format it anyway? (y/Q)")  # 注意：在Python 3中，应使用input()而不是raw_input()
        # 如果用户回答不是'y'，则退出程序
        if answer != "y":
            sys.exit(1)


    def confirmWithUserGitRepoIsNotClean(self, gitRepo):
        cprint("[WARN] Git repo is not clean: " + gitRepo, "yellow")
        answer = raw_input("Are you sure to code format files in it anyway? (y/Q)")
        if answer != "y":
            sys.exit(1)

    def checkInputFilesAreInCleanGitReposAndAreTracked(self):
        if self.args.verify or self.args.yes:
            return
        gitRepos = sets.Set()
        for formatterInstance in self.codeFormatterInstances:
            for fileName in formatterInstance.inputFiles:
                gitRepo = self.getGitRepoForFile(fileName)
                if gitRepo is None:
                    self.confirmWithUserFileIsOutsideGit(fileName)
                else:
                    self.gitUpdateIndexRefresh(gitRepo)
                    if not self.isTrackedFile(fileName):
                        self.confirmWithUserFileIsUntracked(fileName)
                    elif gitRepo not in gitRepos:
                        gitRepos.add(gitRepo)
                        if not self.isCleanGitRepo(gitRepo):
                            self.confirmWithUserGitRepoIsNotClean(gitRepo)

    def getGitRepoForFile(self, fileName):
        if not self.isInsideGitRepo(fileName):
            return None
        try:
            gitProcess = subprocess.Popen(["git", "rev-parse", "--show-toplevel"],
                                          stdin=subprocess.PIPE,
                                          stdout=subprocess.PIPE,
                                          stderr=subprocess.PIPE,
                                          cwd=os.path.dirname(fileName))
            gitOutput, _ = gitProcess.communicate()
            if gitProcess.returncode == 0:
                return gitOutput.rstrip('\r\n')
        except OSError:
            cprint("[ERROR] Failed to run 'git rev-parse --show-toplevel' for " + fileName, "red")
        return None

    def isInsideGitRepo(self, fileName):
        try:
            gitProcess = subprocess.Popen(["git", "rev-parse", "--is-inside-work-tree"],
                                          stdin=subprocess.PIPE,
                                          stdout=subprocess.PIPE,
                                          stderr=subprocess.PIPE,
                                          cwd=os.path.dirname(fileName))
            gitOutput, _ = gitProcess.communicate()
            if gitProcess.returncode == 0:
                return gitOutput.rstrip('\r\n') == "true"
        except OSError:
            cprint("[ERROR] Failed to run 'git rev-parse --is-inside-work-tree' for " + fileName, "red")
        return False

    def isTrackedFile(self, fileName):
        try:
            gitProcess = subprocess.Popen(["git", "ls-files", "--error-unmatch", "--", os.path.basename(fileName)],
                                          stdin=subprocess.PIPE,
                                          stdout=subprocess.PIPE,
                                          stderr=subprocess.PIPE,
                                          cwd=os.path.dirname(fileName))
            _, _ = gitProcess.communicate()
            if gitProcess.returncode == 0:
                return True
        except OSError:
            cprint("[ERROR] Failed to run 'git ls-files --error-unmatch' for " + fileName, "red")
        return False

    def isCleanGitRepo(self, gitRepo):
        try:
            gitProcess = subprocess.Popen(["git", "diff-index", "--quiet", "HEAD", "--"],
                                          stdin=subprocess.PIPE,
                                          stdout=subprocess.PIPE,
                                          stderr=subprocess.PIPE,
                                          cwd=gitRepo)
            _, _ = gitProcess.communicate()
            if gitProcess.returncode == 0:
                return True
        except OSError:
            cprint("[ERROR] Failed to run 'git diff-index --quiet HEAD --' for " + gitRepo, "red")
        return False

    def gitUpdateIndexRefresh(self, gitRepo):
        try:
            gitProcess = subprocess.Popen(["git", "update-index", "-q", "--ignore-submodules", "--refresh"],
                                          stdin=subprocess.PIPE,
                                          stdout=subprocess.PIPE,
                                          stderr=subprocess.PIPE,
                                          cwd=gitRepo)
            _, _ = gitProcess.communicate()
            if gitProcess.returncode == 0:
                return True
        except OSError:
            cprint("[ERROR] Failed to run 'git update-index -q --ignore-submodules --refresh' for " + gitRepo, "red")
        return False

    def verifyFormatterVersion(self):
        for formatterInstance in self.codeFormatterInstances:
            if len(formatterInstance.inputFiles) > 0:
                formatterInstance.verifyFormatterVersion()

    def printMode(self):
        if self.args.verify:
            cprint("VERIFY MODE", attrs=["bold"])
        else:
            cprint("FORMAT MODE", attrs=["bold"])

    def processFiles(self):
        for formatterInstance in self.codeFormatterInstances:
            for fileName in formatterInstance.inputFiles:
                if self.args.verify:
                    self.failure |= formatterInstance.verifyFile(fileName, self.args.diff)
                else:
                    self.failure |= formatterInstance.formatFile(fileName)

    def numberOfInputFiles(self):
        count = 0
        for formatterInstance in self.codeFormatterInstances:
            count += len(formatterInstance.inputFiles)
        return count

    def confirmWithUser(self):
        if self.numberOfInputFiles() == 0:
            cprint("[WARN] No input files (or file endings unknown)", "yellow")
        elif (not self.args.verify) and (not self.args.yes) and self.numberOfInputFiles() > 1:
            for formatterInstance in self.codeFormatterInstances:
                formatterInstance.printInputFiles()
            answer = raw_input("Are you sure to code format " + str(self.numberOfInputFiles()) + " files? (y/N)")
            if answer != "y":
                sys.exit(1)


def main():
    codeFormat = CodeFormat()
    codeFormat.parseCommandLine()
    codeFormat.printMode()

    codeFormat.addCodeFormatter(CodeFormatterClang())
    codeFormat.addCodeFormatter(CodeFormatterAutopep())

    codeFormat.scanForInputFiles()
    codeFormat.verifyFormatterVersion()
    codeFormat.confirmWithUser()
    codeFormat.checkInputFilesAreInCleanGitReposAndAreTracked()
    codeFormat.processFiles()
    if codeFormat.failure:
        cprint("FAILURE", "red")
        sys.exit(1)
    else:
        cprint("SUCCESS", "green")
        sys.exit(0)

if __name__ == "__main__":
    main()
