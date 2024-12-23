
# This recipe shows how to draw traffic light actor bounding boxes from a world snapshot.

# ....
debug = world.debug# 获取世界对象的调试工具
world_snapshot = world.get_snapshot()# 获取当前世界的一个快

for actor_snapshot in world_snapshot:# 遍历世界快照中的每一个角色快照
    actual_actor = world.get_actor(actor_snapshot.id)# 通过角色快照中记录的角色ID
   # 首先判断实际的 actor（可能是游戏、模拟场景等中的某个实体对象，具体取决于使用的框架或应用场景）的类型 ID 是否为 'traffic.traffic_light'，
   # 这里应该是通过类型 ID 来区分不同种类的实体，比如车辆、行人、交通信号灯等，当前是专门针对交通信号灯这种类型的实体进行后续操作。
    if actual_actor.type_id == 'traffic.traffic_light':
        debug.draw_box(carla.BoundingBox(actor_snapshot.get_transform().location,carla.Vector3D(0.5,0.5,2)),actor_snapshot.get_transform().rotation, 0.05, carla.Color(255,0,0,0),0)
# ...

