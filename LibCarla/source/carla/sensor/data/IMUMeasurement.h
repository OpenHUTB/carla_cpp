// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// 表明该代码遵循MIT许可协议，可通过对应链接查看协议具体内容
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

// 引入Carla项目中几何相关的Vector3D头文件，用于表示三维向量，在这个类中可能用于存储诸如加速度、角速度等具有方向和大小的物理量信息
#include "carla/geom/Vector3D.h"
// 引入Carla项目中传感器序列化相关的IMUSerializer头文件，其功能是对惯性测量单元（IMU）的数据进行序列化和反序列化操作，方便数据的存储、传输以及后续的恢复使用
#include "carla/sensor/s11n/IMUSerializer.h"
// 引入Carla项目中传感器数据相关的SensorData头文件，IMUMeasurement类继承自SensorData类，意味着它能复用SensorData类已有的通用属性和行为，在此基础上拓展针对IMU测量数据的特定功能
#include "carla/sensor/SensorData.h"

namespace carla {
namespace sensor {
namespace data {

  // 定义IMUMeasurement类，用于表示惯性测量单元（IMU）测量得到的数据信息，继承自SensorData类，以遵循整个传感器数据相关的统一框架，并添加自身特有的IMU测量相关功能
  class IMUMeasurement : public SensorData {
  protected:
    // 使用类型别名Super来指代SensorData类型，这样在后续代码中可以更简洁地引用父类类型，方便代码书写和阅读，同时也更清晰地表明继承关系
    using Super = SensorData;

    // 使用类型别名Serializer来指代s11n::IMUSerializer类型，便于后续直接调用该序列化器的相关函数，在代码中体现出对IMU数据序列化和反序列化操作的关联性
    using Serializer = s11n::IMUSerializer;

    // 将Serializer声明为友元类，这使得IMUSerializer类能够访问IMUMeasurement类的私有成员，便于在进行数据的序列化和反序列化过程中，直接操作类内部的相关数据成员，保证数据处理的完整性和准确性
    friend Serializer;

    // 显式定义的构造函数，接收一个RawData类型的常量引用参数（表示原始数据），用于从传入的原始数据中初始化IMUMeasurement对象的各项属性，创建一个有效的IMU测量数据对象
    explicit IMUMeasurement(const RawData &data)
      // 首先调用父类（SensorData）的构造函数，传递原始数据data，完成父类部分的初始化，确保继承体系下的初始化顺序正确，继承父类的相关属性和行为
      : Super(data),
        // 通过调用Serializer类的DeserializeRawData方法，从原始数据中反序列化出加速度计相关的数据，并使用其初始化_accelerometer成员变量，该变量用于存储IMU测量得到的加速度信息，以三维向量形式表示其在空间中的方向和大小
        _accelerometer(Serializer::DeserializeRawData(data).accelerometer),
        // 同样借助Serializer类的DeserializeRawData方法，从原始数据中反序列化出陀螺仪相关的数据，以此初始化_gyroscope成员变量，该变量用于存储IMU测量得到的角速度信息，同样以三维向量形式呈现其在不同方向上的旋转速度情况
        _gyroscope(Serializer::DeserializeRawData(data).gyroscope),
        // 还是调用Serializer类的DeserializeRawData方法，从原始数据中反序列化出罗盘（指南针）相关的数据，用来初始化_compass成员变量，该变量存储的是罗盘测量得到的方向信息，以浮点数形式表示（可能是角度等相关表示形式，具体取决于项目内的定义）
        _compass(Serializer::DeserializeRawData(data).compass) {}

  public:
    // 获取加速度计测量得到的加速度信息，返回一个geom::Vector3D类型的三维向量，外部代码可以通过调用此方法获取IMU中加速度计所测得的加速度大小和方向，用于后续如物体运动分析、姿态估计等相关操作
    geom::Vector3D GetAccelerometer() const {
      return _accelerometer;
    }

    // 获取陀螺仪测量得到的角速度信息，返回一个geom::Vector3D类型的三维向量，以便外部代码能够获取IMU中陀螺仪所检测到的物体绕各个轴的旋转速度情况，常用于姿态解算、稳定控制等应用场景
    geom::Vector3D GetGyroscope() const {
      return _gyroscope;
    }

    // 获取罗盘测量得到的方向信息，返回一个浮点数，该值代表了根据罗盘测量所得的某个方向相关的数据（例如可能是相对于地磁北极的角度等，具体取决于项目中的定义），可用于确定物体的朝向等操作
    float GetCompass() const {
      return _compass;
    }

  private:
    // 用于存储加速度计测量得到的加速度信息，是一个geom::Vector3D类型的三维向量，其值通过构造函数从原始数据中反序列化获取，在后续的使用中保持不变，供外部通过相应的访问函数来获取该数据
    geom::Vector3D _accelerometer;
    // 用于存储陀螺仪测量得到的角速度信息，同样是一个geom::Vector3D类型的三维向量，其数据来源于构造函数对原始数据的反序列化操作，用于反映物体的旋转状态相关信息，外部可通过对应的访问函数获取
    geom::Vector3D _gyroscope;
    // 用于存储罗盘测量得到的方向信息，以浮点数类型表示，其具体数值由构造函数从原始数据中解析而来，用于提供物体朝向等相关的方向参考，可通过访问函数对外提供该数据
    float _compass;

  };

} // namespace data
} // namespace sensor
} // namespace carla
