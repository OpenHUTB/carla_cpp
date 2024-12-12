#include "USDCARLAInterface.h"
#include "OmniversePxr.h"
#include "OmniverseUSDImporter.h"
#include "OmniverseUSDHelper.h"

bool UUSDCARLAInterface::ImportUSD(
    const FString& Path, const FString& Dest, 
    bool bImportUnusedReferences, bool bImportAsBlueprint)
{
    // 创建导入设置对象
    FOmniverseImportSettings Settings;
    // 设置是否导入未使用的引用
    Settings.bImportUnusedReferences = bImportUnusedReferences;
    // 设置是否作为蓝图导入
    Settings.bImportAsBlueprint = bImportAsBlueprint;
    // 调用 FOmniverseUSDImporter 的 LoadUSD 函数进行 USD 导入操作
    return FOmniverseUSDImporter::LoadUSD(Path, Dest, Settings);
}

TArray<FUSDCARLALight> UUSDCARLAInterface::GetUSDLights(const FString& Path)
{
    // 存储结果的灯光数组
    TArray<FUSDCARLALight> Result;
    // 从路径加载 USD 阶段
    pxr::UsdStageRefPtr Stage = FOmniverseUSDHelper::LoadUSDStageFromPath(Path);
    // 存储灯光原语的向量
    std::vector<pxr::UsdPrim> LightPrims;
    // 遍历阶段中的所有原语
    const auto& PrimRange = Stage->Traverse();
    for (const auto& Prim : PrimRange) {
        // 如果原语是灯光类型
        if (Prim.IsA<pxr::UsdLuxLight>()) {
            LightPrims.push_back(Prim);
        }
    }
    for (pxr::UsdPrim& LightPrim : LightPrims)
    {
        // 创建一个灯光对象
        pxr::UsdLuxLight Light(LightPrim);
        // 获取原语的名称
        std::string StdName = LightPrim.GetName();

        pxr::GfVec3f PxColor;
        pxr::VtValue vtValue;
        // 获取灯光的颜色属性
        if (Light.GetColorAttr().Get(&vtValue)) {
            PxColor = vtValue.Get<pxr::GfVec3f>();
        }
        // 创建变换缓存
        pxr::UsdGeomXformCache Cache;
        // 获取原语的本地到世界变换矩阵
        pxr::GfMatrix4d Transform = Cache.GetLocalToWorldTransform(LightPrim);
        // 提取变换矩阵的平移部分
        pxr::GfVec3d PxLocation = Transform.ExtractTranslation();

        // 将 std::string 转换为 FString 作为名称
        FString Name(StdName.size(), UTF8_TO_TCHAR(StdName.c_str()));
        // 将颜色转换为 FLinearColor
        FLinearColor Color(PxColor[0], PxColor[1], PxColor[2]);
        // 转换位置并乘以缩放因子
        FVector Location(PxLocation[0], -PxLocation[1], PxLocation[2]);
        Location *= 100.f;

        // 创建灯光参数对象
        FUSDCARLALight LightParams {Name, Location, Color};
        // 将灯光参数添加到结果数组
        Result.Add(LightParams);
    }
    return Result;
}

TArray<FUSDCARLAWheelData> UUSDCARLAInterface::GetUSDWheelData(const FString& Path)
{
    // 从路径加载 USD 阶段
    pxr::UsdStageRefPtr Stage = FOmniverseUSDHelper::LoadUSDStageFromPath(Path);

    // 定义车轮的 USD 物理路径
    const std::string UsdPhysxWheelPath = "/vehicle/_physx/_physxWheels/";
    // 定义悬挂的 USD 物理路径
    const std::string UsdPhysxSuspensionPath = "/vehicle/_physx/_physxSuspensions/";
    // 获取浮点数属性值的 lambda 函数
    auto GetFloatAttributeValue = [](pxr::UsdPrim& Prim, const std::string& AttrName) -> float
    {
        // 获取指定名称的属性
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
    // 从关系中获取原语的 lambda 函数
    auto GetPrimFromRelationship = [&](
        pxr::UsdRelationship& Relationship) -> pxr::UsdPrim
    {
        std::vector<pxr::SdfPath> Targets;
        // 获取关系的目标
        Relationship.GetTargets(&Targets);
        if (!Targets.size())
        {
            return pxr::UsdPrim();
        }
        // 从阶段中根据路径获取原语
        return Stage->GetPrimAtPath(Targets.front());
    };
    // 解析车轮数据的 lambda 函数
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
            // 打印日志信息
            UE_LOG(LogOmniverseUsd, Warning, TEXT("Wheel prim fail"));
            return false;
        }
        // 定义车轮、轮胎和悬挂的关系
        pxr::UsdRelationship WheelRelationship;
        pxr::UsdRelationship TireRelationship;
        pxr::UsdRelationship SuspensionRelationship;
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
            // 打印日志信息
            UE_LOG(LogOmniverseUsd, Warning, TEXT("Relationships fail: %d %d %d"), 
                bool(WheelRelationship), bool(TireRelationship), bool(SuspensionRelationship));
            return false;
        }
        // 从关系中获取相应的原语
        pxr::UsdPrim PhysxWheelPrim = GetPrimFromRelationship(WheelRelationship);
        pxr::UsdPrim PhysxTirePrim = GetPrimFromRelationship(TireRelationship);
        pxr::UsdPrim PhysxSuspensionlPrim = GetPrimFromRelationship(SuspensionRelationship);

        if (!PhysxWheelPrim ||!PhysxTirePrim ||!PhysxSuspensionlPrim)
        {
            // 打印日志信息
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

        // 打印 USD 数据日志信息
        UE_LOG(LogOmniverseUsd, Warning, TEXT("USD values: %f %f %f %f %f %f %f %f %f"), 
        OutWheelData.MaxBrakeTorque, OutWheelData.MaxHandBrakeTorque, OutWheelData.MaxSteerAngle,
        OutWheelData.SpringStrength, OutWheelData.MaxCompression, OutWheelData.MaxDroop,
        OutWheelData.LateralStiffnessX, OutWheelData.LateralStiffnessY, OutWheelData.LongitudinalStiffness);

        return true;
    };
    // 初始化默认的车轮数据，若有物理数据将被 ParseWheelData 覆盖
    FUSDCARLAWheelData Wheel0 = 
        {8.f, 16.f, 0.61f, 0.f, 0.f, 0.1f, 0.f, 20.f, 3000.f}; // 左前轮
    FUSDCARLAWheelData Wheel1 = 
        {8.f, 16.f, 0.61f, 0.f, 0.f, 0.1f, 0.f, 20.f, 3000.f}; // 右前轮
    FUSDCARLAWheelData Wheel2 = 
        {8.f, 16.f, 0.f,   0.f, 5.f, 0.1f, 0.f, 20.f, 3000.f}; // 左后轮
    FUSDCARLAWheelData Wheel3 = 
        {8.f, 16.f, 0.f,   0.f, 5.f, 0.1f, 0.f, 20.f, 3000.f}; // 右后轮
    // 解析每个车轮的数据
    ParseWheelData("wheel_0", Wheel0);
    ParseWheelData("wheel_1", Wheel1);
    ParseWheelData("wheel_2", Wheel2);
    ParseWheelData("wheel_3", Wheel3);

    return {Wheel0, Wheel1, Wheel2, Wheel3};
}
