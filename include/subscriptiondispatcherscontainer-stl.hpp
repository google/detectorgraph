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

#ifndef DETECTORGRAPH_INCLUDE_SUBSCRIPTIONDISPATCHERCONTAINER_STL_HPP_
#define DETECTORGRAPH_INCLUDE_SUBSCRIPTIONDISPATCHERCONTAINER_STL_HPP_

#include "subscriptiondispatcher.hpp"

#include <vector>

namespace DetectorGraph
{
/**
 * @brief _Internal_ - Manages any number of SubscriptionDispatchers.
 *
 * This class is responsible for creating and owning SubscriptionDispatcher
 * for a particular detector. A detector that subscribes to two Topics will
 * have two SubscriptionDispatchers.
 */
class SubscriptionDispatchersContainer
{
public:
    template<class TTopicState>
    void CreateDispatcher(Topic<TTopicState>* topic, SubscriberInterface<TTopicState>* subscriber)
    {
        SubscriptionDispatcherInterface* dispacher = new SubscriptionDispatcher<TTopicState>(topic, subscriber);
        mInDispatchers.push_back(dispacher);
    }

    const std::vector<SubscriptionDispatcherInterface*>& GetDispatchers() const
    {
        return mInDispatchers;
    }

    const size_t GetSize() const
    {
        return mInDispatchers.size();
    }

    ~SubscriptionDispatchersContainer()
    {
        for (unsigned i = 0; i != mInDispatchers.GetSize(); ++i)
        {
            delete GetDispatchers()[i];
        }
    }
private:
    std::vector<SubscriptionDispatcherInterface*> mInDispatchers;
};

} // namespace DetectorGraph

#endif // DETECTORGRAPH_INCLUDE_SUBSCRIPTIONDISPATCHERCONTAINER_STL_HPP_
