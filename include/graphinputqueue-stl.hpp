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

#ifndef DETECTORGRAPH_INCLUDE_GRAPHINPUTQUEUE_STL_HPP_
#define DETECTORGRAPH_INCLUDE_GRAPHINPUTQUEUE_STL_HPP_

#include "graphinputdispatcher.hpp"

#include <queue>

namespace DetectorGraph
{
/**
 * @brief _Internal_ - Provides an STL implementation of GraphInputQueue
 */
class GraphInputQueue
{
public:
    GraphInputQueue() : mInputQueue() {}

    template<class TTopicState>
    void Enqueue(Topic<TTopicState>& aTopic, const TTopicState& aTopicState)
    {
        mInputQueue.push(new GraphInputDispatcher<TTopicState>(aTopic, aTopicState));
    }

    bool DequeueAndDispatch()
    {
        if (!mInputQueue.empty())
        {
            GraphInputDispatcherInterface* nextInput = mInputQueue.front();
            mInputQueue.pop();

            // Will call Topic->Publish(aTopicState)
            nextInput->Dispatch();

            delete nextInput;

            return true;
        }
        else
        {
            return false;
        }
    }

    bool IsEmpty() const
    {
        return mInputQueue.empty();
    }

    ~GraphInputQueue()
    {
        while (!mInputQueue.empty())
        {
            GraphInputDispatcherInterface* nextInput = mInputQueue.front();
            mInputQueue.pop();
            delete nextInput;
        }
    }

private:
    std::queue< GraphInputDispatcherInterface* > mInputQueue;
};

} // namespace DetectorGraph

#endif // DETECTORGRAPH_INCLUDE_GRAPHINPUTQUEUE_STL_HPP_
