// ��Ȩ���� (c) 2023 �����������δ�ѧ (UAB) ������Ӿ����� (CVC)��
//
// ����Ʒ���� MIT ���֤��������Ȩ��
// ���踱��������� <https://opensource.org/licenses/MIT>��

#pragma once

// ����ͷ�ļ�
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MeshDescription.h"
#include "ProceduralMeshComponent.h"
// Carla C++ ͷ�ļ�

// Carla ���ͷ�ļ�
#include "Carla/Util/ProceduralCustomMesh.h"

#include "MapGenFunctionLibrary.generated.h"

// ������־���
DECLARE_LOG_CATEGORY_EXTERN(LogCarlaMapGenFunctionLibrary, Log, All);

// ���� UMapGenFunctionLibrary ��
UCLASS(BlueprintType)
class CARLA_API UMapGenFunctionLibrary : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()
public:
  // �����������ͼ�ɵ��ú���
  UFUNCTION(BlueprintCallable)
  static UStaticMesh* CreateMesh(
      const FProceduralCustomMesh& Data,
      const TArray<FProcMeshTangent>& ParamTangents,
      UMaterialInstance* MaterialInstance,
      FString MapName,
      FString FolderName,
      FName MeshName);

  // �����ݹ������������ľ�̬����
  static FMeshDescription BuildMeshDescriptionFromData(
      const FProceduralCustomMesh& Data,
      const TArray<FProcMeshTangent>& ParamTangents,
      UMaterialInstance* MaterialInstance );

  // ��ȡ����ī����ͶӰ����ͼ�ɵ��ú���
  UFUNCTION(BlueprintCallable)
  static FVector2D GetTransversemercProjection(float lat, float lon, float lat0, float lon0);

  // ʹ�߳����ߵ���ͼ�ɵ��ú���
  UFUNCTION(BlueprintCallable)
  static void SetThreadToSleep(float seconds);

  // ����ͼ��ˢ����Ⱦ�������ͼ�ɵ��ú���
  UFUNCTION(BlueprintCallable)
  static void FlushRenderingCommandsInBlueprint();

  // ����GEngine����ͼ�ɵ��ú���
  UFUNCTION(BlueprintCallable)
  static void CleanupGEngine();
};