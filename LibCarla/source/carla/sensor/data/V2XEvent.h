// Copyright (c) 2024 Institut fuer Technik der Informationsverarbeitung (ITIV) at the
// Karlsruhe Institute of Technology
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/sensor/data/Array.h"
#include "carla/sensor/s11n/V2XSerializer.h"
#include "carla/sensor/data/V2XData.h"

namespace carla
{
    namespace sensor
    {
        namespace data
        {

            class CAMEvent : public Array<data::CAMData> // 定义一个名为 CAMEvent 的类，它继承自 Array<data::CAMData>，意味着它拥有 Array<data::CAMData> 的成员和行为，
 // 可能是用于处理特定的 CAM（也许是某种车辆通信相关的数据类型）事件数据
            {
                using Super = Array<data::CAMData>;
               // 使用 using 关键字给父类 Array<data::CAMData> 起了一个别名 Super，方便后续代码中对父类类型的引用
                         protected:
                using Serializer = s11n::CAMDataSerializer;
               // 同样使用 using 关键字，给 s11n::CAMDataSerializer 类型起了一个别名 Serializer，
              // 可能用于后续涉及到该类型序列化相关操作时方便书写代码

                friend Serializer;
          // 将 Serializer 声明为友元，意味着 Serializer 类型（应该是一个序列化相关的类）可以访问 CAMEvent 类中的私有和保护成员
                explicit CAMEvent(RawData &&data)
                    : Super(0u, std::move(data)) {}
 // 定义 CAMEvent 类的构造函数，接收一个右值引用类型的 RawData 参数，
                // 通过初始化列表调用父类的构造函数，传递初始大小为0以及移动后的 data 参数，用于初始化对象
            public:
                Super::size_type GetMessageCount() const
// 定义一个公共的成员函数 GetMessageCount，返回类型是父类中定义的表示大小类型（size_type），
                // 函数功能应该是获取消息的数量，由于函数被声明为 const，表明调用该函数不会修改对象的状态

                {
                    return Super::size();// 返回父类对象中存储的数据元素个数，即消息数量，通过调用父类的 size 函数来实现
                }
            };

            class CustomV2XEvent : public Array<data::CustomV2XData> // 定义名为 CustomV2XEvent 的类，继承自 Array<data::CustomV2XData>，可能用于处理自定义的V2X事件数据

            {
                using Super = Array<data::CustomV2XData>;
 // 给父类 Array<data::CustomV2XData> 起别名 Super，方便后续代码对父类类型引用

            protected:
                using Serializer = s11n::CustomV2XDataSerializer;
// 给 s11n::CustomV2XDataSerializer 类型起别名 Serializer，可能和该类的序列化操作有关
                friend Serializer;
 // 将 Serializer 声明为友元，使其能访问类的私有和保护成员
                explicit CustomV2XEvent(RawData &&data)
                    : Super(0u, std::move(data)) {}
 // 定义构造函数，接收右值引用的 RawData 参数，通过初始化列表调用父类构造函数进行初始化，初始大小设为0并移动 data 参数
            public:
                Super::size_type GetMessageCount() const
 // 定义公共成员函数 GetMessageCount，返回类型为父类的 size_type，用于获取消息数量，函数为 const 表明不修改对象状态
                {
                    return Super::size();
// 通过调用父类的 size 函数来获取并返回存储的数据元素个数，也就是消息数量
                }
            };

        } // namespace data
    }     // namespace sensor
} // namespace carla
