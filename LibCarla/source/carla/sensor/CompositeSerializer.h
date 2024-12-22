// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"
#include "carla/Memory.h"
#include "carla/sensor/CompileTimeTypeMap.h"
#include "carla/sensor/RawData.h"

namespace carla {
namespace sensor {
// 类 SensorData 的前向声明，可能在其他地方有完整定义。
  class SensorData;

  // ===========================================================================
  // -- CompositeSerializer ----------------------------------------------------
  // ===========================================================================

   /// 编译时映射表，用于将传感器对象映射到序列化器。
    ///
    /// 针对每个传感器，会调用相应的序列化器对其数据进行序列化和反序列化操作。
    ///
    /// 请勿直接使用，请使用传感器注册实例（SensorRegistry实例）。
  template <typename... Items>
  class CompositeSerializer : public CompileTimeTypeMap<Items...> {
    using Super = CompileTimeTypeMap<Items...>;

  public:
// 定义了一个智能指针类型 interpreted_type，指向 SensorData 对象。
    using interpreted_type = SharedPtr<SensorData>;

    /// Serialize the arguments provided into a Buffer by calling to the
    /// serializer registered for the given @a Sensor type.
    template <typename Sensor, typename... Args>
    static Buffer Serialize(Sensor &sensor, Args &&... args);

    /// Deserializes a Buffer by calling the "Deserialize" function of the
    /// serializer that generated the Buffer.
    static interpreted_type Deserialize(Buffer &&data);

  private:
// 这个模板函数是反序列化的内部实现，根据索引从 Buffer 中反序列化数据。
    template <size_t Index, typename Data>
    static interpreted_type Deserialize_impl(Data &&data) {
		// 获取对应索引的序列化器类型。
      using Serializer = typename Super::template get_by_index<Index>::type;
	  // 调用序列化器的反序列化函数进行反序列化。
      return Serializer::Deserialize(std::forward<Data>(data));
    }
// 这个模板函数也是反序列化的内部实现，根据索引序列和数据进行反序列化。
    template <typename Data, size_t... Is>
    static interpreted_type Deserialize_impl(size_t i, Data &&data, std::index_sequence<Is...>) {
      // This function is equivalent to creating a switch statement with a case
      // for each element in the map, the compiler should be able to optimize it
      // into a jump table. See https://stackoverflow.com/a/46282159/5308925.
      interpreted_type result;
      std::initializer_list<int> ({
          (i == Is ? (result = Deserialize_impl<Is>(std::forward<Data>(data))), 0 : 0)...
      });
      return result;
    }
// 这个模板函数是反序列化的入口函数，根据索引和数据调用内部实现进行反序列化。
    template <typename Data>
    static interpreted_type Deserialize(size_t index, Data &&data) {
      return Deserialize_impl(
          index,
          std::forward<Data>(data),
          std::make_index_sequence<Super::size()>());
    }
  };

  // ===========================================================================
  // -- CompositeSerializer implementation -------------------------------------
  // ===========================================================================
 // 这个模板函数实现了将传感器的数据序列化到 Buffer 的功能。
  template <typename... Items>
  template <typename Sensor, typename... Args>
  inline Buffer CompositeSerializer<Items...>::Serialize(Sensor &sensor, Args &&... args) {
	// 去除 Sensor 的 const 修饰，得到实际的传感器类型 TheSensor。
    using TheSensor = typename std::remove_const<Sensor>::type;
	// 获取对应传感器类型的序列化器类型 Serializer。
    using Serializer = typename Super::template get<TheSensor*>::type;
	// 调用序列化器的序列化函数进行序列化，并返回 Buffer。
    return Serializer::Serialize(sensor, std::forward<Args>(args)...);
  }
// 这个函数实现了反序列化 Buffer 中的数据，返回一个指向 SensorData 的智能指针。
  template <typename... Items>
  inline typename CompositeSerializer<Items...>::interpreted_type
  CompositeSerializer<Items...>::Deserialize(Buffer &&data) {
	  // 将移动后的 Buffer 包装成 RawData 对象。
    RawData message{std::move(data)};
	// 从 RawData 中获取传感器类型标识。
    size_t index = message.GetSensorTypeId();
	    // 根据传感器类型标识调用反序列化函数进行反序列化。
    return Deserialize(index, std::move(message));
  }

} // namespace sensor
} // namespace carla
