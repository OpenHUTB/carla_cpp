// ��Ȩ���� (c) 2017 ������ѧ������Ӿ����� (CVC)��  
//   
// ����Ʒ���� MIT ���֤���������ɡ�  
// ���֤������� <https://opensource.org/licenses/MIT>��  

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
                // ����һ��ת���������������ڵ���ʱ��ȡÿ����Ա�ĵ�ǰ״̬��  
                return boost::make_transform_iterator(it, [this](auto& v) {
                    return v.Get(_episode);
                    });
            }

        public:

            /// ������Ա ID ������Ա��  
            SharedPtr<Actor> Find(ActorId actor_id) const;

            /// ������Ա�б�ֻ�������� ID ƥ�� @a wildcard_pattern ����Ա��  
            SharedPtr<ActorList> Filter(const std::string& wildcard_pattern) const;

            SharedPtr<Actor> operator[](size_t pos) const {
                return _actors[pos].Get(_episode);
            }

            SharedPtr<Actor> at(size_t pos) const {
                return _actors.at(pos).Get(_episode);
            }

            auto begin() const {
                // ȡ����Ա�б����ʼ��������  
                return MakeIterator(_actors.begin());
            }

            auto end() const {
                // ȡ����Ա�б�Ľ�����������  
                return MakeIterator(_actors.end());
            }

            bool empty() const {
                // �ж���Ա�б��Ƿ�Ϊ�ա�  
                return _actors.empty();
            }

            size_t size() const {
                // ������Ա�б�Ĵ�С��  
                return _actors.size();
            }

        private:

            friend class World;

            // ���캯��������һ�� EpisodeProxy ��һ����Ա��������  
            ActorList(detail::EpisodeProxy episode, std::vector<rpc::Actor> actors);

            detail::EpisodeProxy _episode; // ��ǰ Episode �����á�  

            std::vector<detail::ActorVariant> _actors; // �洢��Ա�ı����б�  
        };

    } // namespace client  
} // namespace carla  
