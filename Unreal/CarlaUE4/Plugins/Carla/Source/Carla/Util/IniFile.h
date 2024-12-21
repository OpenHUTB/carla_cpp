// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// 这是一个预处理指令，用于确保头文件只被包含一次，避免重复定义等问题
#pragma once  

// 包含 "ConfigCacheIni.h" 头文件，可能其中定义了与配置文件相关的类、结构体等内容，
// 是当前类操作的基础依赖
#include "ConfigCacheIni.h"  
// 包含标准库中关于数值极限的头文件，用于后续对数值类型范围的判断等操作
#include <limits>  

// 为类 FIniFile 提供了一个 API 标识，具体取决于所在项目中 CARLA_API 的定义，
// 通常用于表明该类是对外公开的接口类之类的用途
class CARLA_API FIniFile : private NonCopyable
{
private:
    // 定义一个模板函数，用于安全地将一种类型的值转换为另一种类型的值
    // @param source 要转换的源值
    // @param target 转换后存储的目标值（引用传递，用于接收转换结果）
    template <typename TARGET, typename SOURCE>
    static void SafeCastTo(SOURCE source, TARGET &target)
    {
        // 判断源值是否在目标类型所能表示的数值范围内，如果在范围内，则进行类型转换
        if ((source >= std::numeric_limits<TARGET>::lowest()) &&
            (source <= std::numeric_limits<TARGET>::max())) {
            target = static_cast<TARGET>(source);
        } else {
            // 如果类型转换失败，输出错误日志信息到 Unreal 的日志系统中（假设 UE_LOG 是 Unreal 中的日志宏）
            UE_LOG(LogCarla, Error, TEXT("FIniFile: Type cast failed"));
        }
    }

public:
    // ===========================================================================
    /// @name Constructor  构造函数相关区域的开始标记，用于文档化代码结构，方便阅读代码时区分不同功能块
    // ===========================================================================
    /// @{  开始一个代码块的文档化分组，一般配合 @} 使用来明确一个功能组的范围

    // 默认构造函数，使用默认实现，通常用于创建一个初始状态的对象实例
    FIniFile() = default;  

    // 显式构造函数，接受一个文件名作为参数
    // @param FileName 要读取的配置文件的文件名（类型为 FString，应该是 Unreal 中的字符串类型）
    explicit FIniFile(const FString &FileName)
    {
        // 调用 ConfigFile（FConfigFile 类型的成员变量）的 Read 函数，尝试读取指定的配置文件
        ConfigFile.Read(FileName);
    }

    /// @}  结束构造函数相关的代码块文档化分组
    // ===========================================================================
    /// @name Other functions  其他功能函数相关区域的开始标记
    // ===========================================================================
    /// @{

    // 尝试将另一个配置文件的内容合并到当前配置文件对象中
    // @param FileName 要合并的配置文件的文件名
    // @return 合并操作是否成功的布尔值
    bool Combine(const FString &FileName)
    {
        return ConfigFile.Combine(FileName);
    }

    // 处理输入的配置文件内容字符串，将其解析并应用到当前配置文件对象中
    // @param INIFileContents 包含配置文件内容的字符串
    void ProcessInputFileContents(const FString &INIFileContents)
    {
        ConfigFile.ProcessInputFileContents(INIFileContents);
    }

    // 检查配置文件中是否存在指定的节（section）
    // @param Section 要检查的配置文件节名称（字符串类型）
    // @return 如果配置文件存在且能找到指定节，则返回 true，否则返回 false
    bool HasSection(const FString &Section) const
    {
        return (ConfigFile.Num() > 0) && (ConfigFile.Find(Section)!= nullptr);
    }

    // 如果配置文件中不存在指定的节，则添加该节（添加一个空的配置节对象）
    // @param Section 要添加的配置文件节名称
    void AddSectionIfMissing(const FString &Section)
    {
        if (!HasSection(Section)) {
            ConfigFile.Add(Section, FConfigSection());
        }
    }

    // 将配置文件对象的内容写入磁盘上指定的文件中
    // @param Filename 要写入的目标文件名
    // @return 写入操作是否成功的布尔值
    bool Write(const FString &Filename)
    {
        return ConfigFile.Write(Filename);
    }

    // 获取当前配置文件对象对应的 Unreal 的 FConfigFile 对象（可能用于更底层的操作等）
    // @return 不可修改的（const）FConfigFile 对象引用
    const FConfigFile &GetFConfigFile() const
    {
        return ConfigFile;
    }

    /// @}  结束其他功能函数相关的代码块文档化分组
    // ===========================================================================
    /// @name Get functions  获取配置项相关函数区域的开始标记
    // ===========================================================================
    /// @{

    // 获取配置文件中指定节、指定键对应的整数值，并安全地转换为指定类型存储到目标变量中
    // @param Section 配置文件节名称
    // @param Key 配置文件键名称
    // @param Target 用于接收获取到的整数值的目标变量（引用传递，会被修改），类型由模板参数指定
    template <typename T>
    void GetInt(const TCHAR* Section, const TCHAR* Key, T &Target) const
    {
        int64 Value;
        // 先尝试从配置文件中获取 int64 类型的值
        if (ConfigFile.GetInt64(Section, Key, Value)) {
            // 如果获取成功，调用 SafeCastTo 函数安全地转换为目标类型并赋值给 Target
            SafeCastTo<T>(Value, Target);
        }
    }

    // 获取配置文件中指定节、指定键对应的字符串值，并赋值给目标字符串变量
    // @param Section 配置文件节名称
    // @param Key 配置文件键名称
    // @param Target 用于接收获取到的字符串值的目标变量（引用传递，会被修改）
    void GetString(const TCHAR* Section, const TCHAR* Key, FString &Target) const
    {
        FString Value;
        // 尝试从配置文件中获取字符串值
        if (ConfigFile.GetString(Section, Key, Value)) {
            Target = Value;
        }
    }

    // 获取配置文件中指定节、指定键对应的布尔值，并赋值给目标布尔变量
    // @param Section 配置文件节名称
    // @param Key 配置文件键名称
    // @param Target 用于接收获取到的布尔值的目标变量（引用传递，会被修改）
    void GetBool(const TCHAR* Section, const TCHAR* Key, bool &Target) const
    {
        bool Value;
        // 尝试从配置文件中获取布尔值
        if (ConfigFile.GetBool(Section, Key, Value)) {
            Target = Value;
        }
    }

    // 获取配置文件中指定节、指定键对应的浮点数值，可根据指定的因子进行缩放，并赋值给目标浮点变量
    // @param Section 配置文件节名称
    // @param Key 配置文件键名称
    // @param Target 用于接收获取到的浮点数值的目标变量（引用传递，会被修改）
    // @param Factor 用于缩放获取到的浮点数值的因子，默认值为 1.0f
    void GetFloat(const TCHAR* Section, const TCHAR* Key, float &Target, const float Factor = 1.0f) const
    {
        FString Value;
        // 先尝试从配置文件中获取字符串形式的数值表示
        if (ConfigFile.GetString(Section, Key, Value)) {
            // 将获取到的字符串转换为浮点数，并根据因子进行缩放后赋值给 Target
            Target = Factor * FCString::Atof(*Value);
        }
    }

    // 获取配置文件中指定节、指定键对应的线性颜色值，并赋值给目标线性颜色变量
    // @param Section 配置文件节名称
    // @param Key 配置文件键名称
    // @param Target 用于接收获取到的线性颜色值的目标变量（引用传递，会被修改）
    void GetLinearColor(const TCHAR* Section, const TCHAR* Key, FLinearColor &Target) const
    {
        FString Value;
        // 尝试从配置文件中获取表示线性颜色的字符串值
        if (ConfigFile.GetString(Section, Key, Value)) {
            // 从字符串初始化目标线性颜色对象
            Target.InitFromString(Value);
        }
    }

    /// @}  结束获取配置项相关函数的代码块文档化分组
    // ===========================================================================
    /// @name Set functions  设置配置项相关函数区域的开始标记
    // ===========================================================================
    /// @{

    // 在配置文件中设置指定节、指定键对应的整数值（实际存储为 int64 类型）
    // @param Section 配置文件节名称
    // @param Key 配置文件键名称
    // @param Value 要设置的整数值
    void SetInt(const TCHAR* Section, const TCHAR* Key, const int64 Value)
    {
        ConfigFile.SetInt64(Section, Key, Value);
    }

    // 在配置文件中设置指定节、指定键对应的字符串值
    // @param Section 配置文件节名称
    // @param Key 配置文件键名称
    // @param Value 要设置的字符串值（以 TCHAR* 类型传入）
    void SetString(const TCHAR* Section, const TCHAR* Key, const TCHAR* Value)
    {
        ConfigFile.SetString(Section, Key, Value);
    }

    // 在配置文件中设置指定节、指定键对应的字符串值（以 FString 类型传入）
    // 内部会调用上面的 SetString 函数将 FString 转换为 TCHAR* 后进行设置
    // @param Section 配置文件节名称
    // @param Key 配置文件键名称
    // @param Value 要设置的字符串值（FString 类型）
    void SetString(const TCHAR* Section, const TCHAR* Key, const FString &Value)
    {
        SetString(Section, Key, *Value);
    }

    // 在配置文件中设置指定节、指定键对应的布尔值，实际是以字符串 "True" 或 "False" 形式存储
    // @param Section 配置文件节名称
    // @param Key 配置文件键名称
    // @param Value 要设置的布尔值
    void SetBool(const TCHAR* Section, const TCHAR* Key, const bool Value)
    {
        SetString(Section, Key, Value? TEXT("True") : TEXT("False"));
    }

    // 在配置文件中设置指定节、指定键对应的浮点数值，会先将浮点数转换为字符串形式再进行存储
    // @param Section 配置文件节名称
    // @param Key 配置文件键名称
    // @param Value 要设置的浮点数值
    void SetFloat(const TCHAR* Section, const TCHAR* Key, const float Value)
    {
        SetString(Section, Key, FText::AsNumber(Value).ToString());
    }

    // 在配置文件中设置指定节、指定键对应的线性颜色值，会先将线性颜色对象转换为字符串形式再进行存储
    // @param Section 配置文件节名称
    // @param Key 配置文件键名称
    // @param Value 要设置的线性颜色值（FLinearColor 类型）
    void SetLinearColor(const TCHAR* Section, const TCHAR* Key, const FLinearColor &Value)
    {
        SetString(Section, Key, Value.ToString());
    }

    /// @}  结束设置配置项相关函数的代码块文档化分组

private:
    // FConfigFile 类型的成员变量，用于实际存储和操作配置文件的内容，是整个类功能实现的核心数据对象
    FConfigFile ConfigFile;
};
