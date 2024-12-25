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

#ifndef PUGIXML_VERSION
// 定义版本宏；计算结果为主要版本 * 100 + 次要版本，以便在小于比较中安全使用。
#	define PUGIXML_VERSION 190
#endif

// 包含用户配置文件（这可以定义各种配置宏）
#include "pugiconfig.hpp"

#ifndef HEADER_PUGIXML_HPP
#define HEADER_PUGIXML_HPP

// 包含 stddef.h 以便使用 size_t 和 ptrdiff_t
#include <stddef.h>

// 包括XPath的异常头部
#if !defined(PUGIXML_NO_XPATH) && !defined(PUGIXML_NO_EXCEPTIONS)
#	include <exception>
#endif

// 包含STL头文件
#ifndef PUGIXML_NO_STL
#	include <iterator>
#	include <iosfwd>
#	include <string>
#endif

// 已弃用功能的宏
#ifndef PUGIXML_DEPRECATED
#	if defined(__GNUC__)
#		define PUGIXML_DEPRECATED __attribute__((deprecated))
#	elif defined(_MSC_VER) && _MSC_VER >= 1300
#		define PUGIXML_DEPRECATED __declspec(deprecated)
#	else
#		define PUGIXML_DEPRECATED
#	endif
#endif

// 如果没有定义API，则假定为默认值
#ifndef PUGIXML_API
#	define PUGIXML_API
#endif

// 如果没有定义类的 API，则假定为默认值
#ifndef PUGIXML_CLASS
#	define PUGIXML_CLASS PUGIXML_API
#endif

// 如果没有定义函数的API，则假设为默认值。
#ifndef PUGIXML_FUNCTION
#	define PUGIXML_FUNCTION PUGIXML_API
#endif

//如果该平台已知具有长整型支持，请启用长整型功能。
#ifndef PUGIXML_HAS_LONG_LONG
#	if __cplusplus >= 201103
#		define PUGIXML_HAS_LONG_LONG
#	elif defined(_MSC_VER) && _MSC_VER >= 1400
#		define PUGIXML_HAS_LONG_LONG
#	endif
#endif

// 如果已知该平台支持移动语义，则编译移动构造函数/操作符的实现。
#ifndef PUGIXML_HAS_MOVE
#	if __cplusplus >= 201103
#		define PUGIXML_HAS_MOVE
#	elif defined(_MSC_VER) && _MSC_VER >= 1600
#		define PUGIXML_HAS_MOVE
#	endif
#endif

// HACK（安德烈）：禁用异常，因为它们在虚幻引擎中不可用
#define PUGIXML_NOEXCEPT

//如果C++是2011或更高版本，添加'noexcept'说明符
#ifndef PUGIXML_NOEXCEPT
#	if __cplusplus >= 201103
#		define PUGIXML_NOEXCEPT noexcept
#	elif defined(_MSC_VER) && _MSC_VER >= 1900
#		define PUGIXML_NOEXCEPT noexcept
#	else
#		define PUGIXML_NOEXCEPT
#	endif
#endif

// 某些函数在紧凑模式下不能是 noexcept。
#ifdef PUGIXML_COMPACT
#	define PUGIXML_NOEXCEPT_IF_NOT_COMPACT
#else
#	define PUGIXML_NOEXCEPT_IF_NOT_COMPACT PUGIXML_NOEXCEPT
#endif

// 如果C++是2011或更高版本，请添加'override'限定符。
#ifndef PUGIXML_OVERRIDE
#	if __cplusplus >= 201103
#		define PUGIXML_OVERRIDE override
#	elif defined(_MSC_VER) && _MSC_VER >= 1700
#		define PUGIXML_OVERRIDE override
#	else
#		define PUGIXML_OVERRIDE
#	endif
#endif

// 字符接口宏
#ifdef PUGIXML_WCHAR_MODE
#	define PUGIXML_TEXT(t) L ## t
#	define PUGIXML_CHAR wchar_t
#else
#	define PUGIXML_TEXT(t) t
#	define PUGIXML_CHAR char
#endif

namespace pugi
{
	//用于所有内部存储和操作的字符类型;取决于 PUGIXML_WCHAR_MODE
	typedef PUGIXML_CHAR char_t;

#ifndef PUGIXML_NO_STL
	// String 类型，用于使用 STL 字符串的操作;取决于 PUGIXML_WCHAR_MODE
	typedef std::basic_string<PUGIXML_CHAR, std::char_traits<PUGIXML_CHAR>, std::allocator<PUGIXML_CHAR> > string_t;
#endif
}

// PugiXML命名空间
namespace pugi
{
	// 树节点类型
	enum xml_node_type
	{
		node_null,			// 空（null）节点句柄
		node_document,		//文档树的绝对根
		node_element,		//元素标签，即 '<node/>'
		node_pcdata,		// 普通字符数据，即'text'
		node_cdata,			//字符数据，即 '<![CDATA[text]]>'
		node_comment,		// 评论标签，即 '<!-- text -->'
		node_pi,			// 处理指令，即 '<?name?>'
		node_declaration,	// 文档声明，即 '<?xml version="1.0"?>'
		node_doctype		// 文档类型声明，即 '<!DOCTYPE doc>'
	};

	// 解析选项

	// 最小解析模式（相当于关闭所有其他标志）。
	//仅元素和PCDATA部分被添加到DOM树中，未执行文本转换。
	const unsigned int parse_minimal = 0x0000;

	// 此标志用于确定是否将处理指令（节点_pi）添加到 DOM 树中。该标志默认处于关闭状态。
	const unsigned int parse_pi = 0x0001;

	// 此标志用于确定是否将注释（节点_注释）添加到 DOM 树中。该标志默认处于关闭状态。
	const unsigned int parse_comments = 0x0002;

	// 此标志用于确定是否将 CDATA（字符数据）段（节点_cdata）添加到 DOM 树中。该标志默认处于开启状态
	const unsigned int parse_cdata = 0x0004;

	// 此标志用于确定仅由空白字符组成的纯文本字符数据（节点_pcdata）是否被添加到 DOM 树中。
	// 该标志默认处于关闭状态；将其开启通常会导致解析速度变慢以及内存消耗增加。
	const unsigned int parse_ws_pcdata = 0x0008;

	const unsigned int parse_escapes = 0x0010;

	// 此标志用于确定在行结束（EOL）字符在解析过程中是否被规范化（转换为十六进制的 #xA）。该标志默认处于开启状态。
	const unsigned int parse_eol = 0x0020;

	// 此标志用于确定在解析期间是否依据 CDATA（字符数据）规范化规则对属性值进行规范化处理。该标志默认处于开启状态。
	const unsigned int parse_wconv_attribute = 0x0040;

	// 此标志用于确定在解析期间是否依据名称标记（NMTOKENS）规范化规则对属性值进行规范化处理。该标志默认处于关闭状态。
        const unsigned int parse_wnorm_attribute = 0x0080;

	// 此标志用于确定文档声明（节点_声明）是否被添加到 DOM 树中。该标志默认处于关闭状态。
	const unsigned int parse_declaration = 0x0100;

	// 此标志用于确定文档类型声明（节点_文档类型）是否被添加到 DOM 树中。该标志默认处于关闭状态。
	const unsigned int parse_doctype = 0x0200;

	//此标志用于确定纯文本字符数据（节点_pcdata）是否被添加到 DOM 树中，这类纯文本字符数据需满足是其父节点的唯一子节点，并且仅（由…… 组成，句子未完整，原句后面应该还有相关描述内容）。
	// 仅由空白字符组成且为其父节点唯一子节点的纯文本字符数据（节点_pcdata）是否被添加到 DOM 树中。
	// 该标志默认处于关闭状态；将其开启可能会导致解析速度变慢以及内存消耗增加。
	const unsigned int parse_ws_pcdata_single = 0x0400;

	//此标志用于确定是否要从纯文本字符数据中去除开头和结尾处的空白字符。该标志默认处于关闭状态。
	const unsigned int parse_trim_pcdata = 0x0800;

	//此标志用于确定没有父节点的纯文本字符数据是否被添加到 DOM 树中，以及空文档（句子似乎未表述完整，后面应该还有相关内容，比如空文档会怎样等进一步的说明）。
	// 是否为空文档也算作有效文档。该标志默认处于关闭状态。
	const unsigned int parse_fragment = 0x1000;

	// 此标志用于确定纯文本字符数据是否存储在父元素的值中。这会极大地改变（句子似乎未表述完整，后面应该还有如 “改变什么的结构” 等相关内容）。
	// 文档的结构；仅建议在内存受限的环境中解析包含大量纯文本字符数据（PCDATA）节点的文档时使用该标志。
	//该标志默认是关闭的。
	const unsigned int parse_embed_pcdata = 0x2000;

	// 默认解析模式。
	// 元素、可解析字符数据（PCDATA）以及字符数据（CDATA）部分会被添加到文档对象模型（DOM）树中，字符 / 引用实体将被展开，
	//行结束符会被规范化，属性值会依据字符数据规范化规则进行规范化处理。
	const unsigned int parse_default = parse_cdata | parse_escapes | parse_wconv_attribute | parse_eol;

	//完整解析模式。
	//所有类型的节点都会被添加到文档对象模型（DOM）树中，字符 / 引用实体将被展开，
	//  行结束符会被规范化，属性值会使用字符数据规范化规则进行规范化处理。
	const unsigned int parse_full = parse_default | parse_pi | parse_comments | parse_declaration | parse_doctype;

	// 这些标志确定 XML 文档的输入数据的编码
	enum xml_encoding
	{
		encoding_auto,		//使用 BOM 或 < / < 自动检测输入编码？检波;如果未找到 BOM，请使用 UTF8
		encoding_utf8,		// UTF8 编码
		encoding_utf16_le,	// 小端序 UTF16
		encoding_utf16_be,	// 大端序 UTF16
		encoding_utf16,		// 采用本机字节序的 UTF16
		encoding_utf32_le,	// 小端序 UTF32
		encoding_utf32_be,	//大端序 UTF32
		encoding_utf32,		// 采用本机字节序的 UTF32
		encoding_wchar,		//  与 wchar_t 相同的编码（UTF16 或 UTF32）
		encoding_latin1
	};

	// 格式化标志

	// 根据节点在 DOM 树中的深度，使用相应数量的缩进字符串对写入输出流的节点进行缩进。此标志默认处于开启状态。
	const unsigned int format_indent = 0x01;

	//将特定编码的字节顺序标记（BOM）写入输出流。此标志默认处于关闭状态。
	const unsigned int format_write_bom = 0x02;

	//使用原始输出模式（不进行缩进，也不写入换行符）。此标志默认处于关闭状态。
	const unsigned int format_raw = 0x04;

	//  即便文档中没有 XML 声明，也省略默认的 XML 声明。此标志默认处于关闭状态。
	const unsigned int format_no_declaration = 0x08;

	// 不对属性值和 PCDATA 内容进行转义。此标志默认处于关闭状态。
	const unsigned int format_no_escapes = 0x10;

	//在 xml_document::save_file 中使用文本模式打开文件。这会在某些系统上启用特殊字符（例如换行符）的转换。此标志默认处于关闭状态。
	const unsigned int format_save_file_text = 0x20;

	// 将每个属性写在新的一行，并带有适当的缩进。此标志默认处于关闭状态。
	const unsigned int format_indent_attributes = 0x40;

	// 不输出空元素标签，而是即便没有子元素，也显式地写出开始标签和结束标签。此标志默认处于关闭状态。
	const unsigned int format_no_empty_element_tags = 0x80;

	//  默认的格式化标志集。
	//根据节点在 DOM 树中的深度对其进行缩进，如果文档没有默认声明则输出该声明。
	const unsigned int format_default = format_indent;

	//前置声明
	struct xml_attribute_struct;
	struct xml_node_struct;

	class xml_node_iterator;
	class xml_attribute_iterator;
	class xml_named_node_iterator;

	class xml_tree_walker;

	struct xml_parse_result;

	class xml_node;

	class xml_text;

	#ifndef PUGIXML_NO_XPATH
	class xpath_node;
	class xpath_node_set;
	class xpath_query;
	class xpath_variable_set;
	#endif

	//基于范围的 for 循环支持
	template <typename It> class xml_object_range
	{
	public:
// 定义常量迭代器的类型别名
		typedef It const_iterator;
// 定义迭代器的类型别名
		typedef It iterator;
// 构造函数，通过传入起始迭代器和结束迭代器来初始化对象
		xml_object_range(It b, It e): _begin(b), _end(e)
		{
		}
// 返回起始迭代器（该函数为常量成员函数，返回值为常量迭代器类型）
		It begin() const { return _begin; }
// 返回结束迭代器（同样是常量成员函数，返回值为常量迭代器类型）
		It end() const { return _end; }

	private:
// 用于存储起始迭代器和结束迭代器的私有成员变量
		It _begin, _end;
	};

	//  用于节点打印的写入器接口（可参考 xml_node::print 相关内容）
	class PUGIXML_CLASS xml_writer
	{
	public:
		virtual ~xml_writer() {}

		//将内存块写入流 / 文件或其他对象中
		virtual void write(const void* data, size_t size) = 0;
	};

	//针对 FILE的 xml_writer 实现类
	class PUGIXML_CLASS xml_writer_file: public xml_writer
	{
	public:
		// 通过一个 FILE对象构造写入器；使用 void * 是为了避免头文件对 stdio 的依赖
		xml_writer_file(void* file);

		virtual void write(const void* data, size_t size) PUGIXML_OVERRIDE;

	private:
		void* file;
	};

	#ifndef PUGIXML_NO_STL
	// 用于流的 xml_writer（XML 写入器）实现。
	class PUGIXML_CLASS xml_writer_stream: public xml_writer
	{
	public:
		//从一个输出流对象构造写入器。例如在相关代码中
		xml_writer_stream(std::basic_ostream<char, std::char_traits<char> >& stream);
		xml_writer_stream(std::basic_ostream<wchar_t, std::char_traits<wchar_t> >& stream);

		virtual void write(const void* data, size_t size) PUGIXML_OVERRIDE;

	private:
		std::basic_ostream<char, std::char_traits<char> >* narrow_stream;
		std::basic_ostream<wchar_t, std::char_traits<wchar_t> >* wide_stream;
	};
	#endif

	// 一个用于操作文档对象模型（DOM）树中属性的轻量级句柄。
	class PUGIXML_CLASS xml_attribute
	{
		friend class xml_attribute_iterator;
		friend class xml_node;

	private:
		xml_attribute_struct* _attr;

		typedef void (*unspecified_bool_type)(xml_attribute***);

	public:
		//默认构造函数。构造一个空的属性。
		xml_attribute();

		// 从内部指针构造属性
		explicit xml_attribute(xml_attribute_struct* attr);

		// 安全布尔转换操作符
		operator unspecified_bool_type() const;

		//Borland C++ 解决办法
		bool operator!() const;

		// 比较操作符（比较所包装的属性指针）
		bool operator==(const xml_attribute& r) const;
		bool operator!=(const xml_attribute& r) const;
		bool operator<(const xml_attribute& r) const;
		bool operator>(const xml_attribute& r) const;
		bool operator<=(const xml_attribute& r) const;
		bool operator>=(const xml_attribute& r) const;

		// 检查属性是否为空
		bool empty() const;

		//  获取属性名称 / 值，如果属性为空则返回 ""（空字符串）
		const char_t* name() const;
		const char_t* value() const;

		// 获取属性值，如果属性为空则返回默认值
		const char_t* as_string(const char_t* def = PUGIXML_TEXT("")) const;

		//将属性值作为数字获取，如果转换不成功或者属性为空则返回默认值
		int as_int(int def = 0) const;// 该函数用于将对象所代表的数据转换为unsigned int类型并返回
// 参数def是默认值，默认值为0，调用函数时若没有传入参数则使用该默认值，函数声明为常成员函数，不会改变对象状态
		unsigned int as_uint(unsigned int def = 0) const;// 该函数用于将对象所代表的数据转换为double类型并返回
// 参数def是默认值，默认值为0，调用函数时若没有传入参数则使用该默认值，同样是常成员函数，不会对对象本身做修改
		double as_double(double def = 0) const;// 该函数用于将对象所代表的数据转换为float类型并返回
// 参数def是默认值，默认值为0，也是常成员函数，调用时若不传参就使用默认值，保证不改变对象的状态
		float as_float(float def = 0) const;

	#ifdef PUGIXML_HAS_LONG_LONG
		long long as_llong(long long def = 0) const;
		unsigned long long as_ullong(unsigned long long def = 0) const;
	#endif

		//将属性值作为布尔值获取（如果第一个字符在 '1tTyY' 集合中则返回 true，如果属性为空则返回默认值）
		bool as_bool(bool def = false) const;

		// 设置属性名称 / 值（如果属性为空或者内存不足则返回 false）
		bool set_name(const char_t* rhs);
		bool set_value(const char_t* rhs);

		// 通过类型转换来设置属性值（数字会被转换为字符串，布尔值会被转换为 "true" 或 "false"）
		bool set_value(int rhs);
		bool set_value(unsigned int rhs);
		bool set_value(long rhs);
		bool set_value(unsigned long rhs);
		bool set_value(double rhs);
		bool set_value(float rhs);
		bool set_value(bool rhs);

	#ifdef PUGIXML_HAS_LONG_LONG
		bool set_value(long long rhs);
		bool set_value(unsigned long long rhs);
	#endif

		//设置属性值（等同于不进行错误检查的 set_value 操作）
		xml_attribute& operator=(const char_t* rhs);
		xml_attribute& operator=(int rhs);
		xml_attribute& operator=(unsigned int rhs);
		xml_attribute& operator=(long rhs);
		xml_attribute& operator=(unsigned long rhs);
		xml_attribute& operator=(double rhs);
		xml_attribute& operator=(float rhs);
		xml_attribute& operator=(bool rhs);

	#ifdef PUGIXML_HAS_LONG_LONG
		xml_attribute& operator=(long long rhs);
		xml_attribute& operator=(unsigned long long rhs);
	#endif

		// 获取父节点的属性列表中的下一个 / 上一个属性
		xml_attribute next_attribute() const;
		xml_attribute previous_attribute() const;

		//获取哈希值（对于指向同一个对象的句柄来说是唯一的）
		size_t hash_value() const;

		//  获取内部指针
		xml_attribute_struct* internal_object() const;
	};

#ifdef __BORLANDC__
	//Borland C++ 解决办法
	bool PUGIXML_FUNCTION operator&&(const xml_attribute& lhs, bool rhs);
	bool PUGIXML_FUNCTION operator||(const xml_attribute& lhs, bool rhs);
#endif

	//一个用于操作 DOM（文档对象模型）树中节点的轻量级句柄
	class PUGIXML_CLASS xml_node// 定义名为xml_node的类，可能处于PUGIXML_CLASS相关的作用域下（具体取决于完整代码，可能是命名空间或类嵌套等情况）
	{
		friend class xml_attribute_iterator;// 声明xml_attribute_iterator类为xml_node类的友元类，意味着它可以访问xml_node类的私有和保护成员
		friend class xml_node_iterator;// 声明xml_node_iterator类为xml_node类的友元类
		friend class xml_named_node_iterator;// 声明xml_named_node_iterator类为xml_node类的友元类

	protected:
		xml_node_struct* _root;// 定义一个指向xml_node_struct类型的指针，xml_node_struct大概率是在别处定义的、与XML节点表示相关的结构体

		typedef void (*unspecified_bool_type)(xml_node***);// 定义一个函数指针类型unspecified_bool_type，它指向的函数接收一个指向xml_node类型的三级指针作为参数，且函数无返回值
    // 具体用途可能和xml_node类相关的布尔判断、回调等机制有关，需结合更多代码上下文才能明确其确切作用

	public:
		// 默认构造函数。构造一个空节点。
		xml_node();

		// 从内部指针构造节点
		explicit xml_node(xml_node_struct* p);

		// 安全布尔转换操作符
		operator unspecified_bool_type() const;

		// Borland C++ 解决办法
		bool operator!() const;

		//  比较操作符（比较所包装的节点指针）
		bool operator==(const xml_node& r) const;
		bool operator!=(const xml_node& r) const;
		bool operator<(const xml_node& r) const;
		bool operator>(const xml_node& r) const;
		bool operator<=(const xml_node& r) const;
		bool operator>=(const xml_node& r) const;

		//检查节点是否为空。
		bool empty() const;

		// 获取节点类型
		xml_node_type type() const;

		// 获取节点名称，如果节点为空或者没有名称则返回 ""（空字符串）
		const char_t* name() const;

		// 获取节点名称，如果节点为空或者没有名称则返回 ""（空字符串）
		// 注意：对于 <node>文本</node> 这样的形式，node.value () 并不会返回 "文本"！请使用 child_value () 或 text () 方法来访问节点内部的文本。
		const char_t* value() const;

		// 获取属性列表
		xml_attribute first_attribute() const;
		xml_attribute last_attribute() const;

		// 获取子节点列表
		xml_node first_child() const;
		xml_node last_child() const;

		//获取父节点的子节点列表中的下一个 / 上一个兄弟节点
		xml_node next_sibling() const;
		xml_node previous_sibling() const;

		//获取父节点
		xml_node parent() const;

		// 获取此节点所属的 DOM（文档对象模型）树的根节点
		xml_node root() const;

		// 获取当前节点的文本对象
		xml_text text() const;

		//  获取具有指定名称的子节点、属性或下一个 / 上一个兄弟节点
		xml_node child(const char_t* name) const;
		xml_attribute attribute(const char_t* name) const;
		xml_node next_sibling(const char_t* name) const;
		xml_node previous_sibling(const char_t* name) const;

		//从提示位置开始获取属性（并更新提示，以便快速搜索一系列属性）
		xml_attribute attribute(const char_t* name, xml_attribute& hint) const;

		// 获取当前节点的子节点值；也就是类型为 PCDATA（已解析字符数据）/CDATA（字符数据）的第一个子节点的值
		const char_t* child_value() const;

		// 获取具有指定名称的子节点的子节点值。等同于 child (name).child_value ()。
		const char_t* child_value(const char_t* name) const;

		//设置节点名称 / 值（如果节点为空、内存不足或者节点不能有名称 / 值，则返回 false）
		bool set_name(const char_t* rhs);
		bool set_value(const char_t* rhs);

		//添加具有指定名称的属性。返回添加后的属性，若出错则返回空属性。
		xml_attribute append_attribute(const char_t* name);
		xml_attribute prepend_attribute(const char_t* name);
		xml_attribute insert_attribute_after(const char_t* name, const xml_attribute& attr);
		xml_attribute insert_attribute_before(const char_t* name, const xml_attribute& attr);

		// 添加指定属性的副本。返回添加后的属性，若出错则返回空属性。
		xml_attribute append_copy(const xml_attribute& proto);
		xml_attribute prepend_copy(const xml_attribute& proto);
		xml_attribute insert_copy_after(const xml_attribute& proto, const xml_attribute& attr);
		xml_attribute insert_copy_before(const xml_attribute& proto, const xml_attribute& attr);

		// 添加具有指定类型的子节点。返回添加后的节点，若出错则返回空节点。
		xml_node append_child(xml_node_type type = node_element);
		xml_node prepend_child(xml_node_type type = node_element);
		xml_node insert_child_after(xml_node_type type, const xml_node& node);
		xml_node insert_child_before(xml_node_type type, const xml_node& node);

		// 添加具有指定名称的子元素。返回添加后的节点，若出错则返回空节点。
		xml_node append_child(const char_t* name);
		xml_node prepend_child(const char_t* name);
		xml_node insert_child_after(const char_t* name, const xml_node& node);
		xml_node insert_child_before(const char_t* name, const xml_node& node);

		// 将指定节点的副本作为子节点添加。返回添加后的节点，若出错则返回空节点。
		xml_node append_copy(const xml_node& proto);
		xml_node prepend_copy(const xml_node& proto);
		xml_node insert_copy_after(const xml_node& proto, const xml_node& node);
		xml_node insert_copy_before(const xml_node& proto, const xml_node& node);

		// 将指定节点移动使其成为此节点的子节点。返回移动后的节点，若出错则返回空节点。
		xml_node append_move(const xml_node& moved);
		xml_node prepend_move(const xml_node& moved);
		xml_node insert_move_after(const xml_node& moved, const xml_node& node);
		xml_node insert_move_before(const xml_node& moved, const xml_node& node);

		//删除指定属性
		bool remove_attribute(const xml_attribute& a);
		bool remove_attribute(const char_t* name);

		//删除指定子节点
		bool remove_child(const xml_node& n);
		bool remove_child(const char_t* name);

		// 将缓冲区解析为一个 XML 文档片段，并将所有节点作为当前节点的子节点进行追加。
		// 会复制 / 转换缓冲区内容，所以在函数返回后，该缓冲区可以被删除或更改。
		//注意：append_buffer 会分配内存，其生命周期与所属文档相同；移除已追加的节点并不会立即回收该内存
                xml_parse_result append_buffer(const void* contents, size_t size, unsigned int options = parse_default, xml_encoding encoding = encoding_auto);

		// 使用谓词查找属性。返回使谓词返回为真的第一个属性。
		template <typename Predicate> xml_attribute find_attribute(Predicate pred) const
		{
			if (!_root) return xml_attribute();

			for (xml_attribute attrib = first_attribute(); attrib; attrib = attrib.next_attribute())
				if (pred(attrib))
					return attrib;

			return xml_attribute();
		}

		// 使用谓词查找子节点。返回使谓词返回值为真的第一个子节点。
		template <typename Predicate> xml_node find_child(Predicate pred) const
		{
			if (!_root) return xml_node();

			for (xml_node node = first_child(); node; node = node.next_sibling())
				if (pred(node))
					return node;

			return xml_node();
		}

		//使用谓词从子树中查找节点。返回子树中（深度优先遍历的情况下）使谓词返回值为真的第一个节点。
		template <typename Predicate> xml_node find_node(Predicate pred) const
		{
			if (!_root) return xml_node();

			xml_node cur = first_child();

			while (cur._root && cur._root != _root)
			{
				if (pred(cur)) return cur;

				if (cur.first_child()) cur = cur.first_child();
				else if (cur.next_sibling()) cur = cur.next_sibling();
				else
				{
					while (!cur.next_sibling() && cur._root != _root) cur = cur.parent();

					if (cur._root != _root) cur = cur.next_sibling();
				}
			}

			return xml_node();
		}

		// 通过属性名称 / 值查找子节点
		xml_node find_child_by_attribute(const char_t* name, const char_t* attr_name, const char_t* attr_value) const;
		xml_node find_child_by_attribute(const char_t* attr_name, const char_t* attr_value) const;

	#ifndef PUGIXML_NO_STL
		// 将从根节点开始的绝对节点路径作为文本字符串获取。
		string_t path(char_t delimiter = '/') const;
	#endif

		//通过由节点名称以及 “.” 或 “..” 元素组成的路径来搜索节点。
		xml_node first_element_by_path(const char_t* path, char_t delimiter = '/') const;

		// 使用 xml_tree_walker 递归遍历子树
		bool traverse(xml_tree_walker& walker);

	#ifndef PUGIXML_NO_XPATH
		// 通过计算 XPath 查询来选择单个节点。返回结果节点集中的第一个节点。
		xpath_node select_node(const char_t* query, xpath_variable_set* variables = 0) const;
		xpath_node select_node(const xpath_query& query) const;

		// 通过计算 XPath 查询来选择节点集
		xpath_node_set select_nodes(const char_t* query, xpath_variable_set* variables = 0) const;
		xpath_node_set select_nodes(const xpath_query& query) const;

		//已弃用：请改用 select_node）通过计算 XPath 查询来选择单个节点。
		PUGIXML_DEPRECATED xpath_node select_single_node(const char_t* query, xpath_variable_set* variables = 0) const;
		PUGIXML_DEPRECATED xpath_node select_single_node(const xpath_query& query) const;

	#endif

		// 使用写入器对象打印子树
		void print(xml_writer& writer, const char_t* indent = PUGIXML_TEXT("\t"), unsigned int flags = format_default, xml_encoding encoding = encoding_auto, unsigned int depth = 0) const;

	#ifndef PUGIXML_NO_STL
		//将子树打印到流中
		void print(std::basic_ostream<char, std::char_traits<char> >& os, const char_t* indent = PUGIXML_TEXT("\t"), unsigned int flags = format_default, xml_encoding encoding = encoding_auto, unsigned int depth = 0) const;
		void print(std::basic_ostream<wchar_t, std::char_traits<wchar_t> >& os, const char_t* indent = PUGIXML_TEXT("\t"), unsigned int flags = format_default, unsigned int depth = 0) const;
	#endif

		// 子节点迭代器
		typedef xml_node_iterator iterator;

		iterator begin() const;
		iterator end() const;

		// 属性迭代器
		typedef xml_attribute_iterator attribute_iterator;

		attribute_iterator attributes_begin() const;
		attribute_iterator attributes_end() const;

		// 基于范围的 for 循环支持
		xml_object_range<xml_node_iterator> children() const;
		xml_object_range<xml_named_node_iterator> children(const char_t* name) const;
		xml_object_range<xml_attribute_iterator> attributes() const;

		//出于调试目的，获取节点在已解析文件 / 字符串中的偏移量（以 char_t 为单位）
		ptrdiff_t offset_debug() const;

		//  获取哈希值（对于指向同一个对象的句柄来说是唯一的）
		size_t hash_value() const;

		//获取内部指针
		xml_node_struct* internal_object() const;
	};

#ifdef __BORLANDC__
	//Borland C++ 解决办法
	bool PUGIXML_FUNCTION operator&&(const xml_node& lhs, bool rhs);
	bool PUGIXML_FUNCTION operator||(const xml_node& lhs, bool rhs);
#endif

	//一个用于处理已解析字符数据（PCDATA）节点内文本的辅助工具
	class PUGIXML_CLASS xml_text
	{
		friend class xml_node;

		xml_node_struct* _root;

		typedef void (*unspecified_bool_type)(xml_text***);

		explicit xml_text(xml_node_struct* root);

		xml_node_struct* _data_new();
		xml_node_struct* _data() const;

	public:
		//默认构造函数。构造一个空对象。
		xml_text();

		//安全布尔转换操作符
		operator unspecified_bool_type() const;

		//  Borland C++ 解决办法
		bool operator!() const;

		//检查文本对象是否为空
		bool empty() const;

		// 获取文本内容，如果对象为空则返回 ""（空字符串）
		const char_t* get() const;

		//获取文本内容，如果对象为空则返回默认值
		const char_t* as_string(const char_t* def = PUGIXML_TEXT("")) const;

		// 将文本内容作为数字获取，如果转换不成功或者对象为空则返回默认值
		int as_int(int def = 0) const;
		unsigned int as_uint(unsigned int def = 0) const;
		double as_double(double def = 0) const;
		float as_float(float def = 0) const;

	#ifdef PUGIXML_HAS_LONG_LONG
		long long as_llong(long long def = 0) const;
		unsigned long long as_ullong(unsigned long long def = 0) const;
	#endif

		//  将文本内容作为布尔值获取（如果第一个字符在 '1tTyY' 集合中则返回 true，如果对象为空则返回默认值）
		bool as_bool(bool def = false) const;

		//设置文本（如果对象为空或者内存不足则返回 false）
		bool set(const char_t* rhs);

		//通过类型转换来设置文本（数字会被转换为字符串，布尔值会被转换为 "true" 或 "false"）
		bool set(int rhs);
		bool set(unsigned int rhs);
		bool set(long rhs);
		bool set(unsigned long rhs);
		bool set(double rhs);
		bool set(float rhs);
		bool set(bool rhs);

	#ifdef PUGIXML_HAS_LONG_LONG
		bool set(long long rhs);
		bool set(unsigned long long rhs);
	#endif

		// 设置文本（等同于不进行错误检查的设置操作）
		xml_text& operator=(const char_t* rhs);
		xml_text& operator=(int rhs);
		xml_text& operator=(unsigned int rhs);
		xml_text& operator=(long rhs);
		xml_text& operator=(unsigned long rhs);
		xml_text& operator=(double rhs);
		xml_text& operator=(float rhs);
		xml_text& operator=(bool rhs);

	#ifdef PUGIXML_HAS_LONG_LONG
		xml_text& operator=(long long rhs);
		xml_text& operator=(unsigned long long rhs);
	#endif

		// 获取此对象的数据节点（已解析字符数据节点 node_pcdata 或者字符数据节点 node_cdata）
		xml_node data() const;
	};

#ifdef __BORLANDC__
	//Borland C++ 解决办法
	bool PUGIXML_FUNCTION operator&&(const xml_text& lhs, bool rhs);
	bool PUGIXML_FUNCTION operator||(const xml_text& lhs, bool rhs);
#endif

	//  子节点迭代器（一个针对 xml_node 集合的双向迭代器）
	class PUGIXML_CLASS xml_node_iterator
	{
		friend class xml_node;

	private:
		mutable xml_node _wrap;
		xml_node _parent;

		xml_node_iterator(xml_node_struct* ref, xml_node_struct* parent);

	public:
		// 迭代器特性
		typedef ptrdiff_t difference_type;
		typedef xml_node value_type;
		typedef xml_node* pointer;
		typedef xml_node& reference;

	#ifndef PUGIXML_NO_STL
		typedef std::bidirectional_iterator_tag iterator_category;
	#endif

		// 默认构造函数
		xml_node_iterator();

		//构造一个指向指定节点的迭代器
		xml_node_iterator(const xml_node& node);

		//  迭代器运算符
		bool operator==(const xml_node_iterator& rhs) const;
		bool operator!=(const xml_node_iterator& rhs) const;

		xml_node& operator*() const;
		xml_node* operator->() const;

		const xml_node_iterator& operator++();
		xml_node_iterator operator++(int);

		const xml_node_iterator& operator--();
		xml_node_iterator operator--(int);
	};

	//  属性迭代器（一个针对 xml_attribute 集合的双向迭代器）
	class PUGIXML_CLASS xml_attribute_iterator
	{
		friend class xml_node;

	private:
		mutable xml_attribute _wrap;
		xml_node _parent;

		xml_attribute_iterator(xml_attribute_struct* ref, xml_node_struct* parent);

	public:
		// 迭代器特性
		typedef ptrdiff_t difference_type;
		typedef xml_attribute value_type;
		typedef xml_attribute* pointer;
		typedef xml_attribute& reference;

	#ifndef PUGIXML_NO_STL
		typedef std::bidirectional_iterator_tag iterator_category;
	#endif

		//默认构造函数
		xml_attribute_iterator();

		//构造一个指向指定属性的迭代器
		xml_attribute_iterator(const xml_attribute& attr, const xml_node& parent);

		// 迭代器运算符
		bool operator==(const xml_attribute_iterator& rhs) const;
		bool operator!=(const xml_attribute_iterator& rhs) const;

		xml_attribute& operator*() const;
		xml_attribute* operator->() const;

		const xml_attribute_iterator& operator++();
		xml_attribute_iterator operator++(int);

		const xml_attribute_iterator& operator--();
		xml_attribute_iterator operator--(int);
	};

	// 命名节点范围辅助类
	class PUGIXML_CLASS xml_named_node_iterator
	{
		friend class xml_node;

	public:
		//  迭代器特性
		typedef ptrdiff_t difference_type;
		typedef xml_node value_type;
		typedef xml_node* pointer;
		typedef xml_node& reference;

	#ifndef PUGIXML_NO_STL
		typedef std::bidirectional_iterator_tag iterator_category;
	#endif

		// 默认构造函数
		xml_named_node_iterator();

		// 构造一个指向指定节点的迭代器
		xml_named_node_iterator(const xml_node& node, const char_t* name);

		// 迭代器运算符
		bool operator==(const xml_named_node_iterator& rhs) const;
		bool operator!=(const xml_named_node_iterator& rhs) const;

		xml_node& operator*() const;
		xml_node* operator->() const;

		const xml_named_node_iterator& operator++();
		xml_named_node_iterator operator++(int);

		const xml_named_node_iterator& operator--();
		xml_named_node_iterator operator--(int);

	private:
		mutable xml_node _wrap;
		xml_node _parent;
		const char_t* _name;

		xml_named_node_iterator(xml_node_struct* ref, xml_node_struct* parent, const char_t* name);
	};

	// 抽象树遍历器类（详见 xml_node::traverse 方法）
	class PUGIXML_CLASS xml_tree_walker
	{
		friend class xml_node;

	private:
		int _depth;

	protected:
		//获取当前遍历深度
		int depth() const;

	public:
		xml_tree_walker();
		virtual ~xml_tree_walker();

		// 遍历开始时调用的回调函数
		virtual bool begin(xml_node& node);

		// 遍历每个节点时调用的回调函数
		virtual bool for_each(xml_node& node) = 0;

		//  遍历结束时调用的回调函数
		virtual bool end(xml_node& node);
	};

	// 解析状态，作为 xml_parse_result 对象的一部分返回
	enum xml_parse_status
	{
		status_ok = 0,				// 无错误

		status_file_not_found,		//在 load_file () 方法执行期间文件未找到
		status_io_error,			//  从文件 / 流读取时出错
		status_out_of_memory,		// 无法分配内存
		status_internal_error,		// 发生内部错误
		status_unrecognized_tag,	// 解析器无法确定标签类型

		status_bad_pi,				// 在解析文档声明 / 处理指令时发生解析错误
		status_bad_comment,			// 在解析注释时发生解析错误
		status_bad_pi,				//在解析 CDATA（字符数据）区段时发生解析错误
		status_bad_cdata,			// P在解析文档类型声明时发生解析错误
		status_bad_doctype,			// 在解析 PCDATA（已解析字符数据）区段时发生解析错误
		status_bad_pcdata,			// 在解析起始元素标签时发生解析错误
		status_bad_start_element,	// 在解析结束元素标签时发生解析错误
		status_bad_attribute,		// 在解析结束元素标签时发生解析错误
		status_bad_end_element,		// Parsing error occurred while parsing end element tag
		status_end_element_mismatch,// 存在起始 - 结束标签不匹配的情况（结束标签名称不正确、某些标签未关闭或者存在多余的结束标签）
		status_append_invalid_root,	// 由于根节点类型不是 node_element（元素节点）或 node_document（文档节点），所以无法追加节点（这是 xml_node::append_buffer 方法特有的情况）

		status_no_document_element	//解析后得到的文档中没有元素节点
	};

	// 解析结果
	struct PUGIXML_CLASS xml_parse_result
	{
		// 解析状态（参见 xml_parse_status）
		xml_parse_status status;

		//  最后解析的偏移量（以 char_t 为单位，从输入数据起始处开始计算）
		ptrdiff_t offset;

		//  源文档编码
		xml_encoding encoding;

		// 默认构造函数，将对象初始化为失败状态
		xml_parse_result();

		// 转换为布尔类型的运算符
		operator bool() const;

		// 获取错误描述
		const char* description() const;
	};

	//  文档类（DOM 树的根节点）
	class PUGIXML_CLASS xml_document: public xml_node
	{
	private:
		char_t* _buffer;

		char _memory[192];

		// 不可复制语义
		xml_document(const xml_document&);
		xml_document& operator=(const xml_document&);

		void _create();
		void _destroy();
		void _move(xml_document& rhs) PUGIXML_NOEXCEPT_IF_NOT_COMPACT;

	public:
		// 默认构造函数，创建一个空文档
		xml_document();

		//析构函数，使指向此文档的所有节点 / 属性句柄失效
		~xml_document();

	#ifdef PUGIXML_HAS_MOVE
		//移动语义支持
		xml_document(xml_document&& rhs) PUGIXML_NOEXCEPT_IF_NOT_COMPACT;
		xml_document& operator=(xml_document&& rhs) PUGIXML_NOEXCEPT_IF_NOT_COMPACT;
	#endif

		// 移除所有节点，留下一个空文档
		void reset();

		//移除所有节点，然后复制指定文档的全部内容
		void reset(const xml_document& proto);

	#ifndef PUGIXML_NO_STL
		//从流中加载文档。
		xml_parse_result load(std::basic_istream<char, std::char_traits<char> >& stream, unsigned int options = parse_default, xml_encoding encoding = encoding_auto);
		xml_parse_result load(std::basic_istream<wchar_t, std::char_traits<wchar_t> >& stream, unsigned int options = parse_default);
	#endif

		// （已弃用：请改用 load_string）从以零结尾的字符串中加载文档。不应用编码转换。
		PUGIXML_DEPRECATED xml_parse_result load(const char_t* contents, unsigned int options = parse_default);

		//从以零结尾的字符串中加载文档。不应用编码转换。
		xml_parse_result load_string(const char_t* contents, unsigned int options = parse_default);

		// 从文件中加载文档
		xml_parse_result load_file(const char* path, unsigned int options = parse_default, xml_encoding encoding = encoding_auto);
		xml_parse_result load_file(const wchar_t* path, unsigned int options = parse_default, xml_encoding encoding = encoding_auto);

		//  从缓冲区加载文档。会复制 / 转换缓冲区内容，所以在函数返回后，该缓冲区可以被删除或更改。
		xml_parse_result load_buffer(const void* contents, size_t size, unsigned int options = parse_default, xml_encoding encoding = encoding_auto);

		//  从缓冲区加载文档，使用该缓冲区进行原地解析（缓冲区会被修改并用于存储文档数据）。
		//你应当确保缓冲区数据在文档的整个生命周期内都持续存在，并且在文档销毁后手动释放缓冲区内存。
		xml_parse_result load_buffer_inplace(void* contents, size_t size, unsigned int options = parse_default, xml_encoding encoding = encoding_auto);

		// 从缓冲区加载文档，使用该缓冲区进行原地解析（缓冲区会被修改并用于存储文档数据）。
		//你应当使用 pugixml 分配函数来分配缓冲区；文档在不再需要该缓冲区时会自动释放它（之后你不能再使用该缓冲区了）。
		xml_parse_result load_buffer_inplace_own(void* contents, size_t size, unsigned int options = parse_default, xml_encoding encoding = encoding_auto);

		// 将 XML 文档保存到写入器（其语义与 xml_node::print 稍有不同，详见相关文档）。
		void save(xml_writer& writer, const char_t* indent = PUGIXML_TEXT("\t"), unsigned int flags = format_default, xml_encoding encoding = encoding_auto) const;

	#ifndef PUGIXML_NO_STL
		//将 XML 文档保存到流中（其语义与 xml_node::print 稍有不同，详见相关文档）。
		void save(std::basic_ostream<char, std::char_traits<char> >& stream, const char_t* indent = PUGIXML_TEXT("\t"), unsigned int flags = format_default, xml_encoding encoding = encoding_auto) const;
		void save(std::basic_ostream<wchar_t, std::char_traits<wchar_t> >& stream, const char_t* indent = PUGIXML_TEXT("\t"), unsigned int flags = format_default) const;
	#endif

		//将 XML 保存到文件
		bool save_file(const char* path, const char_t* indent = PUGIXML_TEXT("\t"), unsigned int flags = format_default, xml_encoding encoding = encoding_auto) const;
		bool save_file(const wchar_t* path, const char_t* indent = PUGIXML_TEXT("\t"), unsigned int flags = format_default, xml_encoding encoding = encoding_auto) const;

		// 获取文档元素
		xml_node document_element() const;
	};

#ifndef PUGIXML_NO_XPATH
	// XPath 查询返回类型
	enum xpath_value_type
	{
		xpath_type_none,	  // 未知类型（查询未能编译成功）
		xpath_type_node_set,  //节点集（xpath_node_set 类型）
		xpath_type_number,	  //数字
		xpath_type_string,	  // 字符串
		xpath_type_boolean	  // 布尔值
	};

	//  XPath 解析结果
	struct PUGIXML_CLASS xpath_parse_result
	{
		// 错误消息（若无错误则为 0）
		const char* error;

		// 最后解析的偏移量（以 char_t 为单位，从字符串起始处开始计算）
		ptrdiff_t offset;

		// 默认构造函数，将对象初始化为失败状态
		xpath_parse_result();

		//转换为布尔类型的运算符
		operator bool() const;

		// 获取错误描述
		const char* description() const;
	};

	//单个 XPath 变量
	class PUGIXML_CLASS xpath_variable
	{
		friend class xpath_variable_set;

	protected:
		xpath_value_type _type;
		xpath_variable* _next;

		xpath_variable(xpath_value_type type);

		//  不可复制语义
		xpath_variable(const xpath_variable&);
		xpath_variable& operator=(const xpath_variable&);

	public:
		// 获取变量名称
		const char_t* name() const;

		// 获取变量类型
		xpath_value_type type() const;

		// 获取变量值；不进行类型转换，若出现类型不匹配错误则返回默认值（false、非数字、空字符串、空节点集）
		bool get_boolean() const;
		double get_number() const;
		const char_t* get_string() const;
		const xpath_node_set& get_node_set() const;

		//设置变量值；不进行类型转换，若出现类型不匹配错误则返回 false
		bool set(bool value);
		bool set(double value);
		bool set(const char_t* value);
		bool set(const xpath_node_set& value);
	};

	// 一组 XPath 变量
	class PUGIXML_CLASS xpath_variable_set
	{
	private:
		xpath_variable* _data[64];

		void _assign(const xpath_variable_set& rhs);
		void _swap(xpath_variable_set& rhs);

		xpath_variable* _find(const char_t* name) const;

		static bool _clone(xpath_variable* var, xpath_variable** out_result);
		static void _destroy(xpath_variable* var);

	public:
		// 默认构造函数/析构函数
		xpath_variable_set();
		~xpath_variable_set();

		// 复制构造函数/赋值运算符
		xpath_variable_set(const xpath_variable_set& rhs);
		xpath_variable_set& operator=(const xpath_variable_set& rhs);

	#ifdef PUGIXML_HAS_MOVE
		//移动语义支持
		xpath_variable_set(xpath_variable_set&& rhs) PUGIXML_NOEXCEPT;
		xpath_variable_set& operator=(xpath_variable_set&& rhs) PUGIXML_NOEXCEPT;
	#endif

		//添加新变量或获取现有变量（如果类型匹配）
		xpath_variable* add(const char_t* name, xpath_value_type type);

		//设置现有变量的值;不执行类型转换，如果没有此类变量或类型不匹配，则返回 false
		bool set(const char_t* name, bool value);
		bool set(const char_t* name, double value);
		bool set(const char_t* name, const char_t* value);
		bool set(const char_t* name, const xpath_node_set& value);

		//按名称获取现有变量
		xpath_variable* get(const char_t* name);
		const xpath_variable* get(const char_t* name) const;
	};

	// 已编译的 XPath 查询对象
	class PUGIXML_CLASS xpath_query
	{
	private:
		void* _impl;
		xpath_parse_result _result;

		typedef void (*unspecified_bool_type)(xpath_query***);

		//不可复制的语义
		xpath_query(const xpath_query&);
		xpath_query& operator=(const xpath_query&);

	public:
		//从 XPath 表达式构造一个编译的对象。
		// 如果未定义 PUGIXML_NO_EXCEPTIONS，则在编译错误时引发xpath_exception。
		explicit xpath_query(const char_t* query, xpath_variable_set* variables = 0);

		// 构造 函数
		xpath_query();

		//破坏者
		~xpath_query();

	#ifdef PUGIXML_HAS_MOVE
		// 移动语义支持
		xpath_query(xpath_query&& rhs) PUGIXML_NOEXCEPT;
		xpath_query& operator=(xpath_query&& rhs) PUGIXML_NOEXCEPT;
	#endif

		//获取查询表达式返回类型
		xpath_value_type return_type() const;

		// 在指定上下文中将表达式计算为布尔值;如有必要，执行类型转换。
		//如果未定义 PUGIXML_NO_EXCEPTIONS，则引发 std：：bad_alloc on 内存不足错误。
		bool evaluate_boolean(const xpath_node& n) const;

		//在指定上下文中将 expression 评估为 double 值;如有必要，执行类型转换。
		//如果未定义 PUGIXML_NO_EXCEPTIONS，则引发 std：：bad_alloc on 内存不足错误。
		double evaluate_number(const xpath_node& n) const;

	#ifndef PUGIXML_NO_STL
		// 在指定上下文中将表达式评估为字符串值;如有必要，执行类型转换。
		// 如果未定义 PUGIXML_NO_EXCEPTIONS，则引发 std：：bad_alloc on 内存不足错误。
		string_t evaluate_string(const xpath_node& n) const;
	#endif

		//在指定上下文中将表达式评估为字符串值;如有必要，执行类型转换。
		// 大多数容量字符都会写入目标缓冲区，并返回完整的结果大小（包括终止零）。
		// 如果未定义 PUGIXML_NO_EXCEPTIONS，则引发 std：：bad_alloc on 内存不足错误。
		//如果定义了 PUGIXML_NO_EXCEPTIONS，则返回空集。
		size_t evaluate_string(char_t* buffer, size_t capacity, const xpath_node& n) const;

		// 将表达式评估为指定上下文中的节点集。
		//如果未定义 PUGIXML_NO_EXCEPTIONS，则引发 xpath_exception 类型 mismatch 和 std：：bad_alloc 内存不足错误。
		// 如果定义了 PUGIXML_NO_EXCEPTIONS，则返回空节点集。
		xpath_node_set evaluate_node_set(const xpath_node& n) const;

		// 将表达式评估为指定上下文中的节点集。
		// 按文档顺序返回第一个节点，如果节点集为空，则返回空节点。
		//如果未定义 PUGIXML_NO_EXCEPTIONS，则引发 xpath_exception 类型 mismatch 和 std：：bad_alloc 内存不足错误。
		//如果定义了 PUGIXML_NO_EXCEPTIONS，则返回空节点。
		xpath_node evaluate_node(const xpath_node& n) const;

		// 获取解析结果（用于在 PUGIXML_NO_EXCEPTIONS 模式下获取编译错误）
		const xpath_parse_result& result() const;

		//安全的 bool 转换运算符
		operator unspecified_bool_type() const;

		//Borland C++ 解决方法
		bool operator!() const;
	};

	#ifndef PUGIXML_NO_EXCEPTIONS
	// XPath 异常类
	class PUGIXML_CLASS xpath_exception: public std::exception
	{
	private:
		xpath_parse_result _result;

	public:
		// 根据解析结果构造异常
		explicit xpath_exception(const xpath_parse_result& result);

		// 获取错误消息
		virtual const char* what() const noexcept PUGIXML_OVERRIDE;

		// 获取解析结果
		const xpath_parse_result& result() const;
	};
	#endif

	// XPath 节点类（要么是 xml_node，要么是 xml_attribute）
	class PUGIXML_CLASS xpath_node
	{
	private:
		xml_node _node;
		xml_attribute _attribute;

		typedef void (*unspecified_bool_type)(xpath_node***);

	public:
		// 默认构造函数；用于构造空的 XPath 节点
		xpath_node();

		//从 XML 节点 / 属性构造 XPath 节点
		xpath_node(const xml_node& node);
		xpath_node(const xml_attribute& attribute, const xml_node& parent);

		// 获取节点 / 属性（如果有的话）
		xml_node node() const;
		xml_attribute attribute() const;

		// 获取所包含节点 / 属性的父节点
		xml_node parent() const;

		// Safe bool 转换运算符
		operator unspecified_bool_type() const;

		// Borland C++ 解决方法
		bool operator!() const;

		// 比较运算符
		bool operator==(const xpath_node& n) const;
		bool operator!=(const xpath_node& n) const;
	};

#ifdef __BORLANDC__
	// Borland C++ 解决方法
	bool PUGIXML_FUNCTION operator&&(const xpath_node& lhs, bool rhs);
	bool PUGIXML_FUNCTION operator||(const xpath_node& lhs, bool rhs);
#endif

	// A fixed-size collection of XPath nodes
	class PUGIXML_CLASS xpath_node_set
	{
	public:
		// 一个固定大小的 XPath 节点集合
		enum type_t
		{
			type_unsorted,			// 未排序的
			type_sorted,			// 按照文档顺序（升序）排序
			type_sorted_reverse		// 按照文档顺序（降序）排序
		};

		// Constant iterator type
		typedef const xpath_node* const_iterator;

		// 我们将非常量迭代器定义为与常量迭代器相同，以便各种泛型算法（例如 Boost 库中的foreach）能够正常工作
		typedef const xpath_node* iterator;

		// 默认构造函数。构造空集合。
		xpath_node_set();

		// 通过迭代器范围构造一个集合；不会检查数据是否存在重复项，并且不会根据所提供的类型对数据进行排序，所以要多加小心
		xpath_node_set(const_iterator begin, const_iterator end, type_t type = type_unsorted);

		//析构函数
		~xpath_node_set();

		// 拷贝构造函数 / 赋值运算符
		xpath_node_set(const xpath_node_set& ns);
		xpath_node_set& operator=(const xpath_node_set& ns);

	#ifdef PUGIXML_HAS_MOVE
		// Move semantics support
		xpath_node_set(xpath_node_set&& rhs) PUGIXML_NOEXCEPT;
		xpath_node_set& operator=(xpath_node_set&& rhs) PUGIXML_NOEXCEPT;
	#endif

		// Get collection type
		type_t type() const;

		// Get collection size
		size_t size() const;

		// Indexing operator
		const xpath_node& operator[](size_t index) const;

		// Collection iterators
		const_iterator begin() const;
		const_iterator end() const;

		// Sort the collection in ascending/descending order by document order
		void sort(bool reverse = false);

		// Get first node in the collection by document order
		xpath_node first() const;

		// Check if collection is empty
		bool empty() const;

	private:
		type_t _type;

		xpath_node _storage;

		xpath_node* _begin;
		xpath_node* _end;

		void _assign(const_iterator begin, const_iterator end, type_t type);
		void _move(xpath_node_set& rhs) PUGIXML_NOEXCEPT;
	};
#endif

#ifndef PUGIXML_NO_STL
	// Convert wide string to UTF8
	std::basic_string<char, std::char_traits<char>, std::allocator<char> > PUGIXML_FUNCTION as_utf8(const wchar_t* str);
	std::basic_string<char, std::char_traits<char>, std::allocator<char> > PUGIXML_FUNCTION as_utf8(const std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> >& str);

	// Convert UTF8 to wide string
	std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> > PUGIXML_FUNCTION as_wide(const char* str);
	std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> > PUGIXML_FUNCTION as_wide(const std::basic_string<char, std::char_traits<char>, std::allocator<char> >& str);
#endif

	// Memory allocation function interface; returns pointer to allocated memory or NULL on failure
	typedef void* (*allocation_function)(size_t size);

	// Memory deallocation function interface
	typedef void (*deallocation_function)(void* ptr);

	// Override default memory management functions. All subsequent allocations/deallocations will be performed via supplied functions.
	void PUGIXML_FUNCTION set_memory_management_functions(allocation_function allocate, deallocation_function deallocate);

	// Get current memory management functions
	allocation_function PUGIXML_FUNCTION get_memory_allocation_function();
	deallocation_function PUGIXML_FUNCTION get_memory_deallocation_function();
}

#if !defined(PUGIXML_NO_STL) && (defined(_MSC_VER) || defined(__ICC))
namespace std
{
	// Workarounds for (non-standard) iterator category detection for older versions (MSVC7/IC8 and earlier)
	std::bidirectional_iterator_tag PUGIXML_FUNCTION _Iter_cat(const pugi::xml_node_iterator&);
	std::bidirectional_iterator_tag PUGIXML_FUNCTION _Iter_cat(const pugi::xml_attribute_iterator&);
	std::bidirectional_iterator_tag PUGIXML_FUNCTION _Iter_cat(const pugi::xml_named_node_iterator&);
}
#endif

#if !defined(PUGIXML_NO_STL) && defined(__SUNPRO_CC)
namespace std
{
	// Workarounds for (non-standard) iterator category detection
	std::bidirectional_iterator_tag PUGIXML_FUNCTION __iterator_category(const pugi::xml_node_iterator&);
	std::bidirectional_iterator_tag PUGIXML_FUNCTION __iterator_category(const pugi::xml_attribute_iterator&);
	std::bidirectional_iterator_tag PUGIXML_FUNCTION __iterator_category(const pugi::xml_named_node_iterator&);
}
#endif

#endif

// Make sure implementation is included in header-only mode
// Use macro expansion in #include to work around QMake (QTBUG-11923)
#if defined(PUGIXML_HEADER_ONLY) && !defined(PUGIXML_SOURCE)
#	define PUGIXML_SOURCE "pugixml.cpp"
#	include PUGIXML_SOURCE
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
