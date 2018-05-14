// Copyright 2017 Nest Labs, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef DETECTORGRAPH_INCLUDE_TOPICREGISTRY_LITE_HPP_
#define DETECTORGRAPH_INCLUDE_TOPICREGISTRY_LITE_HPP_

#include "topic.hpp"
#include "topicstate.hpp"
#include "dgassert.hpp"
#include <detectorgraphliteconfig.hpp>

namespace DetectorGraph
{
/**
 * @brief _Internal_ - A limited-size Container for Topic<T>s
 *
 * Graph owns this to manage all the registered topics
 * This Container uses a single pointer array to store
 * BaseTopic* indexed by the topic's public ID.
 */
class TopicRegistry
{
    BaseTopic* registry[DetectorGraphConfig::kMaxNumberOfTopics];
public:
    TopicRegistry() : registry() {}
    template<class TTopicState> Topic<TTopicState>* Resolve()
    {
#if __cplusplus >= 201103L
        static_assert(std::is_base_of<TopicState, TTopicState>::value,
            "Trying to Resolve non-Topic type.");
#endif
        DG_ASSERT(TopicState::GetId<TTopicState>() != TopicState::kAnonymousTopicState);
        DG_ASSERT(TopicState::GetId<TTopicState>() < DetectorGraphConfig::kMaxNumberOfTopics);

        // The assert below will fail if a detector is added without having all
        // its dependencies (Publishing & Subscribed Topics) added beforehand.
        // For the Lite version of the DetectorGraph the suggested pattern is
        // to instantiate all Topics before Detectors and then add them
        // following the Topologic sort order.
        DG_ASSERT(registry[TopicState::GetId<TTopicState>()] != NULL);
        return static_cast<Topic<TTopicState>*>(registry[TopicState::GetId<TTopicState>()]);
    }

    template<class TTopicState> void Register(Topic<TTopicState>* obj)
    {
#if __cplusplus >= 201103L
        static_assert(std::is_base_of<TopicState, TTopicState>::value,
            "Trying to Register non-Topic type.");
#endif
        DG_ASSERT(TopicState::GetId<TTopicState>() != TopicState::kAnonymousTopicState);

        // The below will fail if you have more Topics than kMaxNumberOfTopics.
        // Bump that config value when necessary (or link both sizes when
        // possible)
        DG_ASSERT(TopicState::GetId<TTopicState>() < DetectorGraphConfig::kMaxNumberOfTopics);
        DG_ASSERT(registry[TopicState::GetId<TTopicState>()] == NULL);
        registry[TopicState::GetId<TTopicState>()] = obj;
    }
};

} // namespace DetectorGraph

#endif // DETECTORGRAPH_INCLUDE_TOPICREGISTRY_LITE_HPP_
