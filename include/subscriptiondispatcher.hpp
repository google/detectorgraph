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

#ifndef DETECTORGRAPH_INCLUDE_SUBSCRIPTIONDISPATCHER_HPP_
#define DETECTORGRAPH_INCLUDE_SUBSCRIPTIONDISPATCHER_HPP_

#include "topic.hpp"
#include "vertex.hpp"

namespace DetectorGraph
{
/**
 * @brief _Internal_ - Provide interface for a SubscriptionDispatcher
 */
class SubscriptionDispatcherInterface
{
public:
    virtual ~SubscriptionDispatcherInterface() {}
    virtual Vertex* GetTopicVertex() = 0;
    virtual void Dispatch() = 0;
};
/**
 * @brief _Internal_ - Implements the data-out edge from a topic to one of its subscriber.
 *
 * Topics aggregate data and provide functionality do dispatch data but does
 * not embody the programmatic link between topic and subscriber - this class
 * does that.
 */
template<class T>
class SubscriptionDispatcher : public SubscriptionDispatcherInterface
{
public:
    /**
     * @brief Constructor
     *
     * @param aTopic a specific topic this dispatcher manages
     * @param aSubscriber a subscriber to consume the data
     */
    SubscriptionDispatcher(Topic<T>* aTopic, SubscriberInterface<T>* aSubscriber)
    {
        mTopic = aTopic;
        mSubscriber = aSubscriber;
    }

    void Dispatch()
    {
        mTopic->DispatchIntoSubscriber(mSubscriber);
    }

    Vertex* GetTopicVertex()
    {
        return mTopic;
    }
private:
    Topic<T>* mTopic;
    SubscriberInterface<T>* mSubscriber;
};

} // namespace DetectorGraph

#endif // DETECTORGRAPH_INCLUDE_SUBSCRIPTIONDISPATCHER_HPP_
