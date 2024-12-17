// Copyright (c) 2019 Intel Labs.
// 表明该代码遵循MIT许可协议，可通过对应链接查看协议详情
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// 引入Carla项目中几何相关的GeoLocation头文件，大概率用于表示地理位置相关的信息，比如经纬度、海拔等，与后续的全球导航卫星系统（GNSS）测量数据相关联
#include "carla/geom/GeoLocation.h"
// 引入Carla项目中传感器数据相关的SensorData头文件，GnssMeasurement类继承自SensorData类，意味着它会继承SensorData类已有的属性和行为，在此基础上构建特定于GNSS测量数据的功能
#include "carla/sensor/SensorData.h"
// 引入Carla项目中传感器序列化相关的GnssSerializer头文件，用于对GNSS测量数据进行序列化和反序列化操作，方便数据的存储、传输以及恢复等处理
#include "carla/sensor/s11n/GnssSerializer.h"

namespace carla {
namespace sensor {
namespace data {

  // 定义GnssMeasurement类，用于表示全球导航卫星系统（GNSS）测量数据的相关变化情况，继承自SensorData类，以复用传感器数据相关的通用逻辑，并拓展针对GNSS测量的特定功能
  /// A change of GNSS Measurement.
  class GnssMeasurement : public SensorData {
    // 使用类型别名Super来指代SensorData类型，方便在后续代码中简洁地表示父类，提高代码可读性和书写便利性
    using Super = SensorData;

  protected:
    // 使用类型别名Serializer来指代s11n::GnssSerializer类型，便于后续调用该序列化器相关的函数，使得代码在处理序列化和反序列化操作时更加清晰明了
    using Serializer = s11n::GnssSerializer;
    // 将Serializer声明为友元类，这使得GnssSerializer类能够访问GnssMeasurement类的私有成员，方便在进行数据序列化和反序列化操作时直接操作类内部的数据成员
    friend Serializer;

    // 显式定义的构造函数，接收一个右值引用类型的原始数据（RawData &&data）来创建一个GnssMeasurement对象，用于从给定的原始数据中初始化该GNSS测量对象的相关属性
    explicit GnssMeasurement(const RawData &&data)
      // 调用父类（SensorData）的构造函数，传递原始数据data，先完成父类部分的初始化工作，保证继承体系下的初始化顺序和完整性
      : Super(data){

      // 通过调用Serializer类的DeserializeRawData方法，从传入的原始数据中反序列化出地理位置相关的数据，将其赋值给_geo_location成员变量，以此初始化该GNSS测量对象所对应的地理位置信息
      geom::GeoLocation gnss_data = Serializer::DeserializeRawData(data);
      _geo_location = gnss_data;

    }

  public:
    // 获取该GNSS测量对应的地理位置信息，返回一个geom::GeoLocation类型的对象，外部代码可以通过调用此方法获取完整的地理位置详情，用于后续诸如地图定位、路径规划等相关操作
    geom::GeoLocation GetGeoLocation() const {
      return _geo_location;
    }

    // 获取该地理位置的经度信息，返回一个双精度浮点数类型的经度值，方便在只需要经度数据的场景下进行单独使用，比如判断在东西方向上的位置等
    double GetLongitude() const {
      return _geo_location.longitude;
    }

    // 获取该地理位置的纬度信息，返回一个双精度浮点数类型的纬度值，可用于确定在南北方向上的位置，常用于地图相关的坐标定位等操作
    double GetLatitude() const {
      return _geo_location.latitude;
    }

    // 获取该地理位置的海拔高度信息，返回一个双精度浮点数类型的海拔值，对于涉及高度差、地形分析等应用场景有重要作用
    double GetAltitude() const {
      return _geo_location.altitude;
    }

  private:
    // 用于存储该GNSS测量所对应的地理位置信息，是一个geom::GeoLocation类型的对象，包含了经度、纬度和海拔等详细的地理位置属性，这些属性通过构造函数从原始数据中反序列化得到并保存
    geom::GeoLocation _geo_location;

  };

} // namespace data
} // namespace sensor
} // namespace carla
