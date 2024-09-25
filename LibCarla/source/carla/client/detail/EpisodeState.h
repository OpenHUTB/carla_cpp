// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once // ��ֹͷ�ļ����ظ�����

#include "carla/Iterator.h" // ���������ͷ�ļ�
#include "carla/ListView.h" // �����б���ͼͷ�ļ�
#include "carla/NonCopyable.h" // ���벻�ɸ������ͷ�ļ�
#include "carla/client/ActorSnapshot.h" // ��������߿���ͷ�ļ�
#include "carla/client/Timestamp.h" // ����ʱ���ͷ�ļ�
#include "carla/geom/Vector3DInt.h" // ������ά��������ͷ�ļ�
#include "carla/sensor/data/RawEpisodeState.h" // ����ԭʼ�缯״̬����ͷ�ļ�

#include <boost/optional.hpp> // ����Boost��ѡ����ͷ�ļ�

#include <memory> // ��������ָ��ͷ�ļ�
#include <unordered_map> // ��������ӳ��ͷ�ļ�

namespace carla { // ����carla�����ռ�
namespace client { // ����client�������ռ�
namespace detail { // ����detail�������ռ�

  /// ��ʾĳһ֡�����в����ߵ�״̬
  class EpisodeState
    : public std::enable_shared_from_this<EpisodeState>, // ����������ָ��
      private NonCopyable { // ��ֹ����

      using SimulationState = sensor::s11n::EpisodeStateSerializer::SimulationState; // ����ģ��״̬����

  public:

    // ���캯�������ܾ缯ID
    explicit EpisodeState(uint64_t episode_id) : _episode_id(episode_id) {}

    // ���캯��������ԭʼ�缯״̬
    explicit EpisodeState(const sensor::data::RawEpisodeState &state);

    // ��ȡ�缯ID
    auto GetEpisodeId() const {
      return _episode_id;
    }

    // ��ȡ��ǰ֡��
    auto GetFrame() const {
      return _timestamp.frame;
    }

    // ��ȡʱ���
    const auto &GetTimestamp() const {
      return _timestamp;
    }

    // ��ȡģ��״̬
    SimulationState GetsimulationState() const {
      return _simulation_state;
    }

    // ����ͼ�Ƿ����仯
    bool HasMapChanged() const {
      return (_simulation_state & SimulationState::MapChange) != SimulationState::None;
    }

    // �����ո����Ƿ������
    bool IsLightUpdatePending() const {
      return (_simulation_state & SimulationState::PendingLightUpdate)  != 0;
    }

    // ����Ƿ����ָ���Ĳ����߿���
    bool ContainsActorSnapshot(ActorId actor_id) const {
      return _actors.find(actor_id) != _actors.end();
    }

    // ��ȡָ�������ߵĿ���
    ActorSnapshot GetActorSnapshot(ActorId id) const {
      ActorSnapshot state; // ���������߿��ն���
      CopyActorSnapshotIfPresent(id, state); // ���ƿ��գ�������ڣ�
      return state; // ���ؿ���
    }

    // ��ȡָ�������ߵĿ��գ�������ڣ�
    boost::optional<ActorSnapshot> GetActorSnapshotIfPresent(ActorId id) const {
      boost::optional<ActorSnapshot> state; // ������ѡ����
      CopyActorSnapshotIfPresent(id, state); // ���ƿ��գ�������ڣ�
      return state; // ���ؿ�ѡ����
    }

    // ��ȡ���в�����ID
    auto GetActorIds() const {
      return MakeListView( // �����б���ͼ
          iterator::make_map_keys_const_iterator(_actors.begin()), // ��ȡ������ID������
          iterator::make_map_keys_const_iterator(_actors.end())); // ��ȡ������ID������
    }

    // ��ȡ����������
    size_t size() const {
      return _actors.size(); // ���ز���������
    }

    // ���ز����߿��յĿ�ʼ������
    auto begin() const {
      return iterator::make_map_values_const_iterator(_actors.begin()); // ���ز����߿���ֵ�Ŀ�ʼ������
    }

    // ���ز����߿��յĽ���������
    auto end() const {
      return iterator::make_map_values_const_iterator(_actors.end()); // ���ز����߿���ֵ�Ľ���������
    }

  private:

    // ����ָ�������ߵĿ��գ�������ڣ�
    template <typename T>
    void CopyActorSnapshotIfPresent(ActorId id, T &value) const {
      auto it = _actors.find(id); // ���Ҳ�����
      if (it != _actors.end()) { // ����ҵ���
        value = it->second; // ���ƿ���
      }
    }

    const uint64_t _episode_id; // �洢�缯ID

    const Timestamp _timestamp; // �洢ʱ���

    geom::Vector3DInt _map_origin; // �洢��ͼԭ��

    SimulationState _simulation_state; // �洢ģ��״̬

    std::unordered_map<ActorId, ActorSnapshot> _actors; // �洢�����߿��յ�����ӳ��
  };

} // namespace detail
} // namespace client
} // namespace carla

