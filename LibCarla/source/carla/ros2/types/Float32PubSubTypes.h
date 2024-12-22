// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*!
 * @file Float32PubSubTypes.h
 * 此头文件包含序列化函数的声明。
 * 该文件由fastcdrgen工具生成。
 */

// 头文件保护机制，防止该头文件被重复包含。如果_FAST_DDS_GENERATED_STD_MSGS_MSG_FLOAT32_PUBSUBTYPES_H_未被定义，
// 则定义它，并开始头文件内容的编译；如果已经定义了，说明该头文件已经被包含过一次，编译器会跳过此次包含，避免重复定义错误
#ifndef _FAST_DDS_GENERATED_STD_MSGS_MSG_FLOAT32_PUBSUBTYPES_H_
#define _FAST_DDS_GENERATED_STD_MSGS_MSG_FLOAT32_PUBSUBTYPES_H_

// 包含Fast DDS中用于定义主题数据类型（TopicDataType）相关的头文件，这是整个代码基于Fast DDS框架进行数据处理的基础，
// 后续定义的类需要继承自其中的相关类，并实现相应虚函数来适配Fast DDS的数据发布与订阅机制
#include <fastdds/dds/topic/TopicDataType.hpp>
// 包含用于计算MD5哈希值的相关头文件，在数据处理过程中（比如生成数据的唯一标识、进行数据完整性校验等操作）可能会用到MD5算法，
// 这里引入相关头文件以便后续使用MD5相关的功能
#include <fastrtps/utils/md5.h>
// 包含Float32类型的定义所在头文件（推测是自定义的消息类型相关的头文件），意味着后续的代码逻辑是围绕对Float32这种特定消息类型的数据进行处理展开的，
// 比如实现该类型数据的序列化、反序列化等操作
#include "Float32.h"

// 检查生成的代码版本与当前安装的Fast DDS是否兼容，如果不兼容则报错提示需要重新生成。
// GEN_API_VER应该是一个预定义的宏，用于标识生成代码对应的版本号，通过这个条件判断确保代码能在当前Fast DDS环境下正确运行，
// 避免因版本不一致而出现的潜在问题
#if!defined(GEN_API_VER) || (GEN_API_VER!= 1)
#error \
    Generated Float32 is not compatible with current installed Fast DDS. Please, regenerate it with fastddsgen.
#endif  // GEN_API_VER

// 定义在std_msgs命名空间下，用于组织与消息类型相关的代码逻辑，将不同的消息类型及其相关操作放在各自的命名空间内，
// 可以有效避免命名冲突，使代码结构更加清晰、模块化，便于代码的维护和扩展
namespace std_msgs {
    namespace msg {
        // 以下部分代码在非SWIG（SWIG是一种用于连接C/C++与其他编程语言的工具，此处表示在不涉及SWIG相关编译的情况下）的情况进行定义
        #ifndef SWIG
        namespace detail {
            // 定义一个模板结构体Float32_rob，用于通过标签获取特定类型的值。
            // 它是一个模板化的结构体，通过不同的模板参数实例化，可以实现根据特定“标签”来获取相应类型的值，这种设计模式在一些需要灵活获取类型成员值的场景中很有用
            template<typename Tag, typename Tag::type M>
            struct Float32_rob {
                // 友元函数，用于获取指定标签对应的类型的值。通过将这个函数定义为友元函数，它可以访问结构体内部的私有成员（虽然此处没有体现私有成员，但从设计角度看具备这样的能力），
                // 根据传入的标签Tag，返回与之关联的类型的值M，实现了一种按照特定标识获取对应值的机制
                friend constexpr typename Tag::type get(
                        Tag) {
                    return M;
                }
            };

            // 定义结构体Float32_f，其内部定义了一个类型别名type，用于指向Float32类中成员的指针类型（这里明确指向float类型成员）。
            // 这个结构体的作用是为了抽象出Float32类中特定成员（float类型成员）的指针类型，方便后续在代码中统一使用这个类型别名来操作该成员指针，
            // 使得代码对于成员指针的操作更具可读性和可维护性
            struct Float32_f {
                typedef float Float32::* type;
                // 友元函数声明，用于获取Float32_f对应的类型（即成员指针类型）。通过友元函数声明，外部代码可以获取到这个结构体中定义的类型别名所代表的实际类型，
                // 便于在其他地方根据这个类型进行相应的操作（比如获取成员指针、通过指针访问成员等）
                friend constexpr type get(
                        Float32_f);
            };

            // 特化Float32_rob模板结构体，将其与Float32类中的m_data成员关联起来，用于后续通过标签获取该成员相关操作。
            // 这里利用模板特化的方式，针对Float32类中的m_data成员进行了具体化的配置，使得前面定义的通用模板结构体Float32_rob在处理Float32类的m_data成员时具有特定的行为，
            // 即可以通过特定的标签（这里就是Float32_f结构体相关的标识）来获取到m_data成员的值或者对其进行相关操作，增强了代码的针对性和灵活性
            template struct Float32_rob<Float32_f, &Float32::m_data>;

            // 定义一个模板函数，用于计算给定类型中特定成员相对于该类型起始地址的偏移量。
            // 在一些底层的数据处理场景中，比如序列化、内存布局调整等操作时，需要知道某个成员在整个类型结构体中的具体位置偏移情况，
            // 这个函数通过巧妙的指针运算来实现计算这种偏移量的功能，为后续相关操作提供必要的基础信息
            template <typename T, typename Tag>
            inline size_t constexpr Float32_offset_of() {
                // 通过巧妙的指针运算获取成员的偏移量，具体步骤如下：
                // 1. 先将空指针0转换为T*类型指针，这里利用了C++中指针的类型转换特性，将一个表示地址为0的空指针转换为指向类型T的指针，
                // 其目的是为了模拟一个类型T的对象的起始地址（因为地址为0只是一个理论上的起始点，实际不会访问这个地址的数据）。
                // 2. 再通过获取成员指针并转换为char类型的引用，即通过对模拟的T类型对象指针调用成员指针操作符（*）获取成员指针，然后将其转换为char类型的引用，
                // 这样做是因为在C++中，char类型的大小为1字节，通过将成员指针转换为char类型引用，可以方便地计算出相对于起始地址的字节偏移量。
                // 3. 最后取其地址得到偏移量，即获取这个char类型引用的地址，由于前面已经将其与模拟的起始地址关联起来，所以得到的地址值实际上就是成员相对于类型起始地址的偏移字节数。
                return ((::size_t) &reinterpret_cast<char const volatile&>((((T*)0)->*get(Tag()))));
            }
        }
        #endif

        /*!
         * @brief 此类表示由用户在IDL文件中定义的Float32类型的主题数据类型（TopicDataType）。
         * @ingroup FLOAT32
         * 它继承自eprosima::fastdds::dds::TopicDataType，意味着需要实现其中定义的一些虚函数来满足主题数据类型的相关操作要求，比如序列化、反序列化等。
         * 这个类在Fast DDS框架下扮演着关键角色，负责管理和处理与Float32消息类型相关的数据交互操作，包括数据的转换、传输以及标识等方面的功能实现。
         */
        class Float32PubSubType : public eprosima::fastdds::dds::TopicDataType {
        public:
            // 定义类型别名，表明此主题数据类型对应的实际数据类型为Float32。使用类型别名可以让代码在引用实际数据类型时更加简洁明了，
            // 提高代码的可读性，同时也方便后续进行类型相关的操作和判断，例如在函数参数传递、返回值类型定义等场景中可以直接使用这个别名来指代Float32类型
            typedef Float32 type;

            // 构造函数，通常用于初始化一些内部状态等，导出供外部使用（可能涉及动态库相关的标记，如在生成动态链接库时，用于正确导出函数，使得外部程序可以调用该构造函数来创建此类的对象）。
            // 在对象创建时，通过这个构造函数可以完成一些必要的初始化工作，例如设置一些默认值、分配相关资源等，为后续对象的正常使用做好准备
            eProsima_user_DllExport Float32PubSubType();

            // 析构函数，用于释放资源等清理操作，标记为虚函数以支持多态性，导出供外部使用（同样可能涉及动态库相关的标记，确保在动态库环境下能正确调用析构函数进行资源释放）。
            // 当对象的生命周期结束时，系统会自动调用析构函数来回收对象占用的资源，比如释放动态分配的内存、关闭打开的文件等操作，避免出现资源泄漏的问题
            eProsima_user_DllExport virtual ~Float32PubSubType() override;

            // 序列化函数，将给定的数据（这里是Float32类型的数据）序列化为适合网络传输等的格式（存储在SerializedPayload_t结构体中），若成功序列化返回true，否则返回false，需实现此虚函数。
            // 在Fast DDS框架中，为了能够在网络或者其他通信场景中传输Float32类型的数据，需要按照规定的序列化规则将其转换为特定的格式，
            // 这个函数就是负责执行这个转换过程的关键接口，它接收原始的Float32类型数据指针以及用于存储序列化结果的结构体指针，按照一定的逻辑将数据进行序列化处理，
            // 并根据处理结果返回相应的布尔值来表示序列化是否成功
            eProsima_user_DllExport virtual bool serialize(
                    void* data,
                    eprosima::fastrtps::rtps::SerializedPayload_t* payload) override;

            // 反序列化函数，将接收到的序列化数据（SerializedPayload_t结构体中的内容）转换为实际的Float32类型的数据存储在给定的内存地址（data指针指向的内存），若成功反序列化返回true，否则返回false，需实现此虚函数。
            // 与序列化函数相对应，当接收到经过序列化后的网络数据或者其他来源的数据时，需要通过这个函数按照相同的序列化规则将其还原为本地可用的Float32类型数据，
            // 它根据传入的序列化数据结构体以及用于存储还原后数据的内存地址指针，进行相应的解析和赋值操作，并根据操作结果返回表示反序列化是否成功的布尔值
            eProsima_user_DllExport virtual bool deserialize(
                    eprosima::fastrtps::rtps::SerializedPayload_t* payload,
                    void* data) override;

            // 获取序列化后数据大小的函数提供者，返回一个函数对象，调用该函数对象可以获取给定数据序列化后的大小（以字节为单位），需实现此虚函数。
            // 在很多数据处理场景中，比如在分配缓冲区、判断数据传输量或者进行内存管理等操作时，需要知道经过序列化后的数据具体占用多少字节空间，
            // 这个函数的作用就是提供一种获取该大小信息的方式，通过返回一个函数对象，外部代码可以在需要的时候调用这个函数对象来获取相应的序列化后数据大小值
            eProsima_user_DllExport virtual std::function<uint32_t()> getSerializedSizeProvider(
                    void* data) override;

            // 获取数据键值的函数，根据给定的数据（这里是Float32类型的数据）生成一个实例句柄（InstanceHandle_t），用于标识数据实例等用途，可选择是否强制使用MD5算法来生成，需实现此虚函数。
            // 在Fast DDS框架的数据分发、查找以及区分不同的数据实例等操作中，需要一种方式来唯一标识每个数据实例，这个函数就是根据Float32类型的数据生成对应的标识（实例句柄），
            // 通过对数据进行特定的处理（可以选择是否使用MD5算法来增强标识的唯一性和稳定性），生成一个InstanceHandle_t类型的实例句柄，用于后续相关操作中对数据实例的区分和定位
            eProsima_user_DllExport virtual bool getKey(
                    void* data,
                    eprosima::fastrtps::rtps::InstanceHandle_t* ihandle,
                    bool force_md5 = false) override;

            // 创建对应类型数据的内存空间，返回指向该内存空间的指针，需实现此虚函数。
            // 在准备发布数据或者接收数据并存储到本地时，往往需要动态分配内存来存放相应的Float32类型数据，这个函数就是负责创建合适的内存空间，
            // 并返回指向该内存空间的指针，以便后续可以将数据存储到这块内存中进行进一步的处理
            eProsima_user_DllExport virtual void* createData() override;

            // 释放之前通过createData创建的数据内存空间，需实现此虚函数。
            // 与createData函数相对应，当之前动态分配的内存空间不再被使用时（例如数据已经处理完毕或者对象生命周期结束等情况），
            // 需要通过这个函数来释放相应的内存资源，避免出现内存泄漏的问题，保证内存资源的有效管理和合理利用
            eProsima_user_DllExport virtual void deleteData(
                    void* data) override;

        #ifdef TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED
            // 判断此主题数据类型是否是有界的（例如数据长度是否固定等含义），如果宏定义了TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED，则实现此内联函数，这里返回true表示是有界的。
            // 在Fast DDS框架处理不同的数据类型时，有时候需要知道数据类型是否具有固定的边界（比如数据长度是否固定不变），以便采用不同的处理策略，
            // 这个内联函数就是用于提供这样的判断信息，根据具体的实现逻辑返回相应的布尔值来表示该数据类型是否为有界类型
            eProsima_user_DllExport inline bool is_bounded() const override {
                return true;
            }

        #endif  // TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED

        #ifdef TOPIC_DATA_TYPE_API_HAS_IS_PLAIN
            // 判断此主题数据类型是否是简单的（具体含义由相关实现定义），如果宏定义了TOPIC_DATA_TYPE_API_HAS_IS_PLAIN，则调用is_plain_impl函数来判断并返回结果。
            // 对于不同的数据类型，框架可能需要区分其复杂程度（例如是否只包含基本数据类型、是否有复杂的嵌套结构等），这里通过调用具体的实现函数is_plain_impl来进行判断，
            // 并返回相应的布尔值来表示该数据类型是否属于简单类型，以便在后续的优化或者特定处理逻辑中根据这个判断结果进行相应的操作
            eProsima_user_DllExport inline bool is_plain() const override {
                return is_plain_impl();
            }

        #endif  // TOPIC_DATA_TYPE_API_HAS_IS_PLAIN

        #ifdef TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE
            // 在给定的内存地址（memory指针指向的内存）构造一个样本数据（这里就是创建一个Float32类型的对象），如果宏定义了TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE，则实现此内联函数。
            // 在一些数据处理场景中，比如进行数据测试、批量创建初始数据等情况时，需要在指定的内存位置创建相应类型的数据对象，
            // 这个内联函数就是提供了这样一种在给定内存地址创建Float32类型对象的功能，创建成功后返回相应的布尔值表示操作是否成功
            eProsima_user_DllExport inline bool construct_sample(
                    memory) const override {
                new (memory) Float32();
                return true;
            }

        #endif  // TOPIC_DATA_TYPE_API_HAS_CONSTRUCT_SAMPLE

            // 用于存储MD5哈希值相关的对象，可能用于数据标识、校验等用途。在涉及到数据的唯一性标识、完整性校验或者根据数据生成特定的哈希码等操作时，
            // 会利用这个MD5对象来执行相应的计算和处理，通过调用其相关的成员函数来实现MD5算法的功能，比如计算数据的MD5哈希值等
            MD5 m_md5;
            // 用于存储键值缓冲区相关的指针，可能在生成键值等操作中使用。这个缓冲区可以临时存储与键值相关的数据，例如在生成数据的实例句柄（通过调用getKey函数）时，
            // 可能会先将一些与键值相关的数据存储在这个缓冲区中进行临时处理，然后再根据具体情况生成最终的实例句柄或者进行其他相关操作
            unsigned char* m_keyBuffer;

        private:
                // 静态常量函数，用于判断此主题数据类型是否是简单的具体实现逻辑，通过比较成员偏移量和 float 类型大小等方式来判断
            static constexpr bool is_plain_impl() {
                return 4ULL == (detail::Float32_offset_of<Float32, detail::Float32_f>() + sizeof(float));
            }
        };
    }
}

// 结束头文件的条件编译定义，与开头的#ifndef 对应，确保整个头文件内容只会在一次编译过程中被包含和处理，避免出现重复定义等编译错误，保证代码编译的正确性和稳定性。
#endif // _FAST_DDS_GENERATED_STD_MSGS_MSG_FLOAT32_PUBSUBTYPES_H_
