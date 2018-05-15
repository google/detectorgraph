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

#ifndef DETECTORGRAPH_INCLUDE_GRAPHSTATESTORE_HPP_
#define DETECTORGRAPH_INCLUDE_GRAPHSTATESTORE_HPP_

#include <iostream>
#include <map>
#include <stdint.h>

#include "sharedptr.hpp"
#include "topicstate.hpp"
#include "graph.hpp"
#include "statesnapshot.hpp"

namespace DetectorGraph
{

/**
 * @brief A StateSnapshot keeper for DetectorGraph TopicStates.
 *
 * This class is responsible for maintaining a look back queue of previous graph
 * states (in the form of StateSnapshots) in a no-duplication and safe-sharing
 * fashion.
 */
class GraphStateStore
{
public:
    /**
     * @brief Constructs an empty graph store.
     */
    GraphStateStore();

    /**
     * @brief Default Destructor.
     *
     * Note that this destructor doesn't explicitly delete any of the
     * StateSnapshots in mStatesLookbackQueue since that's implicitly
     * handled by the ptr::shared_ptrs.
     */
    ~GraphStateStore();

    /**
     * @brief Takes a new state snapshot and appends it to the look back queue.
     *
     * This method takes a graph output list and combines it with the previous
     * StateSnapshot (if existent) to generate a new StateSnapshot.
     */
    void TakeNewSnapshot(const std::list< ptr::shared_ptr<const TopicState> >& arTopicStates);

    /**
     * @brief Returns a safe shared pointer to the latest complete StateSnapshot
     *
     * Always returns a valid ptr::shared_ptr even though the StateSnapshot might be
     * empty.
     */
    ptr::shared_ptr<const StateSnapshot> GetLastState() const;

    // TODO(DGRAPH-19): APIs for accessing earlier snapshots.

private:
    std::queue< ptr::shared_ptr<const StateSnapshot> > mStatesLookbackQueue;
};

}

#endif // DETECTORGRAPH_INCLUDE_GRAPHSTATESTORE_HPP_
