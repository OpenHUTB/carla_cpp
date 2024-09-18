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
  #include <netinet/in.h> ///< sockaddr_in
  #include <arpa/inet.h>  ///< getsockname
  #include <unistd.h>     ///< close
#endif

#define SOCK_INVALID_INDEX  -1