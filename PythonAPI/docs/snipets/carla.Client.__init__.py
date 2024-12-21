# 这段代码用于解析运行脚本时创建客户端所需的参数。它通常出现在 PythonAPI/Examples 目录下的每个脚本中。

# 创建一个参数解析器对象，description 参数用于描述该脚本的功能，这里使用了文档字符串 __doc__，通常包含脚本的详细说明。
argparser = argparse.ArgumentParser(
    description=__doc__)
# 添加一个名为 --host 的命令行参数，metavar 是在帮助信息中显示的参数名称占位符，default 设置默认值为 '127.0.0.1'，
# help 提供了参数的描述信息，即主机服务器的 IP 地址，默认是本地回环地址 127.0.0.1。
argparser.add_argument(
    '--host',
    metavar='H',
    default='127.0.0.1',
    help='IP of the host server (default: 127.0.0.1)')
# 添加一个名为 -p 或 --port 的命令行参数，用于指定 TCP 端口号，metavar 同样是占位符，default 设置默认端口为 2000，
# type=int 指明参数类型为整数，help 描述了该参数的作用是指定要监听的 TCP 端口，默认值是 2000。
argparser.add_argument(
    '-p', '--port',
    metavar='P',
    default=2000,
    type=int,
    help='TCP port to listen to (default: 2000)')
# 添加一个名为 -s 或 --speed 的命令行参数，用于指定天气变化的速率，metavar 为占位符，default 设置默认速率为 1.0，
# type=float 说明参数类型为浮点数，help 解释了该参数的含义，即天气变化的速率，默认值是 1.0。
argparser.add_argument(
    '-s', '--speed',
    metavar='FACTOR',
    default=1.0,
    type=float,
    help='rate at which the weather changes (default: 1.0)')
# 解析命令行参数，将解析结果存储在 args 对象中，后续可以通过访问 args 的属性来获取相应参数的值。
args = argparser.parse_args()

# 从解析后的参数 args 中获取天气变化速率参数值，并赋值给 speed_factor 变量。
speed_factor = args.speed
# 根据天气变化速率计算更新频率，更新频率等于 0.1 除以天气变化速率，这里 0.1 可能是一个固定的时间间隔系数，
# 用于根据不同的天气变化速率来调整更新的频繁程度。
update_freq = 0.1 / speed_factor

# 使用解析得到的主机 IP 和端口号创建一个 carla.Client 对象，这个客户端对象通常用于与 CARLA 仿真环境进行交互，
# 连接到指定 IP 和端口的服务器上，以便后续发送指令、获取数据等操作。
client = carla.Client(args.host, args.port)
