// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/client/detail/ActorVariant.h" // ����ActorVariantͷ�ļ�

#include "carla/client/detail/ActorFactory.h" // ����ActorFactoryͷ�ļ�
#include "carla/client/ActorList.h" // ����ActorListͷ�ļ�

namespace carla { // ����carla�����ռ�
namespace client { // ����client�������ռ�
namespace detail { // ����detail�������ռ�

  void ActorVariant::MakeActor(EpisodeProxy episode) const { // ����MakeActor����������һ��EpisodeProxy����
    _value = detail::ActorFactory::MakeActor( // ����ActorFactory��MakeActor��������һ����Ա
        episode, // ���뵱ǰ��episode
        boost::variant2::get<rpc::Actor>(std::move(_value)), // ��_variant�л�ȡrpc::Actor�����ƶ�
        GarbageCollectionPolicy::Disabled); // �����������ղ���Ϊ����
  }

} // namespace detail
} // namespace client
} // namespace carla

