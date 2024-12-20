/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

#include <fbxsdk.h>

// declare global
FbxManager*   gSdkManager = NULL;

// materials
FbxSurfacePhong* gMatRoad;
FbxSurfacePhong* gMatSidewalk;
FbxSurfacePhong* gMatCross;
FbxSurfacePhong* gMatGrass;
FbxSurfacePhong* gMatBlock;

#ifdef IOS_REF
	#undef  IOS_REF
	#define IOS_REF (*(gSdkManager->GetIOSettings()))
#endif

// Create a material that will be applied to a polygon
FbxSurfacePhong* CreateMaterial(FbxScene* pScene, char *name)
{
    // Create material
    FbxSurfacePhong* lMaterial = FbxSurfacePhong::Create(pScene, name);

    return lMaterial;
}

bool Find(const char *name, const char *str)
{
    size_t lenName = strlen(name);
    size_t lenStr = strlen(str);

    if (lenName == 0 || lenStr == 0 || lenStr > lenName) return false;

    std::string strName(name);
    std::string strSub(str);

    return (strName.find(strSub) != std::string::npos);
}

void SetMaterials(FbxNode* pNode)
{
	if (!pNode) return;

    FbxSurfacePhong* mat = gMatBlock;

    // only for mesh nodes
    FbxMesh* lMesh = pNode->GetMesh();
    if(lMesh)
    {
        // remove
        pNode->RemoveAllMaterials();
        // check nomenclature
        const char *name = pNode->GetName();
        if (Find(name, "Road_Road") || Find(name, "Roads_Road"))
            mat = gMatRoad;
        else if (Find(name, "Road_Marking") || Find(name, "Roads_Marking"))
            mat = gMatRoad;
        else if (Find(name, "Road_Curb") || Find(name, "Roads_Curb"))
            mat = gMatRoad;
        else if (Find(name, "Road_Gutter") || Find(name, "Roads_Gutter"))
            mat = gMatRoad;
        else if (Find(name, "Road_Sidewalk") || Find(name, "Roads_Sidewalk"))
            mat = gMatSidewalk;
        else if (Find(name, "Road_Crosswalk") || Find(name, "Roads_Crosswalk"))
            mat = gMatCross;
        else if (Find(name, "Road_Grass") || Find(name, "Roads_Grass"))
            mat = gMatGrass;

        printf("Node %s : %s\n", name, mat->GetName());
        pNode->AddMaterial(mat);
    }

    //recursively traverse each node in the scene
    int i, lCount = pNode->GetChildCount();
    for (i = 0; i < lCount; i++)
    {
        SetMaterials(pNode->GetChild(i));
    }
}

// Creates an importer object, and uses it to
// import a file into a scene.
bool LoadScene(
               FbxManager* pSdkManager,  // Use this memory manager...
               FbxScene* pScene,            // to import into this scene
               const char* pFilename         // the data from this file.
               )
{
    int lFileMajor, lFileMinor, lFileRevision;
    int lSDKMajor,  lSDKMinor,  lSDKRevision;
    bool lStatus;

    // Get the version number of the FBX files generated by the
    // version of FBX SDK that you are using.
    FbxManager::GetFileFormatVersion(lSDKMajor, lSDKMinor, lSDKRevision);

    // Create an importer.
    FbxImporter* lImporter = FbxImporter::Create(pSdkManager,"");

    // Initialize the importer by providing a filename.
    const bool lImportStatus = lImporter->Initialize(pFilename, -1, pSdkManager->GetIOSettings() );

    // Get the version number of the FBX file format.
    lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);

    if( !lImportStatus )  // Problem with the file to be imported
    {
        FbxString error = lImporter->GetStatus().GetErrorString();
        printf("Call to FbxImporter::Initialize() failed.");
        printf("Error returned: %s", error.Buffer());

        if (lImporter->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion)
        {
            printf("FBX version number for this FBX SDK is %d.%d.%d",
                lSDKMajor, lSDKMinor, lSDKRevision);
            printf("FBX version number for file %s is %d.%d.%d",
                pFilename, lFileMajor, lFileMinor, lFileRevision);
        }

        return false;
    }

    if (lImporter->IsFBX())
    {
        IOS_REF.SetBoolProp(IMP_FBX_MATERIAL,        false);
        IOS_REF.SetBoolProp(IMP_FBX_TEXTURE,         false);
        IOS_REF.SetBoolProp(IMP_FBX_LINK,            false);
        IOS_REF.SetBoolProp(IMP_FBX_SHAPE,           false);
        IOS_REF.SetBoolProp(IMP_FBX_GOBO,            false);
        IOS_REF.SetBoolProp(IMP_FBX_ANIMATION,       false);
        IOS_REF.SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, false);
    }

    // Import the scene.
    lStatus = lImporter->Import(pScene);

    // Destroy the importer
    lImporter->Destroy();

    return lStatus;
}

// Exports a scene to a file
// SaveScene函数用于将给定的FBX场景保存为指定格式的文件
// 它接受一系列参数来控制保存的相关设置，并根据操作结果返回成功（true）或失败（false）
// 参数说明如下：
// pSdkManager：指向FbxManager对象的指针，FbxManager是FBX SDK的核心管理类，用于管理整个FBX相关的资源、插件等操作，在这里作为保存场景操作的基础管理器，提供必要的上下文环境和功能支持。
// pScene：指向FbxScene对象的指针，代表了要保存的FBX场景，包含了场景中的所有模型、动画、材质等相关数据，是实际要保存到文件中的内容主体。
// pFilename：以字符串形式指定了要保存的文件路径及文件名，例如可以是"example.fbx"这样的形式，用于明确保存的目标位置和文件名称。
// pFileFormat：指定了保存文件时所采用的文件格式，不同的整数值对应不同的FBX支持的格式选项，通过这个参数可以控制生成的文件格式符合特定的需求。
// pEmbedMedia：布尔类型的参数，用于决定是否将相关的媒体资源（比如纹理图片等）嵌入到保存的FBX文件中，如果为true则嵌入，为false则不嵌入，具体取决于使用场景的需求。
bool SaveScene(
               FbxManager* pSdkManager,
               FbxScene* pScene,
               const char* pFilename,
               int pFileFormat,
               bool pEmbedMedia
               )
{
    bool lStatus = true;

    // Create an exporter.
    FbxExporter* lExporter = FbxExporter::Create(pSdkManager, "");

    pFileFormat = pSdkManager->GetIOPluginRegistry()->FindWriterIDByDescription("Alias OBJ (*.obj)");

    // Initialize the exporter by providing a filename.
    if(lExporter->Initialize(pFilename, pFileFormat, pSdkManager->GetIOSettings()) == false)
    {
        printf("Call to FbxExporter::Initialize() failed.");
        printf("Error returned: %s", lExporter->GetStatus().GetErrorString());
        return false;
    }

    if (pSdkManager->GetIOPluginRegistry()->WriterIsFBX(pFileFormat))
    {
        // Export options determine what kind of data is to be imported.
        // The default (except for the option eEXPORT_TEXTURE_AS_EMBEDDED)
        // is true, but here we set the options explicitly.
        IOS_REF.SetBoolProp(EXP_FBX_MATERIAL,        true);
        IOS_REF.SetBoolProp(EXP_FBX_TEXTURE,         true);
        IOS_REF.SetBoolProp(EXP_FBX_EMBEDDED,        pEmbedMedia);
        IOS_REF.SetBoolProp(EXP_FBX_SHAPE,           true);
        IOS_REF.SetBoolProp(EXP_FBX_GOBO,            true);
        IOS_REF.SetBoolProp(EXP_FBX_ANIMATION,       true);
        IOS_REF.SetBoolProp(EXP_FBX_GLOBAL_SETTINGS, true);
    }

    // get root node of the fbx scene
    FbxNode* lRootNode = pScene->GetRootNode();

    // rotate
    lRootNode->LclRotation.Set(FbxVector4(-90.0, 0.0, 0.0));

    // set materials following nomenclature of RoadRunner
    SetMaterials(lRootNode);

    // Export the scene.
    lStatus = lExporter->Export(pScene);

    // Destroy the exporter.
    lExporter->Destroy();

    return lStatus;
}

int main(int argc, char **argv)
{
    // Creates an instance of the SDK manager.
    gSdkManager = FbxManager::Create();

	// create an IOSettings object
	FbxIOSettings * ios = FbxIOSettings::Create(gSdkManager, IOSROOT );
	gSdkManager->SetIOSettings(ios);

	// Create a scene
	FbxScene* lScene = FbxScene::Create(gSdkManager,"");

    // import
    bool r = LoadScene(gSdkManager, lScene, argv[1]);
    if(!r)
    {
        printf("------- Import failed ----------------------------");
        // Destroy the scene
		lScene->Destroy();
        return 0;
    }

    gMatRoad     = CreateMaterial(lScene, "road");
    gMatSidewalk = CreateMaterial(lScene, "sidewalk");
    gMatCross    = CreateMaterial(lScene, "crosswalk");
    gMatGrass    = CreateMaterial(lScene, "grass");
    gMatBlock    = CreateMaterial(lScene, "block");

    // export
    r = SaveScene(gSdkManager, lScene, argv[2], -1,	false);
    if(!r)
        printf("------- Export failed ----------------------------");

	// destroy the scene
	lScene->Destroy();

    // Destroys an instance of the SDK manager
    if (gSdkManager) gSdkManager->Destroy();

    return 1;
}
