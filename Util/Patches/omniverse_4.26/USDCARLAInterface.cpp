#include "USDCARLAInterface.h"
#include "OmniversePxr.h"
#include "OmniverseUSDImporter.h"
#include "OmniverseUSDHelper.h"


bool UUSDCARLAInterface::ImportUSD(
    const FString& Path, const FString& Dest, 
    bool bImportUnusedReferences, bool bImportAsBlueprint)
{
    // 创建一个 FOmniverseImportSettings 对象，用于存储导入设置
    FOmniverseImportSettings Settings;
    // 设置是否导入未使用的引用
    Settings.bImportUnusedReferences = bImportUnusedReferences;
    // 设置是否将导入内容作为蓝图导入
    Settings.bImportAsBlueprint = bImportAsBlueprint;
    // 调用 FOmniverseUSDImporter 类的 LoadUSD 函数进行 USD 导入操作，传入路径、目标和导入设置，返回导入结果
    return FOmniverseUSDImporter::LoadUSD(Path, Dest, Settings);
}

TArray<FUSDCARLALight> UUSDCARLAInterface::GetUSDLights(const FString& Path)
{
    // 存储从 USD 文件中提取的灯光信息的数组
    TArray<FUSDCARLALight> Result;
    // 从指定路径加载 USD 阶段
    pxr::UsdStageRefPtr Stage = FOmniverseUSDHelper::LoadUSDStageFromPath(Path);
    // 存储找到的灯光原语的向量
    std::vector<pxr::UsdPrim> LightPrims;
    // 遍历 USD 阶段中的所有原语
    const auto& PrimRange = Stage->Traverse();
    for (const auto& Prim : PrimRange) {
        // 如果原语是灯光原语（pxr::UsdLuxLight 类型），将其添加到 LightPrims 向量中
        if (Prim.IsA<pxr::UsdLuxLight>()) {
            LightPrims.push_back(Prim);
        }
    }
    // 遍历存储的灯光原语
    for (pxr::UsdPrim& LightPrim : LightPrims)
    {
        // 创建一个 UsdLuxLight 对象
        pxr::UsdLuxLight Light(LightPrim);
        // 获取原语的名称
        std::string StdName = LightPrim.GetName();

        // 存储灯光的颜色信息
        pxr::GfVec3f PxColor;
        // 存储属性值
        pxr::VtValue vtValue;
        // 获取灯光的颜色属性值
        if (Light.GetColorAttr().Get(&vtValue)) {
            PxColor = vtValue.Get<pxr::GfVec3f>();
        }
        // 创建一个用于存储变换信息的缓存
        pxr::UsdGeomXformCache Cache;
        // 获取灯光原语的本地到世界的变换矩阵
        pxr::GfMatrix4d Transform = Cache.GetLocalToWorldTransform(LightPrim);
        // 从变换矩阵中提取平移信息
        pxr::GfVec3d PxLocation = Transform.ExtractTranslation();

        // 将原语名称从 std::string 转换为 FString
        FString Name(StdName.size(), UTF8_TO_TCHAR(StdName.c_str()));
        // 将颜色信息从 pxr::GfVec3f 转换为 FLinearColor
        FLinearColor Color(PxColor[0], PxColor[1], PxColor[2]);
        // 转换位置信息并乘以 100 进行缩放
        FVector Location(PxLocation[0], -PxLocation[1], PxLocation[2]);
        Location *= 100.f;

        // 创建一个 FUSDCARLALight 对象存储灯光信息
        FUSDCARLALight LightParams {Name, Location, Color};
        // 将灯光信息添加到结果数组中
        Result.Add(LightParams);
    }
    return Result;
}

TArray<FUSDCARLAWheelData> UUSDCARLAInterface::GetUSDWheelData(const FString& Path)
{
    // 从指定路径加载 USD 阶段
    pxr::UsdStageRefPtr Stage = FOmniverseUSDHelper::LoadUSDStageFromPath(Path);

    // 存储 USD 中车轮物理对象的路径
    const std::string UsdPhysxWheelPath = "/vehicle/_physx/_physxWheels/";
    // 存储 USD 中悬挂物理对象的路径
    const std::string UsdPhysxSuspensionPath = "/vehicle/_physx/_physxSuspensions/";
    // 定义一个 lambda 函数，用于获取指定原语的浮点数属性值
    auto GetFloatAttributeValue = [](pxr::UsdPrim& Prim, const std::string& AttrName) -> float
    {
        // 获取指定原语的指定属性
        pxr::UsdAttribute Attribute = Prim.GetAttribute(pxr::TfToken(AttrName));
        if(!Attribute)
        {
            return 0.f;
        }
        float Value = 0.f;
        // 获取属性的值
        Attribute.Get(&Value);
        return Value;
    };
    // 定义一个 lambda 函数，用于从关系中获取原语
    auto GetPrimFromRelationship = [&](
        pxr::UsdRelationship& Relationship) -> pxr::UsdPrim
    {
        // 存储关系的目标
        std::vector<pxr::SdfPath> Targets;
        // 获取关系的目标列表
        Relationship.GetTargets(&Targets);
        if (!Targets.size())
        {
            return pxr::UsdPrim();
        }
        // 从 USD 阶段中根据目标列表的第一个元素获取原语
        return Stage->GetPrimAtPath(Targets.front());
    };
    // 定义一个 lambda 函数，用于解析车轮数据
    auto ParseWheelData = [&](
        const std::string& WheelName, 
        FUSDCARLAWheelData& OutWheelData) -> bool
    {
        // 构建车轮原语的路径
        pxr::SdfPath WheelPath(UsdPhysxWheelPath + WheelName);
        // 根据路径获取车轮原语
        pxr::UsdPrim WheelPrim = Stage->GetPrimAtPath(WheelPath);
        if(!WheelPrim)
        {
            // 输出日志信息表示车轮原语获取失败
            UE_LOG(LogOmniverseUsd, Warning, TEXT("Wheel prim fail"));
            return false;
        }
        // 存储车轮、轮胎和悬挂的关系
        pxr::UsdRelationship WheelRelationship;
        pxr::UsdRelationship TireRelationship;
        pxr::UsdRelationship SuspensionRelationship;
        // 遍历车轮原语的属性
        for (pxr::UsdProperty& Property : WheelPrim.GetProperties())
        {
            FString Name (Property.GetBaseName().GetText());
            if(Name == "wheel")
            {
                // 找到车轮关系
                WheelRelationship = Property.As<pxr::UsdRelationship>();
            }
            if(Name == "tire")
            {
                // 找到轮胎关系
                TireRelationship = Property.As<pxr::UsdRelationship>();
            }
            if(Name == "suspension")
            {
                // 找到悬挂关系
                SuspensionRelationship = Property.As<pxr::UsdRelationship>();
            }
        }
        if(!WheelRelationship ||!TireRelationship ||!SuspensionRelationship)
        {
            // 输出日志信息表示关系获取失败
            UE_LOG(LogOmniverseUsd, Warning, TEXT("Relationships fail: %d %d %d"), 
                bool(WheelRelationship), bool(TireRelationship), bool(SuspensionRelationship));
            return false;
        }
        // 从车轮关系中获取物理车轮原语
        pxr::UsdPrim PhysxWheelPrim = GetPrimFromRelationship(WheelRelationship);
        // 从轮胎关系中获取物理轮胎原语
        pxr::UsdPrim PhysxTirePrim = GetPrimFromRelationship(TireRelationship);
        // 从悬挂关系中获取物理悬挂原语
        pxr::UsdPrim PhysxSuspensionlPrim = GetPrimFromRelationship(SuspensionRelationship);

        if (!PhysxWheelPrim ||!PhysxTirePrim ||!PhysxSuspensionlPrim)
        {
            // 输出日志信息表示原语获取失败
            UE_LOG(LogOmniverseUsd, Warning, TEXT("Prims fail: %d %d %d"), 
                bool(PhysxWheelPrim), bool(PhysxTirePrim), bool(PhysxSuspensionlPrim));
            return false;
        }
        // 获取车轮的最大制动扭矩
        OutWheelData.MaxBrakeTorque = 
            GetFloatAttributeValue(PhysxWheelPrim, "physxVehicleWheel:maxBrakeTorque");
        // 获取车轮的最大手刹扭矩
        OutWheelData.MaxHandBrakeTorque = 
            GetFloatAttributeValue(PhysxWheelPrim, "physxVehicleWheel:maxHandBrakeTorque");
        // 获取车轮的最大转向角度
        OutWheelData.MaxSteerAngle = 
            GetFloatAttributeValue(PhysxWheelPrim, "physxVehicleWheel:maxSteerAngle");
        // 获取悬挂的弹簧强度
        OutWheelData.SpringStrength = 
            GetFloatAttributeValue(PhysxSuspensionlPrim, "physxVehicleSuspension:springStrength");
        // 获取悬挂的最大压缩量
        OutWheelData.MaxCompression = 
            GetFloatAttributeValue(PhysxSuspensionlPrim, "physxVehicleSuspension:maxCompression");
        // 获取悬挂的最大下沉量
        OutWheelData.MaxDroop = 
            GetFloatAttributeValue(PhysxSuspensionlPrim, "physxVehicleSuspension:maxDroop");
        // 获取轮胎的横向刚度 X
        OutWheelData.LateralStiffnessX = 
            GetFloatAttributeValue(PhysxTirePrim, "physxVehicleTire:latStiffX");
        // 获取轮胎的横向刚度 Y
        OutWheelData.LateralStiffnessY = 
            GetFloatAttributeValue(PhysxTirePrim, "physxVehicleTire:latStiffY");
        // 获取轮胎的纵向刚度
        OutWheelData.LongitudinalStiffness = 
            GetFloatAttributeValue(
            PhysxTirePrim, "physxVehicleTire:longitudinalStiffnessPerUnitGravity");

        // 输出日志信息，显示 USD 数据的值
        UE_LOG(LogOmniverseUsd, Warning, TEXT("USD values: %f %f %f %f %f %f %f %f %f"), 
        OutWheelData.MaxBrakeTorque, OutWheelData.MaxHandBrakeTorque, OutWheelData.MaxSteerAngle,
        OutWheelData.SpringStrength, OutWheelData.MaxCompression, OutWheelData.MaxDroop,
        OutWheelData.LateralStiffnessX, OutWheelData.LateralStiffnessY, OutWheelData.LongitudinalStiffness);

        return true;
    };
    // 初始化默认的车轮数据，用于左前轮
    FUSDCARLAWheelData Wheel0 = 
        {8.f, 16.f, 0.61f, 0.f, 0.f, 0.1f, 0.f, 20.f, 3000.f}; 
    // 初始化默认的车轮数据，用于右前轮
    FUSDCARLAWheelData Wheel1 = 
        {8.f, 16.f, 0.61f, 0.f, 0.f, 0.1f, 0.f, 20.f, 3000.f}; 
    // 初始化默认的车轮数据，用于左后轮
    FUSDCARLAWheelData Wheel2 = 
        {8.f, 16.f, 0.f,   0.f, 5.f, 0.1f, 0.f, 20.f, 3000.f}; 
    // 初始化默认的车轮数据，用于右后轮
    FUSDCARLAWheelData Wheel3 = 
        {8.f, 16.f, 0.f,   0.f, 5.f, 0.1f, 0.f, 20.f, 3000.f}; 
    // 解析第一个车轮的数据
    ParseWheelData("wheel_0", Wheel0);
    // 解析第二个车轮的数据
    ParseWheelData("wheel_1", Wheel1);
    // 解析第三个车轮的数据
    ParseWheelData("wheel_2", Wheel2);
    // 解析第四个车轮的数据
    ParseWheelData("wheel_3", Wheel3);

    // 返回存储四个车轮数据的数组
    return {Wheel0, Wheel1, Wheel2, Wheel3};
}
