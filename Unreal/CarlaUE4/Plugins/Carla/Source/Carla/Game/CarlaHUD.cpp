// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaHUD.h"
#include "GameFramework/PlayerController.h"

// ACarlaHUD类的DrawHUD函数，重写了父类（可能是UE4中HUD相关基类）的DrawHUD函数，用于在游戏界面上绘制各种HUD（ Heads-Up Display，抬头显示）元素，比如文本、线条等。
void ACarlaHUD::DrawHUD()
{
    // 调用父类的DrawHUD函数，确保先执行父类中已有的绘制逻辑，这是在UE4等游戏开发框架中重写函数时的常见做法，以保证继承链上的功能完整性。
    Super::DrawHUD();

    // 获取拥有当前HUD的玩家控制器对象指针，如果获取不到（返回nullptr），说明出现了问题，可能是没有正确关联玩家控制器等情况。
    auto Player = GetOwningPlayerController();
    if (Player == nullptr)
    {
        // 使用UE_LOG宏输出错误日志，表明找不到玩家控制器，日志分类为LogCarla，错误级别为Error，日志内容为提示文本"Can't find player controller!"，方便后续排查问题。
        UE_LOG(LogCarla, Error, TEXT("Can't find player controller!"));
        // 如果没有找到玩家控制器，直接返回，不执行后续的绘制逻辑，因为很多绘制操作依赖于玩家控制器来进行坐标转换等相关操作。
        return;
    }

    // 判断是否有调试车辆（DebugVehicle指针不为nullptr时），如果有，则执行相应的调试绘制操作，可能用于在游戏界面上显示车辆相关的调试信息。
    if(DebugVehicle) {
        // 定义一个浮点数YL，用于设置在屏幕上绘制调试信息的纵向位置（这里设置为1600.0f，具体含义取决于游戏界面的坐标系统和布局需求）。
        float YL = 1600.0f;
        // 定义一个浮点数Y0，同样用于绘制调试信息的纵向位置（这里初始化为0.0f，与YL配合确定绘制范围等）。
        float Y0 = 0.0f;
        // 调用DebugVehicle的DrawDebug函数（应该是自定义的用于绘制车辆调试信息的函数），传入当前的画布（Canvas，用于在其上进行绘制操作）、纵向位置参数YL和Y0，来在指定位置绘制车辆相关调试信息。
        DebugVehicle->DrawDebug(Canvas, YL, Y0);
    }

    // 获取当前的时间（以秒为单位），通过调用FPlatformTime::Seconds函数获取平台相关的高精度时间，用于后续判断HUD元素是否过期需要移除等操作。
    double Now = FPlatformTime::Seconds();
    // 定义一个整数变量i，用于循环遍历存储字符串信息的列表（StringList），初始化为0，从列表开头开始处理。
    int i = 0;
    // 开始循环遍历StringList数组，只要i小于数组中元素的个数（即还有元素未处理），就持续循环。
    while (i < StringList.Num())
    {
        // 定义一个二维向量Screen，用于存储将世界坐标转换到屏幕坐标后的结果，后续会通过玩家控制器的ProjectWorldLocationToScreen函数来填充这个坐标值。
        FVector2D Screen;
        // 调用玩家控制器的ProjectWorldLocationToScreen函数，尝试将StringList中当前索引（i）对应的字符串元素的世界坐标位置（StringList[i].Location）转换为屏幕坐标，
        // 如果转换成功（函数返回true），则将结果存储到Screen变量中，并且可以在屏幕上对应的坐标位置绘制该字符串；
        // 第三个参数true可能表示使用某种特定的投影模式或者其他相关设置，具体取决于该函数的定义。
        if (Player->ProjectWorldLocationToScreen(StringList[i].Location, Screen, true))
        {
            // 调用DrawText函数（应该是UE4中用于在屏幕指定坐标位置绘制文本的函数），传入要绘制的字符串（StringList[i].Str）、颜色（StringList[i].Color）以及屏幕坐标（Screen.X和Screen.Y），
            // 在屏幕上对应的位置绘制出该字符串内容，实现HUD字符串的显示。
            DrawText(StringList[i].Str, StringList[i].Color, Screen.X, Screen.Y);
        }

        // 检查当前字符串元素是否已经过期，通过比较当前时间（Now）和该字符串元素预设的过期时间（StringList[i].TimeToDie）来判断，
        // 如果当前时间大于等于过期时间，说明该字符串已经不需要显示了，应该从列表中移除。
        if (Now >= StringList[i].TimeToDie)
        {
            // 调用StringList的RemoveAt函数，从列表中移除当前索引（i）对应的元素，实现移除过期字符串的操作，注意移除元素后列表的索引会自动更新，后续循环处理时要注意这点。
            StringList.RemoveAt(i);
        }
        else
            // 如果当前字符串元素未过期，则将索引i加1，继续处理下一个字符串元素，以遍历整个StringList数组中的所有字符串元素。
            ++i;
    }

    // 类似地，开始处理存储线条信息的LineList数组，循环逻辑和处理字符串列表类似，只是操作针对线条相关的数据和绘制函数。
    while (i < LineList.Num())
    {
        // 定义两个二维向量Begin和End，分别用于存储线条起始点和结束点经过坐标转换后的屏幕坐标值，后续通过玩家控制器的函数来获取实际坐标。
        FVector2D Begin, End;
        // 调用玩家控制器的ProjectWorldLocationToScreen函数，分别尝试将LineList中当前索引（i）对应的线条元素的起始点世界坐标（LineList[i].Begin）和结束点世界坐标（LineList[i].End）转换为屏幕坐标，
        // 只有当起始点和结束点都成功转换为屏幕坐标（两个函数调用都返回true）时，才可以在屏幕上绘制这条线条，这样保证了绘制的线条在屏幕上有正确的显示位置。
        if (Player->ProjectWorldLocationToScreen(LineList[i].Begin, Begin, true) &&
            Player->ProjectWorldLocationToScreen(LineList[i].End, End, true))
        {
            // 调用DrawLine函数（应该是UE4中用于在屏幕上绘制线条的函数），传入线条起始点的屏幕坐标（Begin.X和Begin.Y）、结束点的屏幕坐标（End.X和End.Y）、线条颜色（LineList[i].Color）以及线条粗细（LineList[i].Thickness），
            // 在屏幕上绘制出对应的线条，实现HUD线条的显示。
            DrawLine(Begin.X, Begin.Y, End.X, End.Y, LineList[i].Color, LineList[i].Thickness);
        }

        // 同样检查当前线条元素是否已经过期，通过比较当前时间（Now）和该线条元素预设的过期时间（LineList[i].TimeToDie）来判断，
        // 如果当前时间大于等于过期时间，说明该线条已经不需要显示了，应该从列表中移除。
        if (Now >= LineList[i].TimeToDie)
        {
            // 调用LineList的RemoveAt函数，从列表中移除当前索引（i）对应的元素，实现移除过期线条的操作，移除后列表索引会自动更新，后续循环要相应处理。
            LineList.RemoveAt(i);
        }
        else
            // 如果当前线条元素未过期，则将索引i加1，继续处理下一个线条元素，以遍历整个LineList数组中的所有线条元素。
            ++i;
    }
}

// ACarlaHUD类的成员函数AddHUDString，用于向HUD的字符串列表（StringList）中添加一个新的字符串元素，同时设置其相关属性（位置、颜色、生命周期等）。
void ACarlaHUD::AddHUDString(const FString Str, const FVector Location, const FColor Color, double LifeTime)
{
    // 获取当前的时间（以秒为单位），通过调用FPlatformTime::Seconds函数获取平台相关的高精度时间，用于后续计算该字符串元素的过期时间。
    double Now = FPlatformTime::Seconds();
    // 创建一个名为Obj的HUDString结构体对象（应该是自定义的结构体，包含字符串相关的显示信息，如字符串内容、位置、颜色和过期时间等字段），
    // 使用初始化列表的方式对其进行初始化，将传入的字符串（Str）、位置（Location）、颜色（Color）以及计算好的过期时间（当前时间加上传入的生命周期LifeTime）赋值给对应的字段。
    HUDString Obj { Str, Location, Color, Now + LifeTime };
    // 将构造好的HUDString结构体对象添加到StringList数组末尾，实现向HUD字符串列表中添加新元素的操作，使得后续在DrawHUD函数中可以遍历并绘制这个新添加的字符串。
    StringList.Add(std::move(Obj));
}

// ACarlaHUD类的成员函数AddHUDLine，用于向HUD的线条列表（LineList）中添加一个新的线条元素，同时设置其相关属性（起始点、结束点、粗细、颜色、生命周期等）。
void ACarlaHUD::AddHUDLine(const FVector Begin, const FVector End, const float Thickness, const FColor Color, double LifeTime)
{
    // 获取当前的时间（以秒为单位），通过调用FPlatformTime::Seconds函数获取平台相关的高精度时间，用于后续计算该线条元素的过期时间。
    double Now = FPlatformTime::Seconds();
    // 创建一个名为Obj的HUDLine结构体对象（应该是自定义的结构体，包含线条相关的显示信息，如起始点、结束点、粗细、颜色和过期时间等字段），
    // 使用初始化列表的方式对其进行初始化，将传入的线条起始点（Begin）、结束点（End）、粗细（Thickness）、颜色（Color）以及计算好的过期时间（当前时间加上传入的生命周期LifeTime）赋值给对应的字段。
    HUDLine Obj { Begin, End, Thickness, Color, Now + LifeTime };
    // 将构造好的HUDLine结构体对象添加到LineList数组末尾，实现向HUD线条列表中添加新元素的操作，使得后续在DrawHUD函数中可以遍历并绘制这个新添加的线条。
    LineList.Add(std::move(Obj));
}
