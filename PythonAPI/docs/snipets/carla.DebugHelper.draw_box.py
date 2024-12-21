
# This recipe shows how to draw traffic light actor bounding boxes from a world snapshot.

# ....
debug = world.debug# 获取世界对象的调试工具
world_snapshot = world.get_snapshot()# 获取当前世界的一个快

for actor_snapshot in world_snapshot:# 遍历世界快照中的每一个角色快照
    actual_actor = world.get_actor(actor_snapshot.id)# 通过角色快照中记录的角色ID
    if actual_actor.type_id == 'traffic.traffic_light':
        debug.draw_box(carla.BoundingBox(actor_snapshot.get_transform().location,carla.Vector3D(0.5,0.5,2)),actor_snapshot.get_transform().rotation, 0.05, carla.Color(255,0,0,0),0)
# ...

