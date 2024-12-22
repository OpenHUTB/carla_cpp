// Copyright (c) 2023 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 包含 "EditorCamera.h" 头文件
#include "EditorCamera.h"
// 包含 "unreal.h" 头文件
#include "UnrealClient.h"
// 包含 "Editor/EditorEngine.h" 头文件
#include "Editor/EditorEngine.h"
// 包含 "Editor/EditorViewportClient.h" 头文件
#include "EditorViewportClient.h"

void AEditorCameraUtils::Get()
// 定义 AEditorCameraUtils 类的 Get() 函数
{
    // 获取当前活动视图客户端
    auto ViewportClient = dynamic_cast<FEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient());
    // 创建一个变换对象
    CameraTransform = FTransform();
     // 设置变换对象的位置为视图客户端的视图位置
    CameraTransform.SetLocation(ViewportClient->GetViewLocation());
    // 设置变换对象的旋转为视图客户端的视图旋转
    CameraTransform.SetRotation(FQuat(ViewportClient->GetViewRotation()));
}

void AEditorCameraUtils::Set()
// 定义 AEditorCameraUtils 类的 Set() 函数
{
    // 获取当前活动视图客户端
    auto ViewportClient = dynamic_cast<FEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient());
    // 设置视图客户端的视图位置为变换对象的位置
    ViewportClient->SetViewLocation(CameraTransform.GetLocation());
     // 设置视图客户端的视图旋转为变换对象的旋转
    ViewportClient->SetViewRotation(FRotator(CameraTransform.GetRotation()));
}
