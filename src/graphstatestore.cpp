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

#include "graphstatestore.hpp"

namespace DetectorGraph
{

GraphStateStore::GraphStateStore()
{
    ptr::shared_ptr<const StateSnapshot> tZeroState = ptr::shared_ptr<const StateSnapshot>(new StateSnapshot());
    mStatesLookbackQueue.push(tZeroState);
}

GraphStateStore::~GraphStateStore()
{
}

void GraphStateStore::TakeNewSnapshot(const std::list< ptr::shared_ptr<const TopicState> >& arTopicStates)
{
    // do it
    ptr::shared_ptr<const StateSnapshot> newState;

    // mStatesLookbackQueue is ensured to not be empty by the constructor.
    newState = ptr::shared_ptr<const StateSnapshot>(new StateSnapshot(*(mStatesLookbackQueue.back()), arTopicStates));

    mStatesLookbackQueue.push(newState);

    const size_t MaxLookBack = 2;
    if (mStatesLookbackQueue.size() > MaxLookBack)
    {
        mStatesLookbackQueue.pop();
    }
}

ptr::shared_ptr<const StateSnapshot> GraphStateStore::GetLastState() const
{
    // mStatesLookbackQueue is ensured to not be empty by the constructor.
    return mStatesLookbackQueue.back();
}

}
