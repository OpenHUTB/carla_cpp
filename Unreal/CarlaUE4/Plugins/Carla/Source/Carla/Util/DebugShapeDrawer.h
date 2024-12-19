//  Copyright (c) 2017 Computer Vision Center (CVC) at the Unive
// de Barcelona (UAB).
//
// 本作品根据 MIT 许可证的条款进行许可。
//有关副本，请参阅 <https://opensource.org/licenses/MIT>。

#编译一次

class UWorld;

namespace carla { namespace rpc { class DebugShape; }}

class FDebugShapeDrawer
{
public:

  explicit FDebugShapeDrawer(UWorld &InWorld) : World(InWorld) {}

  void Draw(const carla::rpc::DebugShape &Shape);

private:

  UWorld &World;
};
