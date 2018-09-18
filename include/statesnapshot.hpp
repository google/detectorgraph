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

#ifndef DETECTORGRAPH_INCLUDE_STATESNAPSHOT_HPP_
#define DETECTORGRAPH_INCLUDE_STATESNAPSHOT_HPP_

#include <map>
#include <list>
#include <stdint.h>

#include "sharedptr.hpp"
#include "topicstate.hpp"

namespace DetectorGraph
{

/**
 * @brief The collection of TopicStates that represents the graph state so far.
 *
 * Responsible for conveying and composing a complete state set in the form of
 * a collection of TopicState shared_ptrs.
 */
class StateSnapshot
{
public:
    /**
     * @brief T=0 Constructor
     *
     * Builds an initial and empty StateSnapshot.
     */
    StateSnapshot();

    /**
     * @brief T=0 Priming Constructor
     *
     * Builds a prime StateSnapshot from a TopicState list.
     */
    StateSnapshot(const std::list< ptr::shared_ptr<const TopicState> >& arTopicStates);

    /**
     * @brief T>0 Constructor
     *
     * Builds a StateSnapshot from a previous StateSnapshot and an output list.
     */
    StateSnapshot(const StateSnapshot& arPreviousState, const std::list< ptr::shared_ptr<const TopicState> >& arTopicStates);

    /**
     * @brief Destructor
     *
     * Note that the shared ptrs held by a snapshot are implicitly released by
     * normal shared_ptr destructors.
     */
    ~StateSnapshot();

    /**
     * @brief Returns a TopicState for a given Id
     *
     * Or an empty shared_ptr in case no state with that Id exists.
     */
    ptr::shared_ptr<const TopicState> GetState(TopicStateIdType aId) const;

    /**
     * @brief Returns the specific TopicState for a given its type
     *
     * Or an empty shared_ptr in case no state with that Id exists.
     */
    template<class T>
    ptr::shared_ptr<const T> GetState() const
    {
        return ptr::static_pointer_cast<const T>(GetState(DetectorGraph::TopicState::GetId<T>()));
    }

    /**
     * @brief Returns the number of TopicStates in the Snapshot
     */
    size_t GetMapSize() const;

    /**
     * @brief Returns the version of this snapshot.
     *
     * A Snapshot version is a unique positive integer that is incremented at
     * every new Snapshot creation.
     */
    unsigned int GetStateVersion() const;

    /**
     * @brief Gets TopicStates stored in this snapshot
     *
     * Fills aOutTopicStateList with all public TopicStates stored in this Snapshot.
     * This can be useful for merging the contents of this snapshot into another.
     */
    void GetTopicStates(std::list< ptr::shared_ptr<const TopicState> >& aOutTopicStateList) const;

private:
    void UpdateValues(const std::list< ptr::shared_ptr<const TopicState> >& arTopicStates);

private:
    std::map< TopicStateIdType, ptr::shared_ptr<const TopicState> > mStateStore;
    unsigned int mStateVersion;
};

}

#endif // DETECTORGRAPH_INCLUDE_STATESNAPSHOT_HPP_
