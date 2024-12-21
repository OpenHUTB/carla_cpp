
#目的是销毁行人（相关对象），先让他们停止导航行为，然后再销毁对应的对象（包括行人类的参与者以及其对应的控制器）。

# 停止行人（传入的列表 all_id 中元素排列顺序是 [控制器, 参与者, 控制器, 参与者...] 这样交替的形式）
for i in range(0, len(all_id), 2):
    all_actors[i].stop()

# 销毁行人（对应的参与者和控制器）
client.apply_batch([carla.command.DestroyActor(x) for x in all_id])
