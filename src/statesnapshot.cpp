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

#include "statesnapshot.hpp"

#include "dgassert.hpp"
#include "dglogging.hpp"

namespace DetectorGraph
{

StateSnapshot::StateSnapshot()
{
    mStateVersion = 0;
}

StateSnapshot::StateSnapshot(const StateSnapshot& arPreviousState, const std::list< ptr::shared_ptr<const TopicState> >& arTopicStates)
{
    // by-value copy will do a shallow & safe copy of all ptr::shared_ptrs
    mStateStore = arPreviousState.mStateStore;
    UpdateValues(arTopicStates);
    mStateVersion = arPreviousState.mStateVersion + 1;
}

void StateSnapshot::UpdateValues(const std::list< ptr::shared_ptr<const TopicState> >& arTopicStates)
{
    // Used for detection of consecutive/repeated named topics being published on the same evaluation
    TopicStateIdType previousTopicStateID = TopicState::kAnonymousTopicState;

    typedef std::list< ptr::shared_ptr<const TopicState> >::const_iterator TopicStatePointerIterator;
    for (TopicStatePointerIterator tDataIt = arTopicStates.begin();
        tDataIt != arTopicStates.end();
        ++tDataIt)
    {
        const ptr::shared_ptr<const TopicState> tpTopicState = *tDataIt;
        if (tpTopicState->GetId() != TopicState::kAnonymousTopicState)
        {
            /* A malformed graph could publish more than one named TopicState
             * with the same name on the same evaluation pass. This is not
             * supported by StateSnapshot's map (and the second would just
             * clobber the first one). In order to detect that bad design
             * this check looks for consecutive TopicStates with matching
             * names. The logic here relies on the fact that arTopicStates
             * is composed by concatenating Topics one at a time so that
             * repeated TopicStates would come consecutively.
             * Thus if this check fails, multiple named TopicStates were
             * published on same eval Pass */
            if (tpTopicState->GetId() == previousTopicStateID)
            {
                // LCOV_EXCL_START
                DG_LOG("!!!!!!!!!!!! Duplicate Detected (GetId = %d, previousTopicStateID = %d, Topic %s)",
                    (int)tpTopicState->GetId(), (int)previousTopicStateID, tpTopicState->GetName());
                // LCOV_EXCL_STOP
            } // LCOV_EXCL_LINE

            // Uncomment line below to re-enable paranoid check for development.
            // DG_ASSERT(tpTopicState->GetId() != previousTopicStateID);

            previousTopicStateID = tpTopicState->GetId();

            // Just copying a ptr::shared_ptr around :)
            mStateStore[tpTopicState->GetId()] = tpTopicState;
        }
    }
}

StateSnapshot::~StateSnapshot()
{
}

ptr::shared_ptr<const TopicState> StateSnapshot::GetState(TopicStateIdType aId) const
{
    if (mStateStore.count(aId) > 0)
    {
        return mStateStore.at(aId);
    }

    return ptr::shared_ptr<const TopicState>();
}

void StateSnapshot::GetTopicStates(std::list< ptr::shared_ptr<const TopicState> >& aOutTopicStateList) const
{
    typedef std::map< TopicStateIdType, ptr::shared_ptr<const TopicState> >::const_iterator StateStoreIterator;
    for (StateStoreIterator stateIt = mStateStore.begin();
        stateIt != mStateStore.end();
        ++stateIt)
    {
        aOutTopicStateList.push_back(stateIt->second);
    }
}

size_t StateSnapshot::GetMapSize() const
{
    return mStateStore.size();
}

unsigned int StateSnapshot::GetStateVersion() const
{
    return mStateVersion;
}

}
