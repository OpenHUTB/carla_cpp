// 版权声明，归属于2016年的Proyectos y Sistemas de Mantenimiento SL (eProsima)
// 遵循Apache License 2.0许可协议，使用该代码需遵守相关权限及限制条件
// 更多许可详情参考：http://www.apache.org/licenses/LICENSE-2.0
//
// 以下代码主要是与序列化函数声明相关的头文件内容，由fastcdrgen工具生成
// 注意：如果GEN_API_VER宏未定义或者不等于1，则表示生成的Vector3与当前安装的Fast DDS不兼容，需用fastddsgen工具重新生成
// 包含Fast DDS中主题数据类型相关的头文件
#include <fastdds/dds/topic/TopicDataType.hpp>
// 包含Fast RTPS中MD5相关工具的头文件，可能用于数据校验等操作
#include <fastrtps/utils/md5.h>
// 引入自定义的Vector3类型相关头文件，推测其中定义了Vector3结构体或类等内容
#include "Vector3.h"

#if!defined(GEN_API_VER) || (GEN_API_VER!= 1)
#error "生成的Vector3与当前安装的Fast DDS不兼容，请使用fastddsgen工具重新生成。"
#endif  // GEN_API_VER

// 定义在geometry_msgs命名空间下的msg命名空间，用于组织相关的消息类型等代码逻辑
namespace geometry_msgs {
namespace msg {

#ifndef SWIG
// 以下是detail命名空间内的内容，主要用于辅助处理与Vector3类型相关的一些底层操作细节
namespace detail {

// 定义一个模板结构体Vector3_rob，通过模板参数Tag以及对应类型M来获取特定成员相关信息
// 利用友元函数的方式，使得外部可以获取到这个特定成员的值（这里是由模板参数指定的成员）
template<typename Tag, typename Tag::type M>
struct Vector3_rob {
    friend constexpr typename Tag::type get(Tag) {
        return M;
    }
};

// 定义结构体Vector3_f，用于表示Vector3类型中成员类型为double的指针类型
// 通过友元函数的方式，外部可以获取到这个表示成员指针的类型
struct Vector3_f {
    typedef double Vector3::* type;
    friend constexpr type get(Vector3_f);
};

// 对模板结构体Vector3_rob进行实例化，指定成员为Vector3的m_z成员，便于后续相关操作中针对该成员的处理
template struct Vector3_rob<Vector3_f, &Vector3::m_z>;

// 计算给定类型T中，通过Tag指定的成员的偏移量（以字节为单位）
// 利用了指针操作以及类型转换等技巧来实现偏移量的计算，返回值是一个常量表达式形式的size_t类型大小
template <typename T, typename Tag>
inline size_t constexpr Vector3_offset_of() {
    return ((::size_t) &reinterpret_cast<char const volatile&>((((T*)0)->*get(Tag()))));
}
}
#endif

// Vector3PubSubType类，继承自eprosima::fastdds::dds::TopicDataType，用于处理与Vector3类型相关的数据操作
// 该类主要涉及在Fast DDS框架下，对Vector3类型数据的序列化、反序列化、获取数据大小、获取键值等功能
class Vector3PubSubType : public eprosima::fastdds::dds::TopicDataType {
public:
    // 定义类型别名，表明该类主要操作的数据类型是Vector3
    typedef Vector3 type;

    // 构造函数，用于初始化Vector3PubSubType类的实例，可能进行一些资源分配等初始化操作
    eProsima_user_DllExport Vector3PubSubType();

    // 析构函数，用于释放类实例占用的资源，例如动态分配的内存等
    eProsima_user_DllExport virtual ~Vector3PubSubType() override;

    // 序列化函数，将给定的Vector3类型的数据（通过data指针指向）转换为可在网络等环境传输的SerializedPayload_t类型的数据（存储在payload中）
    // 如果序列化成功返回true，否则返回false
    eProsima_user_DllExport virtual bool serialize(
            void* data,
            eprosima::fastrtps::rtps::SerializedPayload_t* payload) override;

    // 反序列化函数，将接收到的SerializedPayload_t类型的数据（通过payload指针指向）转换为Vector3类型的数据（存储在data指向的内存空间）
    // 如果反序列化成功返回true，否则返回false
    eProsima_user_DllExport virtual bool deserialize(
            eprosima::fastrtps::rtps::SerializedPayload_t* payload,
            void* data) override;

    // 获取序列化后数据大小的函数提供者，返回一个函数对象，该函数对象在调用时会返回给定数据（通过data指针指向）序列化后的大小（以字节为单位）
    eProsima_user_DllExport virtual std::function<uint32_t()> getSerializedSizeProvider(
            void* data) override;

    // 获取数据的键值，将给定的Vector3类型的数据（通过data指针指向）转换为用于唯一标识该数据实例的键值信息（存储在ihandle指向的内存空间）
    // 如果获取键值成功返回true，若force_md5为true可能采用MD5方式获取键值等特殊处理，否则返回false
    eProsima_user_DllExport virtual bool getKey(
            void* data,
            eprosima::fastrtps::rtps::InstanceHandle_t* ihandle,
            bool force_md5 = false) override;

    // 创建用于存储Vector3类型数据的内存空间，返回指向该内存空间的指针，通常用于分配动态内存来存储数据实例
    eProsima_user_DllExport virtual void* createData() override;

    // 释放之前通过createData函数分配的用于存储Vector3类型数据的内存空间，避免内存泄漏
    eProsima_user_DllExport virtual void deleteData(
            void* data) override;

#ifdef TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED
    // 判断该数据类型是否是有界的（可能表示数据大小等有固定范围限制），默认返回true
    eProsima_user_DllExport inline bool is_bounded() const override {
        return true;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED

#ifdef TOPIC_DATA_TYPE_API_HAS_IS_PLAIN
    // 判断该数据类型是否是简单类型（可能表示数据结构比较简单、无复杂嵌套等情况），内部调用is_plain_impl函数来判断
    eProsima_user_DllExport inline bool is_plain() const override {
        return is_plain_impl();
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_IS_PLAIN

#ifdef TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE
    // 在给定的内存空间（通过memory指针指向）构造一个Vector3类型的数据实例，类似调用构造函数的操作
    eProsima_user_DllExport inline bool construct_sample(
            void* memory) const override {
        new (memory) Vector3();
        return true;
    }

#endif  // TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE
    // MD5对象，可能用于计算数据的MD5值，比如在获取键值等操作中使用
    MD5 m_md5;
    // 用于存储键值等相关信息的缓冲区指针，动态分配内存来存储数据
    unsigned char* m_keyBuffer;
private:
    // 用于判断该数据类型是否是简单类型的具体实现函数，通过比较成员偏移量和数据大小等方式来判断
    static constexpr bool is_plain_impl() {
        return 24ULL == (detail::Vector3_offset_of<Vector3, detail::Vector3_f>() + sizeof(double));
    }
};
}
}

#endif // _FAST_DDS_GENERATED_GEOMETRY_MSGS_MSG_VECTOR3_PUBSUBTYPES_H_
