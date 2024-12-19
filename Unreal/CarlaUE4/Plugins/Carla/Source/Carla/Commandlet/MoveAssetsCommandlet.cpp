// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "MoveAssetsCommandlet.h"
#include "SSTags.h"

// UMoveAssetsCommandlet类的构造函数，用于初始化该类的一些属性。
UMoveAssetsCommandlet::UMoveAssetsCommandlet()
{
    // 设置该命令let（Commandlet，在Unreal Engine中常用于执行特定命令行任务的类）不是客户端相关的，通常意味着它不是在客户端运行的逻辑。
    IsClient = false;
    // 设置该命令let是编辑器相关的，表明它主要在编辑器环境下执行相关操作。
    IsEditor = true;
    // 设置该命令let不是服务器相关的，说明它不涉及服务器端的运行逻辑。
    IsServer = false;
    // 设置将日志输出到控制台，方便在执行过程中查看相关的执行信息、报错等内容。
    LogToConsole = true;
}

#if WITH_EDITORONLY_DATA
// UMoveAssetsCommandlet类的成员函数ParseParams，用于解析传入的参数字符串，提取并整理出与移动资产包相关的参数信息。
FMovePackageParams UMoveAssetsCommandlet::ParseParams(const FString &InParams) const
{
    // 创建一个字符串数组Tokens，用于存储解析命令行参数时分割出来的各个“单词”（token），例如按照空格等分隔符分割后的一个个字符串片段。
    TArray<FString> Tokens;
    // 创建一个字符串数组Params，用于存储解析后的具体参数内容，它会根据一定的规则对Tokens进一步处理得到。
    TArray<FString> Params;

    // 调用ParseCommandLine函数（应该是Unreal Engine中用于解析命令行参数的函数），将传入的参数字符串InParams进行解析，
    // 分割后的结果分别存储到Tokens和Params数组中，具体分割规则取决于该函数的实现逻辑。
    ParseCommandLine(*InParams, Tokens, Params);

    // 创建一个FMovePackageParams结构体对象，用于存储最终解析出来的与资产包移动相关的参数，后续会逐步填充其中的字段。
    FMovePackageParams PackageParams;
    // 使用FParse::Value函数（用于从给定的字符串中提取指定键对应的值）从InParams字符串中提取键为"PackageName="对应的值，
    // 并将提取到的值存储到PackageParams结构体的Name字段中，这个值通常代表要操作的资产包的名称。
    FParse::Value(*InParams, TEXT("PackageName="), PackageParams.Name);

    // 创建一个字符串变量Maps，用于存储后续从参数中提取的地图相关的字符串信息。
    FString Maps;
    // 使用FParse::Value函数从InParams字符串中提取键为"Maps="对应的值，存储到Maps字符串变量中，这个值可能包含多个地图名称，以某种格式组合在一起。
    FParse::Value(*InParams, TEXT("Maps="), Maps);

    // 创建一个字符串数组MapNames，用于存储解析后的各个地图名称。
    TArray<FString> MapNames;
    // 使用Maps字符串的ParseIntoArray函数（按照指定的分隔符将字符串分割为多个子字符串并存储到数组中），
    // 以空格作为分隔符，将Maps字符串中的地图名称分割出来，存储到MapNames数组中，并且设置允许为空字符串（true参数表示允许分割出空字符串元素）。
    Maps.ParseIntoArray(MapNames, TEXT(" "), true);

    // 将解析好的地图名称数组赋值给PackageParams结构体的MapNames字段，完成对FMovePackageParams结构体中地图相关参数的设置。
    PackageParams.MapNames = MapNames;

    // 返回填充好的包含资产包名称和地图名称等相关参数的FMovePackageParams结构体对象，供其他函数使用这些解析后的参数信息。
    return PackageParams;
}

// UMoveAssetsCommandlet类的成员函数MoveAssets，用于根据传入的资产包移动相关参数，实际执行资产的移动操作，是整个资产移动流程的核心协调函数。
void UMoveAssetsCommandlet::MoveAssets(const FMovePackageParams &PackageParams)
{
    // 创建一个UObjectLibrary对象实例（用于加载和管理对象资源的库），指定要加载的对象类型为UStaticMesh（静态网格类型，常用于表示游戏中的模型等资源），
    // 第二个参数false表示不加载所有类型的对象（只加载指定的UStaticMesh类型），GIsEditor是一个全局变量（应该是用于判断是否处于编辑器环境的标识）。
    // 这个库实例创建后用于后续加载、管理相关资产资源。
    AssetsObjectLibrary = UObjectLibrary::CreateLibrary(UStaticMesh::StaticClass(), false, GIsEditor);
    // 将创建的AssetsObjectLibrary对象添加到根对象集合（用于管理对象的生命周期等，添加到根可以防止其被意外垃圾回收等情况）。
    AssetsObjectLibrary->AddToRoot();

    // 开始遍历传入的资产包移动参数中指定的每个地图名称，针对每个地图执行资产移动相关操作，具体的移动操作由MoveAssetsFromMapForSemanticSegmentation函数完成。
    for (const auto &Map : PackageParams.MapNames)
    {
        MoveAssetsFromMapForSemanticSegmentation(PackageParams.Name, Map);
    }
}

// 全局函数MoveFiles，用于将一组资产对象移动到指定的目标路径下，主要涉及对资产对象重命名等相关操作来实现移动效果。
void MoveFiles(const TArray<UObject *> &Assets, const FString &DestPath)
{
    // 检查目标路径的长度是否大于0，确保传入的目标路径是有效的，如果路径长度为0则可能不符合移动资产的要求，会触发断言失败（在调试版本中）。
    check(DestPath.Len() > 0);

    // 使用FModuleManager加载名为"AssetTools"的模块（应该是Unreal Engine中提供资产操作工具的模块），并获取对该模块的引用，用于后续调用模块中的资产相关操作函数。
    FAssetToolsModule &AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
    // 创建一个FAssetRenameData类型的数组AssetsAndNames，用于存储资产对象以及对应的新名称、目标路径等重命名相关信息，后续会填充每个资产的对应信息。
    TArray<FAssetRenameData> AssetsAndNames;
    // 遍历传入的资产对象数组Assets，通过迭代器进行遍历操作。
    for (auto AssetIt = Assets.CreateConstIterator(); AssetIt; ++AssetIt)
    {
        // 获取当前迭代位置的资产对象指针。
        UObject *Asset = *AssetIt;

        // 使用ensure宏（在调试版本中进行条件检查，如果条件不满足会输出错误信息并暂停程序执行）检查资产对象是否有效（不为空指针），
        // 如果资产对象为空指针，则跳过当前循环，不进行后续针对该资产的重命名操作。
        if (!ensure(Asset))
        {
            continue;
        }
        // 使用Emplace函数（类似于push_back，但可以直接在数组中就地构造元素，更高效）向AssetsAndNames数组中添加一个FAssetRenameData对象，
        // 该对象包含了当前资产对象、目标路径以及资产对象原本的名称等信息，用于后续进行资产重命名操作时的参数传递。
        AssetsAndNames.Emplace(Asset, DestPath, Asset->GetName());
    }

    // 检查准备好的AssetsAndNames数组中元素个数是否大于0，即是否有需要进行重命名（移动）的资产对象信息，如果有则执行重命名操作。
    if (AssetsAndNames.Num() > 0)
    {
        // 调用AssetToolsModule获取到的对象的RenameAssets函数（应该是实际执行资产重命名操作的函数），传入包含资产对象和重命名信息的AssetsAndNames数组，
        // 实现将资产对象移动到目标路径下的功能（通过重命名改变资产的存储路径来达到移动效果）。
        AssetToolsModule.Get().RenameAssets(AssetsAndNames);
    }
}

// UMoveAssetsCommandlet类的成员函数MoveAssetsFromMapForSemanticSegmentation，用于从指定的地图中加载资产，并按照语义分割的规则将资产分类移动到相应的目标文件夹中。
void UMoveAssetsCommandlet::MoveAssetsFromMapForSemanticSegmentation(
    const FString &PackageName,
    const FString &MapName)
{
    // 构建源路径字符串，格式为 "/Game/" + 资产包名称 + "/Maps/" + 地图名称，这个路径表示要从中加载资产的原始位置，
    // 后续会根据这个路径从相应的地图文件夹下加载相关的静态网格等资产资源。
    const FString SrcPath = TEXT("/Game/") + PackageName + TEXT("/Maps/") + MapName;
    // 调用AssetsObjectLibrary对象的LoadAssetDataFromPath函数，根据构建好的源路径SrcPath加载该路径下的资产数据信息（但此时还未实际加载资产对象本身），
    // 只是获取资产的相关元数据等描述信息，存储在AssetsObjectLibrary内部的数据结构中。
    AssetsObjectLibrary->LoadAssetDataFromPath(*SrcPath);
    // 调用AssetsObjectLibrary对象的LoadAssetsFromAssetData函数，基于之前加载的资产数据信息，实际加载对应的资产对象到内存中，使得后续可以对这些资产进行操作。
    AssetsObjectLibrary->LoadAssetsFromAssetData();

    // 清空MapContents数组（应该是用于存储当前地图中加载的资产数据信息的数组，之前可能存储了其他地图的相关数据，此处清空以便重新填充当前地图的资产数据）。
    MapContents.Empty();
    // 调用AssetsObjectLibrary对象的GetAssetDataList函数，将加载好的当前地图中的资产数据信息填充到MapContents数组中，方便后续遍历和处理这些资产数据。
    AssetsObjectLibrary->GetAssetDataList(MapContents);
    // 调用AssetsObjectLibrary对象的ClearLoaded函数，清除已经加载的资产对象（可能是释放内存等相关操作，比如在不需要这些资产对象在内存中占用空间时进行清理），
    // 但资产的数据信息依然保留在MapContents数组等相关结构中，以便后续继续基于这些数据进行分类移动等操作。
    AssetsObjectLibrary->ClearLoaded();

    // 创建一个字符串数组DestinationPaths，包含了一系列语义分割相关的目标文件夹名称，这些名称对应不同的语义类别，
    // 例如道路（ROAD）、道路标线（ROADLINE）、地形（TERRAIN）等，资产最终会根据分类移动到这些对应的文件夹下。
    TArray<FString> DestinationPaths = {SSTags::ROAD, SSTags::ROADLINE, SSTags::TERRAIN, SSTags::GRASS, SSTags::SIDEWALK, SSTags::CURB, SSTags::GUTTER};

    // 创建一个TMap类型的映射（键值对容器）AssetDataMap，用于存储根据语义分类后的资产对象信息，键是语义分割的目标文件夹名称（如上述的各种类别名称），
    // 值是对应类别的资产对象数组（UObject *类型的数组，表示属于该语义类别的资产对象集合）。
    TMap<FString, TArray<UObject *>> AssetDataMap;
    // 遍历DestinationPaths数组中的每个目标文件夹名称，为每个名称在AssetDataMap中初始化一个空的资产对象数组，
    // 即先创建好各个语义类别对应的键值对，后续再将资产对象填充到相应的类别数组中。
    for (const auto &Paths : DestinationPaths)
    {
        AssetDataMap.Add(Paths, {});
    }

    // 遍历从当前地图中加载并存储在MapContents数组中的每个资产数据信息，对每个资产进行语义分类并添加到对应的AssetDataMap类别数组中。
    for (const auto &MapAsset : MapContents)
    {
        // 创建一个字符串变量AssetName，用于存储资产的名称，后续会根据资产名称来判断其属于哪个语义类别。
        FString AssetName;
        // 使用CastChecked函数（进行类型安全的强制转换，如果转换失败会抛出异常，确保转换的类型正确性）将当前资产数据对应的资产对象转换为UStaticMesh类型，
        // 获取到指向该静态网格资产的指针，并赋值给MeshAsset变量，方便后续操作该资产对象。
        UStaticMesh *MeshAsset = CastChecked<UStaticMesh>(MapAsset.GetAsset());
        // 将资产对象的名称转换为字符串形式，并存储到AssetName变量中，通过调用AssetName的ToString函数实现。
        MapAsset.AssetName.ToString(AssetName);

        if (SrcPath.Len())
        {
            // 获取当前资产所属的最外层包的名称（通过获取资产对象的最外层对象的名称来得到），存储到CurrentPackageName变量中，
            // 这个名称可以用于判断资产是否属于当前正在处理的源路径下的资产包，确保操作的资产来源正确。
            const FString CurrentPackageName = MeshAsset->GetOutermost()->GetName();

            // 使用ensure宏检查当前资产所属的包名称是否以源路径开头（即是否属于当前要处理的地图所在的资产包），
            // 如果不满足则跳过当前资产的处理，继续处理下一个资产，保证只处理符合要求的资产对象。
            if (!ensure(CurrentPackageName.StartsWith(SrcPath)))
            {
                continue;
            }

            // 根据资产名称中是否包含特定的语义标签（通过SSTags中定义的各种标签常量进行判断），将资产对象添加到对应的AssetDataMap类别数组中，实现语义分类。
            // 如果资产名称包含道路相关标签（R_ROAD1或R_ROAD2），则将该资产添加到"ROAD"语义类别的资产数组中。
            if (AssetName.Contains(SSTags::R_ROAD1) || AssetName.Contains(SSTags::R_ROAD2))
            {
                AssetDataMap[SSTags::ROAD].Add(MeshAsset);
            }
            // 如果资产名称包含道路标线相关标签（R_MARKING1或R_MARKING2），则将该资产添加到"ROADLINE"语义类别的资产数组中。
            else if (AssetName.Contains(SSTags::R_MARKING1) || AssetName.Contains(SSTags::R_MARKING2))
            {
                AssetDataMap[SSTags::ROADLINE].Add(MeshAsset);
            }
            // 如果资产名称包含地形相关标签（R_TERRAIN），则将该资产添加到"TERRAIN"语义类别的资产数组中。
            else if (AssetName.Contains(SSTags::R_TERRAIN))
            {
                AssetDataMap[SSTags::TERRAIN].Add(MeshAsset);
            }
            // 如果资产名称包含人行道相关标签（R_SIDEWALK1或R_SIDEWALK2），则将该资产添加到"SIDEWALK"语义类别的资产数组中。
            else if (AssetName.Contains(SSTags::R_SIDEWALK1) || AssetName.Contains(SSTags::R_SIDEWALK2))
            {
                AssetDataMap[SSTags::SIDEWALK].Add(MeshAsset);
            }
            // 如果资产名称包含路边石相关标签（R_CURB1或R_CURB2），则将该资产添加到"CURB"语义类别的资产数组中。
            else if (AssetName.Contains(SSTags::R_CURB1) || AssetName.Contains(SSTags::R_CURB2))
            {
                AssetDataMap[SSTags::CURB].Add(MeshAsset);
            }
            // 如果资产名称包含排水沟相关标签（R_GUTTER1或R_GUTTER2），则将该资产添加到"GUTTER"语义类别的资产数组中。
            else if (AssetName.Contains(SSTags::R_GUTTER1) || AssetName.Contains(SSTags::R_GUTTER2))
            {
                AssetDataMap[SSTags::GUTTER].Add(MeshAsset);
            }
            // 如果资产名称不包含上述任何特定标签，则默认将其添加到"TERRAIN"语义类别的资产数组中，作为一种兜底的分类方式。
            else
            {
                AssetDataMap[SSTags::TERRAIN].Add(MeshAsset);
            }
        }
    }

    // 遍历AssetDataMap映射中的每个键值对（每个语义类别及其对应的资产对象数组），将资产对象移动到相应的语义分割目标文件夹下，
    // 具体的移动操作通过调用MoveFiles函数实现，传入该类别下的资产对象数组和构建好的目标路径字符串。
    for (const auto &Elem : AssetDataMap)
    {
        FString DestPath = TEXT("/Game/") + PackageName + TEXT("/Static/") + Elem.Key + "/" + MapName;
        MoveFiles(Elem.Value, DestPath);
    }
}

// UMoveAssetsCommandlet类的成员函数Main，作为整个命令let的入口函数，负责协调参数解析和资产移动的整体流程，通常在命令行启动该命令let时会执行这个函数。
int32 UMoveAssetsCommandlet::Main(const FString &Params)
{
    // 首先调用ParseParams函数解析传入的参数字符串Params，获取到包含资产包名称、地图名称等相关参数的FMovePackageParams结构体对象，用于后续操作。
    FMovePackageParams PackageParams = ParseParams(Params);

// 调用MoveAssets函数，传入解析好的资产包移动参数PackageParams，开始实际执行资产的移动操作流程，按照参数指定的要求对资产进行分类移动等操作。
// 这里把包含了资产包名称、地图名称等关键信息的PackageParams结构体对象传递给MoveAssets函数，MoveAssets函数会依据这些参数，
// 去加载相应的资产库、遍历指定地图中的资产，然后按照语义分割等规则对资产进行分类整理，并最终将资产移动到对应的目标位置（通过调用其他相关的辅助函数来完成具体的移动操作）。
MoveAssets(PackageParams);

// 函数执行完毕后，返回0表示程序正常结束，通常在命令行程序或者类似的执行环境中，返回0是一种约定俗成的表示成功执行完毕的方式，告知调用者该命令let执行过程没有出现错误，顺利完成了资产移动相关的任务。
return 0;
}
#endif
