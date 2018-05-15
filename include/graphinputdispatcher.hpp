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

#ifndef DETECTORGRAPH_INCLUDE_GRAPHINPUTDISPATCHER_HPP_
#define DETECTORGRAPH_INCLUDE_GRAPHINPUTDISPATCHER_HPP_

#include "topic.hpp"
#include "vertex.hpp"

namespace DetectorGraph
{
/**
 * @brief _Internal_ - Provide interface for a GraphInputDispatcher
 */
class GraphInputDispatcherInterface
{
public:
    virtual ~GraphInputDispatcherInterface() {}
    virtual void Dispatch() = 0;
};
/**
 * @brief _Internal_ - Push data to the graph
 *
 * It pushes data to a specific topic through a graphInputDispatcher.
 * The pending data will be evaluated in the next round.
 */
template<class T>
class GraphInputDispatcher : public GraphInputDispatcherInterface
{
public:
    GraphInputDispatcher(Topic<T>& aTopic, const T& aData)
:   mTopic(aTopic)
,   mData(aData)
    {
    }

    void Dispatch()
    {
        mTopic.Publish(mData);
    }
private:
    Topic<T>& mTopic;
    const T mData;
};

} // namespace DetectorGraph

#endif // DETECTORGRAPH_INCLUDE_GRAPHINPUTDISPATCHER_HPP_
