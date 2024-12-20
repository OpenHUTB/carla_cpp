# This recipe shows in every script provided in PythonAPI/Examples 
# and it is used to parse the client creation arguments when running the script. 

# 创建一个参数解析器对象
argparser = argparse.ArgumentParser(
    description=__doc__)
# 添加一个名为'--host'的命令行参数，默认值为'127.0.0.1'，帮助信息为'IP of the host server (default: 127.0.0.1)'
argparser.add_argument(
    '--host',
    metavar='H',
    default='127.0.0.1',
    help='IP of the host server (default: 127.0.0.1)')
# 添加一个名为'-p'或'--port'的命令行参数，默认值为 2000，数据类型为整数，帮助信息为'TCP port to listen to (default: 2000)'
argparser.add_argument(
    '-p', '--port',
    metavar='P',
    default=2000,
    type=int,
    help='TCP port to listen to (default: 2000)')
# 添加一个名为'-s'或'--speed'的命令行参数，默认值为 1.0，数据类型为浮点数，帮助信息为'rate at which the weather changes (default: 1.0)'
argparser.add_argument(
    '-s', '--speed',
    metavar='FACTOR',
    default=1.0,
    type=float,
    help='rate at which the weather changes (default: 1.0)')
# 解析命令行参数并将结果存储在 args 变量中
args = argparser.parse_args()

# 从解析后的参数中获取速度因子
speed_factor = args.speed
# 计算更新频率，更新频率等于 0.1 除以速度因子
update_freq = 0.1 / speed_factor

# 创建一个 carla.Client 对象，使用解析得到的主机 IP 和端口
client = carla.Client(args.host, args.port)
