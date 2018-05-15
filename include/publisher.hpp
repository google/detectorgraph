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

#ifndef DETECTORGRAPH_INCLUDE_PUBLISHER_HPP_
#define DETECTORGRAPH_INCLUDE_PUBLISHER_HPP_

#include "graph.hpp"
#include "topic.hpp"
#include "dgassert.hpp"

namespace DetectorGraph
{
/**
 * @brief Base class that implements a Publisher behavior.
 *
 * Detectors inherit from a number of versions of this class
 * to implement their output set.
 *
 * A class `FooDetector` acquires the "Publisher of BarTopicState"
 * behavior by inheriting `Publisher` templated to `BarTopicState`.
 *
 * @code
class FooDetector :
    public Detector,
    public Publisher<BarTopicState>
{
    // ...
}
 * @endcode
 *
 * This class serves a tiny purpose:
 * - Abstracts the act of Publishing to a Topic into a type-safe method.
 * - Caches the output Topic for this edge.
 * - Document in a clear way the Publishing behavior of an inheriting Detector.
 *
 * The enable the Publishing arc, detectors must call Detector::SetupPublishing:
 * @code
class FooDetector :
    public Detector,
    public Publisher<BarTopicState>
{
    FooDetector(Graph* graph) : Detector(graph)
    {
        SetupPublishing<BarTopicState>(this);
    }
}
 * @endcode
 *
 * If you publish multiple items you will need to either qualify each `Publish`
 *  or include a `using` declaration in your header. See <a href="http://stackoverflow.com/questions/5368862/why-do-multiple-inherited-functions-with-same-name-but-different-signatures-not">StackOverflow</a>
 *  for more information
 *
 */
template<class T>
class Publisher
{
protected:
    Topic<T>* mTopic;

public:

    Publisher() : mTopic(NULL)
    {
    }

    void SetGraph(Graph* aGraph)
    {
        mTopic = aGraph->ResolveTopic<T>();
    }

    /**
     * @brief Publish a new version of T to a Topic
     */
    void Publish(const T& data)
    {
        DG_ASSERT(mTopic);
        mTopic->Publish(data);
    }
};

} // namespace DetectorGraph

#endif // DETECTORGRAPH_INCLUDE_PUBLISHER_HPP_
