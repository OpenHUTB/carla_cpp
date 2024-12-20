// Copyright (c) 2024 Institut fuer Technik der Informationsverarbeitung (ITIV) at the 
// Karlsruhe Institute of Technology
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/sensor/data/V2XData.h>
#include <carla/sensor/data/LibITS.h>
#include <iostream>
#include <ostream>
#include <vector>

/**********************************************************************************************/
// 将车联网（V2X）传感器发出的协同感知消息（CAM message）转换为Python字典的函数。
std::string GetStationTypeString(const ITSContainer::StationType_t stationType)
{
    switch (stationType)
    {
    case ITSContainer::StationType_unknown:
        return "Uunknown";
    case ITSContainer::StationType_pedestrian:
        return "Pedestrian";
    case ITSContainer::StationType_cyclist:
        return "Cyclist";
    case ITSContainer::StationType_moped:
        return "Moped";
    case ITSContainer::StationType_motorcycle:
        return "Motorcycle";
    case ITSContainer::StationType_passengerCar:
        return "Passenger Car";
    case ITSContainer::StationType_bus:
        return "Bus";
    case ITSContainer::StationType_lightTruck:
        return "Light Truck";
    case ITSContainer::StationType_heavyTruck:
        return "HeavyTruck";
    case ITSContainer::StationType_trailer:
        return "Trailer";
    case ITSContainer::StationType_specialVehicles:
        return "Special Vehicle";
    case ITSContainer::StationType_tram:
        return "Tram";
    case ITSContainer::StationType_roadSideUnit:
        return "Road Side Unit";
    default:
        return "Unknown";
    }
}
// 根据半轴长度枚举值返回对应的字符串描述
std::string GetSemiConfidenceString(const long confidence)
{
  switch(confidence)
  {
    case ITSContainer::SemiAxisLength_oneCentimeter:
      return "Semi Axis Length One Centimeter";
    case ITSContainer::SemiAxisLength_outOfRange:
      return "Semi Axis Length Out Of Range";
    default:
      return "Semi Axis Length Unavailable";
  }
}
// 根据航向值枚举值返回对应的字符串描述
std::string GetSemiOrientationString(const long orientation)
{
  switch(orientation)
  {
    case ITSContainer::HeadingValue_wgs84North:
      return "Heading wgs84 North";
    case ITSContainer::HeadingValue_wgs84East:
      return "Heading wgs84 East";
    case ITSContainer::HeadingValue_wgs84South:
      return "Heading wgs84 South";
    case ITSContainer::HeadingValue_wgs84West:
      return "Heading wgs84 West";
    default:
      return "Heading Unavailable";
  }
}
// 根据海拔置信度枚举值返回对应的字符串描述
// 定义一个函数，用于将ITSContainer中的AltitudeConfidence枚举值转换为对应的字符串表示
std::string GetAltitudeConfidenceString(ITSContainer::AltitudeConfidence_t altitudeConfidence)
{
// 使用switch语句根据altitudeConfidence的值选择对应的字符串
  switch(altitudeConfidence)
  {
// 对于每个AltitudeConfidence枚举值，都返回一个描述性的字符串
    case ITSContainer::AltitudeConfidence_alt_000_01:
      return "AltitudeConfidence_alt_000_01";                // 1%的置信度
    case ITSContainer::AltitudeConfidence_alt_000_02:
      return "AltitudeConfidence_alt_000_02";                 // 2%的置信度
    case ITSContainer::AltitudeConfidence_alt_000_05:
      return "AltitudeConfidence_alt_000_05";                   // 5%的置信度
    case ITSContainer::AltitudeConfidence_alt_000_10:
      return "AltitudeConfidence_alt_000_10";                    // 10%的置信度
    case ITSContainer::AltitudeConfidence_alt_000_20:
      return "AltitudeConfidence_alt_000_20";                 // 20%的置信度
    case ITSContainer::AltitudeConfidence_alt_000_50:
      return "AltitudeConfidence_alt_000_50";                   // 50%的置信度
    case ITSContainer::AltitudeConfidence_alt_001_00:
      return "AltitudeConfidence_alt_001_00";                    // 100%的置信度，即1.00
    case ITSContainer::AltitudeConfidence_alt_002_00:
      return "AltitudeConfidence_alt_002_00";                   / 200%的置信度
    case ITSContainer::AltitudeConfidence_alt_005_00:
      return "AltitudeConfidence_alt_005_00";                     // 500%的置信度
    case ITSContainer::AltitudeConfidence_alt_010_00:
      return "AltitudeConfidence_alt_010_00";                  // 1000%的置信度
    case ITSContainer::AltitudeConfidence_alt_020_00:
      return "AltitudeConfidence_alt_020_00";               // 2000%的置信度
    case ITSContainer::AltitudeConfidence_alt_050_00:
      return "AltitudeConfidence_alt_050_00";                    // 5000%的置信度
    case ITSContainer::AltitudeConfidence_alt_100_00:
      return "AltitudeConfidence_alt_100_00";                  // 10000%的置信度
    case ITSContainer::AltitudeConfidence_alt_200_00:
      return "AltitudeConfidence_alt_200_00";               // 20000%的置信度
    case ITSContainer::AltitudeConfidence_outOfRange:
      return "AltitudeConfidence_alt_outOfRange";           // 表示高度置信度值超出范围
    default:
      return "AltitudeConfidence_unavailable";          // 表示高度置信度信息不可用
  }
}
// 获取参考容器信息并以字典形式返回
static boost::python::dict GetReferenceContainer(const CAM_t message)
{
  boost::python::dict ReferencePosition;
  ITSContainer::ReferencePosition_t ref = message.cam.camParameters.basicContainer.referencePosition;
// 设置参考位置的纬度信息
  ReferencePosition["Latitude"] = ref.latitude;
// 设置参考位置的经度信息
  ReferencePosition["Longitude"] = ref.longitude;
  boost::python::dict PosConfidence;
// 获取并设置半长轴置信度对应的字符串描述
  PosConfidence["Semi Major Confidence"] = GetSemiConfidenceString(ref.positionConfidenceEllipse.semiMajorConfidence);
// 获取并设置半短轴置信度对应的字符串描述
  PosConfidence["Semi Minor Confidence"] = GetSemiConfidenceString(ref.positionConfidenceEllipse.semiMinorConfidence);
// 获取并设置半长轴方向对应的字符串描述
  PosConfidence["Semi Major Orientation"] = GetSemiOrientationString(ref.positionConfidenceEllipse.semiMajorOrientation);
// 将位置置信度信息添加到参考位置字典中
  ReferencePosition["Position Confidence Eliipse"] = PosConfidence;
  boost::python::dict Altitude;
// 设置海拔值
  Altitude["Altitude Value"] = ref.altitude.altitudeValue;
// 获取并设置海拔置信度对应的字符串描述
  Altitude["Altitude Confidence"] = GetAltitudeConfidenceString(ref.altitude.altitudeConfidence);
  return ReferencePosition;
}
// 获取基本容器信息并以字典形式返回
static boost::python::dict GetBasicContainer(const CAM_t message)
{
    boost::python::dict BasicContainer;
// 获取并设置站点类型对应的字符串描述
    BasicContainer["Station Type"] = GetStationTypeString(message.cam.camParameters.basicContainer.stationType);
// 获取参考容器信息并添加到基本容器字典中
    BasicContainer["Reference Position"] = GetReferenceContainer(message);
    return BasicContainer;
}
// 根据航向置信度枚举值返回对应的字符串描述
std::string GetHeadingConfidenceString(ITSContainer::HeadingConfidence_t confidence)
{
    switch (confidence)
    {
    case ITSContainer::HeadingConfidence_equalOrWithinZeroPointOneDegree:
        return "Equal or With in Zero Point One Degree";
    case ITSContainer::HeadingConfidence_equalOrWithinOneDegree:
        return "Equal or With in One Degree";
    case ITSContainer::HeadingConfidence_outOfRange:
        return "Out of Range";
    default:
        return "Unavailable";
    }
}
// 根据速度置信度枚举值返回对应的字符串描述
std::string GetSpeedConfidenceString(ITSContainer::SpeedConfidence_t confidence)
{
    switch (confidence)
    {
    case ITSContainer::SpeedConfidence_equalOrWithInOneCentimerterPerSec:
        return "Equal or With in One Centimeter per Sec";
    case ITSContainer::SpeedConfidence_equalOrWithinOneMeterPerSec:
        return "Equal or With in One Meter per Sec";
    case ITSContainer::SpeedConfidence_outOfRange:
        return "Out of Range";
    default:
        return "Unavailable";
    }
}
// 根据车辆长度置信度指示枚举值返回对应的字符串描述
std::string GetVehicleLengthConfidenceString(ITSContainer::VehicleLengthConfidenceIndication_t confidence)
{
    switch (confidence)
    {
    case ITSContainer::VehicleLengthConfidenceIndication_noTrailerPresent:
        return "No Trailer Present";
    case ITSContainer::VehicleLengthConfidenceIndication_trailerPresentWithKnownLength:
        return "Trailer Present With Known Length";
    case ITSContainer::VehicleLengthConfidenceIndication_trailerPresentWithUnknownLength:
        return "Trailer Present With Unknown Length";
    case ITSContainer::VehicleLengthConfidenceIndication_trailerPresenceIsUnknown:
        return "Trailer Presence Is Unknown";
    default:
        return "Unavailable";
    }
}
// 根据加速度置信度枚举值返回对应的字符串描述
std::string GetAccelerationConfidenceString(ITSContainer::AccelerationConfidence_t confidence)
{
    switch (confidence)
    {
    case ITSContainer::AccelerationConfindence_pointOneMeterPerSecSquared:
        return "Point One Meter Per Sec Squared";
    case ITSContainer::AccelerationConfidence_outOfRange:
        return "Out Of Range";
    default:
        return "Unavailable";
    }
}
// 根据曲率置信度枚举值返回对应的字符串描述
std::string GetCurvatureConfidenceString(ITSContainer::CurvatureConfidence_t confidence)
{
    switch (confidence)
    {
    case ITSContainer::CurvatureConfidence_onePerMeter_0_00002:
        return "One Per Meter 0_00002";
    case ITSContainer::CurvatureConfidence_onePerMeter_0_0001:
        return "One Per Meter 0_0001";
    case ITSContainer::CurvatureConfidence_onePerMeter_0_0005:
        return "One Per Meter 0_0005";
    case ITSContainer::CurvatureConfidence_onePerMeter_0_002:
        return "One Per Meter 0_002";
    case ITSContainer::CurvatureConfidence_onePerMeter_0_01:
        return "One Per Meter 0_01";
    case ITSContainer::CurvatureConfidence_onePerMeter_0_1:
        return "One Per Meter 0_1";
    case ITSContainer::CurvatureConfidence_outOfRange:
        return "Out Of Range";
    default:
        return "Unavailable";
    }
}
// 根据偏航率置信度枚举值返回对应的字符串描述
std::string GetYawRateConfidenceString(ITSContainer::YawRateConfidence_t confidence)
{
    switch (confidence)
    {
    case ITSContainer::YawRateConfidence_degSec_000_01:
        return "degSec 000_01";
    case ITSContainer::YawRateConfidence_degSec_000_05:
        return "degSec 000_05";
    case ITSContainer::YawRateConfidence_degSec_000_10:
        return "degSec 000_10";
    case ITSContainer::YawRateConfidence_degSec_001_00:
        return "degSec 001_00";
    case ITSContainer::YawRateConfidence_degSec_005_00:
        return "degSec 005_00";
    case ITSContainer::YawRateConfidence_degSec_010_00:
        return "degSec 010_00";
    case ITSContainer::YawRateConfidence_degSec_100_00:
        return "degSec 100_00";
    case ITSContainer::YawRateConfidence_outOfRange:
        return "Out Of Range";
    default:
        return "Unavailable";
    }
}
// 根据方向盘角度置信度枚举值返回对应的字符串描述

std::string GetSteeringWheelConfidenceString(ITSContainer::SteeringWheelAngleConfidence_t confidence)
{
// 根据传入的置信度值进行不同情况的处理
    switch (confidence)
    {
    case ITSContainer::SteeringWheelAngleConfidence_equalOrWithinOnePointFiveDegree:
// 如果置信度在特定范围内，返回相应描述字符串
        return "Equal or With in 1.5 degree";
    case ITSContainer::SteeringWheelAngleConfidence_outOfRange:
// 如果超出范围，返回相应描述字符串
        return "Out of Range";
    default:
// 其他情况返回表示不可用的字符串
        return "Unavailable";
    }
}
// 获取基本车辆容器高频信息并以字典形式返回
static boost::python::dict GetBVCHighFrequency(const CAM_t message)
{
    boost::python::dict BVCHighFrequency;
// 获取基本车辆容器高频部分的数据结构
    CAMContainer::BasicVehicleContainerHighFrequency_t bvchf = message.cam.camParameters.highFrequencyContainer.basicVehicleContainerHighFrequency;
// 处理航向相关信息
    boost::python::dict HeadingValueConfidence;
    HeadingValueConfidence["Value"] = bvchf.heading.headingValue;
// 获取并设置航向置信度对应的字符串描述
    HeadingValueConfidence["Confidence"] = GetHeadingConfidenceString(bvchf.heading.headingConfidence);
    BVCHighFrequency["Heading"] = HeadingValueConfidence;
// 处理速度相关信息
    boost::python::dict SpeedValueConfidence;
    SpeedValueConfidence["Value"] = bvchf.speed.speedValue;
// 获取并设置速度置信度对应的字符串描述
    SpeedValueConfidence["Confidence"] = GetSpeedConfidenceString(bvchf.speed.speedConfidence);
    BVCHighFrequency["Speed"] = SpeedValueConfidence;
// 设置行驶方向信息
    BVCHighFrequency["Drive Direction"] = bvchf.driveDirection;
// 处理车辆长度相关信息
    boost::python::dict VehicleLengthValueConfidence;
    VehicleLengthValueConfidence["Value"] = bvchf.vehicleLength.vehicleLengthValue;
// 获取并设置车辆长度置信度对应的字符串描述
    VehicleLengthValueConfidence["Confidence"] = GetVehicleLengthConfidenceString(bvchf.vehicleLength.vehicleLengthConfidenceIndication);
    BVCHighFrequency["Vehicle Length"] = VehicleLengthValueConfidence;
// 设置车辆宽度信息
    BVCHighFrequency["Vehicle Width"] = bvchf.vehicleWidth;
// 处理纵向加速度相关信息
    boost::python::dict LongitudinalAccelerationValueConfidence;
    LongitudinalAccelerationValueConfidence["Value"] = bvchf.longitudinalAcceleration.longitudinalAccelerationValue;
// 获取并设置纵向加速度置信度对应的字符串描述
    LongitudinalAccelerationValueConfidence["Confidence"] = GetAccelerationConfidenceString(bvchf.longitudinalAcceleration.longitudinalAccelerationConfidence);
    BVCHighFrequency["Longitudinal Acceleration"] = LongitudinalAccelerationValueConfidence;
// 处理曲率相关信息
    boost::python::dict CurvatureValueConfidence;
    CurvatureValueConfidence["Value"] = bvchf.curvature.curvatureValue;
// 获取并设置曲率置信度对应的字符串描述
    CurvatureValueConfidence["Confidence"] = GetCurvatureConfidenceString(bvchf.curvature.curvatureConfidence);
    BVCHighFrequency["Curvature"] = CurvatureValueConfidence;
    BVCHighFrequency["Curvature Calculation Mode"] = bvchf.curvatureCalculationMode;
// 处理偏航率相关信息
    boost::python::dict YawValueConfidence;
    YawValueConfidence["Value"] = bvchf.yawRate.yawRateValue;
// 获取并设置偏航率置信度对应的字符串描述
    YawValueConfidence["Confidence"] = GetYawRateConfidenceString(bvchf.yawRate.yawRateConfidence);
    BVCHighFrequency["Yaw Rate"] = YawValueConfidence;
// 处理加速度控制相关信息（如果可用）
    boost::python::dict ValueConfidence;
    if (bvchf.accelerationControlAvailable)
    {
        BVCHighFrequency["Acceleration Control"] = bvchf.accelerationControl;
    }
    else
    {
        BVCHighFrequency["Acceleration Control"] = boost::python::object();
    }
// 处理车道位置相关信息（如果可用）
    if (bvchf.lanePositionAvailable)
    {
        BVCHighFrequency["Lane Position"] = bvchf.lanePosition;
    }
    else
    {
        BVCHighFrequency["Lane Position"] = boost::python::object();
    }
// 处理方向盘角度相关信息（如果可用）
    if (bvchf.steeringWheelAngleAvailable)
    {
        boost::python::dict ValueConfidence;
        ValueConfidence["Value"] = bvchf.steeringWheelAngle.steeringWheelAngleValue;
// 获取并设置方向盘角度置信度对应的字符串描述
        ValueConfidence["Confidence"] = GetSteeringWheelConfidenceString(bvchf.steeringWheelAngle.steeringWheelAngleConfidence);
        BVCHighFrequency["Steering Wheel Angle"] = ValueConfidence;
    }
    else
    {
        BVCHighFrequency["Steering Wheel Angle"] = boost::python::object();
    }
// 处理横向加速度相关信息（如果可用）
    if (bvchf.lateralAccelerationAvailable)
    {
        boost::python::dict ValueConfidence;
        ValueConfidence["Value"] = bvchf.lateralAcceleration.lateralAccelerationValue;
// 获取并设置横向加速度置信度对应的字符串描述
        ValueConfidence["Confidence"] = GetAccelerationConfidenceString(bvchf.lateralAcceleration.lateralAccelerationConfidence);
        BVCHighFrequency["Lateral Acceleration"] = ValueConfidence;
    }
    else
    {
        BVCHighFrequency["Lateral Acceleration"] = boost::python::object();
    }
// 处理垂直加速度相关信息（如果可用）
    if (bvchf.verticalAccelerationAvailable)
    {
        boost::python::dict ValueConfidence;
        ValueConfidence["Value"] = bvchf.verticalAcceleration.verticalAccelerationValue;
// 获取并设置垂直加速度置信度对应的字符串描述
        ValueConfidence["Confidence"] = GetAccelerationConfidenceString(bvchf.verticalAcceleration.verticalAccelerationConfidence);
        BVCHighFrequency["Vertical Acceleration"] = ValueConfidence;
    }
    else
    {
        BVCHighFrequency["Vertical Acceleration"] = boost::python::object();
    }
// 处理性能等级相关信息（如果可用）
    if (bvchf.performanceClassAvailable)
    {
        BVCHighFrequency["Performance class"] = bvchf.performanceClass;
    }
    else
    {
        BVCHighFrequency["Performance class"] = boost::python::object();
    }
// 处理DSRC收费区域相关信息（如果可用）
    if (bvchf.cenDsrcTollingZoneAvailable)
    {
        boost::python::dict ValueConfidence;
        ValueConfidence["Protected Zone Latitude"] = bvchf.cenDsrcTollingZone.protectedZoneLatitude;
        ValueConfidence["Protected Zone Longitude"] = bvchf.cenDsrcTollingZone.protectedZoneLongitude;
        if (bvchf.cenDsrcTollingZone.cenDsrcTollingZoneIDAvailable)
        {
            ValueConfidence["Cen DSRC Tolling Zone ID"] = bvchf.cenDsrcTollingZone.cenDsrcTollingZoneID;
        }
        BVCHighFrequency["Cen DSRC Tolling Zone"] = ValueConfidence;
    }
    else
    {
        BVCHighFrequency["Cen DSRC Tolling Zone"] = boost::python::object();
    }

    return BVCHighFrequency;
}
// 获取受保护通信区域信息列表并以列表形式返回
static boost::python::list GetProtectedCommunicationZone(const CAMContainer::RSUContainerHighFrequency_t rsuMessage)
{
    boost::python::list PCZlist;
// 遍历受保护通信区域的数据列表
    for (ITSContainer::ProtectedCommunicationZone_t data : rsuMessage.protectedCommunicationZonesRSU.list)
    {
        boost::python::dict PCZDict;
// 设置受保护区域类型信息
        PCZDict["Protected Zone Type"] = data.protectedZoneType;
// 如果有效期时间可用，设置该信息
        if (data.expiryTimeAvailable)
        {
            PCZDict["Expiry Time"] = data.expiryTime;
        }
// 设置受保护区域的纬度信息
        PCZDict["Protected Zone Latitude"] = data.protectedZoneLatitude;
// 设置受保护区域的经度信息
        PCZDict["Protected Zone Longitude"] = data.protectedZoneLongitude;
// 如果受保护区域ID可用，设置该信息
        if (data.protectedZoneIDAvailable)
        {
            PCZDict["Protected Zone ID"] = data.protectedZoneID;
        }
// 如果受保护区域半径可用，设置该信息
        if (data.protectedZoneRadiusAvailable)
        {
            PCZDict["Protected Zone Radius"] = data.protectedZoneRadius;
        }
// 将当前受保护区域的字典信息添加到列表中
        PCZlist.append(PCZDict);
    }
    return PCZlist;
}
// 获取RSU高频信息并以字典形式返回
static boost::python::dict GetRSUHighFrequency(const CAM_t message)
{
    boost::python::dict RSU;
// 获取RSU容器高频部分的数据结构
    CAMContainer::RSUContainerHighFrequency_t rsu = message.cam.camParameters.highFrequencyContainer.rsuContainerHighFrequency;
// 获取并设置受保护通信区域信息列表
    RSU["Protected Communication Zone"] = GetProtectedCommunicationZone(rsu);
    return RSU;
}
// 获取高频容器信息并以字典形式返回
static boost::python::dict GetHighFrequencyContainer(const CAM_t message)
{
    boost::python::dict HFC;
// 获取高频容器的数据结构
    CAMContainer::HighFrequencyContainer_t hfc = message.cam.camParameters.highFrequencyContainer;
    switch (hfc.present)
    {
    case CAMContainer::HighFrequencyContainer_PR_basicVehicleContainerHighFrequency:
        HFC["High Frequency Container Present"] = "Basic Vehicle Container High Frequency";
// 获取并设置基本车辆容器高频信息
        HFC["Basic Vehicle Container High Frequency"] = GetBVCHighFrequency(message);
        break;
    case CAMContainer::HighFrequencyContainer_PR_rsuContainerHighFrequency:
        HFC["High Frequency Container Present"] = "RSU Container High Frequency";
// 获取并设置RSU容器高频信息
        HFC["RSU Container High Frequency"] = GetRSUHighFrequency(message);
        break;
    default:
        HFC["High Frequency Container Present"] = "No container present";
    }
    return HFC;
}
// 根据车辆角色枚举值返回对应的字符串描述
std::string GetVehicleRoleString(ITSContainer::VehicleRole_t vehicleRole)
{
    switch (vehicleRole)
    {
    case ITSContainer::VehicleRole_publicTransport:
        return "Public Transport";
    case ITSContainer::VehicleRole_specialTransport:
        return "Special Transport";
    case ITSContainer::VehicleRole_dangerousGoods:
        return "Dangerous Goods";
    case ITSContainer::VehicleRole_roadWork:
        return "Road Work";
    case ITSContainer::VehicleRole_rescue:
        return "Rescue";
    case ITSContainer::VehicleRole_emergency:
        return "Emergency";
    case ITSContainer::VehicleRole_safetyCar:
        return "Safety Car";
    case ITSContainer::VehicleRole_agriculture:
        return "Agriculture";
    case ITSContainer::VehicleRole_commercial:
        return "Commercial";
    case ITSContainer::VehicleRole_military:
        return "Military";
    case ITSContainer::VehicleRole_roadOperator:
        return "Road Operator";
    case ITSContainer::VehicleRole_taxi:
        return "Taxi";
    case ITSContainer::VehicleRole_reserved1:
        return "Reserved 1";
    case ITSContainer::VehicleRole_reserved2:
        return "Reserved 2";
    case ITSContainer::VehicleRole_reserved3:
        return "Reserved 3";
    default:
        return "Default";
    }
}
// 获取路径历史信息列表并以列表形式返回
boost::python::list GetPathHistory(const ITSContainer::PathHistory_t pathHistory)
{

    boost::python::list PathHistoryList;
// 遍历路径历史数据中的每个路径点
    for (ITSContainer::PathPoint_t pathPoint : pathHistory.data)
    {
        boost::python::dict PathHistory;
// 设置路径点的纬度变化信息
        PathHistory["Delta Latitude"] = pathPoint.pathPosition.deltaLatitude;
// 设置路径点的经度变化信息
        PathHistory["Delta Longitude"] = pathPoint.pathPosition.deltaLongitude;
// 设置路径点的海拔变化信息
        PathHistory["Delta Altitude"] = pathPoint.pathPosition.deltaAltitude;
        if (pathPoint.pathDeltaTime != nullptr)
        {
            PathHistory["Delta Time"] = *pathPoint.pathDeltaTime;
        }
        else
        {
            PathHistory["Delta Time"] = boost::python::object();
        }
// 将当前路径点的字典信息添加到列表中
        PathHistoryList.append(PathHistory);
    }
    return PathHistoryList;
}
// 获取基本车辆容器低频信息并以字典形式返回
boost::python::dict GetBVCLowFrequency(const CAMContainer::BasicVehicleContainerLowFrequency_t bvc)
{
    boost::python::dict BVC;
// 获取并设置车辆角色对应的字符串描述
    BVC["Vehicle Role"] = GetVehicleRoleString(bvc.vehicleRole);
// 设置车辆外部灯光信息
    BVC["Exterior Light"] = bvc.exteriorLights;
    if (bvc.pathHistory.NumberOfPathPoint != 0)
    {
// 如果路径历史点数量不为0，获取并设置路径历史信息
        BVC["Path History"] = GetPathHistory(bvc.pathHistory);
    }
    else
    {
        BVC["Path History"] = boost::python::object();
    }
    return BVC;
}
// 获取低频容器信息并以字典形式返回
boost::python::dict GetLowFrequencyContainer(const CAM_t message)
{
    boost::python::dict LFC = boost::python::dict();
// 获取低频容器的数据结构
    CAMContainer::LowFrequencyContainer_t lfc = message.cam.camParameters.lowFrequencyContainer;
    switch (lfc.present)
    {
    case CAMContainer::LowFrequencyContainer_PR_basicVehicleContainerLowFrequency:
        LFC["Low Frequency Container Present"] = "Basic Vehicle Container Low Frequency";
// 获取并设置基本车辆容器低频信息
        LFC["Basic Vehicle Low Frequency"] = GetBVCLowFrequency(lfc.basicVehicleContainerLowFrequency);
        break;
    default:
        LFC["Low Frequency Container Present"] = "No Container Present";
        break;
    }
    return LFC;
}
// 获取CAM参数信息并以字典形式返回
static boost::python::dict GetCAMParameters(const CAM_t message)
{
    boost::python::dict CAMParams;
    try
    {
// 获取并设置基本容器信息
        CAMParams["Basic Container"] = GetBasicContainer(message);
// 获取并设置高频容器信息
        CAMParams["High Frequency Container"] = GetHighFrequencyContainer(message);
// 获取并设置低频容器信息
        CAMParams["Low Frequency Container"] = GetLowFrequencyContainer(message);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
    return CAMParams;
}
// 获取协同感知信息并以字典形式返回
static boost::python::dict GetCoopAwarness(const CAM_t message)
{

    boost::python::dict Coop;
// 获取协同感知的数据结构
    CAMContainer::CoopAwareness_t coop = message.cam;
// 设置生成时间间隔信息
    Coop["Generation Delta Time"] = coop.generationDeltaTime;
// 获取并设置CAM参数信息
    Coop["CAM Parameters"] = GetCAMParameters(message);
    return Coop;
}
// 根据消息ID枚举值返回对应的字符串描述
std::string GetMessageIDString(const long messageId)
{
    switch (messageId)
    {
    case ITSContainer::messageID_custom:
        return "CUSTOM";
    case ITSContainer::messageID_denm:
        return "DENM";
    case ITSContainer::messageID_cam:
        return "CAM";
    case ITSContainer::messageID_poi:
        return "POI";
    case ITSContainer::messageID_spat:
        return "SPAT";
    case ITSContainer::messageID_map:
        return "MAP";
    case ITSContainer::messageID_ivi:
        return "IVI";
    case ITSContainer::messageID_ev_rsr:
        return "EV_RSR";
    default:
        return "Not Found";
    }
}
 // 获取消息头部信息并以字典形式返回
// 传入参数为包含消息头部相关信息的结构体ITSContainer::ItsPduHeader_t
static boost::python::dict GetMessageHeader(const ITSContainer::ItsPduHeader_t header)
{
// 创建一个用于存储消息头部信息的字典
    boost::python::dict Header;
// 将协议版本信息存入字典，键为"Protocol Version"，值为传入头部结构体中的协议版本值
    Header["Protocol Version"] = header.protocolVersion;
// 获取并将消息ID对应的字符串描述存入字典，键为"Message ID"
    // 通过调用GetMessageIDString函数将消息ID枚举值转换为对应的字符串
    Header["Message ID"] = GetMessageIDString(header.messageID);
// 将站点ID信息存入字典，键为"Station ID"，值为传入头部结构体中的站点ID值
    Header["Station ID"] = header.stationID;
// 返回存储了消息头部信息的字典
    return Header;
}
// 获取CAM消息相关信息并以字典形式返回
// 传入参数为包含CAM消息数据的carla::sensor::data::CAMData结构体
boost::python::dict GetCAMMessage(const carla::sensor::data::CAMData message)
{
// 创建一个用于存储CAM消息信息的字典
    boost::python::dict CAM;
// 从传入的消息结构体中获取CAM消息的具体内容，存储在CAM_t类型的变量中
    CAM_t CAMMessage = message.Message;
// 获取消息头部信息并将其存入CAM字典中，键为"Header"
// 通过调用GetMessageHeader函数获取消息头部信息
    CAM["Header"] = GetMessageHeader(CAMMessage.header);
// 获取协同感知相关信息（通过调用GetCoopAwarness函数）并将其存入CAM字典中，键为"Message"
    CAM["Message"] = GetCoopAwarness(CAMMessage);
// 返回存储了CAM消息完整信息的字典
    return CAM;
}
// 获取自定义V2X消息相关信息并以字典形式返回
// 传入参数为包含自定义V2X消息数据的carla::sensor::data::CustomV2XData结构体
boost::python::dict GetCustomV2XMessage(const carla::sensor::data::CustomV2XData message)
{
// 创建一个用于存储自定义V2X消息信息的字典
    boost::python::dict CustomV2X;
// 从传入的消息结构体中获取自定义V2X消息的具体内容，存储在CustomV2XM类型的变量中
    CustomV2XM V2XMessage = message.Message;
// 获取消息头部信息并将其存入CustomV2X字典中，键为"Header"
// 通过调用GetMessageHeader函数获取消息头部信息
    CustomV2X["Header"] = GetMessageHeader(V2XMessage.header);
// 将自定义V2X消息的具体内容转换为字符串类型，并将其存入CustomV2X字典中，键为"Message"
    CustomV2X["Message"] = std::string(V2XMessage.message);
// 返回存储了自定义V2X消息完整信息的字典
    return CustomV2X;
}
