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

#ifndef DETECTORGRAPH_INCLUDE_GRAPHINPUTQUEUE_LITE_HPP_
#define DETECTORGRAPH_INCLUDE_GRAPHINPUTQUEUE_LITE_HPP_

#include "graphinputdispatcher.hpp"

#include "dgassert.hpp"

namespace DetectorGraph
{
/**
 * @brief _Internal_ - Provides an bare-bones version of GraphInputQueue
 */
class GraphInputQueue
{
private:
    struct InputQueueNode
    {
        InputQueueNode(uint8_t* aDispatcherStorage)
        : dispatcherStorage(aDispatcherStorage), dispatcher(NULL)
        , next(NULL), busy(false)
        {
        }

        uint8_t* dispatcherStorage;
        GraphInputDispatcherInterface* dispatcher;
        InputQueueNode* next;
        bool busy;
    };

public:
    GraphInputQueue() : mHeadNode(NULL), mTailNode(NULL) {}

    template<class TTopicState>
    void Enqueue(Topic<TTopicState>& aTopic, const TTopicState& aTopicState)
    {
        InputQueueNode* node = GetQueueNode<TTopicState>();

        DG_ASSERT(!node->busy);
        // WARNING: Give how we keep the storage of GraphInputDispatcher and
        // nodes it's impossible to, for a given TTopicState, have two nodes
        // enqueued at the same time. Here we're choosing to assert if we
        // encounter this scenario. In practice this would happen if a
        // TopicState is being FuturePublished faster than it's being consumed
        // - and the only way to achieve that is by calling it twice for the
        // same topic in a single evaluation pass or if two FuturePublished
        // topics are competing for the next evaluation pass and they cause one
        // another to be re-published - this is very contrived and deep.
        //
        // In the weird world where the above is a problem we could clobber the
        // dispatcher with the new data and just not enqueue it. This is not
        // necessarily wrong but violates the current rules, would be
        // unnecessary complexity and would certainly hide graph design bugs.

        node->dispatcher =
            new(node->dispatcherStorage) GraphInputDispatcher<TTopicState>(
                aTopic, aTopicState);

        node->busy = true;
        EnqueueNode(node);
    }

    bool DequeueAndDispatch()
    {
        InputQueueNode* nextNode = DequeueNode();
        if (nextNode)
        {
            GraphInputDispatcherInterface* nextInput = nextNode->dispatcher;

            // Will call Topic->Publish(aTopicState)
            nextInput->Dispatch();

            nextNode->busy = false;

            return true;
        }
        else
        {
            return false;
        }
    }

    bool IsEmpty() const
    {
        return mHeadNode == NULL;
    }

    ~GraphInputQueue()
    {
        InputQueueNode* nextNode = DequeueNode();

        while(nextNode != NULL)
        {
            nextNode->busy = false;
            nextNode = DequeueNode();
        }
    }

private:
    void EnqueueNode(InputQueueNode* node)
    {
        if (mTailNode == NULL)
        {
            mHeadNode = node;
            mTailNode = node;
            mTailNode->next = NULL;
        }
        else
        {
            mTailNode->next = node;
            mTailNode = node;
            mTailNode->next = NULL;
        }
    }

    InputQueueNode* DequeueNode()
    {
        InputQueueNode* tmp = mHeadNode;
        if (mHeadNode)
        {
            if (mHeadNode->next == NULL)
            {
                mTailNode = NULL;
            }
            mHeadNode = mHeadNode->next;
        }
        return tmp;
    }

private:
    InputQueueNode* mHeadNode;
    InputQueueNode* mTailNode;

    template<class TTopicState>
    InputQueueNode* GetQueueNode()
    {
        static uint8_t mInputDispatcherStorage[sizeof(GraphInputDispatcher<TTopicState>)];
        static InputQueueNode node = InputQueueNode(mInputDispatcherStorage);
        return &node;
    }
};

} // namespace DetectorGraph

#endif // DETECTORGRAPH_INCLUDE_GRAPHINPUTQUEUE_LITE_HPP_
