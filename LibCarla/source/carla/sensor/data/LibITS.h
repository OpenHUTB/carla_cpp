#pragma once
#include <stdint.h>
#include <vector>
#include <cstring>

class ITSContainer
{
    
public:

    typedef bool OptionalValueAvailable_t;
    
    /* Latitude Dependencies*/
    typedef enum Latitude
    {
        Latitude_oneMicroDegreeNorth = 10,
        Latitude_oneMicroDegreeSouth = -10,
        Latitude_unavailable = 900000001
    } e_Latitude;

    /* Latitude */
    typedef long Latitude_t;

    /* Longitude Dependencies */
    typedef enum Longitude
    {
        Longitude_oneMicroDegreeEast = 10,
        Longitude_oneMicroDegreeWest = -10,
        Longitude_unavailable = 1800000001
    } e_Longitude;

    /* Longitude */
    typedef long Longitude_t;

    /* SemiAxisLength Dependencies */
    typedef enum SemiAxisLength
    {
        SemiAxisLength_oneCentimeter = 1,
        SemiAxisLength_outOfRange = 4094,
        SemiAxisLength_unavailable = 4095
    } e_SemiAxisLength;

    /* SemiAxisLength*/
    typedef long SemiAxisLength_t;

    /* HeadingValue Dependencies */
    typedef enum HeadingValue
    {
        HeadingValue_wgs84North = 0,
        HeadingValue_wgs84East = 900,
        HeadingValue_wgs84South = 1800,
        HeadingValue_wgs84West = 2700,
        HeadingValue_unavailable = 3601
    } e_HeadingValue;

    /* HeadingValue */
    typedef long HeadingValue_t;

     /* HeadingConfidence Dependencies */
    typedef enum HeadingConfidence {
        HeadingConfidence_equalOrWithinZeroPointOneDegree   = 1,
        HeadingConfidence_equalOrWithinOneDegree    = 10,
        HeadingConfidence_outOfRange    = 126,
        HeadingConfidence_unavailable   = 127
    } e_HeadingConfidence;
 
    /* HeadingConfidence */
    typedef long HeadingConfidence_t;

    /* PosConfidenceEllipse*/
    typedef struct PosConfidenceEllipse
    {
        SemiAxisLength_t semiMajorConfidence;
        SemiAxisLength_t semiMinorConfidence;
        HeadingValue_t semiMajorOrientation;
    } PosConfidenceEllipse_t;
    
    /* AltitudeValue Dependencies */
    typedef enum AltitudeValue
    {
        AltitudeValue_referenceEllipsoidSurface = 0,
        AltitudeValue_oneCentimeter = 1,
        AltitudeValue_unavailable = 800001
    } e_AltitudeValue;

    /* AltitudeValue */
    typedef long AltitudeValue_t;

    /* AltitudeConfidence Dependencies */
    typedef enum AltitudeConfidence
    {
        AltitudeConfidence_alt_000_01   = 0,
        AltitudeConfidence_alt_000_02   = 1,
        AltitudeConfidence_alt_000_05   = 2,
        AltitudeConfidence_alt_000_10   = 3,
        AltitudeConfidence_alt_000_20   = 4,
        AltitudeConfidence_alt_000_50   = 5,
        AltitudeConfidence_alt_001_00   = 6,
        AltitudeConfidence_alt_002_00   = 7,
        AltitudeConfidence_alt_005_00   = 8,
        AltitudeConfidence_alt_010_00   = 9,
        AltitudeConfidence_alt_020_00   = 10,
        AltitudeConfidence_alt_050_00   = 11,
        AltitudeConfidence_alt_100_00   = 12,
        AltitudeConfidence_alt_200_00   = 13,
        AltitudeConfidence_outOfRange   = 14,
        AltitudeConfidence_unavailable  = 15
    }e_AltitudeConfidence;

    /* AltitudeConfidence */
    typedef long AltitudeConfidence_t;

    /* Altitude */
    typedef struct Altitude
    {
        AltitudeValue_t altitudeValue;
        AltitudeConfidence_t altitudeConfidence;
    }Altitude_t;

    /* ReferencePosition */
    typedef struct ReferencePosition
    {
        Latitude_t latitude;
        Longitude_t longitude;
        PosConfidenceEllipse_t positionConfidenceEllipse;
        Altitude_t altitude;
    } ReferencePosition_t;

    /* StationType Dependencies */
    typedef enum StationType
    {
        StationType_unknown = 0,
        StationType_pedestrian = 1,
        StationType_cyclist = 2,
        StationType_moped = 3,
        StationType_motorcycle = 4,
        StationType_passengerCar = 5,
        StationType_bus = 6,
        StationType_lightTruck = 7,
        StationType_heavyTruck = 8,
        StationType_trailer = 9,
        StationType_specialVehicles = 10,
        StationType_tram = 11,
        StationType_roadSideUnit = 15        
    } e_StationType;

    /* StationType */
    typedef long StationType_t;

    /* StationID*/
    typedef long StationID_t;
    // typedef unsigned long StationID_t;

    /* Dependencies */
    typedef enum protocolVersion
    {
        protocolVersion_currentVersion = 1
    } e_protocolVersion;

    typedef enum messageID
    {
        messageID_custom = 0,
        messageID_denm = 1,
        messageID_cam = 2,
        messageID_poi = 3,
        messageID_spat = 4,
        messageID_map = 5,
        messageID_ivi = 6,
        messageID_ev_rsr = 7
    } e_messageID;

    typedef struct ItsPduHeader
    {
        long protocolVersion;
        long messageID;
        StationID_t stationID;
    } ItsPduHeader_t;

    /* Heading */
    typedef struct Heading
    {
        HeadingValue_t headingValue;
        HeadingConfidence_t headingConfidence;
    } Heading_t;

    /* SpeedValue Dependencies */
    typedef enum SpeedValue
    {
        SpeedValue_standstill = 0,
        SpeedValue_oneCentimeterPerSec = 1,
        SpeedValue_unavailable = 16383
    } e_SpeedValue;

    /* SpeedValue */
    typedef long SpeedValue_t;

    /* SpeedConfidence Dependencies */
    typedef enum SpeedConfidence
    {
        SpeedConfidence_equalOrWithInOneCentimerterPerSec = 1,
        SpeedConfidence_equalOrWithinOneMeterPerSec = 100,
        SpeedConfidence_outOfRange = 126,
        SpeedConfidence_unavailable = 127
    } e_SpeedConfidence;

    /* SpeedConfidence */
    typedef long SpeedConfidence_t;

    /* Speed */
    typedef struct speed
    {
        SpeedValue_t speedValue;
        SpeedConfidence_t speedConfidence;
    } Speed_t;

    /* DriveDirection Dependencies */
    typedef enum DriveDirection 
    {
        DriveDirection_forward  = 0,
        DriveDirection_backward = 1,
        DriveDirection_unavailable  = 2
    } e_DriveDirection;
 
    /* DriveDirection */
    typedef long DriveDirection_t;

    /* VehicleLength Dependencies */
    typedef enum VehicleLengthValue
    {
        VehicleLengthValue_tenCentimeters = 1,
        VehicleLengthValue_outOfRange = 1022,
        VehicleLengthValue_unavailable = 1023
    } e_VehicleLengthValue;

    /* VehicleLengthValue */
    typedef long VehicleLengthValue_t;

    /* VehicleLengthConfidenceIndication Dependencies */
    typedef enum VehicleLengthConfidenceIndication
    {
        VehicleLengthConfidenceIndication_noTrailerPresent = 0,
        VehicleLengthConfidenceIndication_trailerPresentWithKnownLength = 1,
        VehicleLengthConfidenceIndication_trailerPresentWithUnknownLength  = 2,
        VehicleLengthConfidenceIndication_trailerPresenceIsUnknown = 3,
        VehicleLengthConfidenceIndication_unavailable = 4
    } e_VehicleLengthConfidenceIndication;

    /* VehicleLengthConfidenceIndication */
    typedef long VehicleLengthConfidenceIndication_t;

    /* VehicleLength */
    typedef struct VehicleLength
    {
        VehicleLengthValue_t vehicleLengthValue;
        VehicleLengthConfidenceIndication_t vehicleLengthConfidenceIndication;
    } VehicleLength_t;

    /* VehicleWidth Dependencies */
    typedef enum VehicleWidth
    {
        VehicleWidth_tenCentimeters = 1,
        VehicleWidth_outOfRange = 61,
        VehicleWidth_unavailable = 62
    } e_VehicleWidth;

    /* VehicleWidth */
    typedef long VehicleWidth_t;

    /* LongitudinalAcceleration Dependencies */
    typedef enum LongitudinalAcceletationValue
    {
        LongitudinalAccelerationValue_pointOneMeterPerSecSquaredForward = 1,
        LongitudinalAccelerationValue_pointOneMeterPerSecSquaredBackWard = -1,
        LongitudinalAccelerationValue_unavailable = 161
    } e_LongitudinalAccelerationValue;

    /* LongitudinalAcclerationValue */
    typedef long LongitudinalAccelerationValue_t;

    /* AccelerationConfidence Dependencies */
    typedef enum AccelerationConfidence
    {
        AccelerationConfindence_pointOneMeterPerSecSquared = 1,
        AccelerationConfidence_outOfRange = 101,
        AccelerationConfidence_unavailable = 102
    } e_AccelerationConfidence;

    /* AccelerationConfidence*/
    typedef long AccelerationConfidence_t;

    /* LongitudinalAcceleration */
    typedef struct LongitudinalAcceleration
    {
        LongitudinalAccelerationValue_t longitudinalAccelerationValue;
        AccelerationConfidence_t longitudinalAccelerationConfidence;
    } LongitudinalAcceleration_t;

    /* CurvatureValue Dependencies */
    typedef enum CurvatureValue
    {
        CurvatureValue_straight = 0,
        CurvatureValue_reciprocalOf1MeterRadiusToRight = -30000,
        CurvatureValue_reciprocalOf1MeterRadiusToLeft = 30000,
        CurvatureValue_unavailable = 30001
    } e_CurvatureValue;

    /* CurvatureValue */
    typedef long CurvatureValue_t;

    /* CurvatureConfidence Dependencies*/
    typedef enum CurvatureConfidence
    {
        CurvatureConfidence_onePerMeter_0_00002 = 0, // 表示每米的曲率置信度为0.00002的情况，对应枚举值为0
        CurvatureConfidence_onePerMeter_0_0001  = 1, // 表示每米的曲率置信度为0.0001的情况，对应枚举值为1
        CurvatureConfidence_onePerMeter_0_0005  = 2, // 表示每米的曲率置信度为0.0005的情况，对应枚举值为2
        CurvatureConfidence_onePerMeter_0_002   = 3, // 表示每米的曲率置信度为0.002的情况，对应枚举值为3
        CurvatureConfidence_onePerMeter_0_01    = 4, // 表示每米的曲率置信度为0.01的情况，对应枚举值为4
        CurvatureConfidence_onePerMeter_0_1 = 5,// 表示每米的曲率置信度为0.1的情况，对应枚举值为5
        CurvatureConfidence_outOfRange  = 6,// 表示超出范围的曲率置信度情况，对应枚举值为6
        CurvatureConfidence_unavailable = 7 // 表示不可用的曲率置信度情况，对应枚举值为7
    } e_CurvatureConfidence;
// 为CurvatureConfidence类型定义一个别名CurvatureConfidence_t，方便代码中使用，本质上是long类型
    /* CurvatureConfidence */
    typedef long CurvatureConfidence_t;

    /* Curvature */// 定义结构体Curvature，用于封装曲率相关的信息，包含曲率值和曲率置信度两个成员
    typedef struct Curvature
    {
        CurvatureValue_t curvatureValue; // 表示曲率的具体数值，其类型应该是之前定义的CurvatureValue_t（此处未给出其定义细节）
        CurvatureConfidence_t curvatureConfidence; // 表示该曲率对应的置信度，类型为CurvatureConfidence_t（即long类型的别名）
    } Curvature_t;

    /* CurvatureCalculationMode Dependencies */
    typedef enum CurvatureCalculationMode// 定义枚举类型CurvatureCalculationMode，用于表示不同的曲率计算模式取值情况
    {
        CurvatureCalculationMode_yarRateUsed = 0, // 表示使用偏航率（yaw rate）进行曲率计算的模式，对应枚举值为0
        CurvatureCalculationMode_yarRateNotUsed = 1, // 表示不使用偏航率进行曲率计算的模式，对应枚举值为1
        CurvatureCalculationMode_unavailable = 2 // 表示曲率计算模式不可用的情况，对应枚举值为2
    } e_CurvatureCalculationMode;

    /* CurvatureCalculationMode */// 为CurvatureCalculationMode类型定义一个别名CurvatureCalculationMode_t，方便代码中使用，本质上是long类型
    typedef long CurvatureCalculationMode_t;

    /* YawRateValue Dependencies */// 定义枚举类型YawRateValue，用于表示不同的偏航率值情况
    typedef enum YawRateValue
    {
        YawRateValue_straight = 0, // 表示车辆处于直线行驶状态，对应的偏航率值为0，枚举值设为0
        YawRateValue_degSec_000_01ToRight   = -1, // 表示偏航率为每秒向右转0.01度的情况，用负数表示方向，对应枚举值为 -1
        YawRateValue_degSec_000_01ToLeft    = 1, // 表示偏航率为每秒向左转0.01度的情况，用正数表示方向，对应枚举值为1
        YawRateValue_unavailable    = 32767// 表示偏航率不可用的情况，对应一个较大的特定值32767作为标识
    } e_YawRateValue;

    /* YawRateValue */// 为YawRateValue类型定义一个别名YawRateValue_t，方便代码中使用，本质上是long类型
    typedef long YawRateValue_t;

    /* YawRateConfidence Dependencies */
    typedef enum YawRateConfidence {
        YawRateConfidence_degSec_000_01 = 0,// 表示偏航率的置信度为每秒0.00001度的级别，对应枚举值为0
        YawRateConfidence_degSec_000_05 = 1,  // 表示偏航率的置信度为每秒0.00005度的级别，对应枚举值为1
        YawRateConfidence_degSec_000_10 = 2, // 表示偏航率的置信度为每秒0.00010度的级别，对应枚举值为2
        YawRateConfidence_degSec_001_00 = 3,// 表示偏航率的置信度为每秒0.00100度的级别，对应枚举值为3
        YawRateConfidence_degSec_005_00 = 4,// 表示偏航率的置信度为每秒0.00500度的级别，对应枚举值为4
        YawRateConfidence_degSec_010_00 = 5, // 表示偏航率的置信度为每秒0.01000度的级别，对应枚举值为5
        YawRateConfidence_degSec_100_00 = 6,// 表示偏航率的置信度为每秒0.10000度的级别，对应枚举值为6
        YawRateConfidence_outOfRange    = 7, // 表示偏航率置信度超出正常范围的情况，对应枚举值为7
        YawRateConfidence_unavailable   = 8 // 表示偏航率置信度不可用的情况，对应枚举值为8
    } e_YawRateConfidence;
 
    /* YawRateConfidence */
    typedef long YawRateConfidence_t;// 为YawRateConfidence类型定义一个别名YawRateConfidence_t，方便在代码中使用，其本质上是long类型

    /* YawRate */
    typedef struct YawRate
    {
        YawRateValue_t yawRateValue; // 表示偏航率的具体数值，其类型为之前定义的YawRateValue_t（应该在其他地方有对应定义）
        YawRateConfidence_t yawRateConfidence; // 表示该偏航率对应的置信度，类型为YawRateConfidence_t（即long类型别名）
    } YawRate_t;

    /* AccelerationControl Dependencies */
    typedef enum AccelerationControl {
        AccelerationControl_brakePedalEngaged   = 0, // 表示刹车踏板被踩下的状态，对应枚举值为0
        AccelerationControl_gasPedalEngaged = 1, // 表示油门踏板被踩下的状态，对应枚举值为1
        AccelerationControl_emergencyBrakeEngaged   = 2,// 表示紧急刹车被启动的状态，对应枚举值为2
        AccelerationControl_collisionWarningEngaged = 3,// 表示碰撞预警系统被触发的状态，对应枚举值为3
        AccelerationControl_accEngaged  = 4, // 表示加速功能被启用的状态（此处具体哪种加速功能可根据上下文确定），对应枚举值为4
        AccelerationControl_cruiseControlEngaged    = 5, // 表示定速巡航功能被启用的状态，对应枚举值为5
        AccelerationControl_speedLimiterEngaged = 6// 表示限速功能被启用的状态，对应枚举值为6
    } e_AccelerationControl;
 
    /* AccelerationControl */
    typedef uint8_t AccelerationControl_t;// 将AccelerationControl_t定义为uint8_t类型，用于后续表示加速度控制相关的变量等，可能是基于其取值范围适合用8位无符号整数表示

    /* LanePosition Dependencies */
// 定义枚举类型LanePosition，用于列举车辆可能处于的不同车道位置情况
    typedef enum LanePosition {
        LanePosition_offTheRoad = -1, // 表示车辆已经偏离正常道路范围，例如开到道路外面了，对应枚举值为 -1
        LanePosition_hardShoulder   = 0,// 表示车辆处于硬路肩位置，通常是道路边缘供临时停车等用途的区域，对应枚举值为0
        LanePosition_outermostDrivingLane   = 1, // 表示车辆处于最外侧的行车道上，对应枚举值为1
        LanePosition_secondLaneFromOutside  = 2// 表示车辆处于从最外侧数起的第二车道位置，对应枚举值为2
    } e_LanePosition;
 
    /* LanePosition */
    typedef long LanePosition_t;/ 为LanePosition类型定义一个别名LanePosition_t，方便在代码其他地方使用，本质上是long类型，便于统一处理车道位置相关的数据

    /* SteeringWheelAngleValue Dependencies */
    typedef enum SteeringWheelAngleValue {
        SteeringWheelAngleValue_straight    = 0,// 表示方向盘处于正前方直线行驶对应的角度位置，对应枚举值为0
        SteeringWheelAngleValue_onePointFiveDegreesToRight  = -1, // 表示方向盘向右转了1.5度的情况，用负数表示向右转动方向，对应枚举值为 -1
        SteeringWheelAngleValue_onePointFiveDegreesToLeft   = 1, // 表示方向盘向左转了1.5度的情况，用正数表示向左转动方向，对应枚举值为1
        SteeringWheelAngleValue_unavailable = 512 // 表示方向盘角度信息不可用的情况，对应一个特定的枚举值512作为标识
    } e_SteeringWheelAngleValue;
 
    /* SteeringWheelAngleValue */
    typedef long SteeringWheelAngleValue_t;// 为SteeringWheelAngleValue类型定义一个别名SteeringWheelAngleValue_t，方便在代码中使用，本质上是long类型，便于统一处理方向盘角度值相关的数据

    /* SteeringWheelAngleConfidence Dependencies */
    typedef enum SteeringWheelAngleConfidence {
        SteeringWheelAngleConfidence_equalOrWithinOnePointFiveDegree    = 1, // 表示方向盘角度的置信度为等于或在1.5度范围内的情况，对应枚举值为1
        SteeringWheelAngleConfidence_outOfRange = 126,// 表示方向盘角度置信度超出正常范围的情况，对应枚举值为126
        SteeringWheelAngleConfidence_unavailable    = 127// 表示方向盘角度置信度不可用的情况，对应枚举值为127
    } e_SteeringWheelAngleConfidence;
 
    /* SteeringWheelAngleConfidence */
    typedef long SteeringWheelAngleConfidence_t;// 为SteeringWheelAngleConfidence类型定义一个别名SteeringWheelAngleConfidence_t，方便在代码中使用，本质上是long类型，便于统一处理方向盘角度置信度相关的数据

    /* SteeringWheelAngle */
    typedef struct SteeringWheelAngle
    {
        SteeringWheelAngleValue_t steeringWheelAngleValue;// 表示方向盘的具体角度值，其类型为之前定义的SteeringWheelAngleValue_t
        SteeringWheelAngleConfidence_t steeringWheelAngleConfidence;// 表示该方向盘角度对应的置信度，类型为SteeringWheelAngleConfidence_t
    } SteeringWheelAngle_t;

    /* LateralAccelerationValue Dependencies */
    typedef enum LateralAccelerationValue {
        LateralAccelerationValue_pointOneMeterPerSecSquaredToRight  = -1, // 表示车辆横向加速度为每秒平方0.1米且方向向右的情况，用负数表示向右方向，对应枚举值为 -1
        LateralAccelerationValue_pointOneMeterPerSecSquaredToLeft   = 1,  // 表示车辆横向加速度为每秒平方0.1米且方向向左的情况，用正数表示向左方向，对应枚举值为1
        LateralAccelerationValue_unavailable    = 161// 表示车辆横向加速度信息不可用的情况，对应一个特定的枚举值161作为标识
    } e_LateralAccelerationValue;

    /* LateralAccelerationValue */
    typedef long LateralAccelerationValue_t;// 为LateralAccelerationValue类型定义一个别名LateralAccelerationValue_t，方便在代码中使用，本质上是long类型，便于统一处理横向加速度值相关的数据

    /* LateralAcceleration */
    typedef struct LateralAcceleration
    {
        LateralAccelerationValue_t lateralAccelerationValue;// 表示车辆横向加速度的具体数值，其类型为之前定义的LateralAccelerationValue_t
        AccelerationConfidence_t lateralAccelerationConfidence; // 表示该横向加速度对应的置信度，类型为AccelerationConfidence_t
    } LateralAcceleration_t;

    /* VerticalAccelerationValue Dependencies */
    typedef enum VerticalAccelerationValue {
        VerticalAccelerationValue_pointOneMeterPerSecSquaredUp  = 1, // 表示车辆垂直加速度为每秒平方0.1米且方向向上的情况，用正数表示向上方向，对应枚举值为1
        VerticalAccelerationValue_pointOneMeterPerSecSquaredDown    = -1, // 表示车辆垂直加速度为每秒平方0.1米且方向向下的情况，用负数表示向下方向，对应枚举值为 -1
        VerticalAccelerationValue_unavailable   = 161// 表示车辆垂直加速度信息不可用的情况，对应一个特定的枚举值161作为标识
    } e_VerticalAccelerationValue;
 
    /* VerticalAccelerationValue */
    typedef long VerticalAccelerationValue_t;

    /* VerticalAcceleration */
    typedef struct VerticalAcceleration
    {
        VerticalAccelerationValue_t verticalAccelerationValue; // 表示车辆垂直加速度的具体数值，其类型为之前定义的VerticalAccelerationValue_t
        AccelerationConfidence_t verticalAccelerationConfidence;// 表示该垂直加速度对应的置信度，类型为AccelerationConfidence_t
    } VerticalAcceleration_t;

    /* PerformanceClass Dependencies */
    typedef enum PerformanceClass {
        PerformanceClass_unavailable    = 0,// 表示车辆性能等级不可用的情况，对应枚举值为0
        PerformanceClass_performanceClassA  = 1,// 表示车辆属于性能等级A的情况，对应枚举值为1
        PerformanceClass_performanceClassB  = 2 // 表示车辆属于性能等级B的情况，对应枚举值为2
    } e_PerformanceClass;
 
    /* PerformanceClass */
    typedef long PerformanceClass_t;// 为PerformanceClass类型定义一个别名PerformanceClass_t，方便在代码中使用，本质上是long类型，便于统一处理车辆性能等级相关的数据

    /* ProtectedZoneID */
    typedef long ProtectedZoneID_t;
// 定义类型别名ProtectedZoneID_t，用于表示保护区（可能是特定区域相关的概念）的标识，本质上是long类型，方便在后续代码中使用该类型处理相关逻辑

    /* CenDsrcTollingZoneID */
    typedef ProtectedZoneID_t CenDsrcTollingZoneID_t;// 定义类型别名CenDsrcTollingZoneID_t，它等同于ProtectedZoneID_t类型，可能用于表示特定的基于DSRC（专用短程通信）的收费区域标识，方便代码中统一使用该类型进行相关操作。

    /* CenDsrcTollingZone */
    typedef struct CenDsrcTollingZone {
        Latitude_t   protectedZoneLatitude;// 表示该收费区域的纬度信息，其类型应该是之前定义的Latitude_t（可能在其他地方有定义）
        Longitude_t  protectedZoneLongitude; // 表示该收费区域的经度信息，其类型应该是之前定义的Longitude_t（可能在其他地方有定义）
        CenDsrcTollingZoneID_t  cenDsrcTollingZoneID;   /* OPTIONAL */// 表示该基于DSRC的收费区域的标识，类型为CenDsrcTollingZoneID_t，并且此成员是可选的（可能在某些情况下不存在）
        OptionalValueAvailable_t cenDsrcTollingZoneIDAvailable; // 表示上述收费区域标识是否可用的信息，其类型应该是OptionalValueAvailable_t（可能在其他地方有定义）
    } CenDsrcTollingZone_t;

    /* ProtectedZoneType Dependencies */
    typedef enum ProtectedZoneType {
        ProtectedZoneType_cenDsrcTolling    = 0

    } e_ProtectedZoneType;
 
    /* ProtectedZoneType */
    typedef long     ProtectedZoneType_t;// 为ProtectedZoneType类型定义一个别名ProtectedZoneType_t，方便在代码中统一使用，本质上是long类型，便于后续处理与保护区类型相关的数据。

    /* TimestampIts Dependencies */
    typedef enum TimestampIts {
        TimestampIts_utcStartOf2004 = 0,// 表示2004年UTC时间起始时刻对应的时间戳，对应枚举值为0，可作为一个基准时间参考点。
        TimestampIts_oneMillisecAfterUTCStartOf2004 = 1 // 表示在2004年UTC时间起始时刻之后1毫秒的时间戳，对应枚举值为1，用于表示相对起始时刻稍有延迟的时间点。
    } e_TimestampIts;



    /* TimestampIts */
    typedef long TimestampIts_t; // 为TimestampIts类型定义一个别名TimestampIts_t，方便在代码中使用，本质上是long类型，便于统一处理与ITS时间戳相关的数据。

    /* ProtectedZoneRadius Dependencies */
    typedef enum ProtectedZoneRadius {
        ProtectedZoneRadius_oneMeter    = 1 // 表示保护区半径为1米的情况，对应枚举值为1，用于界定保护区的空间范围大小。
    } e_ProtectedZoneRadius;
 
    /* ProtectedZoneRadius */
    typedef long ProtectedZoneRadius_t;

    /* ProtectedCommunicationZone */
    typedef struct ProtectedCommunicationZone {
        ProtectedZoneType_t  protectedZoneType;// 表示该受保护通信区域的类型，其类型为之前定义的ProtectedZoneType_t，用于明确此区域所属的具体类型。
        TimestampIts_t  expiryTime /* OPTIONAL */; // 表示该受保护通信区域的过期时间，类型为TimestampIts_t，此成员是可选的（可能在某些情况下不存在），用于限定区域有效的时间范围。
        OptionalValueAvailable_t expiryTimeAvailable; // 表示上述过期时间是否可用的信息，其类型应该是OptionalValueAvailable_t（可能在其他地方有定义），用于判断过期时间信息的有效性。
        Latitude_t   protectedZoneLatitude; // 表示该受保护通信区域的纬度信息，其类型应该是之前定义的Latitude_t（可能在其他地方有定义），用于确定区域在地理上的纬度位置。
        Longitude_t  protectedZoneLongitude;// 表示该受保护通信区域的经度信息，其类型应该是之前定义的Longitude_t（可能在其他地方有定义），用于确定区域在地理上的经度位置。
        ProtectedZoneRadius_t   protectedZoneRadius    /* OPTIONAL */; // 表示该受保护通信区域的半径信息，类型为ProtectedZoneRadius_t，此成员是可选的（可能在某些情况下不存在），用于进一步明确区域的范围大小。
        OptionalValueAvailable_t protectedZoneRadiusAvailable; // 表示上述区域半径是否可用的信息，其类型应该是OptionalValueAvailable_t（可能在其他地方有定义），用于判断半径信息的有效性。
        ProtectedZoneID_t   protectedZoneID    /* OPTIONAL */;// 表示该受保护通信区域的标识信息，类型为ProtectedZoneID_t，此成员是可选的（可能在某些情况下不存在），用于唯一标识该区域。
        OptionalValueAvailable_t protectedZoneIDAvailable;// 表示上述区域标识是否可用的信息，其类型应该是OptionalValueAvailable_t（可能在其他地方有定义），用于判断标识信息的有效性。
    } ProtectedCommunicationZone_t;

    /* ProtectedCommunicationZonesRSU */
    typedef struct ProtectedCommunicationZonesRSU {
        long ProtectedCommunicationZoneCount; // 表示受保护通信区域的数量，使用long类型来存储，用于记录当前所涉及的受保护通信区域的总个数，方便后续遍历等操作。
        std::vector<ProtectedCommunicationZone_t> list; // 定义一个动态数组（使用std::vector），其中元素类型为ProtectedCommunicationZone_t，用于存储各个受保护通信区域的详细信息，比如区域类型、位置、有效期等相关内容。
    } ProtectedCommunicationZonesRSU_t;

    /* VehicleRole Dependencies */
    typedef enum VehicleRole {
        VehicleRole_default = 0, // 表示默认的车辆角色，对应枚举值为0，可作为一种通用性的设定，用于那些未明确指定特殊角色的车辆情况，在一些通用处理逻辑中可能会用到。
        VehicleRole_publicTransport = 1, // 表示公共交通车辆角色，像公交车、地铁等这类为大众提供公共出行服务的车辆，对应枚举值为1，便于在交通管理等系统中对这类车辆进行针对性处理，比如优先通行权等相关逻辑。
        VehicleRole_specialTransport    = 2, // 表示特殊运输车辆角色，通常用于运输一些具有特殊性质、要求的物品，对应枚举值为2，例如运输文物、机密文件等特殊货物的车辆，有助于对这类有特殊运输需求的车辆进行识别和管理。
        VehicleRole_dangerousGoods  = 3, // 表示运输危险货物的车辆角色，对应枚举值为3，像运输易燃易爆、有毒有害等危险物品的货车等，在交通管控中需要格外关注其行驶路线、安全保障等方面，方便进行特殊处理以确保道路安全。
        VehicleRole_roadWork    = 4, // 表示道路施工相关车辆角色，对应枚举值为4，比如道路维修的工程车、压路车等，用于区分这类参与道路建设、维护作业的车辆，在交通调度等方面可能会有相应的优先安排或者限行规则。
        VehicleRole_rescue  = 5,// 表示救援车辆角色，像救护车、消防车等执行紧急救援任务的车辆，对应枚举值为5，这类车辆在应急情况下往往具有特殊的通行权限，通过此枚举值可方便在系统中识别并给予相应通行保障。
        VehicleRole_emergency   = 6, // 表示应急车辆角色，涵盖多种在紧急突发状况下出动的车辆，对应枚举值为6，比如应对自然灾害、重大事故等场景下的指挥车、抢险车等，突出其紧急性以及在交通中的特殊地位，便于相关应急处置逻辑的实现。
        VehicleRole_safetyCar   = 7,// 表示安全保障车辆角色，例如赛事活动中的安全车、交通管制中的引导车等，对应枚举值为7，用于特定场景下保障交通安全、秩序等用途的车辆识别与相应处理。
        VehicleRole_agriculture = 8, // 表示农业用途车辆角色，对应枚举值为8，像拖拉机、收割机等在农业生产活动中使用的车辆，便于在交通管理中区分这类具有特定使用场景的车辆。
        VehicleRole_commercial  = 9, // 表示商业用途车辆角色，例如货运卡车、物流配送车等从事商业运输活动的车辆，对应枚举值为9，有助于针对这类车辆进行运输调度、费用核算等相关业务逻辑处理。
        VehicleRole_military    = 10,// 表示军事用途车辆角色，对应枚举值为10，像军车等军队相关的车辆，在交通管理中可能涉及保密、优先通行等特殊要求，通过此枚举值可方便进行识别和相应安排。
        VehicleRole_roadOperator    = 11, // 表示道路运营管理相关车辆角色，对应枚举值为11，比如道路巡检车、收费管理车等负责道路运营、维护及管理工作的车辆，便于在交通系统中区分这类具有特定职责的车辆。
        VehicleRole_taxi    = 12,// 表示出租车车辆角色，对应枚举值为12，用于在交通运营系统中对出租车这类提供客运服务的车辆进行明确标识，方便进行计价、调度等相关业务处理。
        VehicleRole_reserved1   = 13,// 表示预留的车辆角色1，对应枚举值为13，主要是为了后续可能的功能扩展、新车辆角色定义等情况预留的位置，方便代码的可扩展性维护。
        VehicleRole_reserved2   = 14, // 表示预留的车辆角色2，对应枚举值为14，同样是出于对未来新增车辆角色的考虑，预留此位置以便后续能灵活添加新的角色分类，不影响现有代码结构。
        VehicleRole_reserved3   = 15 // 表示预留的车辆角色3，对应枚举值为15，为后续进一步拓展车辆角色相关功能提供了空间，使代码在应对业务变化时更具适应性。
    } e_VehicleRole;
 
    /* VehicleRole */
    typedef long VehicleRole_t;

    /* ExteriorLights Dependencies */
    typedef enum ExteriorLights {
        ExteriorLights_lowBeamHeadlightsOn  = 0,// 表示车辆近光灯处于开启状态，对应枚举值为0，常用于判断车辆在正常行驶、光线较暗等情况下近光灯的使用情况，以便进行相关提醒或合规性检查等操作。
        ExteriorLights_highBeamHeadlightsOn = 1, // 表示车辆远光灯处于开启状态，对应枚举值为1，可用于检测车辆是否在不适当的时候开启了远光灯（比如会车时），进而进行相应的提示或管理。
        ExteriorLights_leftTurnSignalOn = 2, // 表示车辆左转向灯处于开启状态，对应枚举值为2，在车辆转向或者变道向左时开启，通过此枚举值可在交通监控等系统中判断车辆的转向意图和操作规范性。
        ExteriorLights_rightTurnSignalOn    = 3,// 表示车辆右转向灯处于开启状态，对应枚举值为3，与左转向灯类似，用于判断车辆向右转向或变道的操作情况，有助于交通管理和安全保障。
        ExteriorLights_daytimeRunningLightsOn   = 4,// 表示车辆日间行车灯处于开启状态，对应枚举值为4，可用于确认车辆在白天行驶时是否按规定开启了日间行车灯，提升车辆的辨识度和行车安全性。
        ExteriorLights_reverseLightOn   = 5,// 表示车辆倒车灯处于开启状态，对应枚举值为5，在车辆倒车操作时点亮，便于后方车辆和行人知晓车辆的倒车动作，通过此枚举值可监控倒车操作的合法性等情况。
        ExteriorLights_fogLightOn   = 6, // 表示车辆雾灯处于开启状态，对应枚举值为6，用于判断车辆在雾天、雨天等能见度较低的特殊天气下是否正确开启雾灯，以保障行车安全和符合交通规则。
        ExteriorLights_parkingLightsOn  = 7// 表示车辆停车灯处于开启状态，对应枚举值为7，可用于检测车辆停车时是否按要求开启停车灯，起到提示周围交通参与者的作用。
    } e_ExteriorLights;

    /* ExteriorLights */
    typedef uint8_t ExteriorLights_t;// 为ExteriorLights类型定义一个别名ExteriorLights_t，方便在代码中使用，本质上是uint8_t类型，因为车辆灯光状态用较小的数值范围就能表示，使用8位无符号整数类型既足够存储这些状态值，又能更节省内存空间，便于统一处理车辆外部灯光相关的数据操作。
    /* DeltaLatitude Dependencies */
    typedef enum DeltaLatitude {
        DeltaLatitude_oneMicrodegreeNorth   = 10, // 表示纬度向北增加1微度的情况，对应枚举值为10，用于体现地理坐标中纬度在向北方向上极其微小的变化量，比如在高精度定位追踪等场景下会用到。
        DeltaLatitude_oneMicrodegreeSouth   = -10,// 表示纬度向南减少1微度的情况，对应枚举值为 -10，用于体现纬度在向南方向上的微小变化情况，同样适用于精确的地理位置变化分析等用途。
        DeltaLatitude_unavailable   = 131072// 表示纬度变化量不可用的情况，对应一个特定的枚举值131072作为标识，在无法获取、确定或者出现异常导致纬度变化量无法表示时，使用此值来标记相应状态。
    } e_DeltaLatitude;
 
    /* DeltaLatitude */
    typedef long DeltaLatitude_t;

    /* DeltaLongitude Dependencies */
    typedef enum DeltaLongitude {
        DeltaLongitude_oneMicrodegreeEast   = 10, // 表示经度向东增加1微度的情况，对应枚举值为10，用于体现地理坐标中经度在向东方向上极其微小的变化量，在高精度的位置追踪、地图绘制等场景下有应用价值。
        DeltaLongitude_oneMicrodegreeWest   = -10,// 表示经度向西减少1微度的情况，对应枚举值为 -10，用于体现经度在向西方向上的微小变化情况，有助于精确分析地理位置在东西方向上的变化。
        DeltaLongitude_unavailable  = 131072// 表示经度变化量不可用的情况，对应一个特定的枚举值131072作为标识，在无法准确获取或出现异常导致经度变化量无法确定时，以此值标记相应状态。
    } e_DeltaLongitude;
 
    /* DeltaLongitude */
    typedef long DeltaLongitude_t;// 为DeltaLongitude类型定义一个别名DeltaLongitude_t，方便在代码中使用，本质上是long类型，便于统一处理与经度变化量相关的数据，使得在涉及地理坐标变化的各种操作中更加便捷、规范。

    /* DeltaAltitude Dependencies */
    typedef enum DeltaAltitude {
        DeltaAltitude_oneCentimeterUp   = 1, // 表示高度向上增加1厘米的情况，对应枚举值为1，用于体现地理坐标中海拔高度在向上方向上的微小变化量，比如在一些需要精确监测高度变化的场景（如无人机飞行、地形测绘等）中会用到。
        DeltaAltitude_oneCentimeterDown = -1, // 表示高度向下减少1厘米的情况，对应枚举值为 -1，用于体现海拔高度在向下方向上的微小变化情况，同样有助于精确分析物体在垂直方向上的位置变化。
        DeltaAltitude_unavailable   = 12800// 表示高度变化量不可用的情况，对应一个特定的枚举值12800作为标识，在无法获取、确定或者出现异常导致高度变化量无法表示时，使用此值来标记相应状态。

    } e_DeltaAltitude;
 
    /* DeltaAltitude */
    typedef long DeltaAltitude_t;

    /* DeltaReferencePosition */
    typedef struct DeltaReferencePosition {
        DeltaLatitude_t  deltaLatitude; // 表示纬度方向上的变化量，其类型为DeltaLatitude_t，存储了纬度的微小增减情况或者不可用状态信息，来自之前定义的DeltaLatitude相关类型。
        DeltaLongitude_t     deltaLongitude; // 表示经度方向上的变化量，其类型为DeltaLongitude_t，记录了经度的微小变化情况或者不可用状态，对应之前定义的DeltaLongitude相关类型。
        DeltaAltitude_t  deltaAltitude;// 表示高度方向上的变化量，其类型为DeltaAltitude_t，体现了海拔高度的微小变化情况或者不可用状态，基于之前定义的DeltaAltitude相关类型。
    } DeltaReferencePosition_t;

    /* PathDeltaTime Dependencies */
    typedef enum PathDeltaTime {
        PathDeltaTime_tenMilliSecondsInPast = 1 // 表示10毫秒之前的时间点，对应枚举值为1，可用于在路径分析、轨迹追踪等场景中，比如查看车辆在10毫秒前处于什么位置、状态等情况，以辅助分析路径变化规律。
    } e_PathDeltaTime;
 
    /* PathDeltaTime */
    typedef long PathDeltaTime_t;// 为PathDeltaTime类型定义一个别名PathDeltaTime_t，方便在代码中使用，本质上是long类型，便于统一处理与路径时间变化量相关的数据，使得在涉及路径时间相关的各种操作中更加便捷、规范。

    /* PathPoint */
    typedef struct PathPoint {
        DeltaReferencePosition_t     pathPosition;// 表示该路径点在地理坐标（纬度、经度、高度）方面的变化量信息，类型为DeltaReferencePosition_t，它整合了三个方向上的位置变化情况，能精确描述路径点的位置变化。
        PathDeltaTime_t *pathDeltaTime  /* OPTIONAL */;// 表示该路径点对应的时间变化量信息，类型为PathDeltaTime_t指针，此成员是可选的（通过注释中的 OPTIONAL 标识），意味着在某些情况下可能不存在时间变化量信息，用于关联路径点与特定的时间节点，比如记录车辆经过该点的时间等情况。

    } PathPoint_t;

    /* PathHistory */
    typedef struct PathHistory {
        long NumberOfPathPoint; // 表示路径中包含的路径点的数量，使用long类型来存储，方便后续对路径点进行遍历等操作时确定循环次数等逻辑。
        std::vector<PathPoint_t> data; // 定义一个动态数组（使用std::vector），其中元素类型为PathPoint_t，用于存储各个路径点的详细信息，从而完整地记录了一段路径的历史轨迹情况。
    } PathHistory_t;
};

class CAMContainer
{
public:

    /* GenerationDeltaTime Dependencies*/
    typedef enum GenerationDeltaTime
    {
        GenerationDeltaTime_oneMilliSec = 1 // 表示1毫秒的时间间隔，对应枚举值为1，可用于设定诸如消息每隔1毫秒生成一次之类的时间规则，在涉及消息定时生成、更新等逻辑中起作用。
    } e_GenerationDeltaTime;

    /* GenerationDeltaTime */
    typedef long GenerationDeltaTime_t;// 为GenerationDeltaTime类型定义一个别名GenerationDeltaTime_t，方便在代码中统一使用，本质上是long类型，便于在程序中对生成时间间隔相关的数据进行处理和传递。

    /* BasicContainer */
    typedef struct BasicContainer
    {
        ITSContainer::StationType_t stationType;// 表示站点类型信息，其类型为ITSContainer::StationType_t（推测ITSContainer是另一个相关的命名空间或结构体等定义了StationType_t类型，此处未完整展示其定义细节），用于明确所属站点的类型，例如是车辆端还是路边单元端等情况。
        ITSContainer::ReferencePosition_t referencePosition; // 表示参考位置信息，类型为ITSContainer::ReferencePosition_t（同样依赖于外部定义的该类型），用于确定一个基本的地理位置参考，比如车辆的初始位置等情况。
    } BasicContainer_t;

    /* HighFrequencyContainer Dependencies */
    typedef enum HighFrequencyContainer_PR : long
    {
        HighFrequencyContainer_PR_NOTHING, /* No components present */// 表示高频容器中没有任何组件存在的情况，对应枚举值用于在逻辑判断中识别容器为空的状态，方便进行相应的初始化或错误处理等操作。
        HighFrequencyContainer_PR_basicVehicleContainerHighFrequency, // 表示高频容器中包含基本车辆高频信息的情况，对应枚举值用于在处理车辆相关高频数据时进行识别和相应的数据提取、处理逻辑。
        HighFrequencyContainer_PR_rsuContainerHighFrequency // 表示高频容器中包含路边单元（RSU）高频信息的情况，对应枚举值用于针对路边单元相关高频数据的处理逻辑，比如更新受保护通信区域信息等操作。
    } HighFrequencyContainer_PR;

    typedef bool OptionalStructAvailable_t;

    /* BasicVehicleContainerHighFrequency*/
    typedef struct BasicVehicleContainerHighFrequency
    {
        ITSContainer::Heading_t heading;// 表示车辆的行驶方向信息，其类型为ITSContainer::Heading_t（依赖外部定义的该类型），用于明确车辆当前的车头朝向角度等情况，有助于判断车辆的行驶轨迹方向。
        ITSContainer::Speed_t speed;// 表示车辆的速度信息，类型为ITSContainer::Speed_t，用于实时传递车辆当前的行驶速度，方便其他相关设备或系统了解车辆的运动快慢情况。
        ITSContainer::DriveDirection_t driveDirection; // 表示车辆的行驶驱动方向信息，类型为ITSContainer::DriveDirection_t，可用于区分车辆是正向行驶还是倒车等不同驱动状态，对交通管理和车辆协作场景有重要意义。
        ITSContainer::VehicleLength_t vehicleLength;// 表示车辆的长度信息，类型为ITSContainer::VehicleLength_t，用于告知其他参与合作感知的对象车辆的尺寸长度情况，在避免碰撞、道路规划等方面可作为参考依据。
        ITSContainer::VehicleWidth_t vehicleWidth; // 表示车辆的宽度信息，类型为ITSContainer::VehicleWidth_t，与车辆长度信息类似，可让外界知晓车辆的横向尺寸，有助于判断车辆在道路上的占位情况等。
        ITSContainer::LongitudinalAcceleration_t longitudinalAcceleration; // 表示车辆的纵向加速度信息，类型为ITSContainer::LongitudinalAcceleration_t，用于实时反映车辆在行驶方向上的加速或减速情况，是判断车辆行驶状态变化的重要指标。
        ITSContainer::Curvature_t curvature; // 表示车辆行驶路径的曲率信息，类型为ITSContainer::Curvature_t，可帮助了解车辆当前行驶轨迹的弯曲程度，对于预测车辆后续行驶路线等有帮助。
        ITSContainer::CurvatureCalculationMode_t   curvatureCalculationMode; // 表示车辆曲率计算模式的信息，类型为ITSContainer::CurvatureCalculationMode_t，用于说明曲率是通过何种方式计算得出的，例如是否使用了偏航率等信息来计算，便于对曲率数据的准确性和可靠性进行评估。
        ITSContainer::YawRate_t    yawRate; // 表示车辆的偏航率信息，类型为ITSContainer::YawRate_t，偏航率对于分析车辆的转向特性、行驶稳定性等方面具有重要作用，能更细致地描述车辆动态行为。
        
        OptionalStructAvailable_t accelerationControlAvailable; // 表示车辆加速度控制相关信息是否可用的标识，类型为OptionalStructAvailable_t（即bool类型），用于判断后续的accelerationControl结构体是否包含有效可用的数据，避免对无效数据进行操作。
        ITSContainer::AccelerationControl_t   accelerationControl    /* OPTIONAL */;  // 表示车辆的加速度控制信息，类型为ITSContainer::AccelerationControl_t，此成员是可选的（通过注释中的 OPTIONAL 标识），意味着在某些情况下可能不存在该信息，它可用于描述车辆当前是通过刹车、油门等哪种控制方式来改变速度等情况。
        
        OptionalStructAvailable_t lanePositionAvailable; // 表示车辆车道位置相关信息是否可用的标识，类型为OptionalStructAvailable_t，用于判断后续的lanePosition结构体是否有有效数据，以便正确处理车道位置相关逻辑。
        ITSContainer::LanePosition_t lanePosition   /* OPTIONAL */; // 表示车辆所在车道位置信息，类型为ITSContainer::LanePosition_t，此成员是可选的，用于明确车辆当前处于哪条车道或者是否偏离道路等位置情况，对交通流分析和车辆协作很重要。
        
        OptionalStructAvailable_t steeringWheelAngleAvailable;// 表示车辆方向盘角度相关信息是否可用的标识，类型为OptionalStructAvailable_t，用于判断后续的steeringWheelAngle结构体是否包含有效可用的数据。
        ITSContainer::SteeringWheelAngle_t   steeringWheelAngle /* OPTIONAL */; // 表示车辆方向盘角度信息，类型为ITSContainer::SteeringWheelAngle_t，此成员是可选的，用于了解车辆方向盘的转动情况，进而推断驾驶员的操作意图等情况。

        OptionalStructAvailable_t lateralAccelerationAvailable; // 表示车辆横向加速度相关信息是否可用的标识，类型为OptionalStructAvailable_t，用于判断后续的lateralAcceleration结构体是否有有效数据。
        ITSContainer::LateralAcceleration_t  lateralAcceleration    /* OPTIONAL */;// 表示车辆横向加速度信息，类型为ITSContainer::LateralAcceleration_t，此成员是可选的，用于描述车辆在横向方向上的加速情况，对分析车辆转弯、变道等横向运动特性有帮助。

        OptionalStructAvailable_t verticalAccelerationAvailable; // 表示车辆垂直加速度相关信息是否可用的标识，类型为OptionalStructAvailable_t，用于判断后续的verticalAcceleration结构体是否包含有效可用的数据。
        ITSContainer::VerticalAcceleration_t verticalAcceleration   /* OPTIONAL */; // 表示车辆垂直加速度信息，类型为ITSContainer::VerticalAcceleration_t，此成员是可选的，用于反映车辆在垂直方向上的加速情况，比如车辆经过颠簸路面等产生的上下方向的加速度变化。

        OptionalStructAvailable_t performanceClassAvailable; // 表示车辆性能等级相关信息是否可用的标识，类型为OptionalStructAvailable_t，用于判断后续的performanceClass结构体是否有有效数据。
        ITSContainer::PerformanceClass_t performanceClass   /* OPTIONAL */; // 表示车辆性能等级信息，类型为ITSContainer::PerformanceClass_t，此成员是可选的，用于区分不同性能级别的车辆，例如高性能车与普通车辆等，在一些特定应用场景中可能会根据性能等级进行不同的处理逻辑。

        OptionalStructAvailable_t cenDsrcTollingZoneAvailable;// 表示车辆所在的基于DSRC的收费区域相关信息是否可用的标识，类型为OptionalStructAvailable_t，用于判断后续的cenDsrcTollingZone结构体是否包含有效可用的数据
        ITSContainer::CenDsrcTollingZone_t   cenDsrcTollingZone /* OPTIONAL */; // 表示车辆所在的基于DSRC的收费区域信息，类型为ITSContainer::CenDsrcTollingZone_t，此成员是可选的，用于确定车辆是否处于特定的收费区域以及该区域的相关详细信息等情况。
        
    } BasicVehicleContainerHighFrequency_t;

    /* RsuContainerHighFrequency */
    typedef struct RSUContainerHighFrequency
    {
        ITSContainer::ProtectedCommunicationZonesRSU_t protectedCommunicationZonesRSU; // 表示路边单元的受保护通信区域相关信息，类型为ITSContainer::ProtectedCommunicationZonesRSU_t，包含了区域数量、各个区域的详细信息等内容，是路边单元在通信等场景下对外展示自身关键状态的一部分。
    } RSUContainerHighFrequency_t;

    /* HighFrequencyContainer */
    typedef struct HighFrequencyContainer
    {
        HighFrequencyContainer_PR present; // 表示当前高频容器的呈现形式，通过HighFrequencyContainer_PR枚举类型来标识，用于判断当前容器中包含的是车辆高频信息还是路边单元高频信息或者为空等情况，进而进行相应的数据处理逻辑分支选择。

        BasicVehicleContainerHighFrequency_t basicVehicleContainerHighFrequency;// 表示基本车辆的高频信息结构体，包含了车辆众多的高频状态数据，用于在present标识为车辆高频信息情况时，提取和处理相关车辆状态数据。
        RSUContainerHighFrequency_t rsuContainerHighFrequency;// 表示路边单元的高频信息结构体，包含了路边单元的相关高频数据，用于在present标识为路边单元高频信息情况时，操作和更新路边单元相关的状态信息。

    } HighFrequencyContainer_t;

    /* Dependencies */
    typedef enum LowFrequencyContainer_PR : long
    {
        LowFrequencyContainer_PR_NOTHING,   /* No components present */// 表示低频容器中没有任何组件存在的情况，对应枚举值用于在逻辑判断中识别容器为空的状态，便于进行初始化或错误处理等操作。
        LowFrequencyContainer_PR_basicVehicleContainerLowFrequency,// 表示低频容器中包含基本车辆低频信息的情况，对应枚举值用于在处理车辆低频数据时进行识别和相应的数据提取、处理逻辑，当前只定义了这一种包含车辆低频信息的情况，后续可能会根据需求扩展更多选项。
        /* Extensions may appear below */
     
    } LowFrequencyContainer_PR;

    /* BasicVehicleContainerLowFrequency */
    typedef struct BasicVehicleContainerLowFrequency {
        ITSContainer::VehicleRole_t    vehicleRole; // 表示车辆的角色信息，类型为ITSContainer::VehicleRole_t，用于明确车辆在不同场景下所扮演的角色，例如是公共交通车辆、救援车辆还是普通私家车等，有助于交通管理和协同作业等场景下对车辆进行分类处理。
        ITSContainer::ExteriorLights_t     exteriorLights; // 表示车辆外部灯光的状态信息，类型为ITSContainer::ExteriorLights_t，用于实时传递车辆各个外部灯光（如近光灯、转向灯等）的开启或关闭情况，方便其他交通参与者知晓车辆的灯光提示信息。
        ITSContainer::PathHistory_t    pathHistory; // 表示车辆的路径历史信息，类型为ITSContainer::PathHistory_t，包含了车辆过去一段时间内经过的多个路径点信息，可用于分析车辆的行驶轨迹、出行习惯等情况。
    } BasicVehicleContainerLowFrequency_t;

    /* LowFrequencyContainer */
    typedef struct LowFrequencyContainer
    {
        LowFrequencyContainer_PR present;
        // Since only option is available
        BasicVehicleContainerLowFrequency_t basicVehicleContainerLowFrequency;

    } LowFrequencyContainer_t;

    /* CamParameters */
    typedef struct CamParameters
    {
        BasicContainer_t basicContainer;
        HighFrequencyContainer_t highFrequencyContainer;
        LowFrequencyContainer_t lowFrequencyContainer; /* OPTIONAL */
        // Optional TODO: SpecialVehicleContainer *specialVehicleContainer
    } CamParameters_t;

    /* CoopAwareness*/
    typedef struct CoopAwareness 
    {
        GenerationDeltaTime_t generationDeltaTime;
        CamParameters_t camParameters;
    } CoopAwareness_t;


};

    /* CoopAwareness */
    typedef struct CAM
    {
        ITSContainer::ItsPduHeader_t header;
        CAMContainer::CoopAwareness_t cam;
    } CAM_t;


    typedef struct CustomV2XM
    {
        ITSContainer::ItsPduHeader_t header;
        char message[100];
    } CustomV2XM_t;
