// 版权所有 （c） 2017 巴塞罗那自治大学 （UAB） 计算机视觉中心 （CVC）。
//
// 本作品根据 MIT 许可证的条款进行许可。
//有关副本，请参阅 <https://opensource.org/licenses/MIT>。

#include "Carla.h"
#include "Carla/Game/CarlaGameInstance.h"

#include "Carla/Settings/CarlaSettings.h"

// 定义UCarlaGameInstance类的构造函数
UCarlaGameInstance::UCarlaGameInstance() {
  CarlaSettings = CreateDefaultSubobject<UCarlaSettings>(TEXT("CarlaSettings"));// 创建一个UCarlaSettings的默认子对象，设置其名称为"CarlaSettings"
  Recorder = CreateDefaultSubobject<ACarlaRecorder>(TEXT("Recorder"));// 创建一个ACarlaRecorder的默认子对象，设置其名称为"Recorder"
  CarlaEngine.SetRecorder(Recorder);// 将Recorder对象设置为CarlaEngine的记录器

  check(CarlaSettings != nullptr);// 检查CarlaSettings是否成功创建，确保其指针不为空
  CarlaSettings->LoadSettings();// 加载设置，通常这会从配置文件或其他来源读取配置信息
  CarlaSettings->LogSettings();// 打印加载的设置，用于调试和验证设置是否正确
}

UCarlaGameInstance::~UCarlaGameInstance() = default;// 定义析构函数，C++中的析构函数通常用于清理资源
