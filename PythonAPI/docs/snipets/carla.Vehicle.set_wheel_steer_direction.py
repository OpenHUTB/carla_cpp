# Sets the appearance of the vehicles front wheels to 40°. Vehicle physics will not be affected.
# 这行注释说明了以下两行代码的总体作用：将车辆前轮的转向外观角度设置为40°，需要注意的是这样的操作并不会影响车辆实际的物理特性（比如车辆真实的转向受力、转向运动等物理层面的表现）。

# 调用 vehicle 对象（应该是代表车辆的一个实例，其所属类中定义了相关操作车辆属性的方法）的 set_wheel_steer_direction 方法，
# 传入参数 carla.VehicleWheelLocation.FR_Wheel，表示要设置的是车辆的右前轮（通过这个特定的枚举值来指定具体车轮位置），
# 第二个参数 40.0 则表示将右前轮的转向外观角度设置为 40°。
vehicle.set_wheel_steer_direction(carla.VehicleWheelLocation.FR_Wheel, 40.0)

# 同样调用 vehicle 对象的 set_wheel_steer_direction 方法，这次传入参数 carla.VehicleWheelLocation.FL_Wheel，
# 它代表车辆的左前轮，也是将左前轮的转向外观角度设置为 40°，与上一行代码一起实现了将车辆两个前轮的转向外观角度都设置为 40°的操作。
vehicle.set_wheel_steer_direction(carla.VehicleWheelLocation.FL_Wheel, 40.0)
