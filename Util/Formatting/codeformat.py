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

    import subprocess  # 导入subprocess模块，用于执行外部命令
from some_module import cprint  # 假设cprint是从某个模块导入的，用于打印彩色文本（需要替换为实际模块）

class CodeFormatter:  # 假设这些方法属于一个名为CodeFormatter的类
    # 类的其他属性和方法...
    # 假设self.command是格式化命令的路径或名称
    # 假设self.formatCommandArguments是格式化命令的附加参数列表

    # 定义一个方法，用于格式化指定的文件
    def formatFile(self, fileName):
        # 构建命令列表，首先添加格式化命令，然后添加附加参数，最后添加文件名
        commandList = [self.command]
        commandList.extend(self.formatCommandArguments)
        commandList.append(fileName)
        
        try:
            # 使用subprocess.check_output执行命令，如果命令成功执行，则捕获其输出（这里不需要输出，所以忽略）
            # stderr=subprocess.STDOUT表示将stderr重定向到stdout，这样错误信息也会包含在输出中（如果需要的话）
            # 但由于我们不需要输出内容，这里主要是为了捕获异常
            subprocess.check_output(commandList, stderr=subprocess.STDOUT)
            # 如果命令成功执行，打印成功信息
            print("[OK] " + fileName)
            # 格式化成功，返回False（这里返回值的逻辑可能需要根据实际情况调整）
            return False
        except subprocess.CalledProcessError as e:
            # 如果命令执行失败（返回非零退出码），则捕获CalledProcessError异常
            # e.output包含命令的输出（包括错误信息，因为stderr被重定向到了stdout）
            # 使用cprint打印错误信息，并返回True表示格式化失败（或需要进一步检查）
            cprint("[ERROR] " + fileName + " (" + e.output.rstrip('\r\n') + ")", "red")
            return True

    # 定义一个方法，用于比较指定文件与标准输入（可能是一个格式化后的版本）的差异
    def performGitDiff(self, fileName, verifyOutput):
        try:
            # 使用subprocess.Popen执行git diff命令
            # --color=always表示始终输出彩色差异（尽管在某些环境下可能无效）
            # --exit-code表示如果文件没有差异则退出码为0，有差异则为1
            # --no-index表示比较两个非Git跟踪的文件
            # --后面跟着的是要比较的两个文件，第一个是要检查的文件，第二个是通过stdin提供的内容（即verifyOutput）
            diffProcess = subprocess.Popen(
                ["git", "diff", "--color=always", "--exit-code", "--no-index", "--", fileName, "-"],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE
            )
            # 将verifyOutput（可能是格式化后的文件内容）作为输入传递给git diff
            # 并获取diff的输出和错误信息（但这里我们只关心输出）
            diffOutput, _ = diffProcess.communicate(verifyOutput)
            # 如果diffProcess的返回码为0，表示文件没有差异，将diffOutput置为空字符串
            if diffProcess.returncode == 0:
                diffOutput = ""
            # 返回元组，第一个元素表示是否有差异（False表示无差异），第二个元素是差异内容
            return (False, diffOutput)
        except OSError:
            # 如果捕获到OSError异常，表示git命令执行失败（可能是git未安装或路径不正确）
            # 使用cprint打印错误信息，并返回元组表示有差异且差异内容为空字符串
            cprint("[ERROR] Failed to run git diff on " + fileName, "red")
            return (True, "")

# 注意：
# 1. 需要将'CodeFormatter'替换为实际的类名（如果类名不同的话）。
# 2. 需要确保'cprint'函数是从正确的模块导入的，或者替换为其他打印彩色文本的方法。
# 3. 'self.command'和'self.formatCommandArguments'应该是类的属性，分别存储格式化命令的路径或名称和附加参数列表。
# 4. 'verifyOutput'参数可能是一个字节串，它包含了与'fileName'进行比较的内容（例如，格式化后的文件内容）。
# 5. 'performGitDiff'方法中的逻辑是将'verifyOutput'作为标准输入传递给'git diff'，并与'fileName'指定的文件进行比较。
#    这种用法比较特殊，通常'git diff'是用来比较两个文件或文件与Git仓库中的版本的，但在这里它被用来比较文件与标准输入。
#    这种方法可能不适用于所有情况，特别是当'verifyOutput'非常大时，因为它需要通过PIPE传递给子进程。

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

    def filterDirectories(self, root, directories):
        # Exclude hidden directories and all directories that have a CODE_FORMAT_IGNORE_FILE
        directories[:] = [directory for directory in directories if
                          not directory.startswith(".") and
                          not os.path.exists(os.path.join(root, directory, CodeFormatterClang.CODE_FORMAT_IGNORE_FILE))]
        return directories

    def isFileNotExcluded(self, fileName):
        if self.args.exclude is not None:
            for excluded in self.args.exclude:
                if excluded in fileName:
                    return False
        if os.path.islink(fileName):
            return False

        return True

    def confirmWithUserFileIsOutsideGit(self, fileName):
        cprint("[WARN] File is not in a Git repo: " + fileName, "yellow")
        answer = raw_input("Are you sure to code format it anyway? (y/Q)")
        if answer != "y":
            sys.exit(1)

    def confirmWithUserFileIsUntracked(self, fileName):
        cprint("[WARN] File is untracked in Git: " + fileName, "yellow")
        answer = raw_input("Are you sure to code format it anyway? (y/Q)")
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
