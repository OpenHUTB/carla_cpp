// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/Junction.h" // ����Junctionͷ�ļ�
#include "carla/client/Map.h" // ����Mapͷ�ļ�
#include "carla/road/element/Waypoint.h" // ����Waypointͷ�ļ�

namespace carla { // ����carla�����ռ�
namespace client { // ����client�������ռ�

  Junction::Junction(SharedPtr<const Map> parent, const road::Junction *junction) : _parent(parent) { // Junction���캯��
    _bounding_box = junction->GetBoundingBox(); // ��ȡ���洢����ڵı߽��
    _id = junction->GetId(); // ��ȡ���洢����ڵ�ID
  }

  std::vector<std::pair<SharedPtr<Waypoint>, SharedPtr<Waypoint>>> Junction::GetWaypoints( // ��ȡ����ڵ�·��
      road::Lane::LaneType type) const { // �����������������
    return _parent->GetJunctionWaypoints(GetId(), type); // �Ӹ�Map��ȡ�뽻�����ص�·��
  }

  geom::BoundingBox Junction::GetBoundingBox() const { // ��ȡ����ڵı߽��
    return _bounding_box; // ���ش洢�ı߽��
  }

} // namespace client
} // namespace carla

