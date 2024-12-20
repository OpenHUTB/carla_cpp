#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import os
import yaml
import re

# 定义表示方法的颜色代码（通常可能用于文档中对方法相关内容进行特定颜色标识等，比如在生成文档时设置对应文本颜色）
COLOR_METHOD = '#7fb800'
# 定义表示参数的颜色代码（类似方法颜色代码的用途，用于文档里对参数相关文本进行颜色设置）
COLOR_PARAM = '#00a6ed'
# 定义表示实例变量的颜色代码（用于文档中实例变量相关文本的颜色标识）
COLOR_INSTANCE_VAR = '#f8805a'
# 定义表示注释的颜色代码（可能用于文档里注释内容的颜色显示区分）
COLOR_NOTE = '#8E8E8E'
# 定义表示警告的颜色代码（在文档展示警告相关信息时设置对应文本颜色）
COLOR_WARNING = '#ED2F2F'

# 编译一个正则表达式对象，用于匹配特定格式的字符串（可能是以'Carla'开头，后面跟着'.'以及一些字母数字下划线组合的字符串形式）
QUERY = re.compile(r'([cC]arla(\.[a-zA-Z0-9_]+)+)')

def create_hyperlinks(text):
    """
    此函数使用正则表达式将输入文本中匹配特定格式（由QUERY正则表达式定义）的字符串转换为带有超链接格式的字符串。
    超链接的格式是将匹配的字符串用方括号括起来，并在后面添加'(#匹配的字符串)'这样的形式，目的可能是用于在文档中生成指向对应内容的链接。

    参数：
    text：要进行处理的原始文本字符串。

    返回值：
    返回处理后的包含超链接格式字符串的文本。
    """
    return re.sub(QUERY, r'[\1](#\1)', text)

def create_getter_setter_hyperlinks(text):
    """
    功能与create_hyperlinks类似，同样是使用正则表达式对输入文本进行处理，将匹配特定格式（由QUERY正则表达式定义）的字符串转换为带有超链接格式的字符串。
    具体应用场景可能和处理获取器、设置器相关内容生成超链接有关，但目前函数体实现和create_hyperlinks一致，可能后续有差异化扩展。

    参数：
    text：待处理的原始文本字符串。

    返回值：
    返回处理后的包含超链接格式字符串的文本。
    """
    return re.sub(QUERY, r'[\1](#\1)', text)

def join(elem, separator=''):
    """
    将输入的可迭代元素（如列表等）连接成一个字符串，使用指定的分隔符进行分隔。

    参数：
    elem：可迭代的元素，例如列表中的元素会被连接起来。
    separator：用于连接元素的分隔符，默认为空字符串，可根据需要传入其他字符或字符串来指定分隔样式。

    返回值：
    返回连接后的字符串。
    """
    return separator.join(elem)


class MarkdownFile:
    """
    用于处理Markdown文件相关内容的类，主要用于构建和管理Markdown格式文本内容。
    """
    def __init__(self):
        """
        类的初始化方法，初始化一些实例变量。
        """
        # 用于存储Markdown文件的内容数据，初始为空字符串
        self._data = ""
        # 用于记录列表的深度（可能用于在生成Markdown列表时控制缩进等格式），初始为0
        self._list_depth = 0
        # 定义换行符及缩进格式，用于在添加文本内容时控制换行和缩进效果
        self.endl = '  \n'

    def data(self):
        """
        获取当前存储的Markdown文件内容数据。

        返回值：
        返回存储的Markdown文件内容字符串。
        """
        return self._data

    def list_depth(self):
        """
        根据当前Markdown文件内容的状态（主要看最后一个字符是否为换行符以及列表深度是否为0），返回相应的缩进空格字符串。
        如果内容末尾不是换行符或者列表深度为0，则返回空字符串；否则根据列表深度返回对应数量的空格缩进字符串，用于控制文本格式。

        返回值：
        返回合适的缩进空格字符串用于后续文本添加时的格式控制。
        """
        if self._data.strip()[-1:]!= '\n' or self._list_depth == 0:
            return ''
        return join(['    ' * self._list_depth])

    def textn(self, buf):
        """
        将传入的文本缓冲区内容添加到当前存储的Markdown文件内容数据中，同时会根据列表深度添加合适的缩进，并在末尾添加定义好的换行符及缩进格式。

        参数：
        buf：要添加的文本缓冲区内容字符串。
        """
        self._data = join([self._data, self.list_depth(), buf, self.endl])



class Documentation:
    """Main documentation class"""
    def __init__(self, path, images_path):
        """
        此类的初始化方法，用于初始化与文档相关的一些路径和文件列表等信息。

        参数：
        path：文档相关的基础路径，可能是文档所在的根目录等，用于后续拼接其他路径来定位具体文件或文件夹。
        images_path：存放代码片段相关图像文件的路径，用于后续查找和处理这些图像文件。
        """
        # 拼接得到存放代码片段（snipets）的文件夹路径，这里是基于传入的文档路径取其所在目录，然后拼接'snipets'文件夹名
        self._snipets_path = os.path.join(os.path.dirname(path), 'snipets')
        # 获取代码片段文件夹下所有以'.py'结尾的文件列表，也就是筛选出Python代码片段文件
        self._files = [f for f in os.listdir(self._snipets_path) if f.endswith('.py')]
        # 初始化一个空列表，用于存储代码片段文件的具体路径
        self._snipets = list()
        for snipet_file in self._files:
            current_snipet_path = os.path.join(self._snipets_path, snipet_file)
            self._snipets.append(current_snipet_path)
        # Gather snipet images
        # 记录代码片段图像文件所在的路径
        self._snipets_images_path = images_path
        # 获取代码片段图像文件夹下的所有文件列表
        self._files_images = [f for f in os.listdir(self._snipets_images_path)]
        # 初始化一个空列表，用于存储代码片段图像文件的具体路径
        self._snipets_images = list()
        for snipet_image in self._files_images:
            current_image_path = os.path.join(self._snipets_images_path, snipet_image)
            self._snipets_images.append(current_image_path)
    def gen_body(self):
        """Generates the documentation body"""
        md = MarkdownFile()
        # Create header for snipets (div container and script to copy)
        md.textn(
        "[comment]: <> (=========================)\n"+
        "[comment]: <> (PYTHON API SCRIPT SNIPETS)\n"+
        "[comment]: <> (=========================)\n"+
        "<div id=\"snipets-container\" class=\"Container\" onmouseover='this.style[\"overflowX\"]=\"scroll\";' onmouseout='this.style[\"overflowX\"]=\"visible\";'></div>\n"+
        "<script>\n"+
        "function CopyToClipboard(containerid) {\n"+
        "if (document.selection) {\n"+
        "var range = document.body.createTextRange();\n"+
        "range.moveToElementText(document.getElementById(containerid));\n"+
        "range.select().createTextRange();\n"+
        "document.execCommand(\"copy\");\n"+
        "} \n"+
        "else if (window.getSelection) {\n"+
        "var range = document.createRange();\n"+
        "range.selectNode(document.getElementById(containerid));\n"+
        "window.getSelection().addRange(range);\n"+
        "document.execCommand(\"copy\");\n"+
        "}\n"+
        "}\n</script>\n"+
        "<script>\n"+
        "function CloseSnipet() {\n"+
        "document.getElementById(\"snipets-container\").innerHTML = null;\n"+
        "}\n"+
        "</script>\n")
        # Create content for every snipet
        for snipet_path in self._snipets:
            current_snipet = open(snipet_path, 'r')
            snipet_name = os.path.basename(current_snipet.name) # Remove path
            snipet_name = os.path.splitext(snipet_name)[0] # Remove extension
            # Header for a snipet
            md.textn("<div id =\""+snipet_name+"-snipet\" style=\"display: none;\">\n"+
            "<p class=\"SnipetFont\">\n"+
            "Snippet for "+snipet_name+"\n"+
            "</p>\n"+
            "<div id=\""+snipet_name+"-code\" class=\"SnipetContent\">\n\n```py\n")
            # The snipet code
            md.textn(current_snipet.read())
            # Closing for a snipet
            md.textn("\n```\n<button id=\"button1\" class=\"CopyScript\" onclick=\"CopyToClipboard('"+snipet_name+"-code')\">Copy snippet</button>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<button id=\"button1\" class=\"CloseSnipet\" onclick=\"CloseSnipet()\">Close snippet</button><br><br>\n")
            # Check if snipet image exists, and add it
            for snipet_path_to_image in self._snipets_images:
                snipet_image_name = os.path.splitext(os.path.basename(snipet_path_to_image))[0]
                if snipet_name == snipet_image_name:
                    #在md.text中添加图像源路径
                    md.textn("\n<img src=\"/img/snipets_images/"+os.path.basename(snipet_path_to_image)+"\">\n")
                    #在md.text中添加HTML的div标签结束部分
            md.textn("</div>\n")
        # Closing div
        #在md.text中添加HTML的div标签结束部分
        md.textn("\n</div>\n")
        返回md.data 的字符串表现形式
        return md.data().strip()


    #生成整个标记文件的函数
    def gen_markdown(self):
        #返回连接后的字符串
        """Generates the whole markdown file"""
        return join([self.gen_body()], '\n').strip()


#主函数
def main():
    """Main function"""
    print("Generating PythonAPI snipets...")
    #脚本路径
    script_path = os.path.dirname(os.path.abspath(__file__)+'/snipets')
    snipets_images_path = os.path.dirname(os.path.dirname(os.path.dirname(
        os.path.abspath(__file__)))) + '/Docs/img/snipets_images'
    #文档对象
    docs = Documentation(script_path, snipets_images_path)
    #代码片段标记文档路径
    snipets_md_path = os.path.join(os.path.dirname(os.path.dirname(
        os.path.dirname(script_path))), 'Docs/python_api_snipets.md')
    with open(snipets_md_path, 'w') as md_file:
        md_file.write(docs.gen_markdown())
    print("Done snipets!")


if __name__ == "__main__":
    main()
