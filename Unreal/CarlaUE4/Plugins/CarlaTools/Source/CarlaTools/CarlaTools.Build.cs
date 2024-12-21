// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using System.IO;
using UnrealBuildTool;

public class CarlaTools : ModuleRules
{
// 定义一个布尔类型的成员变量bUsingOmniverseConnector，并初始化为false
  bool bUsingOmniverseConnector = false;
  private bool IsWindows(ReadOnlyTargetRules Target)
  {
    return (Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32);
  }

	public CarlaTools(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

    // PrivatePCHHeaderFile = "Carla.h";

    if (IsWindows(Target))
    {
      bEnableExceptions = true;
    }

    string CarlaPluginPath = Path.GetFullPath( ModuleDirectory );
    string ConfigDir =  Path.GetFullPath(Path.Combine(CarlaPluginPath, "../../../../Config/"));
    string OptionalModulesFile = Path.Combine(ConfigDir, "OptionalModules.ini");
    string[] text = System.IO.File.ReadAllLines(OptionalModulesFile);
    foreach (string line in text)
    {
      if (line.Contains("Omniverse ON"))
      {
        Console.WriteLine("Enabling OmniverseConnector");
        bUsingOmniverseConnector = true;
        PublicDefinitions.Add("WITH_OMNIVERSE");
        PrivateDefinitions.Add("WITH_OMNIVERSE");
      }
    }

		PublicIncludePaths.AddRange(
			new string[] {
				// ...在此处添加所需的公共包含路径...
			}
			);


		PrivateIncludePaths.AddRange(
			new string[] {
				//...在此处添加所需的其他私有包含路径...
			}
			);


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
        "Core",
        "ProceduralMeshComponent",
        "MeshDescription",
        "RawMesh",
        "AssetTools"
				// ...在此处添加您静态链接的其他公共依赖项...
			}
			);


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
        "MeshMergeUtilities",
        "StreetMapImporting",
        "StreetMapRuntime"
				// ...在此处添加您静态链接的私有依赖项 ...
			}
			);
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
