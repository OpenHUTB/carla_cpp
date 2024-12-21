/**
 * pugixml parser - version 1.9
 * --------------------------------------------------------
 * Copyright (C) 2006-2018, by Arseny Kapoulkine (arseny.kapoulkine@gmail.com)
 * Report bugs and download new versions at http://pugixml.org/
 *
 * This library is distributed under the MIT License. See notice at the end
 * of this file.
 *
 * This work is based on the pugxml parser, which is:
 * Copyright (C) 2003, by Kristen Wegner (kristen@tima.net)
 */

#ifndef HEADER_PUGICONFIG_HPP
#define HEADER_PUGICONFIG_HPP

// 取消注释以启用 wchar_t 模式
// #define PUGIXML_WCHAR_MODE

// 取消注释以启用压缩模式
// #define PUGIXML_COMPACT

// 取消注释以禁用 XPath
// #define PUGIXML_NO_XPATH

// 取消注释以禁用 STL
// #define PUGIXML_NO_STL

// 取消注释此项以禁用异常
// #define PUGIXML_NO_EXCEPTIONS

// 将此项设置为控制公共类/函数的属性，即：
// #define PUGIXML_API __declspec（dllexport） // 从 DLL 导出所有公共符号
// #define PUGIXML_CLASS __declspec（dllimport） // 从 DLL 导入所有类
// #define PUGIXML_FUNCTION __fastcall // 将所有公共函数的调用约定设置为 fastcall
// 在没有 PUGIXML_CLASS/PUGIXML_FUNCTION 定义的情况下，改用 PUGIXML_API

// 调整这些常量以调整与内存相关的行为
// #define PUGIXML_MEMORY_PAGE_SIZE 32768
// #define PUGIXML_MEMORY_OUTPUT_STACK 10240
// #define PUGIXML_MEMORY_XPATH_PAGE_SIZE 4096

// 取消注释此项以切换到仅标头版本
// #define PUGIXML_HEADER_ONLY

//取消注释以启用 long long support
// #define PUGIXML_HAS_LONG_LONG

#endif

/**
 * Copyright (c) 2006-2018 Arseny Kapoulkine
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
