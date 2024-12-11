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
import doc_gen_snipets

# 定义用于在文档中表示不同元素的颜色代码（可能用于后续在生成的文档中进行相应元素的颜色标识）
COLOR_METHOD = '#7fb800'
COLOR_PARAM = '#00a6ed'
COLOR_INSTANCE_VAR = '#f8805a'
COLOR_NOTE = '#8E8E8E'
COLOR_WARNING = '#ED2F2F'

# 编译一个正则表达式对象，用于匹配特定格式的字符串（以Carla开头的带点号分隔的标识符形式）
QUERY = re.compile(r'([cC]arla(\.[a-zA-Z0-9_]+)+)')


# 函数功能：将匹配到的文本转换为带有超链接格式的文本，超链接指向自身（用于文档内的跳转链接创建）
def create_hyperlinks(text):
    return re.sub(QUERY, r'[\1](#\1)', text)


# 与create_hyperlinks功能类似，可能后续用于特定的获取器/设置器相关的超链接创建
def create_getter_setter_hyperlinks(text):
    return re.sub(QUERY, r'[\1](#\1)', text)


# 将可迭代元素使用指定的分隔符连接成字符串
def join(elem, separator=''):
    return separator.join(elem)


# 表示一个Markdown文件的类，用于逐步构建Markdown文档内容
class MarkdownFile:
    def __init__(self):
        # 存储Markdown文档的内容
        self._data = ""
        # 用于记录列表的嵌套深度
        self._list_depth = 0
        # 定义换行符格式（包含空格缩进，用于使生成的文档格式更美观）
        self.endl ='  \n'

    # 获取当前构建的Markdown文档内容
    def data(self):
        return self._data

    # 将一个元素添加到列表中（如果提供了buf参数，则添加带有格式的元素到列表开头）
    def list_push(self, buf=''):
        if buf:
            self.text(join([
                '    ' * self._list_depth if self._list_depth!= 0 else '', '- ', buf]))
        self._list_depth = (self._list_depth + 1)

    # 与list_push类似，但添加元素后会自动添加换行符
    def list_pushn(self, buf):
        self.list_push(join([buf, self.endl]))

    # 减少列表的嵌套深度（模拟列表结束的操作）
    def list_pop(self):
        self._list_depth = max(self._list_depth - 1, 0)

    # 减少列表嵌套深度并添加一个换行符到文档内容中
    def list_popn(self):
        self.list_pop()
        self._data = join([self._data, '\n'])

    # 根据当前列表深度返回对应的缩进字符串（如果数据末尾不是换行符或者列表深度为0则返回空字符串）
    def list_depth(self):
        if self._data.strip()[-1:]!= '\n' or self._list_depth == 0:
            return ''
        return join(['    ' * self._list_depth])

    # 在文档内容中添加一个水平分隔线
    def separator(self):
        self._data = join([self._data, '\n---\n'])

    # 在文档内容中添加一个换行符
    def new_line(self):
        self._data = join([self._data, self.endl])

    # 将给定的文本添加到文档内容中
    def text(self, buf):
        self._data = join([self._data, buf])

    # 将给定的文本添加到文档内容中，并根据当前列表深度添加缩进以及换行符
    def textn(self, buf):
        self._data = join([self._data, self.list_depth(), buf, self.endl])

    # 在文档开头添加一级标题（Python API reference）
    def first_title(self):
        self._data = join([
            self._data, '#Python API reference\n'])

    # 添加指定级别（通过strongness参数指定）的标题到文档中
    def title(self, strongness, buf):
        self._data = join([
            self._data, '\n', self.list_depth(), '#' * strongness, ' ', buf, '\n'])

    # 添加指定级别（通过strongness参数指定）的HTML格式标题到文档中（不同级别有不同的样式设置）
    def title_html(self, strongness, buf):
        if strongness == 5:
            self._data = join([
                self._data, '\n', self.list_depth(), '<h', str(strongness), ' style="margin-top: -20px">', buf, '</h', str(strongness),'>\n','<div style="padding-left:30px;margin-top:-25px"></div>'])
        else:
            self._data = join([
                self._data, '\n', self.list_depth(), '<h', str(strongness), '>', buf, '</h', str(strongness), '>\n'])

    # 添加继承相关的提示信息（表明从哪个类继承而来）到文档中
    def inherit_join(self, inh):
        self._data = join([
            self._data, '<small style="display:block;margin-top:-20px;">Inherited from ', inh, '</small></br>\n'])

    # 添加普通的文本内容（可能后续用于添加各种说明、注释等信息）到文档中
    def note(self, buf):
        self._data = join([self._data, buf])

    # 添加代码块到文档中，可指定代码块的语言（用于语法高亮等功能）
    def code_block(self, buf, language=''):
        return join(['```', language, '\n', self.list_depth(), buf, '\n', self.list_depth(), '```\n'])

    # 对文档字符串进行美化处理，确保结尾有合适的标点符号（如果没有则添加句号）
    def prettify_doc(self, doc):
        punctuation_marks = ['.', '!', '?']
        doc = doc.strip()
        doc += '' if doc[-1:] in punctuation_marks else '.'
        return doc


# 将文本转换为斜体格式
def italic(buf):
    return join(['_', buf, '_'])


# 将文本转换为粗体格式
def bold(buf):
    return join(['**', buf, '**'])

# 创建一个表示代码片段按钮的HTML元素（带有特定的样式和点击事件相关的属性）
def snipet(name,class_key):
    return join(["<button class=\"SnipetButton\" id=\"",class_key,".",name,"-snipet_button\">", "snippet &rarr;", '</button>'])


# 将文本用反引号包裹，用于表示代码样式
def code(buf):
    return join(['`', buf, '`'])


# 将文本用方括号包裹
def brackets(buf):
    return join(['[', buf, ']'])


# 将文本用圆括号包裹
def parentheses(buf):
    return join(['(', buf, ')'])


# 将文本用<small>标签包裹，用于表示小号字体（可能用于显示一些辅助说明等内容）
def small_html(buf):
    return join(['<small>', buf, '</small>'])


# 将文本用<sub><sup>标签包裹，功能不太明确，可能用于特定的排版样式
def small(buf):
    return join(['<sub><sup>', buf, '</sup></sub>'])


# 将文本用<sub>标签包裹，用于表示下标（可能用于数学公式等场景的排版）
def sub(buf):
    return join(['<sub>', buf, '</sub>'])


# 创建一个HTML的锚点元素（用于文档内跳转定位），指定名称为传入的buf参数
def html_key(buf):
    return join(['<a name="', buf, '"></a>'])


# 将文本用指定颜色的<font>标签包裹，用于改变文本颜色（可能用于文档中突出显示不同类型的内容）
def color(col, buf):
    return join(['<font color="', col, '">', buf, '</font>'])


# 检查给定的字典中是否存在指定的键值对（即键存在且对应的值不为空等情况）
def valid_dic_val(dic, value):
    return value in dic and dic[value]


# 表示一个YAML文件的类，用于读取和验证YAML文件内容
class YamlFile:
    """Yaml file class"""
    def __init__(self, path):
        # 存储YAML文件的路径
        self._path = path
        # 读取并解析YAML文件内容
        with open(path) as yaml_file:
            self.data = yaml.safe_load(yaml_file)
        # 验证YAML文件内容的合法性
        self.validate()

    # 验证YAML文件内容是否合法，检查模块、类、实例变量、方法等相关的各种关键信息是否完整正确
    def validate(self):
        # 打印正在验证的文件名（替换路径分隔符并获取文件名部分）
        # print('Validating ' + str(self._path.replace('\\', '/').split('/')[-1:][0]))
        if self.data is None:
            print('\n[ERROR] File: ' + self._path)
            print("This file has no data:")
            exit(0)
        for module in self.data:
            if 'module_name' in module and module['module_name'] is None:
                print('\n[ERROR] File: ' + self._path)
                print("'module_name' is empty in:")
                exit(0)
            if 'classes' in module:
                if not module['classes']:
                    print('\n[ERROR] File: ' + self._path)
                    print("'classes' is empty in:")
                    exit(0)
                for cl in module['classes']:
                    if 'class_name' in cl and cl['class_name'] is None:
                        print('\n[ERROR] File: ' + self._path)
                        print("'class_name' is empty in:")
                        exit(0)
                    if 'instance_variables' in cl and cl['instance_variables']:
                        for iv in cl['instance_variables']:
                            if 'var_name' not in iv:
                                print('\n[ERROR] File: ' + self._path)
                                print("'var_name' not found inside 'instance_variables' of class: " + cl['class_name'])
                                exit(0)
                            if 'var_name' in iv and iv['var_name'] is None:
                                print('\n[ERROR] File: ' + self._path)
                                print("'var_name' is empty in:")
                                exit(0)
                    if 'methods' in cl and cl['methods']:
                        for met in cl['methods']:
                            if 'def_name' not in met:
                                print('\n[ERROR] File: ' + self._path)
                                print("'def_name' is empty in:")
                                exit(0)
                            if 'def_name' in met and met['def_name'] is None:
                                print('\n[ERROR] File: ' + self._path)
                                print("'def_name' is empty in:")
                                exit(0)
                            if 'params' in met and met['params']:
                                for param in met['params']:
                                    if 'param_name' not in param:
                                        print('\n[ERROR] File: ' + self._path)
                                        print("'param_name' not found inside 'params' of class: " + cl['class_name'])
                                        exit(0)
                                    if 'param_name' in param and param['param_name'] is None:
                                        print('\n[ERROR] File: ' + self._path)
                                        print("'param_name' is empty in:")
                                        exit(0)
                                    if 'type' in param and param['type'] is None:
                                        print('\n[ERROR] File: ' + self._path)
                                        print("'type' is empty in:")
                                        exit(0)

    # 获取YAML文件中定义的所有模块信息
    def get_modules(self):
        return [module for module in self.data]


# 向Markdown文档内容中添加一段用于处理代码片段按钮点击事件的JavaScript脚本代码
def append_snipet_button_script(md):
    md.textn("\n\n<script>\n"+
                "function ButtonAction(container_name){\n"+
                    "if(window_big){\n"+
                        "snipet_name = container_name.replace('-snipet_button','-snipet');\n"+
                        "document.getElementById(\"snipets-container\").innerHTML = document.getElementById(snipet_name).innerHTML;\n"+
                    "}\n"+
                    "else{\n"+
                        "document.getElementById(\"snipets-container\").innerHTML = null;"+
                        "code_name = container_name.replace('-snipet_button','-code');\n"+
                        "var range = document.createRange();\n"+
                        "range.selectNode(document.getElementById(code_name));\n"+
                        "alert(range);\n"+
                    "}\n"+
                "}\n"+
                "function WindowResize(){\n"+
                    "if(window.innerWidth > 1200){\n"+
                        "window_big = true;\n"+
                    "}\n"+
                    "else{\n"+
                        "window_big = false;\n"+
                    "}\n"+
                "}\n"+

                "var window_big;\n"+
                "if(window.innerWidth > 1200){\n"+
                    "window_big = true;\n"+
                "}\n"+
                "else{\n"+
                    "window_big = false;\n"+
                "}\n"+

                "buttons = document.getElementsByClassName('SnipetButton')\n"+
                "for (let i = 0; i < buttons.length; i++) {\n"+
                    "buttons[i].addEventListener(\"click\",function(){ButtonAction(buttons[i].id);},true);\n"+
                "}\n"+
                "window.onresize = WindowResize;\n"+
            "</script>\n")


# 向Markdown文档内容中添加代码片段相关的内容（从指定文件读取并添加）
def append_code_snipets(md):
    current_folder = os.path.dirname(os.path.abspath(__file__))
    snipets_path = os.path.join(current_folder, '../../Docs/python_api_snipets.md')
    snipets = open(snipets_path, 'r')
    md.text(snipets.read())
    snipets.close()
    os.remove(snipets_path)


# 根据给定的方法定义信息生成在Python存根文件（stub files）中应有的方法定义格式字符串
def gen_stub_method_def(method):
    """Return python def as it should be written in stub files"""
    param = ''
    method_name = method['def_name']
    for p in method['params']:
        p_type = join([': ', str(p['type'])]) if 'type' in p else ''
        default = join([' = ', str(p['default'])]) if 'default' in p else ''
        param = join([param, p['param_name'], p_type, default, ', '])
    param = param[:-2]  # delete the last ', '
    return_type = join([' -> ', method['return']]) if 'return' in method else ''
    return join([method_name, parentheses(param), return_type])


# 根据给定的方法定义信息生成在文档中应呈现的方法定义格式字符串（带有颜色、超链接等样式）
def gen_doc_method_def(method, class_key, is_indx=False, with_self=True):
    """Return python def as it should be written in docs"""
    param = ''
    snipet_link = ''
    method_name = method['def_name']
    full_method_name = method_name
    if valid_dic_val(method, 'static'):
        with_self = False

    # 对以'_'开头的特殊方法名（如__init__等）进行转义处理，以便在Markdown中正确渲染
    if method_name[0] == '_':
        method_name = '\\' + method_name
    if is_indx:
        method_name = bold(method_name)
    else:
        method_name = bold(color(COLOR_METHOD, method_name))

    if with_self:
        if not 'params' in method or method['params'] is None:
            method['params'] = []
        method['params'].insert(0, {'param_name': 'self'})

    if valid_dic_val(method, 'params'):
        for p in method['params']:
            default = join(['=', str(p['default'])]) if 'default' in p else ''
            if is_indx:
                param = join([param, bold(p['param_name']), default, ', '])
            else:
                param = join([param, color(COLOR_PARAM, bold(p['param_name']) + create_hyperlinks(default)), ', '])

    if with_self:
        method['params'] = method['params'][1:]
        if not method['params']:  # if is empty delete it
            del method['params']

    param = param[:-2]  # delete the last ', '

    # 添加代码片段相关的链接按钮（如果存在对应的代码片段）
    current_folder = os.path.dirname(os.path.abspath(__file__))
    snipets_path = os.path.join(current_folder, '../../Docs/python_api_snipets.md')
    snipets = open(snipets_path, 'r')
    if class_key+'.'+full_method_name+'-snipet' in snipets.read():
        snipet_link = snipet(full_method_name, class_key)

    return join([method_name, parentheses(param),snipet_link])

# 根据给定的双下划线方法（dunder method）定义信息生成在文档中应呈现的方法定义格式字符串（类似gen_doc_method_def，但针对双下划线方法）
def gen_doc_dunder_def(dunder, is_indx=False, with_self=True):
    """Return python def as it should be written in docs"""
    param = ''
    dunder_name = dunder['def_name']
    if valid_dic_val(dunder, 'static'):
        with_self = False

    # 对以'_'开头的双下划线方法名进行转义处理，以便在Markdown中正确渲染
    if dunder_name[0] == '_':
        dunder_name = '\\' + dunder_name
    if is_indx:
        dunder_name = bold(dunder_name)
    else:
        dunder_name = bold(color(COLOR_METHOD, dunder_name))

    if with_self:
        if not 'params' in dunder or dunder['params'] is None:
            dunder['params'] = []
        dunder['params'].insert(0, {'param_name': 'self'})

    if valid_dic_val(dunder, 'params'):
        for p in dunder['params']:
            default = join(['=', str(p['type'])]) if 'type' in p else ''
            if is_indx:
                param = join([param, bold(p['param_name']), default, ', '])
            else:
                param = join([param, color(COLOR_PARAM, bold(p['param_name']) + create_hyperlinks(default)), ', '])

    if with_self:
        dunder['params'] = dunder['params'][1:]
        if not dunder['params']:  # if is empty delete it
            del dunder['params']

    param = param[:-2]  # delete the last ', '
    return join([dunder_name, parentheses(param)])


# 生成实例变量在索引中的展示格式字符串（包含变量名、链接、类型等信息）
def gen_inst_var_indx(inst_var, class_key):
    inst_var_name = inst_var['var_name']
    inst_var_key = join([class_key, inst_var_name], '.')
    return join([
        brackets(bold(inst_var_name)),
        parentheses(inst_var_key), ' ',
        sub(italic('Instance variable'))])


# 生成方法在索引中的展示格式字符串（包含方法定义、链接、类型等信息）
def gen_method_indx(method, class_key):
    method_name = method['def_name']
    method_key = join([class_key, method_name], '.')
    method_def = gen_doc_method_def(method, class_key, True)
    return join([
        brackets(method_def),
        parentheses(method_key), ' ',
        sub(italic('Method'))])


# 向Markdown文档中添加方法的参数相关信息（包括参数名、类型、文档说明等）
def add_doc_method_param(md, param):
    param_name = param['param_name']
    param_type = ''
    param_doc = ''
    param_units = ''
    if valid_dic_val(param, 'type'):
        param_type = create_hyperlinks(param['type'])
    if valid_dic_val(param, 'doc'):
        param_doc = create_hyperlinks(md.prettify_doc(param['doc']))
    if valid_dic_val(param, 'param_units'):
        param_units = small_html(' -'+param['param_units'])
    param_type = '' if not param_type else parentheses(italic(param_type + param_units))
    md.list_push(code(param_name))
    if param_type:
        md.text(''+ param_type)
    if param_doc:
        md.textn(' -'+param_doc)
    else:
        md.new_line()
    md.list_pop()


# 向Markdown文档中添加完整的方法相关信息（定义、文档、参数、返回值、备注、警告等）
def add_doc_method(md, method, class_key):
    method_name = method['def_name']
    method_key = join([class_key, method_name], '.')
    method_def = gen_doc_method_def(method, class_key, False)
    md.list_pushn(join([html_key(method_key), method_def]))

    # 方法文档说明部分
    if valid_dic_val(method, 'doc'):
        md.textn(create_hyperlinks(md.prettify_doc(method['doc'])))

    printed_title = False
    if valid_dic_val(method, 'params'):
        for param in method['params']:
            # is_self = valid_dic_val(param, 'param_name') and param['param_name'] == 'self'
            have_doc = valid_dic_val(param, 'doc')
            have_type = valid_dic_val(param, 'type')
            if not have_doc and not have_type:
                continue
            # 打印一次'Parameters'标题
            if not printed_title:
                printed_title = True
                md.list_push(bold('Parameters:') + '\n')
            add_doc_method_param(md, param)
    if printed_title:
        md.list_pop()

    # 返回值文档说明部分
    if valid_dic_val(method, 'return'):
        md.list_push(bold('Return:') +'')
        return_units = ''
        if valid_dic_val(method, 'return_units'):
            return_units = small_html(' -'+method['return_units'])
        md.textn(italic(create_hyperlinks(method['return']) + return_units))
        md.list_pop()

    # 备注文档说明部分
    if valid_dic_val(method, 'note'):
        md.list_push(bold('Note:') +'')
        md.textn(color(COLOR_NOTE, italic(create_hyperlinks(method['note']))))
        md.list_pop()

    # 警告文档说明部分
    if valid_dic_val(method, 'warning'):
        md.list_push(bold('Warning:') +'')
        md.textn(color(COLOR_WARNING, italic(create_hyperlinks(method['warning']))))
        md.list_pop()

    # 抛出异常文档说明部分
    if valid_dic_val(method, 'raises'):
        md.list_pushn(bold('Raises:') +'' + method['raises'])
        md.list_pop()

    md.list_pop()


# 向Markdown文档中添加获取器/设置器相关的完整信息（类似add_doc_method，但有针对获取器/设置器的额外处理）
def add_doc_getter_setter(md, method, class_key, is_getter, other_list):
    method_name = method['def_name']
    method_key = join([class_key, method_name], '.')
    method_def = gen_doc_method_def(method, class_key, False)
    md.list_pushn(join([html_key(method_key), method_def]))

    # 方法文档说明部分
    if valid_dic_val(method, 'doc'):
        md.textn(create_hyperlinks(md.prettify_doc(method['doc'])))

    printed_title = False
    if valid_dic_val(method, 'params'):
        for param in method['params']:
            # is_self = valid_dic_val(param, 'param_name') and param['param_name'] == 'self'
            have_doc = valid_dic_val(param, 'doc')
            have_type = valid_dic_val(param, 'type')
            if not have_doc and not have_type:
                continue
            # 打印一次'Parameters'标题
            if not printed_title:
                printed_title = True
                md.list_push(bold('Parameters:') + '\n')
            add_doc_method_param(md, param)
    if printed_title:
        md.list_pop()

    # 返回值文档说明部分
    if valid_dic_val(method, 'return'):
        md.list_push(bold('Return:') +'')
        return_units = ''
        if valid_dic_val(method, 'return_units'):
            return_units = small_html(' -'+method['return_units'])
        md.textn(italic(create_hyperlinks(method['return']) + return_units))
        md.list_pop()

    # 如果是获取器/设置器，添加对应的设置器/获取器的链接信息
    for element in other_list:
        el_name = element['def_name']
        if el_name[4:] == method_name[4:]:
            if is_getter:
                md.list_push(bold('Setter:') +'')
            else:
                md.list_push(bold('Getter:') +'')
            md.textn(italic(create_hyperlinks(class_key + '.' + el_name)))
            md.list_pop()

    # 备注文档说明部分
    if valid_dic_val(method, 'note'):
        md.list_push(bold('Note:') +'')
        md.textn(color(COLOR_NOTE, italic(create_hyperlinks(method['note']))))
        md.list_pop()

    # 警告文档说明部分
    if valid_dic_val(method, 'warning'):
        md.list_push(bold('Warning:') +'')
        md.textn(color(COLOR_WARNING, italic(create_hyperlinks(method['warning']))))
        md.list_pop()

    # 抛出异常文档说明部分
    if valid_dic_val(method, 'raises'):
        md.list_pushn(bold('Raises:') +'' + method['raises'])
        md.list_pop()

    md.list_pop()


# 向Markdown文档中添加双下划线方法（dunder method）相关的完整信息（定义、文档、返回值等）
def add_doc_dunder(md, dunder, class_key):
    dunder_name = dunder['def_name']
    dunder_key = join([class_key, dunder_name], '.')
    dunder_def = gen_doc_dunder_def(dunder, False)
    md.list_pushn(join([html_key(dunder_key), dunder_def]))

    # 双下划线方法文档说明部分
    if valid_dic_val(dunder, 'doc'):
        md.textn(create_hyperlinks(md.prettify_doc(dunder['doc'])))

    # 返回值文档说明部分
    if valid_dic_val(dunder, 'return'):
        md.list_push(bold('Return:') +'')
        md.textn(italic(create_hyperlinks(dunder['return'])))
        md.list_pop()

    md.list_pop()


# 向Markdown文档中添加双下划线方法（dunder method）的参数相关信息（类似add_doc_method_param，但针对双下划线方法）
def add_doc_dunder_param(md, param):
    param_name = param['param_name']
    param_type = ''
    if valid_dic_val(param, 'type'):
        param_type = create_hyperlinks(param['type'])
    param_type = '' if not param_type else parentheses(italic(param_type))
    md.list_push(code(param_name))
    if param_type:
        md.text(''+ param_type)
        md.new_line()
    else:
        md.new_line()
    md.list_pop()


# 向Markdown文档中添加实例变量相关的完整信息（定义、类型、文档、备注、警告等）
def add_doc_inst_var(md, inst_var, class_key):
    var_name = inst_var['var_name']
    var_key = join([class_key, var_name], '.')
    var_type = ''
    var_units = ''

    # 实例变量类型部分
    if valid_dic_val(inst_var, 'type'):
        if valid_dic_val(inst_var, 'var_units'):
            var_units = small_html(' -'+inst_var['var_units'])
        var_type =' '+ parentheses(italic(create_hyperlinks(inst_var['type'] + var_units)))
    md.list_pushn(
        html_key(var_key) +
        bold(color(COLOR_INSTANCE_VAR, var_name)) +
        var_type)

    # 实例变量文档说明部分
    if valid_dic_val(inst_var, 'doc'):
        md.textn(create_hyperlinks(md.prettify_doc(inst_var['doc'])))

    # 备注文档说明部分
    if valid_dic_val(inst_var, 'note'):
        md.list_push(bold('Note:') +'')
        md.textn(color(COLOR_NOTE, italic(create_hyperlinks(inst_var['note']))))
        md.list_pop()

    # 警告文档说明部分
    if valid_dic_val(inst_var, 'warning'):
        md.list_push(bold('Warning:') +'')
        md.textn(color(COLOR_WARNING, italic(create_hyperlinks(inst_var['warning']))))
        md.list_pop()

    md.list_pop()


# 表示整个文档生成的主类，用于整合多个YAML文件内容并生成完整的Markdown文档
class Documentation:
    """Main documentation class"""

    def __init__(self, path):
        self._path = path
        # 获取指定路径下所有以.yml结尾的文件列表
        self._files = [f for f in os.listdir(path) if f.endswith('.yml')]
        self._yamls = list()
        for yaml_file in self._files:
            # 实例化YamlFile类，读取并验证每个YAML文件内容，添加到_yamls列表中
            self._yamls.append(YamlFile(os.path.join(path, yaml_file)))
        # 用于合并不同YAML文件中相同模块的信息，最终存储整合后的文档信息字典
        self.master_dict = dict()
        for yaml_file in self._yamls:
            for module in yaml_file.get_modules():
                module_name = module['module_name']
                if module_name not in self.master_dict:
                    self.master_dict[module_name] = module
                elif valid_dic_val(module, 'classes'):
                    for new_module in module['classes']:
                        # 如果'master_dict'中对应模块还没有'classes'键，则创建该键并初始化为空列表
                        if not valid_dic_val(self.master_dict[module_name], 'classes'):
                            self.master_dict[module_name]['classes'] = []
                        self.master_dict[module_name]['classes'].append(new_module)

    # 生成文档的索引部分（包含模块、类、实例变量、方法等的概述信息，以特定格式呈现用于快速导航）
   def gen_overview(self):
        """Generates a referenced index for markdown file"""
        md = MarkdownFile()
        md.title(3, 'Overview')
        for module_name in sorted(self.master_dict):
            module = self.master_dict[module_name]
            module_key = '#' + module_name
            md.list_pushn(
                brackets(bold(module_key[1:])) +
                parentheses(module_key) +'' +
                sub(italic('Module')))
            # 生成类的概述信息（如果有类定义的话）
            if 'classes' in module and module['classes']:
                for cl in sorted(module['classes']):
                    class_name = cl['class_name']
                    class_key = join([module_key, class_name], '.')
                    md.list_pushn(join([
                        brackets(bold(class_name)),
                        parentheses(class_key),'',
                        sub(italic('Class'))]))
                    # 生成类的实例变量概述信息（如果有实例变量定义的话）
                    if 'instance_variables' in cl and cl['instance_variables']:
                        for inst_var in cl['instance_variables']:
                            md.list_push(gen_inst_var_indx(inst_var, class_key))
                            md.list_popn()
                    # 生成类的方法概述信息（如果有方法定义的话）
                    if'methods' in cl and cl['methods']:
                        for method in sorted(cl['methods'], key = lambda i: i['def_name']):
                            md.list_push(gen_method_indx(method, class_key))
                            md.list_popn()
                    md.list_pop()
            md.list_pop()
        return md.data()

    # 生成文档的主体内容部分（包含详细的模块、类、实例变量、各种方法等具体信息展示）
    def gen_body(self):
        """Generates the documentation body"""
        md = MarkdownFile()
        md.first_title()
        md.textn(
        "This reference contains all the details the Python API. To consult a previous reference for a specific CARLA release, change the documentation version using the panel in the bottom right corner.<br>"
        +"This will change the whole documentation to a previous state. Remember that the <i>latest</i> version is the `dev` branch and may show features not available in any packaged versions of CARLA.<hr>")
        for module_name in sorted(self.master_dict):
            module = self.master_dict[module_name]
            module_key = module_name
            # 生成类的文档信息（如果有类定义的话）
            if valid_dic_val(module, 'classes'):
                for cl in sorted(module['classes'], key = lambda i: i['class_name']):
                    class_name = cl['class_name']
                    class_key = join([module_key, class_name], '.')
                    current_title = module_name+'.'+class_name
                    md.title(2, join([current_title,'<a name="',current_title,'"></a>']))
                    # 处理类的继承信息（如果有继承关系的话）
                    if valid_dic_val(cl, 'parent'):
                        inherits = italic(create_hyperlinks(cl['parent']))
                        md.inherit_join(inherits)
                    # 类的主文档说明部分
                    if valid_dic_val(cl, 'doc'):
                        md.textn(create_hyperlinks(md.prettify_doc(cl['doc'])))
                    # 生成实例变量的文档信息（如果有实例变量定义的话）
                    if valid_dic_val(cl, 'instance_variables'):
                        md.title(3, 'Instance Variables')
                        for inst_var in cl['instance_variables']:
                            add_doc_inst_var(md, inst_var, class_key)
                    # 生成方法的文档信息（如果有方法定义的话）
                    if valid_dic_val(cl,'methods'):
                        method_list = list()
                        dunder_list = list()
                        get_list = list()
                        set_list = list()
                        for method in sorted(cl['methods'], key = lambda i: i['def_name']):
                            method_name = method['def_name']
                            if method_name[0] == '_' and method_name!= '__init__':
                                dunder_list.append(method)
                            elif method_name[:4] == 'get_':
                                get_list.append(method)
                            elif method_name[:4] == 'set_':
                                set_list.append(method)
                            else:
                                method_list.append(method)
                        md.title(3, 'Methods')
                        for method in method_list:
                            add_doc_method(md, method, class_key)
                        if len(get_list)>0:
                            md.title(5, 'Getters')
                        for method in get_list:
                            add_doc_getter_setter(md, method, class_key, True, set_list)
                        if len(set_list)>0:
                            md.title(5, 'Setters')
                        for method in set_list:
                            add_doc_getter_setter(md, method, class_key, False, get_list)
                        if len(dunder_list)>0:
                            md.title(5, 'Dunder methods')
                        for method in dunder_list:
                            add_doc_dunder(md, method, class_key)
                    md.separator()
        append_code_snipets(md)
        append_snipet_button_script(md)
        return md.data().strip()

    # 生成完整的Markdown文档内容（整合了文档索引和主体内容部分）
    def gen_markdown(self):
        """Generates the whole markdown file"""
        return join([self.gen_body()], '\n').strip()


def main():
    """Main function"""
    print("Generating PythonAPI documentation...")
    script_path = os.path.dirname(os.path.abspath(__file__))
    doc_gen_snipets.main()
    docs = Documentation(script_path)
    with open(os.path.join(script_path, '../../Docs/python_api.md'), 'w') as md_file:
        md_file.write(docs.gen_markdown())
    print("Done!")


if __name__ == "__main__":
    main()
