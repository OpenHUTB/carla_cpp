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
        # stop
        for actor in actors:
            if (actor.type_id == "sensor.other.gnss"):
                actor.stop()


    def test_multistream(self):
        print("TestStreamming.test_multistream")
        # create the sensor
        world = self.client.get_world()
        bp = world.get_blueprint_library().find('sensor.other.gnss')
        bp.set_attribute("sensor_tick", str(1.0))
        gnss_sensor = world.spawn_actor(bp, carla.Transform())
        gnss_sensor.listen(self.on_gnss_set)
        world.wait_for_tick()

        # create 5 clients
        t = [0] * 5                 #创建一个长度为5的列表t，用于存储5个线程对象，初始值都设为0 
        for i in range(5):          #循环创建5个线程对象，每个线程的目标函数是self.create_client
            t[i] = threading.Thread(target=self.create_client)
            t[i].setDaemon(True)    #将每个线程设置为守护线程。守护线程会在主线程结束时自动退出
            t[i].start()            #启动每个线程，使线程开始执行其目标函数self.create_client中的代码

        # wait for ending clients    #主线程等待之前创建的 5 个子线程全部执行完毕
        for i in range(5):
            t[i].join()

        gnss_sensor.destroy()       #销毁gnss_sensor对象，释放相关资源
