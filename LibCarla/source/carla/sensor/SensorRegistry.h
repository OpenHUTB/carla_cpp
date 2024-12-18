// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#ifndef LIBCARLA_SENSOR_REGISTRY_INCLUDE_H	// 该头文件定义了传感器注册相关的内容，用于管理不同类型传感器及其序列化操作
#define LIBCARLA_SENSOR_REGISTRY_INCLUDE_H

#include "carla/sensor/CompositeSerializer.h"

// =============================================================================
// Follow the 4 steps to register a new sensor.	// 以下是注册新传感器需要遵循的4个步骤相关的代码区域说明
// =============================================================================

// 1. Include the serializer here.
#include "carla/sensor/s11n/CollisionEventSerializer.h"	// 包含各种传感器对应的序列化器头文件，这些序列化器用于将传感器相关数据进行序列化操作
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

// 2. Add a forward-declaration of the sensor here.	// 对各种传感器类进行前置声明，告知编译器这些类在后续会被定义，避免编译时找不到类型定义的错误
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

  // 3. Register the sensor and its serializer in the SensorRegistry.	// 在SensorRegistry中注册传感器及其对应的序列化器，使得可以通过该注册表对相应传感器数据进行序列化和反序列化操作

  /// Contains a registry of all the sensors available and allows serializing
  /// and deserializing sensor data for the types registered.	// 包含了所有可用传感器的注册表，允许对已注册类型的传感器数据进行序列化和反序列化操作
  ///
  /// Use s11n::NoopSerializer if the sensor does not send data (sensors that
  /// work only on client-side).	// 如果传感器不发送数据（仅在客户端工作的传感器），则使用s11n::NoopSerializer
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

// 4. Include the sensor here.		// 包含实际的传感器类定义的头文件，这些头文件中定义了各种传感器类的具体实现等内容
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
