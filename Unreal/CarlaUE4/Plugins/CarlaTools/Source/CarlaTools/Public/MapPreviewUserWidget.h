// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.
// 此作品依据MIT许可证的条款授权。如需副本，请查看 <https://opensource.org/licenses/MIT>。
// 这是一种常见的版权声明及开源协议说明，告知使用者代码的版权归属以及遵循的开源许可情况。
#pragma once
// 这是一个预处理器指令，用于确保头文件只被编译一次，避免重复定义等问题。
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
// 引入UE4核心基础头文件以及用户Widget相关头文件，
// CoreMinimal.h通常包含了UE4基本类型等基础定义，UserWidget.h用于构建基于UE4蓝图的用户界面组件相关功能。

THIRD_PARTY_INCLUDES_START
#include <boost/asio.hpp>
THIRD_PARTY_INCLUDES_END// 引入Boost库的asio模块，asio常用于网络编程，例如创建网络套接字、进行异步网络操作等功能。
#include <memory>
// 引入C++的标准库头文件，用于管理动态内存，像std::unique_ptr等智能指针类型就在这个头文件中定义。
#include "MapPreviewUserWidget.generated.h"
// 这是UE4的代码生成相关头文件，通常配合UE4的反射系统等机制来生成一些必要的代码，用于类的序列化、蓝图支持等功能。
class FSocket;
class UTexture2D;
// 前向声明两个类，FSocket可能是UE4中与套接字相关的类，UTexture2D用于处理纹理相关功能。
// 前向声明可以在不完全包含对应类的完整定义情况下，告知编译器后续会用到这些类，解决编译时类定义先后顺序依赖问题。
UCLASS()
class CARLATOOLS_API UMapPreviewUserWidget : public UUserWidget
{
	GENERATED_BODY()
// 这是UE4的宏，用于标记类的可生成代码部分，配合UE4的代码生成系统来实现诸如反射等功能。
private:// Boost socket
	
  boost::asio::io_service io_service;// boost::asio库中的io_service对象，它是用于调度异步操作的核心对象，
    // 比如管理网络套接字的读写等异步任务的执行和回调等相关操作。
  std::unique_ptr<boost::asio::ip::tcp::socket> SocketPtr;
// 使用std::unique_ptr智能指针来管理一个boost::asio库中的tcp套接字对象，
    // 智能指针可以自动管理所指向对象的内存生命周期，避免内存泄漏等问题，
    // 这里用于后续网络通信相关操作中对tcp套接字的使用。

	bool SendStr(FString Msg);// 自定义的私有函数声明，用于接收表示角落经纬度坐标的字符串（推测从功能命名来看），
    // 同样具体实现应该在.cpp文件中，也是基于相关网络套接字进行接收操作。
	FString RecvCornersLatLonCoords();
// 自定义的私有函数声明，用于接收表示角落经纬度坐标的字符串（推测从功能命名来看），
    // 同样具体实现应该在.cpp文件中，也是基于相关网络套接字进行接收操作。
public:// 这里后续可以添加类的公有成员函数、属性等的定义，目前为空，
    // 公有成员通常可以被外部类访问，用于提供类的对外接口等功能。
// UPROPERTY 是UE4中的宏，用于定义类的属性并指定其在编辑器中的相关特性和访问权限等。

// EditAnywhere 表示这个属性可以在虚幻编辑器的属性窗口中被编辑，并且在蓝图中也可以被读写访问。
// BlueprintReadWrite 表示此属性在蓝图中可以进行读取和写入操作，这使得蓝图可以方便地操作该属性的值。
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* MapTexture;
// 声明一个指向 UTexture2D 类型的指针，用于存储地图纹理相关的数据，
// 通过上述属性修饰符，可以在编辑器和蓝图中方便地对其进行操作，比如赋值不同的地图纹理资源等。

// BlueprintReadOnly 表示这个属性在蓝图中只能进行读取操作，不能修改其值，通常用于对外暴露只读的数据。
	UPROPERTY(BlueprintReadOnly)
	float TopRightLat = 0.f;/ 定义一个表示右上角纬度的浮点型属性，初始值设为0，在蓝图中可读取该值来获取相关地图区域的纬度信息。
	UPROPERTY(BlueprintReadOnly)
	float TopRightLon = 0.f;// 定义一个表示右上角经度的浮点型属性，初始值设为0，用于在蓝图中提供右上角经度相关信息的读取。
	UPROPERTY(BlueprintReadOnly)
	float BottomLeftLat = 0.f;// 定义一个表示左下角纬度的浮点型属性，初始值设为0，方便蓝图获取对应地图区域左下角纬度情况。
	UPROPERTY(BlueprintReadOnly)
	float BottomLeftLon = 0.f;// 定义一个表示左下角经度的浮点型属性，初始值设为0，供蓝图读取左下角经度相关信息。

// UFUNCTION 是UE4中的宏，用于将类中的函数暴露给蓝图使用，并可以指定函数在蓝图中的相关特性，比如是否可调用等。
// BlueprintCallable 表示这个函数在蓝图中可以被调用，使其成为蓝图能够使用的接口方法。

	UFUNCTION(BlueprintCallable)
	void ConnectToSocket(FString DatabasePath, FString StylesheetPath, int Size);
// 声明一个可在蓝图中调用的函数，名为 ConnectToSocket，
// 它接收三个参数：一个是表示数据库路径的字符串（FString 类型）、一个是样式表路径的字符串以及一个整数类型的 Size 参数，
// 推测此函数用于建立与某个套接字的连接，并且可能会根据传入的路径等参数进行相关的初始化配置工作。
	UFUNCTION(BlueprintCallable)
	void RenderMap(FString Latitude, FString Longitude, FString Zoom);
// 声明一个蓝图可调用的函数 RenderMap，
// 接收三个字符串参数，分别代表纬度（Latitude）、经度（Longitude）和缩放级别（Zoom），
// 应该是用于根据给定的地理位置和缩放信息来渲染地图相关内容的函数。
	UFUNCTION(BlueprintCallable)
	void Shutdown();
// 声明一个可在蓝图中调用的 Shutdown 函数，通常用于执行关闭相关操作，比如关闭网络连接、释放资源等清理工作，
// 具体的关闭行为取决于类内对此函数的具体实现逻辑。
	UFUNCTION(BlueprintCallable)
	void CreateTexture();
// 声明一个蓝图可调用的函数 CreateTexture，从函数名推测是用于创建纹理相关资源的操作，
// 可能会涉及到根据一定规则或已有数据生成、初始化纹理对象等功能。
	UFUNCTION(BlueprintCallable)
	void OpenServer();
// 声明一个可在蓝图中调用的 OpenServer 函数，大概率用于启动服务器相关的功能，
// 比如开启监听端口、初始化服务器相关的网络配置等，具体实现依赖类内的代码逻辑。
	UFUNCTION(BlueprintCallable)
	void CloseServer();
// 声明一个可在蓝图中调用的 CloseServer 函数，与 OpenServer 相对应，用于关闭已经开启的服务器，
// 可能会进行诸如关闭网络连接、释放服务器占用的资源等操作。
	UFUNCTION(BlueprintCallable)
	void UpdateLatLonCoordProperties();
// 声明一个蓝图可调用的函数 UpdateLatLonCoordProperties，
// 推测是用于更新上述定义的那些表示经纬度坐标的属性（TopRightLat、TopRightLon 等）的值，
// 可能会从外部获取新的数据来刷新这些坐标相关的属性信息。	
};
