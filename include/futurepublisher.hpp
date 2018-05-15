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

#ifndef DETECTORGRAPH_INCLUDE_FUTUREPUBLISHER_HPP_
#define DETECTORGRAPH_INCLUDE_FUTUREPUBLISHER_HPP_

#include "graph.hpp"
#include "dgassert.hpp"

namespace DetectorGraph
{

/**
 * @brief Publish data to the graph for future evaluation.
 *
 * Detectors inherit from a @ref FuturePublisher<T> to implement the
 * behavior of being a Publisher of T for a future/next evaluation.
 * This differs from a normal Publisher in a very fundamental way, it
 * enables a detector to publish something to an upstream topic - thus
 * constructing a feedback loop.
 * Normally, publishing to an upstream topic would create a directed
 * cycle in the graph thus making it impossible to be topologically
 * sorted.
 *
 * Using FuturePublisher a detector can implement a feedback loop
 * while still making sure the topological nature of graph evaluation
 * is preserved.
 *
 * A class `FooDetector` acquires the "FuturePublisher of BarTopicState"
 * behavior by inheriting `FuturePublisher` templated to `BarTopicState`.
 *
 * This is a template class and so it must be kept minimal to prevent code
 * bloat. This class serves a tiny purpose:
 * - Document in a clear way the "FuturePublishing" behavior of an inheriting
 * Detector.
 * - Caches a pointer to the @ref Graph
 * - Abstracts the act of Publishing to the Graph into a type-safe method.
 *
 * The only requirement to use the @ref FuturePublisher functionality is to
 * call Detector::SetupFuturePublishing from the inheriting Detector's constructor:
 *
 * Below is an [example](@ref counterwithreset.cpp):
 * @snippet counterwithreset.cpp Reset Detector
 *
 * When implementing feedback loops in a graph one should also consider
 * DetectorGraph::Lag as in some cases it's more general and extensible.
 *
 */
template<class T>
class FuturePublisher
{
public:

    FuturePublisher() : mGraph(NULL)
    {
    }

    void SetGraph(Graph* aGraph)
    {
        mGraph = aGraph;
    }

    /**
     * @brief Publish a new version of T to the Graph for future evaluation.
     */
    void PublishOnFutureEvaluation(const T& aData)
    {
        DG_ASSERT(mGraph);
        mGraph->PushData<T>(aData);
    }

protected:
    Graph* mGraph;
};

} // namespace DetectorGraph

#endif // DETECTORGRAPH_INCLUDE_FUTUREPUBLISHER_HPP_
