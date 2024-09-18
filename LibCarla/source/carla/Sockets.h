// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once    // 防止头文件被重复包含

#if _WIN32   // 判断是否是在Windows平台编译
  #include <winsock2.h>    // 引入Windows Sockets 2 API，用于网络通信  
  #include <Ws2tcpip.h>   // 包含TCP/IP协议的函数和数据结构定义 
#else   // 如果不是Windows平台，则假设是类Unix系统（如Linux、macOS） 
  #include <sys/socket.h> // 包含socket API的定义，用于网络通信
  #include <netinet/in.h> //包含IP地址转换等网络相关的数据结构定义
  #include <arpa/inet.h>  // 包含地址转换函数
  #include <unistd.h>     // 提供对POSIX操作系统API的访问
#endif

#define SOCK_INVALID_INDEX  -1  // 定义一个宏，用于表示无效的socket索引或文件描述符。在Windows和类Unix系统上，无效的socket或文件描述符通常表示为-1
