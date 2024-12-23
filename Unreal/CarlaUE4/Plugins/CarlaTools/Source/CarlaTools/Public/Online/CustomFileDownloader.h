// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"// 引入核心的头文件
#include "Interfaces/IHttpRequest.h"// 引入HTTP请求的接口头文件
#include "CustomFileDownloader.generated.h"// 引入生成的类头文件
/**
 *
 */

DECLARE_DELEGATE(FDownloadComplete)// 声明一个下载完成的委托类型


struct FHttpDownloader
{
public:
  FHttpDownloader();// 构造函数

  /**
   *
   * @param Verb - verb to use for request (GET,POST,DELETE,etc)
   * @param Url - url address to connect to
   */
  FHttpDownloader( const FString& InVerb, const FString& InUrl, const FString& InFilename, FDownloadComplete& Delegate  );

  // 启动HTTP请求并等待委托被调用
  void Run(void);

/**
 * 这是一段函数相关的文档注释（通常用于生成代码文档等场景，像 Doxygen 等工具可以解析这样的注释来生成对应的文档），
 * 下面描述的函数应该是作为一个委托（Delegate，一种回调机制，常用于事件处理等场景），会在某个请求完成的时候被调用。
 */
  /**
   * Delegate called when the request completes
   *
   * @param HttpRequest - object that started/processed the request
   *  @brief 表示这个委托函数被调用时传入的第一个参数，名为 HttpRequest，它代表了发起或者处理该请求的对象。
   *  具体来说，这个对象可能包含了请求的各种属性，比如请求的 URL、请求的方法（GET、POST 等）、请求的头部信息等相关内容，
   *  通过这个对象可以获取到请求相关的详细信息以及对请求做进一步的操作（具体取决于该对象所属类的定义及提供的接口）。
   *
   * @param HttpResponse - optional response object if request completed
   *  @brief 表示委托函数被调用时传入的第二个参数，名为 HttpResponse，这是一个可选（optional）的参数，意味着该参数在某些情况下可能为空指针（nullptr）。
   *  当请求成功完成时，这个参数会包含对应的响应对象，通过它可以获取到服务器返回的响应内容，比如响应的状态码、响应的正文数据、响应头部信息等，
   *  如果请求没有正常完成（比如出现网络错误等情况导致没有接收到有效的响应），那么这个参数就不会被赋值，也就是传入的是 nullptr。
   *
   * @param bSucceeded - true if Url connection was made and response was received
   *  @brief 表示委托函数被调用时传入的第三个参数，名为 bSucceeded，它是一个布尔类型（bool）的变量，用于指示请求是否成功完成。
   *  具体判断标准是当成功建立了 URL 连接并且接收到了相应的响应时，这个参数的值会被设置为 true，否则（比如连接失败、超时等情况）就会被设置为 false，
   *  通过这个参数可以方便地在回调函数（也就是这个委托函数）中根据请求的完成情况来执行不同的后续操作，比如进行错误处理或者进一步处理响应数据等。
   */
  void RequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);

private:
  FString Verb;
  FString Url;
  FString Filename;
  FDownloadComplete DelegateToCall;
};

UCLASS(Blueprintable)
class CARLATOOLS_API UCustomFileDownloader : public UObject
{
  GENERATED_BODY()
public:
  UFUNCTION(BlueprintCallable)
  void StartDownload();
  UFUNCTION(BlueprintCallable)
  void ConvertOSMInOpenDrive(FString FilePath, float Lat_0 = 0, float Lon_0 = 0);

  FString ResultFileName;

  FString Url;

  FDownloadComplete DownloadDelegate;
private:
  void RequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);

  FString Payload;
};


