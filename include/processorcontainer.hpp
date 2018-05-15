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

#ifndef DETECTORGRAPH_INCLUDE_PROCESSORCONTAINER_HPP_
#define DETECTORGRAPH_INCLUDE_PROCESSORCONTAINER_HPP_

#include "graph.hpp"

namespace DetectorGraph
{

/**
 * @brief A Base class for a basic Graph container.
 *
 * Below is an example of how to implement a ProcessorContainer:
 *
 @snippet helloworld.cpp ProcessorContainer
 *
 * And an example of how to invoke it:
 @snippet helloworld.cpp Using ProcessorContainer
 *
 * This is the simplest way to build detector graphs and is used throughout the examples.
 */

class ProcessorContainer
{
public:
    ProcessorContainer()
    : mGraph()
    {
    }

    /**
     * @brief Pushes data followed by graph evaluation and output processing.
     *
     * Pushes @param topicState into the graph, performs all pending graph
     * evaluations calling ProcessOutput for each one of them.
     */
    template<class TTopic> void ProcessData(const TTopic& topicState)
    {
        mGraph.PushData<TTopic>(topicState);
        ProcessGraph();
    }

    /// @brief Performs all pending Graph Evaluations with output processing.
    void ProcessGraph()
    {
        while (mGraph.EvaluateIfHasDataPending())
        {
            ProcessOutput();
        }
    }

    /**
     * @brief Called after each Graph Evaluation.
     *
     * Users should provide an implementation for this method where they can
     * inspect specific output topics or process all new outputs generically
     * using Graph::GetOutputList.
     */
    virtual void ProcessOutput() = 0;

    Graph mGraph;

    virtual ~ProcessorContainer() {}
};

}

#endif // DETECTORGRAPH_INCLUDE_PROCESSORCONTAINER_HPP_
