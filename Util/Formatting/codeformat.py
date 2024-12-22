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

    def formatFile(self, fileName):#这个函数的目的是执行一个命令（由self.command和self.formatCommandArguments组成）对指定的fileName进行操作。
        commandList = [self.command]#创建了一个名为commandList的列表，首先将self.command添加到列表中。这是构建要执行的命令的开始部分。
        commandList.extend(self.formatCommandArguments)#使用extend方法将self.formatCommandArguments添加到commandList中。extend方法用于将一个可迭代对象（如列表）中的元素逐个添加到另一个列表中，这样就逐步构建了完整的命令参数列表。
        commandList.append(fileName)#将fileName添加到commandList中，至此，commandList包含了要执行的完整命令及其参数。
        try:#使用subprocess.check_output来执行commandList中的命令。stderr = subprocess.STDOUT表示将标准错误输出重定向到标准输出。如果命令执行成功（没有引发CalledProcessError异常），则在第93行打印[OK]加上文件名，表示操作成功。
            subprocess.check_output(commandList, stderr=subprocess.STDOUT)
            print("[OK] " + fileName)
        except subprocess.CalledProcessError as e:
            cprint("[ERROR] " + fileName + " (" + e.output.rstrip('\r\n') + ")", "red")
            return True
        return False

    def performGitDiff(self, fileName, verifyOutput):#这个函数的目的是对指定的fileName执行git diff操作，并根据操作结果和是否提供verifyOutput进行相应处理。
        try:#使用try - except块来执行git diff操作并处理可能的错误。
            diffProcess = subprocess.Popen(
                ["git", "diff", "--color=always", "--exit-code", "--no-index", "--", fileName, "-"],
                                           stdin=subprocess.PIPE,
                                           stdout=subprocess.PIPE,
                                           stderr=subprocess.PIPE)
            diffOutput, _ = diffProcess.communicate(verifyOutput)#使用diffProcess.communicate方法向进程发送verifyOutput（如果有的话）并获取输出。这里将输出赋值给diffOutput（假设_是一个不需要使用的占位符，表示忽略stderr的输出内容）。
            if diffProcess.returncode == 0:
                diffOutput = ""#如果diffProcess的返回码为0，表示git diff操作成功（根据--exit - code参数的含义），则将diffOutput设置为空字符串。
        except OSError:
            cprint("[ERROR] Failed to run git diff on " + fileName, "red")
            return (True, "")#函数返回一个元组，第一个元素是True（这里可能存在逻辑问题，因为前面的操作并没有明确表明总是返回True，可能需要根据实际情况调整），第二个元素是""（可能是根据前面的逻辑，如果git diff成功则diffOutput为空字符串）。
        return (False, diffOutput)

    def verifyFile(self, fileName, printDiff):## 创建一个命令列表，以self.command开头，添加self.verifyCommandArguments中的元素，最后添加fileName
        commandList = [self.command]#创建了一个初始的命令列表commandList，它以self.command作为起始元素。
        commandList.extend(self.verifyCommandArguments)
        commandList.append(fileName)#将fileName添加到commandList中。
        try:
            verifyOutput = subprocess.check_output(commandList, stderr=subprocess.STDOUT)# 使用subprocess.check_output执行命令列表中的命令，并获取输出
        except subprocess.CalledProcessError as e:#如果执行命令时出现subprocess.CalledProcessError异常（即命令执行失败），则执行以下操作。
            cprint("[ERROR] " + fileName + " (" + e.output.rstrip('\r\n') + ")", "red")#使用cprint（假设是自定义的打印函数，可能用于彩色输出）输出错误信息，包括文件名和错误输出内容（去除末尾的\r\n），颜色为红色。
            return True

        diffOutput = ""#初始化diffOutput为空字符串。
        if self.verifyOutputIsDiff:
            diffOutput = verifyOutput
        else:
            status, diffOutput = self.performGitDiff(fileName, verifyOutput)
            if status:#如果status为True，则直接返回True。
                return True

        if diffOutput != "":#如果diffOutput不为空字符串，则执行以下操作。
            cprint("[NOT OK] " + fileName, "red")#使用cprint输出[NOT OK]和文件名，颜色为红色，表示文件验证未通过。
            if printDiff:#如果printDiff为True，则打印diffOutput（去除末尾的\r\n）。然后在136行返回True。
                print(diffOutput.rstrip('\r\n'))
            return True

        print("[OK] " + fileName)#如果前面的条件都不满足，则打印[OK]和文件名，表示文件验证通过。然后在139行返回False。
        return False


class CodeFormatterClang(CodeFormatter):#这是一个名为CodeFormatterClang的类，它继承自CodeFormatter类（虽然代码中没有明确显示CodeFormatter类的定义，但从_init_方法中调用父类的_init_可以推断）。这个类主要是用于处理与clang - format相关的代码格式化功能。
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

    def verifyFormatterVersion(self):# 定义一个方法，用于验证格式化工具的版本
        CodeFormatter.verifyFormatterVersion(self)# 调用父类或基类的 verifyFormatterVersion 方法
        self.verifyClangFormatFileExistsAndMatchesCheckedIn()# 调用自身的另一个方法，用于验证 Clang 格式化文件是否存在并且与预期的一致

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

    #验证与clang-format文件相关的操作
    def verifyClangFormatFileExistsAndMatchesCheckedIn(self):
        #检查已经签入的文件是否存在
        self.verifyCheckedInClangFormatFileExists()
        #用于存储找到的clang-format文件相关信息
        foundClangFormatFiles = sets.Set()
        #遍历self.inputFiles中的每个文件名fileName
        for fileName in self.inputFiles:
            #获取绝对路径的目录部分
            dirName = os.path.dirname(os.path.abspath(fileName))
            #查找结果
            if not self.findClangFormatFileStartingFrom(dirName, fileName, foundClangFormatFiles):
                sys.exit(1)

    #从给定的目录中查找
    def findClangFormatFileStartingFrom(self, dirName, fileName, foundClangFormatFiles):
        #得到可能存在clangFormat文件的完整路径
        clangFormatFile = os.path.join(dirName, CodeFormatterClang.CLANG_FORMAT_FILE)
        #检查上一步构建的路径对应的文件是否存在
        if os.path.exists(clangFormatFile):
            #检查当前找到的clangFormatFile是否在foundClangFormatFiles集合中
            if clangFormatFile not in foundClangFormatFiles:
                #把当前找到的并且之前没有处理过的clangFormatFile添加到foundClangFormatFiles集合中
                foundClangFormatFiles.add(clangFormatFile)
                #比较两个文件内容是否相同
                if os.path.exists(self.checkedInClangFormatFile) and \
                   not filecmp.cmp(self.checkedInClangFormatFile, clangFormatFile):
                    #打印警告信息   
                    cprint("[WARN] " + clangFormatFile + " does not match " + self.checkedInClangFormatFile, "yellow")
                    #调用这个函数来处理文件无法验证的情况   
                    self.confirmWithUserClangFormatFileCantBeVerified()
                #与前面文件不匹配的情况        
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
    # 调用基类的初始化方法，传入一系列参数来配置代码格式化器
    # 这些参数指定了使用的命令、期望的版本、格式化命令的参数、验证命令的参数等
    CodeFormatter.__init__(self,
                           # 指定用于格式化代码的命令是autopep8
                           command="autopep8",
                           # 不指定期望的autopep8版本（可能表示接受任何版本）
                           expectedVersion="",
                           # 格式化命令的参数列表，这里指定了--in-place（原地修改文件）和--max-line-length=119（最大行长度为119）
                           formatCommandArguments=["--in-place", "--max-line-length=119"],
                           # 验证命令的参数列表，这里指定了--diff（显示差异）和--max-line-length=119（同上）
                           # 验证命令通常用于检查代码是否符合格式要求，而不实际修改代码
                           verifyCommandArguments=["--diff", "--max-line-length=119"],
                           # 指定验证命令的输出应该是diff格式，这有助于比较代码修改前后的差异
                           verifyOutputIsDiff=True,
                           # 指定该格式化器支持的文件扩展名，这里是.py，表示Python文件
                           fileEndings=["py"],
                           # 对支持的文件类型的描述，这里是"python"
                           fileDescription="python",
                           # 指定安装autopep8的命令，这里使用了sudo apt-get来安装python-pep8（注意：python-pep8可能已过时，现在通常直接安装autopep8）
                           # 注意：在实际应用中，直接在代码中执行安装命令可能不是最佳实践，因为这可能需要管理员权限，并且可能会干扰用户的系统
                           installCommand="sudo apt-get install python-pep8 python-autopep8")
       
class CodeFormat:

    def __init__(self):
        self.failure = False
        self.codeFormatterInstances = []
        return

    import argparse
import os
import re
# 假设cprint和SCRIPT_VERSION是从某个模块中导入的，这里不展示该模块的导入代码
# from some_module import cprint, SCRIPT_VERSION

class CodeFormatterManager:  # 假设这个类名是根据上下文推断的，因为您没有提供类定义
    def parseCommandLine(self):
        # 创建一个ArgumentParser对象，用于解析命令行参数
        parser = argparse.ArgumentParser(
            description="Helper script for code formatting.")
        
        # 添加一个位置参数，用于指定要处理的文件或目录，允许多个值
        parser.add_argument("input", nargs="+",
                            help="files or directories to process")
        
        # 添加一个可选参数-v或--verify，如果指定，则不更改文件，只验证格式是否正确
        parser.add_argument("-v", "--verify", action="store_true",
                            help="do not change file, but only verify the format is correct")
        
        # 添加一个可选参数-d或--diff，如果指定，则显示差异，并隐含验证模式
        parser.add_argument("-d", "--diff", action="store_true",
                            help="show diff, implies verify mode")
        
        # 添加一个可选参数-e或--exclude，用于指定要从输入中排除的文件或目录名称中包含的单词列表
        parser.add_argument("-e", "--exclude", nargs="+", metavar="exclude",
                            help="exclude files or directories containing words from the exclude list in their names")
        
        # 添加一个可选参数-y或--yes，如果指定，则在格式化多个文件之前不询问确认
        parser.add_argument("-y", "--yes", action="store_true",
                            help="do not ask for confirmation before formatting more than one file")
        
        # 添加一个版本参数--version，显示脚本的版本信息
        parser.add_argument("--version", action="version", version="%(prog)s " + SCRIPT_VERSION)
        
        # 解析命令行参数，并将结果存储在self.args中
        self.args = parser.parse_args()
        
        # 如果指定了--diff参数，则自动将--verify参数设置为True
        if self.args.diff:
            self.args.verify = True

    def addCodeFormatter(self, codeFormatterInstance):
        # 将一个代码格式化器实例添加到self.codeFormatterInstances列表中
        self.codeFormatterInstances.append(codeFormatterInstance)

    def scanForInputFiles(self):
        # 遍历self.codeFormatterInstances列表中的每个代码格式化器实例
        for formatterInstance in self.codeFormatterInstances:
            # 根据代码格式化器实例支持的文件后缀构建正则表达式模式
            filePattern = re.compile("^[^.].*\.(" + "|".join(formatterInstance.fileEndings) + ")$")
            
            # 初始化代码格式化器实例的inputFiles列表为空
            formatterInstance.inputFiles = []
            
            # 遍历命令行参数中指定的每个文件或目录
            for fileOrDirectory in self.args.input:
                # 如果文件或目录存在
                if os.path.exists(fileOrDirectory):
                    # 调用scanFileOrDirectory方法（该方法未在代码片段中定义，可能是类的其他部分或外部函数）
                    # 将匹配的文件添加到代码格式化器实例的inputFiles列表中
                    formatterInstance.inputFiles.extend(self.scanFileOrDirectory(fileOrDirectory, filePattern))
                else:
                    # 如果文件或目录不存在，则打印警告信息（cprint函数未在代码片段中定义，可能是从某个模块导入的）
                    cprint("[WARN] Cannot find '" + fileOrDirectory + "'", "yellow")

# 注意：scanFileOrDirectory方法和cprint函数以及SCRIPT_VERSION常量在提供的代码片段中未定义，
# 这里假设它们是类的其他部分或外部模块中定义的。

    def scanFileOrDirectory(self, fileOrDirectory, filePattern):
        """
        扫描指定的文件或目录，并返回与给定文件模式匹配且未被排除的文件列表。

        参数:
            fileOrDirectory (str): 要扫描的文件或目录的路径。
            filePattern (re.Pattern): 用于匹配文件名的正则表达式模式。

        返回:
            list: 与文件模式匹配且未被排除的文件路径列表。
        """
        fileList = []  # 初始化空列表以存储匹配的文件路径

        # 检查给定路径是否为目录
        if os.path.isdir(fileOrDirectory):
            # 使用os.walk遍历目录树
            for root, directories, fileNames in os.walk(fileOrDirectory):
                # 过滤目录列表（排除隐藏目录和包含特定忽略文件的目录）
                directories[:] = self.filterDirectories(root, directories)
                
                # 遍历当前目录下的文件名
                for filename in filter(lambda name: filePattern.match(name), fileNames):
                    # 构建文件的完整路径
                    fullFilename = os.path.join(root, filename)
                    # 检查文件是否未被排除
                    if self.isFileNotExcluded(fullFilename):
                        # 将文件添加到列表中
                        fileList.append(fullFilename)
        else:
            # 如果给定路径不是目录，则检查它是否是一个文件且未被排除
            if self.isFileNotExcluded(fileOrDirectory) and (filePattern.match(os.path.basename(fileOrDirectory)) is not None):
                # 将文件添加到列表中
                fileList.append(fileOrDirectory)
        
        # 返回匹配的文件列表
        return fileList

    def filterDirectories(self, root, directories):
        """
        过滤目录列表，排除隐藏目录和包含特定忽略文件的目录。

        参数:
            root (str): 当前遍历的根目录路径。
            directories (list): 要过滤的目录名称列表。

        返回:
            list: 过滤后的目录名称列表。
        """
        # 假设CodeFormatterClang.CODE_FORMAT_IGNORE_FILE是一个类变量，表示忽略文件的名称
        # 这里需要注意，因为CodeFormatterClang类在代码片段中没有定义，我们假设它是存在的
        ignore_file = CodeFormatterClang.CODE_FORMAT_IGNORE_FILE  # 获取忽略文件的名称

        # 使用列表推导式过滤目录
        directories[:] = [directory for directory in directories if
                          # 排除以点开头的隐藏目录
                          not directory.startswith(".") and
                          # 排除包含忽略文件的目录
                          not os.path.exists(os.path.join(root, directory, ignore_file))]
        
        # 返回过滤后的目录列表
        return directories

    def isFileNotExcluded(self, fileName):#定义一个类的方法，用于判断文件是否不被排除
        if self.args.exclude is not None:    # 如果存在排除规则（self.args.exclude不为None）
            for excluded in self.args.exclude:        # 遍历所有的排除项
                if excluded in fileName:            # 如果文件名包含任何一个排除项，则文件被排除，返回False
                    return False
        if os.path.islink(fileName):    # 如果文件是一个符号链接（软链接），则认为文件被排除，返回False
            return False
    # 如果文件不满足以上被排除的条件，则返回True，表示文件不被排除
        return True

    def confirmWithUserFileIsOutsideGit(self, fileName):
        cprint("[WARN] File is not in a Git repo: " + fileName, "yellow")
        answer = raw_input("Are you sure to code format it anyway? (y/Q)")
        if answer != "y":
            sys.exit(1)
# 定义一个类的方法，用于当文件不在Git仓库时与用户确认是否仍要对其进行代码格式化操作
    def confirmWithUserFileIsUntracked(self, fileName):    # 打印警告信息，提示文件不在Git仓库
        cprint("[WARN] File is untracked in Git: " + fileName, "yellow")    # 获取用户输入，询问用户是否仍要格式化代码
        answer = raw_input("Are you sure to code format it anyway? (y/Q)")    # 如果用户输入不是"y"，则以错误状态退出程序（sys.exit(1)）
        if answer != "y":
            sys.exit(1)

    def confirmWithUserGitRepoIsNotClean(self, gitRepo):# 定义一个类的方法，用于当文件在Git中未被跟踪时与用户确认是否仍要对其进行代码格式化操作
        cprint("[WARN] Git repo is not clean: " + gitRepo, "yellow")    # 打印警告信息，提示文件在Git中未被跟踪
        answer = raw_input("Are you sure to code format files in it anyway? (y/Q)")    # 获取用户输入，询问用户是否仍要格式化代码
        if answer != "y":    # 如果用户输入不是"y"，则以错误状态退出程序（sys.exit(1)）
            sys.exit(1)

    def checkInputFilesAreInCleanGitReposAndAreTracked(self):
        if self.args.verify or self.args.yes:
            return
        gitRepos = sets.Set()#gitRepos = sets.Set()创建了一个空集合。这个集合将用来存储已经处理过的Git仓库路径，目的是避免对同一个Git仓库进行重复的检查。
        for formatterInstance in self.codeFormatterInstances:
            for fileName in formatterInstance.inputFiles:
                gitRepo = self.getGitRepoForFile(fileName)#gitRepo = self.getGitRepoForFile(fileName)调用了getGitRepoForFile函数来获取fileName所在的Git仓库的路径。
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
        """
        确定给定文件是否位于Git仓库中，并返回该仓库的顶级目录路径。

        参数:
            fileName (str): 要检查的文件路径。

        返回:
            str 或 None: 如果文件位于Git仓库中，则返回仓库的顶级目录路径；否则返回None。
        """
        # 首先检查文件是否位于Git仓库内
        if not self.isInsideGitRepo(fileName):
            return None

        try:
            # 尝试在文件的所在目录执行'git rev-parse --show-toplevel'命令
            gitProcess = subprocess.Popen(
                ["git", "rev-parse", "--show-toplevel"],
                stdin=subprocess.PIPE,  # 不需要向git进程发送输入，但保持PIPE以避免错误
                stdout=subprocess.PIPE,  # 捕获git进程的输出
                stderr=subprocess.PIPE,  # 捕获git进程的错误输出（虽然这里未使用）
                cwd=os.path.dirname(fileName)  # 在文件的父目录中执行命令
            )
            # 与git进程通信并获取其输出
            gitOutput, _ = gitProcess.communicate()

            # 检查git进程的返回码是否为0（表示成功）
            if gitProcess.returncode == 0:
                # 移除输出字符串末尾的换行符（如果存在）
                # 注意：这里假设gitOutput是字节串，因此使用rstrip('\r\n')后可能需要解码
                # 但由于我们直接返回了字节串，所以这里不进行解码操作
                # 如果需要字符串，可以添加.decode('utf-8')
                return gitOutput.rstrip(b'\r\n')  # 返回仓库的顶级目录路径（字节串形式）

        except OSError:
            # 如果在尝试执行git命令时发生OS错误（例如，git未安装）
            # 使用cprint打印错误信息（这里假设cprint能够处理彩色输出）
            # 注意：cprint函数和模块需要事先定义或导入
            cprint("[ERROR] Failed to run 'git rev-parse --show-toplevel' for " + fileName, "red")

        # 如果发生任何错误，返回None
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
#此函数用来检查指定的文件是否被Git版本控制系统追踪
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
#检查给定的Git仓库是否干净
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
#尝试刷新Git索引，忽略子模块
    def gitUpdateIndexRefresh(self, gitRepo):
    # 尝试执行 git update-index 命令来刷新索引
    try:
        # 在指定的 gitRepo 目录下执行 git update-index 命令
        gitProcess = subprocess.Popen(
            ["git", "update-index", "-q", "--ignore-submodules", "--refresh"],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            cwd=gitRepo
        )
        # 等待进程完成并获取输出（此处忽略输出）
        _, _ = gitProcess.communicate()
        # 如果进程返回码为 0，表示成功
        if gitProcess.returncode == 0:
            return True
    # 如果发生 OSError 异常（例如 Git 不可用）
    except OSError:
        # 打印错误信息
        cprint("[ERROR] Failed to run 'git update-index -q --ignore-submodules --refresh' for " + gitRepo, "red")
    # 如果以上步骤失败，返回 False
    return False

def verifyFormatterVersion(self):
    # 遍历 codeFormatterInstances 列表
    for formatterInstance in self.codeFormatterInstances:
        # 如果 formatterInstance 的 inputFiles 列表不为空
        if len(formatterInstance.inputFiles) > 0:
            # 调用 formatterInstance 的 verifyFormatterVersion 方法
            formatterInstance.verifyFormatterVersion()
            
    def printMode(self):#这个函数的目的是根据self.args.verify的值来打印不同的模式。
        if self.args.verify:
            cprint("VERIFY MODE", attrs=["bold"])
        else:
            cprint("FORMAT MODE", attrs=["bold"])

    def processFiles(self):#这个函数的目的是处理文件，根据self.args.verify的值执行不同的操作。
        for formatterInstance in self.codeFormatterInstances:
            for fileName in formatterInstance.inputFiles:
                if self.args.verify:#对于每个文件，再次检查self.args.verify的值
                    self.failure |= formatterInstance.verifyFile(fileName, self.args.diff)#如果self.args.verify为True，则调用formatterInstance.verifyFile函数处理文件fileName，并且将结果与self.failure进行按位或操作
                else:
                    self.failure |= formatterInstance.formatFile(fileName)#如果self.args.verify为False，则调用formatterInstance.formatFile函数处理文件fileName，并且将结果与self.failure进行按位或操作

    def numberOfInputFiles(self):#这个函数的目的是计算输入文件的数量。
        count = 0#首先初始化一个变量count为0
        for formatterInstance in self.codeFormatterInstances:#然后遍历self.codeFormatterInstances
            count += len(formatterInstance.inputFiles)#对于每个formatterInstance，将其inputFiles的长度累加到count中
        return count

    def confirmWithUser(self):#这个函数的目的是与用户确认操作。
        if self.numberOfInputFiles() == 0:
            cprint("[WARN] No input files (or file endings unknown)", "yellow")
        elif (not self.args.verify) and (not self.args.yes) and self.numberOfInputFiles() > 1:
            for formatterInstance in self.codeFormatterInstances:
                formatterInstance.printInputFiles()
            answer = raw_input("Are you sure to code format " + str(self.numberOfInputFiles()) + " files? (y/N)")
            #如果用户输入不是y，则使用sys.exit(1)退出程序（第12行）。这里sys.exit(1)表示以非0状态退出，通常表示程序出现错误或者用户取消操作。
            if answer != "y":
                sys.exit(1)



def main():
    # 创建CodeFormat类的实例，这个类可能封装了代码格式化的整个流程
    codeFormat = CodeFormat()
    
    # 解析命令行参数，可能包括输入文件路径、格式化选项等
    codeFormat.parseCommandLine()
    
    # 打印当前的操作模式或配置信息
    codeFormat.printMode()
    
    # 添加Clang风格的代码格式化器，用于C/C++代码的格式化
    codeFormat.addCodeFormatter(CodeFormatterClang())
    
    # 添加Autopep8的代码格式化器，用于Python代码的格式化
    codeFormat.addCodeFormatter(CodeFormatterAutopep())
    
    # 扫描指定目录或文件，找到需要格式化的代码文件
    codeFormat.scanForInputFiles()
    
    # 验证所使用的格式化器版本是否与要求相匹配
    codeFormat.verifyFormatterVersion()
    
    # 在继续之前请求用户的确认，以避免意外地修改文件
    codeFormat.confirmWithUser()
    
    # 检查输入文件是否位于干净的Git仓库中，并且这些文件是否被Git跟踪
    # 这是为了确保代码格式化操作在一个可预测的环境中执行
    codeFormat.checkInputFilesAreInCleanGitReposAndAreTracked()
    
    # 对找到的文件执行格式化操作
    codeFormat.processFiles()
    
    # 检查是否有任何格式化操作失败
    if codeFormat.failure:
        # 如果有失败，则以红色打印“FAILURE”并以状态码1退出程序
        # cprint可能是一个自定义或第三方库函数，用于彩色打印
        cprint("FAILURE", "red")
        sys.exit(1)
    else:
        # 如果没有失败，则以绿色打印“SUCCESS”并以状态码0退出程序
        cprint("SUCCESS", "green")
        sys.exit(0)

if __name__ == "__main__":
    main()
