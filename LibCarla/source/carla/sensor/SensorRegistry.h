// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#ifndef LIBCARLA_SENSOR_REGISTRY_INCLUDE_H
#define LIBCARLA_SENSOR_REGISTRY_INCLUDE_H
// 引入CompositeSerializer头文件，它可能是一个用于组合多个序列化相关功能的类模板，
// 在后续构建传感器数据序列化和反序列化的注册机制中会起到关键作用
#include "carla/sensor/CompositeSerializer.h"

// =============================================================================
// 以下是注册一个新传感器需要遵循的4个步骤说明。
// =============================================================================

//  1. 在这里包含传感器对应的序列化器头文件。
// 以下这些头文件分别对应不同传感器类型的数据序列化功能，每个头文件中应该定义了如何将对应传感器的数据转换为可存储或传输的格式（序列化）。
#include "carla/sensor/s11n/CollisionEventSerializer.h"
#include "carla/sensor/s11n/DVSEventArraySerializer.h"
#include "carla/sensor/s11n/EpisodeStateSerializer.h"
#include "carla/sensor/s11n/GnssSerializer.h"
#include "carla/sensor/s11n/ImageSerializer.h"
#include "carla/sensor/s11n/NormalsImageSerializer.h"
#include "carla/sensor/s11n/OpticalFlowImageSerializer.h"
#include "carla/sensor/s11n/IMUSerializer.h"
#include "carla/sensor/s11n/LidarSerializer.h"
#include "carla/sensor/s11n/NoopSerializer.h"
#include "carla/sensor/s11n/ObstacleDetectionEventSerializer.h"
#include "carla/sensor/s11n/RadarSerializer.h"
#include "carla/sensor/s11n/SemanticLidarSerializer.h"
#include "carla/sensor/s11n/GBufferUint8Serializer.h"
#include "carla/sensor/s11n/GBufferFloatSerializer.h"
#include "carla/sensor/s11n/V2XSerializer.h"

// 2. 在这里添加传感器类的前置声明。
// 前置声明这些传感器类，告知编译器存在这些类的定义，但具体的类定义在其他地方（可能后续包含的头文件中或者其他源文件中），
// 这样可以解决在当前代码中使用这些类作为类型时的编译顺序问题，避免编译错误。
class ACollisionSensor;
class ADepthCamera;
class ANormalsCamera;
class ADVSCamera;
class AGnssSensor;
class AInertialMeasurementUnit;
class ALaneInvasionSensor;
class AObstacleDetectionSensor;
class AOpticalFlowCamera;
class ARadar;
class ARayCastSemanticLidar;
class ARayCastLidar;
class ASceneCaptureCamera;
class ASemanticSegmentationCamera;
class AInstanceSegmentationCamera;
class ARssSensor;
class FWorldObserver;
struct FCameraGBufferUint8;
struct FCameraGBufferFloat;
class AV2XSensor;
class ACustomV2XSensor;

namespace carla {
namespace sensor {

  // 3. 在SensorRegistry中注册传感器及其对应的序列化器。
 /// @brief SensorRegistry类，它包含了所有可用传感器的注册信息，并且允许对已注册类型的传感器数据进行序列化和反序列化操作。
    /// @details 对于那些不发送数据（仅在客户端工作的传感器），可以使用s11n::NoopSerializer（空操作序列化器，可能不进行实际的数据转换等操作）。
  /// Contains a registry of all the sensors available and allows serializing
  /// and deserializing sensor data for the types registered.
  ///
  /// Use s11n::NoopSerializer if the sensor does not send data (sensors that
  /// work only on client-side).
  using SensorRegistry = CompositeSerializer<
    std::pair<ACollisionSensor *, s11n::CollisionEventSerializer>,
    std::pair<ADepthCamera *, s11n::ImageSerializer>,
    std::pair<ANormalsCamera *, s11n::NormalsImageSerializer>,
    std::pair<ADVSCamera *, s11n::DVSEventArraySerializer>,
    std::pair<AGnssSensor *, s11n::GnssSerializer>,
    std::pair<AInertialMeasurementUnit *, s11n::IMUSerializer>,
    std::pair<ALaneInvasionSensor *, s11n::NoopSerializer>,
    std::pair<AObstacleDetectionSensor *, s11n::ObstacleDetectionEventSerializer>,
    std::pair<AOpticalFlowCamera *, s11n::OpticalFlowImageSerializer>,
    std::pair<ARadar *, s11n::RadarSerializer>,
    std::pair<ARayCastSemanticLidar *, s11n::SemanticLidarSerializer>,
    std::pair<ARayCastLidar *, s11n::LidarSerializer>,
    std::pair<ARssSensor *, s11n::NoopSerializer>,
    std::pair<ASceneCaptureCamera *, s11n::ImageSerializer>,
    std::pair<ASemanticSegmentationCamera *, s11n::ImageSerializer>,
    std::pair<AInstanceSegmentationCamera *, s11n::ImageSerializer>,
    std::pair<FWorldObserver *, s11n::EpisodeStateSerializer>,
    std::pair<FCameraGBufferUint8 *, s11n::GBufferUint8Serializer>,
    std::pair<FCameraGBufferFloat *, s11n::GBufferFloatSerializer>,
    std::pair<AV2XSensor *, s11n::CAMDataSerializer>,
    std::pair<ACustomV2XSensor *, s11n::CustomV2XDataSerializer>
    

  >;

} // namespace sensor
} // namespace carla

#endif // LIBCARLA_SENSOR_REGISTRY_INCLUDE_H

#ifdef LIBCARLA_SENSOR_REGISTRY_WITH_SENSOR_INCLUDES

// 4. 在这里包含传感器对应的头文件，这些头文件中定义了传感器类的具体实现，包括其成员变量、成员函数等内容，
// 使得在前面只是前置声明的传感器类在这里有了完整的定义，可在后续代码中进行更全面的使用（比如创建对象、调用成员函数等）。
#include "Carla/Sensor/CollisionSensor.h"
#include "Carla/Sensor/DepthCamera.h"
#include "Carla/Sensor/NormalsCamera.h"
#include "Carla/Sensor/DVSCamera.h"
#include "Carla/Sensor/GnssSensor.h"
#include "Carla/Sensor/InertialMeasurementUnit.h"
#include "Carla/Sensor/LaneInvasionSensor.h"
#include "Carla/Sensor/ObstacleDetectionSensor.h"
#include "Carla/Sensor/OpticalFlowCamera.h"
#include "Carla/Sensor/Radar.h"
#include "Carla/Sensor/RayCastLidar.h"
#include "Carla/Sensor/RayCastSemanticLidar.h"
#include "Carla/Sensor/RssSensor.h"
#include "Carla/Sensor/SceneCaptureCamera.h"
#include "Carla/Sensor/SemanticSegmentationCamera.h"
#include "Carla/Sensor/InstanceSegmentationCamera.h"
#include "Carla/Sensor/WorldObserver.h"
#include "Carla/Sensor/V2XSensor.h"
#include "Carla/Sensor/CustomV2XSensor.h"

#endif // LIBCARLA_SENSOR_REGISTRY_WITH_SENSOR_INCLUDES
