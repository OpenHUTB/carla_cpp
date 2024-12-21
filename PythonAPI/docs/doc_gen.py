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

# 定义用于表示不同类型元素在文档中显示颜色的常量，方便后续对相应元素进行样式设置
COLOR_METHOD = '#7fb800'
COLOR_PARAM = '#00a6ed'
COLOR_INSTANCE_VAR = '#f8805a'
COLOR_NOTE = '#8E8E8E'
COLOR_WARNING = '#ED2F2F'

# 编译一个正则表达式对象，用于匹配包含 "Carla" 开头且后面跟着点号以及其他字母、数字、下划线组合的字符串模式，
# 目的可能是为了在文本中识别特定的与Carla相关的标识符
QUERY = re.compile(r'([cC]arla(\.[a-zA-Z0-9_]+)+)')


def create_hyperlinks(text):
    return re.sub(QUERY, r'[\1](#\1)', text)

def create_getter_setter_hyperlinks(text):
    return re.sub(QUERY, r'[\1](#\1)', text)

def join(elem, separator=''):
     """
    将给定的可迭代元素（如列表）拼接成一个字符串，元素之间用指定的分隔符（默认为空字符串）分隔。
    这是一个简单的字符串拼接辅助函数。
    """
    return separator.join(elem)


# 定义一个用于生成Markdown格式文件内容的类，通过一系列方法来逐步构建Markdown文档的结构和内容
class MarkdownFile:
    def __init__(self):
         """
        类的构造函数，用于初始化MarkdownFile对象的一些基础属性。
        """
        self._data = "" # 用于存储Markdown文件的内容，初始为空字符串
        self._list_depth = 0 # 用于记录列表的嵌套深度，初始为0
        self.endl = '  \n' # 定义换行格式，这里使用两个空格加换行符，用于在添加文本内容时控制换行显示效果

    def data(self):
        """
        返回当前已经构建的Markdown文件内容字符串，可用于获取最终生成的文档内容。
        """
        return self._data

    def list_push(self, buf=''):
         """
        用于在Markdown文档中添加列表项。
        如果传入了buf参数（即要添加的列表项内容），则会先按照当前列表深度添加相应的缩进，再添加列表项前缀（'- '）以及内容。
        之后会将列表深度加1，表示进入下一层列表嵌套（如果继续添加列表项，会更缩进一层）。
        """
        if buf:
            self.text(join([
                '    ' * self._list_depth if self._list_depth != 0 else '', '- ', buf]))
        self._list_depth = (self._list_depth + 1)

    def list_pushn(self, buf):
         """
        与list_push类似，也是用于添加列表项，但会在添加完列表项内容后自动添加一个换行符（通过调用join函数结合换行格式endl），
        使得添加的列表项在文档中换行显示，更加清晰。
        """
        self.list_push(join([buf, self.endl]))

    def list_pop(self):
        """
        用于减少列表的嵌套深度，即将列表深度减1，当完成一层列表项的添加，需要回到上一层列表时调用此方法，
        会保证列表深度不小于0（通过取最大值操作）。
        """
        self._list_depth = max(self._list_depth - 1, 0)

    def list_popn(self):
        """
        结合了list_pop和添加换行的操作，先减少列表深度，然后在文档内容字符串中添加一个换行符，
        常用于完成一层列表的处理后，进行换行以准备后续内容的添加。
        """
        self.list_pop()
        self._data = join([self._data, '\n'])

    def list_depth(self):
        """
        根据当前文档内容和列表深度情况，返回相应的缩进字符串。
        如果文档内容末尾不是换行符或者列表深度为0，则返回空字符串；否则返回对应列表深度的缩进字符串（由四个空格重复相应次数组成），
        可用于在添加文本内容时根据列表深度进行正确的缩进排版。
        """
        if self._data.strip()[-1:] != '\n' or self._list_depth == 0:
            return ''
        return join(['    ' * self._list_depth])

    def separator(self):
        """
        在Markdown文档内容中添加一个水平分割线（通过添加"---"），常用于分隔不同的章节或内容块，使文档结构更清晰。
        """
        self._data = join([self._data, '\n---\n'])

    def new_line(self):
         """
        在Markdown文档内容中添加一个换行符，按照预先定义的换行格式endl进行添加，用于简单的换行操作。
        """
        self._data = join([self._data, self.endl])

    def text(self, buf):
        self._data = join([self._data, buf])

    def textn(self, buf):
        self._data = join([self._data, self.list_depth(), buf, self.endl])

    def first_title(self):
        self._data = join([
            self._data, '#Python API reference\n'])

    def title(self, strongness, buf):
        self._data = join([
            self._data, '\n', self.list_depth(), '#' * strongness, ' ', buf, '\n'])

    def title_html(self, strongness, buf):
        if strongness == 5:
            self._data = join([
                self._data, '\n', self.list_depth(), '<h', str(strongness), ' style="margin-top: -20px">', buf, '</h', str(strongness),'>\n','<div style="padding-left:30px;margin-top:-25px"></div>'])
        else:
            self._data = join([
                self._data, '\n', self.list_depth(), '<h', str(strongness), '>', buf, '</h', str(strongness), '>\n'])

    def inherit_join(self, inh):
        self._data = join([
            self._data, '<small style="display:block;margin-top:-20px;">Inherited from ', inh, '</small></br>\n'])

    def note(self, buf):
        self._data = join([self._data, buf])

    def code_block(self, buf, language=''):
        return join(['```', language, '\n', self.list_depth(), buf, '\n', self.list_depth(), '```\n'])

    def prettify_doc(self, doc):
        punctuation_marks = ['.', '!', '?']
        doc = doc.strip()
        doc += '' if doc[-1:] in punctuation_marks else '.'
        return doc


def italic(buf):
    return join(['_', buf, '_'])


def bold(buf):
    return join(['**', buf, '**'])

def snipet(name,class_key):

    return join(["<button class=\"SnipetButton\" id=\"",class_key,".",name,"-snipet_button\">", "snippet &rarr;", '</button>'])

def code(buf):
    return join(['`', buf, '`'])


def brackets(buf):
    return join(['[', buf, ']'])


def parentheses(buf):
    return join(['(', buf, ')'])


def small_html(buf):
    return join(['<small>', buf, '</small>'])


def small(buf):
    return join(['<sub><sup>', buf, '</sup></sub>'])


def sub(buf):
    return join(['<sub>', buf, '</sub>'])


def html_key(buf):
    return join(['<a name="', buf, '"></a>'])


def color(col, buf):
    return join(['<font color="', col, '">', buf, '</font>'])


def valid_dic_val(dic, value):
    return value in dic and dic[value]


# YamlFile类，用于处理YAML文件相关操作
class YamlFile:
    """Yaml file class"""

    def __init__(self, path):
        self._path = path
        # 打开YAML文件并安全加载其内容到self.data属性中
        with open(path) as yaml_file:
            self.data = yaml.safe_load(yaml_file)
        # 调用验证方法，对加载的数据进行合法性验证
        self.validate()

    # 验证方法，用于检查加载的YAML数据是否符合特定要求
    def validate(self):
        # print('Validating ' + str(self._path.replace('\\', '/').split('/')[-1:][0]))
        if self.data is None:
            print('\n[ERROR] File: ' + self._path)
            print("This file has no data:")
            exit(0)
        # 遍历数据中的每个模块（假设self.data是包含多个模块信息的结构，比如列表中包含字典形式的模块信息）
        for module in self.data:
            # 如果模块中有'module_name'键且其值为None，输出错误信息并退出程序
            if 'module_name' in module and module['module_name'] is None:
                print('\n[ERROR] File: ' + self._path)
                print("'module_name' is empty in:")
                exit(0)
            # 如果模块中有'classes'键，说明包含类相关信息，进行进一步验证
            if 'classes' in module:
                # 如果'classes'对应的列表为空，输出错误信息并退出程序
                if not module['classes']:
                    print('\n[ERROR] File: ' + self._path)
                    print("'classes' is empty in:")
                    exit(0)
                # 遍历'classes'列表中的每个类信息（假设是字典形式）
                for cl in module['classes']:
                    # 如果类信息中有'class_name'键且其值为None，输出错误信息并退出程序
                    if 'class_name' in cl and cl['class_name'] is None:
                        print('\n[ERROR] File: ' + self._path)
                        print("'class_name' is empty in:")
                        exit(0)
                    # 如果类信息中有'instance_variables'键且其值不为空（即包含实例变量相关信息），进一步验证实例变量
                    if 'instance_variables' in cl and cl['instance_variables']:
                        for iv in cl['instance_variables']:
                            # 如果实例变量字典中没有'var_name'键，输出错误信息并退出程序
                            if 'var_name' not in iv:
                                print('\n[ERROR] File: ' + self._path)
                                print("'var_name' not found inside 'instance_variables' of class: " + cl['class_name'])
                                exit(0)
                            # 如果实例变量字典中有'var_name'键但其值为None，输出错误信息并退出程序
                            if 'var_name' in iv and iv['var_name'] is None:
                                print('\n[ERROR] File: ' + self._path)
                                print("'var_name' is empty in:")
                                exit(0)
                    # 如果类信息中有'methods'键且其值不为空（即包含方法相关信息），进一步验证方法
                    if 'methods' in cl and cl['methods']:
                        for met in cl['methods']:
                            # 如果方法字典中没有'def_name'键，输出错误信息并退出程序
                            if 'def_name' not in met:
                                print('\n[ERROR] File: ' + self._path)
                                print("'def_name' not found inside 'methods' of class: " + cl['class_name'])
                                exit(0)
                            # 如果方法字典中有'def_name'键但其值为None，输出错误信息并退出程序
                            if 'def_name' in met and met['def_name'] is None:
                                print('\n[ERROR] File: ' + self._path)
                                print("'def_name' is empty in:")
                                exit(0)
                            # 如果方法字典中有'params'键且其值不为空（即包含参数相关信息），进一步验证参数
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

    def get_modules(self):
        return [module for module in self.data]

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

def append_code_snipets(md):
    current_folder = os.path.dirname(os.path.abspath(__file__))
    snipets_path = os.path.join(current_folder, '../../Docs/python_api_snipets.md')
    snipets = open(snipets_path, 'r')
    md.text(snipets.read())
    snipets.close()
    os.remove(snipets_path)


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


def gen_doc_method_def(method, class_key, is_indx=False, with_self=True):
    """Return python def as it should be written in docs"""
    param = ''
    snipet_link = ''
    method_name = method['def_name']
    full_method_name = method_name
    if valid_dic_val(method, 'static'):
        with_self = False

    # to correctly render methods like __init__ in md
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

    # Add snipet
    current_folder = os.path.dirname(os.path.abspath(__file__))
    snipets_path = os.path.join(current_folder, '../../Docs/python_api_snipets.md')
    snipets = open(snipets_path, 'r')
    if class_key+'.'+full_method_name+'-snipet' in snipets.read():
        snipet_link = snipet(full_method_name, class_key)

    return join([method_name, parentheses(param),snipet_link])

def gen_doc_dunder_def(dunder, is_indx=False, with_self=True):
    """Return python def as it should be written in docs"""
    param = ''
    dunder_name = dunder['def_name']
    if valid_dic_val(dunder, 'static'):
        with_self = False

    # to correctly render methods like __init__ in md
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


def gen_inst_var_indx(inst_var, class_key):
    inst_var_name = inst_var['var_name']
    inst_var_key = join([class_key, inst_var_name], '.')
    return join([
        brackets(bold(inst_var_name)),
        parentheses(inst_var_key), ' ',
        sub(italic('Instance variable'))])


def gen_method_indx(method, class_key):
    method_name = method['def_name']
    method_key = join([class_key, method_name], '.')
    method_def = gen_doc_method_def(method, class_key, True)
    return join([
        brackets(method_def),
        parentheses(method_key), ' ',
        sub(italic('Method'))])


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
        param_units = small_html(' - '+param['param_units'])
    param_type = '' if not param_type else parentheses(italic(param_type+param_units))
    md.list_push(code(param_name))
    if param_type:
        md.text(' ' + param_type)
    if param_doc:
        md.textn(' - ' + param_doc)
    else:
        md.new_line()
    md.list_pop()


def add_doc_method(md, method, class_key):
    method_name = method['def_name']
    method_key = join([class_key, method_name], '.')
    method_def = gen_doc_method_def(method, class_key, False)
    md.list_pushn(join([html_key(method_key), method_def]))

    # Method doc
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
            # Print the 'Parameters' title once
            if not printed_title:
                printed_title = True
                md.list_push(bold('Parameters:') + '\n')
            add_doc_method_param(md, param)
    if printed_title:
        md.list_pop()

    # Return doc
    if valid_dic_val(method, 'return'):
        md.list_push(bold('Return:') + ' ')
        return_units = ''
        if valid_dic_val(method, 'return_units'):
            return_units = small_html(' - '+method['return_units'])
        md.textn(italic(create_hyperlinks(method['return'])+return_units))
        md.list_pop()

    # Note doc
    if valid_dic_val(method, 'note'):
        md.list_push(bold('Note:') + ' ')
        md.textn(color(COLOR_NOTE, italic(create_hyperlinks(method['note']))))
        md.list_pop()

    # Warning doc
    if valid_dic_val(method, 'warning'):
        md.list_push(bold('Warning:') + ' ')
        md.textn(color(COLOR_WARNING, italic(create_hyperlinks(method['warning']))))
        md.list_pop()

    # Raises error doc
    if valid_dic_val(method, 'raises'):
        md.list_pushn(bold('Raises:') + ' ' + method['raises'])
        md.list_pop()

    md.list_pop()

def add_doc_getter_setter(md, method, class_key, is_getter, other_list):
    method_name = method['def_name']
    method_key = join([class_key, method_name], '.')
    method_def = gen_doc_method_def(method, class_key, False)
    md.list_pushn(join([html_key(method_key), method_def]))

    # Method doc
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
            # Print the 'Parameters' title once
            if not printed_title:
                printed_title = True
                md.list_push(bold('Parameters:') + '\n')
            add_doc_method_param(md, param)
    if printed_title:
        md.list_pop()

    # Return doc
    if valid_dic_val(method, 'return'):
        md.list_push(bold('Return:') + ' ')
        return_units = ''
        if valid_dic_val(method, 'return_units'):
            return_units = small_html(' - '+method['return_units'])
        md.textn(italic(create_hyperlinks(method['return'])+return_units))
        md.list_pop()

    # If setter/getter
    for element in other_list:
        el_name = element['def_name']
        if el_name[4:] == method_name[4:]:
            if is_getter:
                md.list_push(bold('Setter:') + ' ')
            else:
                md.list_push(bold('Getter:') + ' ')
            md.textn(italic(create_hyperlinks(class_key+'.'+el_name)))
            md.list_pop()

    # Note doc
    if valid_dic_val(method, 'note'):
        md.list_push(bold('Note:') + ' ')
        md.textn(color(COLOR_NOTE, italic(create_hyperlinks(method['note']))))
        md.list_pop()

    # Warning doc
    if valid_dic_val(method, 'warning'):
        md.list_push(bold('Warning:') + ' ')
        md.textn(color(COLOR_WARNING, italic(create_hyperlinks(method['warning']))))
        md.list_pop()

    # Raises error doc
    if valid_dic_val(method, 'raises'):
        md.list_pushn(bold('Raises:') + ' ' + method['raises'])
        md.list_pop()

    md.list_pop()

def add_doc_dunder(md, dunder, class_key):
    dunder_name = dunder['def_name']
    dunder_key = join([class_key, dunder_name], '.')
    dunder_def = gen_doc_dunder_def(dunder, False)
    md.list_pushn(join([html_key(dunder_key), dunder_def]))

    # Dunder doc
    if valid_dic_val(dunder, 'doc'):
        md.textn(create_hyperlinks(md.prettify_doc(dunder['doc'])))

    # Return doc
    if valid_dic_val(dunder, 'return'):
        md.list_push(bold('Return:') + ' ')
        md.textn(italic(create_hyperlinks(dunder['return'])))
        md.list_pop()

    md.list_pop()

def add_doc_dunder_param(md, param):
    param_name = param['param_name']
    param_type = ''
    if valid_dic_val(param, 'type'):
        param_type = create_hyperlinks(param['type'])
    param_type = '' if not param_type else parentheses(italic(param_type))
    md.list_push(code(param_name))
    if param_type:
        md.text(' ' + param_type)
        md.new_line()
    else:
        md.new_line()
    md.list_pop()


def add_doc_inst_var(md, inst_var, class_key):
    var_name = inst_var['var_name']
    var_key = join([class_key, var_name], '.')
    var_type = ''
    var_units = ''

    # Instance variable type
    if valid_dic_val(inst_var, 'type'):
        if valid_dic_val(inst_var, 'var_units'):
            var_units = small_html(' - '+inst_var['var_units'])
        var_type = ' ' + parentheses(italic(create_hyperlinks(inst_var['type']+var_units)))
    md.list_pushn(
        html_key(var_key) +
        bold(color(COLOR_INSTANCE_VAR, var_name)) +
        var_type)

    # Instance variable doc
    if valid_dic_val(inst_var, 'doc'):
        md.textn(create_hyperlinks(md.prettify_doc(inst_var['doc'])))

    # Note doc
    if valid_dic_val(inst_var, 'note'):
        md.list_push(bold('Note:') + ' ')
        md.textn(color(COLOR_NOTE, italic(create_hyperlinks(inst_var['note']))))
        md.list_pop()

    # Warning doc
    if valid_dic_val(inst_var, 'warning'):
        md.list_push(bold('Warning:') + ' ')
        md.textn(color(COLOR_WARNING, italic(create_hyperlinks(inst_var['warning']))))
        md.list_pop()

    md.list_pop()

class Documentation:
    """Main documentation class"""

    def __init__(self, path):
        self._path = path
        self._files = [f for f in os.listdir(path) if f.endswith('.yml')]
        self._yamls = list()
        for yaml_file in self._files:
            self._yamls.append(YamlFile(os.path.join(path, yaml_file)))
        # Merge same modules of different files
        self.master_dict = dict()
        for yaml_file in self._yamls:
            for module in yaml_file.get_modules():
                module_name = module['module_name']
                if module_name not in self.master_dict:
                    self.master_dict[module_name] = module
                elif valid_dic_val(module, 'classes'):
                    for new_module in module['classes']:
                        # Create the 'classes' key if does not exist already
                        if not valid_dic_val(self.master_dict[module_name], 'classes'):
                            self.master_dict[module_name]['classes'] = []
                        self.master_dict[module_name]['classes'].append(new_module)

    def gen_overview(self):
        """Generates a referenced index for markdown file"""
        md = MarkdownFile()
        md.title(3, 'Overview')
        for module_name in sorted(self.master_dict):
            module = self.master_dict[module_name]
            module_key = '#' + module_name
            md.list_pushn(
                brackets(bold(module_key[1:])) +
                parentheses(module_key) + ' ' +
                sub(italic('Module')))
            # Generate class overview (if any)
            if 'classes' in module and module['classes']:
                for cl in sorted(module['classes']):
                    class_name = cl['class_name']
                    class_key = join([module_key, class_name], '.')
                    md.list_pushn(join([
                        brackets(bold(class_name)),
                        parentheses(class_key), ' ',
                        sub(italic('Class'))]))
                    # Generate class instance variables overview (if any)
                    if 'instance_variables' in cl and cl['instance_variables']:
                        for inst_var in cl['instance_variables']:
                            md.list_push(gen_inst_var_indx(inst_var, class_key))
                            md.list_popn()
                    # Generate class methods overview (if any)
                    if 'methods' in cl and cl['methods']:
                        for method in sorted(cl['methods'], key = lambda i: i['def_name']):
                            md.list_push(gen_method_indx(method, class_key))
                            md.list_popn()
                    md.list_pop()
            md.list_pop()
        return md.data()

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
            # Generate class doc (if any)
            if valid_dic_val(module, 'classes'):
                for cl in sorted(module['classes'], key = lambda i: i['class_name']):
                    class_name = cl['class_name']
                    class_key = join([module_key, class_name], '.')
                    current_title = module_name+'.'+class_name
                    md.title(2, join([current_title,'<a name="',current_title,'"></a>']))
                    # Inheritance
                    if valid_dic_val(cl, 'parent'):
                        inherits = italic(create_hyperlinks(cl['parent']))
                        md.inherit_join(inherits)
                    # Class main doc
                    if valid_dic_val(cl, 'doc'):
                        md.textn(create_hyperlinks(md.prettify_doc(cl['doc'])))
                    # Generate instance variable doc (if any)
                    if valid_dic_val(cl, 'instance_variables'):
                        md.title(3, 'Instance Variables')
                        for inst_var in cl['instance_variables']:
                            add_doc_inst_var(md, inst_var, class_key)
                    # Generate method doc (if any)
                    if valid_dic_val(cl, 'methods'):
                        method_list = list()
                        dunder_list = list()
                        get_list = list()
                        set_list = list()
                        for method in sorted(cl['methods'], key = lambda i: i['def_name']):
                            method_name = method['def_name']
                            if method_name[0] == '_' and method_name != '__init__':
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
