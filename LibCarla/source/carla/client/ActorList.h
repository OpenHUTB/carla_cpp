// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/client/detail/ActorVariant.h"

#include <boost/iterator/transform_iterator.hpp>

#include <vector>

namespace carla {
namespace client {

  class ActorList : public EnableSharedFromThis<ActorList> {
  private:

    template <typename It>
    auto MakeIterator(It it) const {
      return boost::make_transform_iterator(it, [this](auto &v) {
        return v.Get(_episode);
      });
    }

  public:

    /// 根据 id 寻找参与者。
    SharedPtr<Actor> Find(ActorId actor_id) const;

    /// 过滤类型 ID 与 @a wildcard_pattern 匹配的参与者列表。
    SharedPtr<ActorList> Filter(const std::string &wildcard_pattern) const;

    SharedPtr<Actor> operator[](size_t pos) const {
      return _actors[pos].Get(_episode);
    }

    SharedPtr<Actor> at(size_t pos) const {
      return _actors.at(pos).Get(_episode);
    }

    auto begin() const {
      return MakeIterator(_actors.begin());
    }

    auto end() const {
      return MakeIterator(_actors.end());
    }

    bool empty() const {
      return _actors.empty();
    }

    size_t size() const {
      return _actors.size();
    }

  private:

    friend class World;

    ActorList(detail::EpisodeProxy episode, std::vector<rpc::Actor> actors);

    detail::EpisodeProxy _episode;

    std::vector<detail::ActorVariant> _actors;
  };

} // namespace client
} // namespace carla
