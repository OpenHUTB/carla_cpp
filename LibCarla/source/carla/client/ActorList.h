// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/detail/ActorVariant.h"   // ����ActorVariantͷ�ļ�

#include <boost/iterator/transform_iterator.hpp>   // ����Boost���е�transform_iterator

#include <vector>  // ����vector��

namespace carla {   // ���������ռ�carla
namespace client {  // ���������ռ�client

  class ActorList : public EnableSharedFromThis<ActorList> {   // ����ActorList�࣬�̳���EnableSharedFromThis
  private:

    template <typename It>  // ģ�庯���������������͵ĵ�����
    auto MakeIterator(It it) const {   // �����任������
      return boost::make_transform_iterator(it, [this](auto &v) {   // ʹ��Boost�����任������
        return v.Get(_episode);  // ��Actor�л�ȡ��ǰepisode
      });
    }

  public:

    /// ����actor_id������Ա
    SharedPtr<Actor> Find(ActorId actor_id) const;   // ������Ա������

    ///���������ģʽƥ�����Ա�б�
    SharedPtr<ActorList> Filter(const std::string &wildcard_pattern) const;   // ������Ա������

    SharedPtr<Actor> operator[](size_t pos) const {   // �����±������
      return _actors[pos].Get(_episode);  // ����λ�û�ȡ��Ա
    }

    SharedPtr<Actor> at(size_t pos) const {  // ����ָ��λ�õ���Ա
      return _actors.at(pos).Get(_episode);  // ʹ��at������ȡ��Ա
    }

    auto begin() const {   // ��ȡ��ʼ������
      return MakeIterator(_actors.begin());  // ����MakeIterator���ɿ�ʼ������
    }

    auto end() const {   // ��ȡ����������
      return MakeIterator(_actors.end());  // ����MakeIterator���ɽ���������
    }

    bool empty() const {   // �����Ա�б��Ƿ�Ϊ��
      return _actors.empty();   // ������Ա�б��Ƿ�Ϊ��
    }

    size_t size() const {   // ��ȡ��Ա�б�Ĵ�С
      return _actors.size();   // ������Ա����
    }

  private:

    friend class World;  // ����World��Ϊ��Ԫ��

    ActorList(detail::EpisodeProxy episode, std::vector<rpc::Actor> actors);  // ���캯������

    detail::EpisodeProxy _episode;   // �洢��ǰ��episode

    std::vector<detail::ActorVariant> _actors;  // �洢��Ա�ı����б�
  };

} // namespace client
} // namespace carla
