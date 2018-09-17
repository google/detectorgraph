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

#include "graph.hpp"
#include "detector.hpp"
#include "futurepublisher.hpp"
#include "sharedptr.hpp"
#include "processorcontainer.hpp"

#include <iostream>

using std::cout;
using std::endl;

/**
 * @file resuminggraph.cpp
 * @brief Basic Counter Graph with persistent storage.
 *
 * @section Introduction
 * This examples cover the most basic way to preserve state across Graph
 * instances.
 *
 * This covers how to resume state and a neat way to define initial state.
 *
 * @section te Architecture
 *
 * The example is composed of one Detector that counts `EventHappened`
 * and publishes a total count of those events.
 *
 *  @dot "ResumingCounter"
digraph GraphAnalyzer {
    rankdir = "LR";
    node[fontname=Helvetica];
    size="12,5";
    "EventHappened" [label="0:EventHappened",style=filled, shape=box, color=lightblue];
        "EventHappened" -> "EventCounter";
        "EventCounter" -> "EventCount";
    "EventCounter" [label="1:EventCounter", color=blue];
    "EventCount" [label="2:EventCount",style=filled, shape=box, color=red];
}
 *  @enddot
 *
 * @section resume-from-snapshot ResumeFromSnapshotTopicState
 * The framework offers this convenience TopicState that easily bridges output
 * and input data without breaking the Topological sort.
 * TODO(cscotti): more.
 *
 * @section Graph
 * The graph instantiated and evaluation code is unaffected. Both
 @snippet counterwithreset.cpp CounterWithResetGraph
 * and
 @snippet counterwithreset.cpp main
 * work in the same way as with simple graphs.
 *
 * Running the program produces:
 \verbatim
DetectorGraph: Graph Initialized
EventCount.count = 1
EventCount.count = 2
EventCount.count = 3
EventCount.count = 4
EventCount.count = 5
EventCount.count = 0
EventCount.count = 1
EventCount.count = 2
 \endverbatim
 *
 * One important thing to note in this case is that ProcessOutput is called 8
 * times even though `main` only pushes 7 TopicStates into the graph - this is
 * by design and allows all graph outputs to continue to be inspected exactly
 * once per-evaluation.
 * This is the case for all graphs with closed loops.
 *
 * @cond DO_NOT_DOCUMENT
 */

//! [EventHappened]
struct EventHappened : public DetectorGraph::TopicState
{
};
//! [EventHappened]

//! [EventCount]
struct EventCount : public DetectorGraph::TopicState
{
    EventCount(int aCount = 0) : count(aCount) {}
    int count;
};
//! [EventCount]

//! [Reset]
struct Reset : public DetectorGraph::TopicState
{
};
//! [Reset]

//![ResumingCountDetector]
class ResumingCountDetector : public DetectorGraph::Detector
, public DetectorGraph::SubscriberInterface<DetectorGraph::ResumeFromSnapshotTopicState>
, public DetectorGraph::SubscriberInterface<EventHappened>
, public DetectorGraph::Publisher<EventCount>
{
public:
    ResumingCountDetector(DetectorGraph::Graph* graph) : DetectorGraph::Detector(graph), mEventCount(0)
    {
        Subscribe<DetectorGraph::ResumeFromSnapshotTopicState>(this);
        Subscribe<EventHappened>(this);
        SetupPublishing<EventCount>(this);
    }

    void Evaluate(const DetectorGraph::ResumeFromSnapshotTopicState& aSnapshot)
    {
        // TODO
        // if (aSnapshot.ResumeFromSnapshotTopicState...)
    }
    void Evaluate(const EventHappened&)
    {
        mEventCount.count++;
    }
    void CompleteEvaluation()
    {
        Publish(mEventCount);
    }

private:
    EventCount mEventCount;
};
//![ResumingCountDetector]

//![ResumingGraph]
class ResumingGraph : public DetectorGraph::ProcessorContainer
{
public:
    ResumingGraph()
    : mResumingCountDetector(&mGraph)
    {
        // Post ResumeFromSnapshotTopicState
    }

    ResumingCountDetector mResumingCountDetector;

    virtual void ProcessOutput()
    {
        // update snapshot.
        // dump to file
    }
};
//![ResumingGraph]

//![main]
int main()
{
    ResumingGraph resumingGraph;
    for (int i = 0; i < 7; ++i)
    {
        resumingGraph.ProcessData(EventHappened());
    }
}
//![main]

/// @endcond DO_NOT_DOCUMENT
