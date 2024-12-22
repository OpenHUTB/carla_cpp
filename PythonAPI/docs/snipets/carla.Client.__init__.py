
# This recipe shows in every script provided in PythonAPI/Examples 
# and it is used to parse the client creation arguments when running the script. 

    argparser = argparse.ArgumentParser(# 创建一个命令行参数解析器对象
        description=__doc__)
    argparser.add_argument(
        '--host',# 添加一个名为'--host'的命令行参数
        metavar='H',
        default='127.0.0.1',# 参数默认值设置为'127.0.0.1'
        help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument(
        '-p', '--port',# 添加一个名为'-p'（短选项）或'--port'（长选项）的命令行参数，其在帮助信息中的显示名称为'P'
        metavar='P',
        default=2000,# 参数默认值设置为2000，并且指定该参数类型为整数
        type=int,
        help='TCP port to listen to (default: 2000)')# 帮助信息提示该参数用于指定要监听的TCP端口
    argparser.add_argument(
        '-s', '--speed',# 添加一个名为'-s'（短选项）或'--speed'（长选项）的命令行参数，其在帮助信息中的显示名称为'FACTOR'
        metavar='FACTOR',
        default=1.0,# 参数默认值设置为1.0，并且指定该参数类型为浮点数
        type=float,
        help='rate at which the weather changes (default: 1.0)')# 帮助信息提示该参数用于指定天气变化的速率
    args = argparser.parse_args()
 #解析命令行传入的参数，将解析结果保存到args对象中，后续可以通过该对象获取各个参数对应的值
    speed_factor = args.speed
    update_freq = 0.1 / speed_factor
# 使用解析得到的主机IP（args.host）和端口（args.port）创建一个carla客户端对象
    client = carla.Client(args.host, args.port)

