# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.


from . import SmokeTest

import time
import threading
import carla

class TestStreamming(SmokeTest):

    lat = 0.0
    lon = 0.0

    def on_gnss_set(self, event):           #函数用于处理gnss事件
        self.lat = event.latitude           #将从event中获取的latitude（纬度）赋值给self.lat
        self.lon = event.longitude          #将从event中获取的longitude（经度）赋值给self.lon

    def on_gnss_check(self, event):
        self.assertAlmostEqual(event.latitude, self.lat, places=4)
        self.assertAlmostEqual(event.longitude, self.lon, places=4)

    def create_client(self):
        client = carla.Client(*self.testing_address)
        client.set_timeout(60.0)
        world = client.get_world()
        actors = world.get_actors()
        for actor in actors:
            if (actor.type_id == "sensor.other.gnss"):
                actor.listen(self.on_gnss_check)
        time.sleep(5)
        # 停止所有类型为'sensor.other.gnss'的传感器，结束数据监听
        for actor in actors:
            if (actor.type_id == "sensor.other.gnss"):
                actor.stop()


    def test_multistream(self):
        print("TestStreamming.test_multistream")
        # 通过客户端获取对应的世界对象，用于后续创建传感器等操作
        world = self.client.get_world()
        bp = world.get_blueprint_library().find('sensor.other.gnss')
        bp.set_attribute("sensor_tick", str(1.0))
        gnss_sensor = world.spawn_actor(bp, carla.Transform())
        gnss_sensor.listen(self.on_gnss_set)
        world.wait_for_tick()

         # 创建一个包含5个元素的列表，用于存储后续创建的线程对象
        t = [0] * 5
        for i in range(5):
            t[i] = threading.Thread(target=self.create_client)
            t[i].setDaemon(True)
            t[i].start()

       # 循环等待所有线程执行结束
        for i in range(5):
            t[i].join()

        gnss_sensor.destroy()
