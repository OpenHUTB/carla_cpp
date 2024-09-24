// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/ActorList.h"  // ����ActorList���ͷ�ļ�

#include "carla/StringUtil.h"  // �����ַ����������ͷ�ļ�
#include "carla/client/detail/ActorFactory.h"  // �����ɫ�������ͷ�ļ�

#include <iterator>  // �����������صı�׼��

namespace carla {
namespace client {

   ActorList::ActorList(  // ActorList���캯��
       detail::EpisodeProxy episode,  // ����ĳ����������
       std::vector<rpc::Actor> actors)  // ����Ĳ������б�
      : _episode(std::move(episode)),  // �ƶ����崫�ݳ�������
        _actors(std::make_move_iterator(actors.begin()), std::make_move_iterator(actors.end())) {}  // ʹ���ƶ���������ʼ���������б�

   SharedPtr<Actor> ActorList::Find(const ActorId actor_id) const {  // ����ָ��ID�Ĳ�����
     for (auto& actor : _actors) {  // �������в�����
        if (actor_id == actor.GetId()) {     // ����ҵ�ƥ���ID
          return actor.Get(_episode);  // ���ز����ߵĹ���ָ��
        }
     }
     return nullptr;  // ���δ�ҵ������ؿ�ָ��
   }

   SharedPtr<ActorList> ActorList::Filter(const std::string& wildcard_pattern) const {  // ����ͨ���ģʽ���˲�����
     SharedPtr<ActorList> filtered(new ActorList(_episode, {}));  // ����һ���µ�ActorList���ڴ�Ź��˺�Ĳ�����
     for (auto&& actor : _actors) {  // �������в�����
        if (StringUtil::Match(actor.GetTypeId(), wildcard_pattern)) {  // ���������������ͨ���ƥ��
           filtered->_actors.push_back(actor);  // ��ƥ��Ĳ����߼��뵽���˺���б���
        }
     }
     return filtered;  // ���ع��˺�Ĳ������б�
   }

} // namespace client
} // namespace carla
