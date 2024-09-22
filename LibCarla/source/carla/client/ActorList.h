// 版权所有 (c) 2017 巴萨大学计算机视觉中心 (CVC)。  
//   
// 本作品根据 MIT 许可证条款进行许可。  
// 许可证副本请见 <https://opensource.org/licenses/MIT>。  

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
                // 创建一个转换迭代器，用于在迭代时获取每个演员的当前状态。  
                return boost::make_transform_iterator(it, [this](auto& v) {
                    return v.Get(_episode);
                    });
            }

        public:

            /// 根据演员 ID 查找演员。  
            SharedPtr<Actor> Find(ActorId actor_id) const;

            /// 过滤演员列表，只保留类型 ID 匹配 @a wildcard_pattern 的演员。  
            SharedPtr<ActorList> Filter(const std::string& wildcard_pattern) const;

            SharedPtr<Actor> operator[](size_t pos) const {
                return _actors[pos].Get(_episode);
            }

            SharedPtr<Actor> at(size_t pos) const {
                return _actors.at(pos).Get(_episode);
            }

            auto begin() const {
                // 取得演员列表的起始迭代器。  
                return MakeIterator(_actors.begin());
            }

            auto end() const {
                // 取得演员列表的结束迭代器。  
                return MakeIterator(_actors.end());
            }

            bool empty() const {
                // 判断演员列表是否为空。  
                return _actors.empty();
            }

            size_t size() const {
                // 返回演员列表的大小。  
                return _actors.size();
            }

        private:

            friend class World;

            // 构造函数，接受一个 EpisodeProxy 和一个演员的向量。  
            ActorList(detail::EpisodeProxy episode, std::vector<rpc::Actor> actors);

            detail::EpisodeProxy _episode; // 当前 Episode 的引用。  

            std::vector<detail::ActorVariant> _actors; // 存储演员的变体列表。  
        };

    } // namespace client  
} // namespace carla  
