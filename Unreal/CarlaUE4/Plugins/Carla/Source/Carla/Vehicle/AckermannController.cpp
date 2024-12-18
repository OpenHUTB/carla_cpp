// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "AckermannController.h"
#include "CarlaWheeledVehicle.h"

// =============================================================================
// -- 构造函数和析构函数 ---------------------------------------------------------
// =============================================================================

FAckermannController::~FAckermannController() {}

// =============================================================================
// -- FAckermannController -----------------------------------------------------
// =============================================================================

FAckermannControllerSettings FAckermannController::GetSettings() const {
  FAckermannControllerSettings Settings;

  Settings.SpeedKp = SpeedController.Kp;
  Settings.SpeedKi = SpeedController.Ki;
  Settings.SpeedKd = SpeedController.Kd;

  Settings.AccelKp = AccelerationController.Kp;
  Settings.AccelKi = AccelerationController.Ki;
  Settings.AccelKd = AccelerationController.Kd;

  return Settings;
}

void FAckermannController::ApplySettings(const FAckermannControllerSettings& Settings) {
  SpeedController.Kp = Settings.SpeedKp;
  SpeedController.Ki = Settings.SpeedKi;
  SpeedController.Kd = Settings.SpeedKd;

  AccelerationController.Kp = Settings.AccelKp;
  AccelerationController.Ki = Settings.AccelKi;
  AccelerationController.Kd = Settings.AccelKd;
}

void FAckermannController::SetTargetPoint(const FVehicleAckermannControl& AckermannControl) {
  UserTargetPoint = AckermannControl;

  TargetSteer = FMath::Clamp(UserTargetPoint.Steer, -VehicleMaxSteering, VehicleMaxSteering);
  TargetSteerSpeed = FMath::Abs(UserTargetPoint.SteerSpeed);
  TargetSpeed = UserTargetPoint.Speed;
  TargetAcceleration = FMath::Abs(UserTargetPoint.Acceleration);
  TargetJerk = FMath::Abs(UserTargetPoint.Jerk);

}

void FAckermannController::Reset() {
  // 重置控制器
  SpeedController.Reset();
  AccelerationController.Reset();

  // 重置控制器参数
  Steer = 0.0f;
  Throttle = 0.0f;
  Brake = 0.0f;
  bReverse = false;

  SpeedControlAccelDelta = 0.0f;
  SpeedControlAccelTarget = 0.0f;

  AccelControlPedalDelta = 0.0f;
  AccelControlPedalTarget = 0.0f;

  // 重置车辆状态
  VehicleSpeed = 0.0f;
  VehicleAcceleration = 0.0f;

  LastVehicleSpeed = 0.0f;
  LastVehicleAcceleration = 0.0f;
}

void FAckermannController::RunLoop(FVehicleControl& Control) {
  TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

  // 横向控制
  RunControlSteering();

  // 纵向控制
  bool bStopped = RunControlFullStop();
  if (!bStopped) {
    RunControlReverse();
    RunControlSpeed();
    RunControlAcceleration();
    UpdateVehicleControlCommand();
  }

  // 更新控制命令
  Control.Steer = Steer / VehicleMaxSteering;
  Control.Throttle = FMath::Clamp(Throttle, 0.0f, 1.0f);
  Control.Brake = FMath::Clamp(Brake, 0.0f, 1.0f);
  Control.bReverse = bReverse;
}

void FAckermannController::RunControlSteering() {
  if (FMath::Abs(TargetSteerSpeed) < 0.001) {
    Steer = TargetSteer;
  } else {
    float SteerDelta = TargetSteerSpeed * DeltaTime;
    if (FMath::Abs(TargetSteer - VehicleSteer) < SteerDelta) {
      Steer = TargetSteer;
    } else {
      float SteerDirection = (TargetSteer > VehicleSteer) ? 1.0f : -1.0f;
      Steer = VehicleSteer + SteerDirection * (TargetSteerSpeed * DeltaTime);
    }

  }
}

bool FAckermannController::RunControlFullStop() {
  // 从这个速度在全刹车打开
  float FullStopEpsilon = 0.1; //[m/s]

  if (FMath::Abs(VehicleSpeed) < FullStopEpsilon && FMath::Abs(UserTargetPoint.Speed) < FullStopEpsilon) {
    Brake = 1.0;
    Throttle = 0.0;
    return true;
  }
  return false;
}

void FAckermannController::RunControlReverse() {
  // 从这个位置上可以切换到倒档
  float StandingStillEpsilon = 0.1;  // [m/s]

  if (FMath::Abs(VehicleSpeed) < StandingStillEpsilon) {
    // 停车不动，允许改变行驶方向
    if (UserTargetPoint.Speed < 0) {
      // 改变驾驶方向到倒车。
      bReverse = true;
    } else if (UserTargetPoint.Speed >= 0) {
      // 将驾驶方向改为前进。
      bReverse = false;
    }
  } else {
    if (FMath::Sign(VehicleSpeed) * FMath::Sign(UserTargetPoint.Speed) == -1) {
      // 请求改变驾驶方向。
      // 首先，我们必须完全停下来，然后才能改变行驶方向
      TargetSpeed = 0.0;
    }
  }

}

void FAckermannController::RunControlSpeed() {
  SpeedController.SetTargetPoint(TargetSpeed);
  SpeedControlAccelDelta = SpeedController.Run(VehicleSpeed, DeltaTime);

  // 剪裁边界
  float ClippingLowerBorder = -FMath::Abs(TargetAcceleration);
  float ClippingUpperBorder = FMath::Abs(TargetAcceleration);
  if (FMath::Abs(TargetAcceleration) < 0.0001f) {
    // 根据 AckermannDrive 的定义：如果为零，则使用最大值
    ClippingLowerBorder = -MaxDecel;
    ClippingUpperBorder = MaxAccel;
  }
  SpeedControlAccelTarget += SpeedControlAccelDelta;
  SpeedControlAccelTarget = FMath::Clamp(SpeedControlAccelTarget,
      ClippingLowerBorder, ClippingUpperBorder);
}

void FAckermannController::RunControlAcceleration() {
  AccelerationController.SetTargetPoint(SpeedControlAccelTarget);
  AccelControlPedalDelta = AccelerationController.Run(VehicleAcceleration, DeltaTime);

  // 剪裁边界
  AccelControlPedalTarget += AccelControlPedalDelta;
  AccelControlPedalTarget = FMath::Clamp(AccelControlPedalTarget, -1.0f, 1.0f);

}

void FAckermannController::UpdateVehicleControlCommand() {

  if (AccelControlPedalTarget < 0.0f) {
    if (bReverse) {
      Throttle = FMath::Abs(AccelControlPedalTarget);
      Brake = 0.0f;
    } else {
      Throttle = 0.0f;
      Brake = FMath::Abs(AccelControlPedalTarget);
    }
  } else {
    if (bReverse) {
      Throttle = 0.0f;
      Brake = FMath::Abs(AccelControlPedalTarget);
    } else {
      Throttle = FMath::Abs(AccelControlPedalTarget);
      Brake = 0.0f;
    }
  }
}

void FAckermannController::UpdateVehicleState(const ACarlaWheeledVehicle* Vehicle) {
  TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

  LastVehicleSpeed = VehicleSpeed;
  LastVehicleAcceleration = VehicleAcceleration;

  // 更新仿真状态
  DeltaTime = Vehicle->GetWorld()->GetDeltaSeconds();

  // 更新车辆状态
  VehicleSteer = Vehicle->GetVehicleControl().Steer * VehicleMaxSteering;
  VehicleSpeed = Vehicle->GetVehicleForwardSpeed() / 100.0f;  // From cm/s to m/s
  float CurrentAcceleration = (VehicleSpeed - LastVehicleSpeed) / DeltaTime;
  // 对加速度应用平均滤波器。
  VehicleAcceleration = (4.0f*LastVehicleAcceleration + CurrentAcceleration) / 5.0f;
}

void FAckermannController::UpdateVehiclePhysics(const ACarlaWheeledVehicle* Vehicle) {
  VehicleMaxSteering = FMath::DegreesToRadians(Vehicle->GetMaximumSteerAngle());
}
