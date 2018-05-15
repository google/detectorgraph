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

#ifndef DETECTORGRAPH_INCLUDE_SUBSCRIPTIONDISPATCHERCONTAINER_LITE_HPP_
#define DETECTORGRAPH_INCLUDE_SUBSCRIPTIONDISPATCHERCONTAINER_LITE_HPP_

#include "subscriptiondispatcher.hpp"
#include "dgassert.hpp"
#include "detectorgraphliteconfig.hpp"

namespace DetectorGraph
{

/**
 * @brief _Internal_ - Manages a number of SubscriptionDispatchers
 */
class SubscriptionDispatchersContainer
{
public:

    SubscriptionDispatchersContainer() : mNumInDispatchers()
    {
    }

    template<class TTopicState>
    void CreateDispatcher(Topic<TTopicState>* topic, SubscriberInterface<TTopicState>* subscriber)
    {
        // TODO
        DG_ASSERT(sizeof(SubscriptionDispatcher<TTopicState>) == sizeof(SubscriptionDispatcher<TopicState>));
        uint8_t* dispatcherStorage = &(mDispatchersStorage[mNumInDispatchers * sizeof(SubscriptionDispatcher<TopicState>)]);

        SubscriptionDispatcherInterface* dispatcher = new(dispatcherStorage) SubscriptionDispatcher<TTopicState>(topic, subscriber);

        // The below will fail if one of your Detectors has more "in" edges
        // than kMaxNumberOfInEdges. That value should be the max number of
        // subscriptions by any detector.
        // Bump that config value when necessary.
        DG_ASSERT(mNumInDispatchers < DetectorGraphConfig::kMaxNumberOfInEdges);
        mInDispatchers[mNumInDispatchers++] = dispatcher;
    }

    SubscriptionDispatcherInterface* const (& GetDispatchers() const)[DetectorGraphConfig::kMaxNumberOfInEdges]
    {
        return mInDispatchers;
    }

    const size_t GetSize() const
    {
        return mNumInDispatchers;
    }

    ~SubscriptionDispatchersContainer()
    {
    }
private:
    uint8_t mDispatchersStorage[DetectorGraphConfig::kMaxNumberOfInEdges * sizeof(SubscriptionDispatcher<TopicState>)];
    SubscriptionDispatcherInterface* mInDispatchers[DetectorGraphConfig::kMaxNumberOfInEdges];
    size_t mNumInDispatchers;
};

} // namespace DetectorGraph

#endif // DETECTORGRAPH_INCLUDE_SUBSCRIPTIONDISPATCHERCONTAINER_LITE_HPP_
