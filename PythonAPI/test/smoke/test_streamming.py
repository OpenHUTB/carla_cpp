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


    def test_multistream(self):                       #函数用于测试多流相关功能
        print("TestStreamming.test_multistream")      #打印当前正在执行的测试方法名称
        # create the sensor
        world = self.client.get_world()               #通过客户端对象（self.client）获取整个仿真世界的对象
        bp = world.get_blueprint_library().find('sensor.other.gnss')
        #从世界对象（world）的蓝图库中查找名为'sensor.other.gnss'的传感器蓝图
        bp.set_attribute("sensor_tick", str(1.0))
        #对找到的传感器蓝图（bp）设置一个名为'sensor_tick'的属性，将其值设置为字符串形式的'1.0'
        gnss_sensor = world.spawn_actor(bp, carla.Transform())
        #依据之前获取的传感器蓝图（bp），在世界中的默认位置（通过carla.Transform()给定初始的变换信息
        gnss_sensor.listen(self.on_gnss_set)
        #让创建好的GNSS传感器实例（gnss_sensor）开始监听一个回调函数self.on_gnss_set
        world.wait_for_tick()
        #让程序暂停，等待整个仿真世界完成一次时间推进

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
