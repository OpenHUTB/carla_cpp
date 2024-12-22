// 引入必要的命名空间
using System;
using System.IO;
using UnrealBuildTool;

// 定义CarlaTools类，它继承自ModuleRules类
public class CarlaTools : ModuleRules
{
    // 定义一个布尔类型的成员变量，用于指示是否使用Omniverse连接器，并初始化为false
    bool bUsingOmniverseConnector = false;

    // 定义一个私有方法，用于检查目标平台是否为Windows
    private bool IsWindows(ReadOnlyTargetRules Target)
    {
        // 返回目标平台是否为Win64或Win32
        return (Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32);
    }

    // CarlaTools类的构造函数，接收一个ReadOnlyTargetRules类型的参数
    public CarlaTools(ReadOnlyTargetRules Target) : base(Target)
    {
        // 设置PCH（预编译头文件）的使用模式为显式或共享PCH
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        // 如果目标平台是Windows，则启用异常处理
        if (IsWindows(Target))
        {
            bEnableExceptions = true;
        }

        // 获取Carla插件的完整路径
        string CarlaPluginPath = Path.GetFullPath(ModuleDirectory);
        // 获取配置目录的完整路径
        string ConfigDir = Path.GetFullPath(Path.Combine(CarlaPluginPath, "../../../../Config/"));
        // 获取可选模块配置文件的完整路径
        string OptionalModulesFile = Path.Combine(ConfigDir, "OptionalModules.ini");
        // 读取可选模块配置文件的所有行
        string[] text = System.IO.File.ReadAllLines(OptionalModulesFile);
        // 遍历每一行，检查是否包含启用Omniverse的指令
        foreach (string line in text)
        {
            if (line.Contains("Omniverse ON"))
            {
                // 输出启用Omniverse连接器的消息
                Console.WriteLine("Enabling OmniverseConnector");
                // 将bUsingOmniverseConnector设置为true
                bUsingOmniverseConnector = true;
                // 向公共和私有定义中添加WITH_OMNIVERSE宏
                PublicDefinitions.Add("WITH_OMNIVERSE");
                PrivateDefinitions.Add("WITH_OMNIVERSE");
            }
        }

        // 添加所需的公共包含路径（此处为空，需根据实际情况添加）
        PublicIncludePaths.AddRange(
            new string[] {
                // ...在此处添加所需的公共包含路径...
            }
        );

        // 添加所需的其他私有包含路径（此处为空，需根据实际情况添加）
        PrivateIncludePaths.AddRange(
            new string[] {
                // ...在此处添加所需的其他私有包含路径...
            }
        );

        // 添加公共依赖模块（这些模块在链接时将被视为公共依赖）
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "ProceduralMeshComponent",
                "MeshDescription",
                "RawMesh",
                "AssetTools",
                // ...在此处添加您静态链接的其他公共依赖项...
            }
        );

        // 添加私有依赖模块（这些模块在链接时将被视为私有依赖）
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "UnrealEd",
                "Blutility",
                "UMG",
                "EditorScriptingUtilities",
                "Landscape",
                "Foliage",
                "FoliageEdit",
                "MeshMergeUtilities",
                "Carla",
                "StaticMeshDescription",
                "PhysXVehicles",
                "Json",
                "JsonUtilities",
                "Networking",
                "Sockets",
                "HTTP",
                "RHI",
                "RenderCore",
                // 注意：MeshMergeUtilities被重复添加了，应该移除一个重复项
                "StreetMapImporting",
                "StreetMapRuntime"
                // ...在此处添加您静态链接的私有依赖项 ...
            }
        );
    }
}
    if(bUsingOmniverseConnector)
    {
      PrivateDependencyModuleNames.AddRange(
        new string[]
        {
          "OmniverseUSD",
          "OmniverseRuntime"
        });
    }

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ...在此处添加您的模块动态加载的任何模块...
			}
			);
		AddCarlaServerDependency(Target);
	}

  private bool UseDebugLibs(ReadOnlyTargetRules Target)
  {
    if (IsWindows(Target))
    {
      // 在 Windows 中，Unreal 即使在调试模式下也使用发布 C++ 运行时 （CRT），因此除非我们重新编译引擎，否则我们无法链接调试库。
      return false;
    }
    else
    {
      return false;
    }
  }

  delegate string ADelegate(string s);

  private void AddBoostLibs(string LibPath)
  {
    string [] files = Directory.GetFiles(LibPath, "*boost*.lib");
    foreach (string file in files)
    {
      PublicAdditionalLibraries.Add(file);
    }
  }


	private void AddCarlaServerDependency(ReadOnlyTargetRules Target)
	{
		string LibCarlaInstallPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../../Carla/CarlaDependencies"));

		ADelegate GetLibName = (string BaseName) => {
			if (IsWindows(Target))
			{
				return BaseName + ".lib";
			}
			else
			{
				return "lib" + BaseName + ".a";
			}
		};

    // 链接依赖项。
    if (IsWindows(Target))
    {
      AddBoostLibs(Path.Combine(LibCarlaInstallPath, "lib"));
      PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", GetLibName("rpc")));

      if (UseDebugLibs(Target))
      {
        PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", GetLibName("carla_server_debug")));
      }
      else
      {
        PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", GetLibName("carla_server")));
      }
    }
    else
    {
      PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", GetLibName("rpc")));
      if (UseDebugLibs(Target))
      {
        PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", GetLibName("carla_server_debug")));
      }
      else
      {
        PublicAdditionalLibraries.Add(Path.Combine(LibCarlaInstallPath, "lib", GetLibName("carla_server")));
      }
    }
    // Include path.
    string LibCarlaIncludePath = Path.Combine(LibCarlaInstallPath, "include");

    PublicIncludePaths.Add(LibCarlaIncludePath);
    PrivateIncludePaths.Add(LibCarlaIncludePath);

    PublicDefinitions.Add("ASIO_NO_EXCEPTIONS");
    PublicDefinitions.Add("BOOST_NO_EXCEPTIONS");
    // PublicDefinitions.Add("LIBCARLA_NO_EXCEPTIONS");
    PublicDefinitions.Add("PUGIXML_NO_EXCEPTIONS");
    PublicDefinitions.Add("BOOST_DISABLE_ABI_HEADERS");
    PublicDefinitions.Add("BOOST_TYPE_INDEX_FORCE_NO_RTTI_COMPATIBILITY");
	}
}
