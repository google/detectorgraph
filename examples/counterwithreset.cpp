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
 * @file counterwithreset.cpp
 * @brief Basic Counter with a FuturePublisher-based loop.
 *
 * @section ex-cwr-introduction Introduction
 * This examples cover the most basic way to close a loop in a graph - using a
 * DetectorGraph::FuturePublisher. Unmarked loops are forbidden by the Framework
 * as a Topographical sort is only possible in Directed Acyclic Graphs.
 *
 * Intuitively, if there were cycles in a graph it would be impossible for the
 * Framework to provide it's dependency guarantee:
 * > For any Detector, all its dependencies will be evaluated before its own
 * > evaluation.
 *
 * To construct closed loops the graph designer must explicitly define, per
 * cycle/loop, which dependency (i.e. Topic) the above restriction will
 * be waived.
 *
 * @section ex-cwr-arch Architecture
 *
 * The example is composed of two Detectors. The first counts `EventHappened`
 * and the second detects the condition at which the counter should be reset.
 *
 *  @dot "CounterWithReset"
digraph GraphAnalyzer {
    rankdir = "LR";
    node[fontname=Helvetica];
    size="12,5";
    "EventHappened" [label="0:EventHappened",style=filled, shape=box, color=lightblue];
        "EventHappened" -> "EventCounter";
        "EventCounter" -> "EventCount";
    "EventCounter" [label="1:EventCounter", color=blue];
    "EventCount" [label="2:EventCount",style=filled, shape=box, color=red];
        "EventCount" -> "ResetDetector";
        "ResetDetector" -> "Reset" [style=dotted, color=red, constraint=false];
    "ResetDetector" [label="3:ResetDetector", color=blue];
    "Reset" [label="4:Reset",style=filled, shape=box, color=red];
        "Reset" -> "EventCounter";
}
 *  @enddot
 *
 * @section ex-cwr-fpvslag FuturePublisher vs. Lag\<T\>
 * The framework offers two ways of closing loops in a graph,
 * DetectorGraph::FuturePublisher and DetectorGraph::Lag - this example
 * employs the former.
 *
 * In this graph it is clear that the TopicState used as a graph _output_ is
 * `EventCount` and that `Reset` is produced purely for the feedback path. It
 * is also clear to the the designer of `ResetDetecor` that the published
 * `Reset` TopicState should be evaluated in the next Evaluation pass and not
 * in the current one. In such cases it is reasonable to put the responsibility
 * for closing the loop on the writer of `ResetDetector`:
 * @snippetlineno counterwithreset.cpp Reset Detector
 *
 * In cases where downstream parts of the graph also subscribe to the
 * TopicState used in the feedback loop, DetectorGraph::Lag should be used
 * instead. For more info on that see other [Feedback Loop examples](
 * @ref feedback-loops) (e.g. [Robot Localization](@ref robotlocalization.cpp))
 *
 * @section ex-cwr-graph Graph
 * The graph instantiated and evaluation code is unaffected. Both
 @snippetlineno counterwithreset.cpp CounterWithResetGraph
 * and
 @snippetlineno counterwithreset.cpp main
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

//![EventCountDetector]
class EventCountDetector : public DetectorGraph::Detector
, public DetectorGraph::SubscriberInterface<EventHappened>
, public DetectorGraph::SubscriberInterface<Reset>
, public DetectorGraph::Publisher<EventCount>
{
public:
    EventCountDetector(DetectorGraph::Graph* graph) : DetectorGraph::Detector(graph), mEventCount(0)
    {
        Subscribe<EventHappened>(this);
        Subscribe<Reset>(this);
        SetupPublishing<EventCount>(this);
    }

    void Evaluate(const EventHappened&)
    {
        mEventCount.count++;
    }
    void Evaluate(const Reset&)
    {
        mEventCount.count = 0;
    }
    void CompleteEvaluation()
    {
        Publish(mEventCount);
    }

private:
    EventCount mEventCount;
};
//![EventCountDetector]

//![Reset Detector]
class ResetDetector : public DetectorGraph::Detector
, public DetectorGraph::SubscriberInterface<EventCount>
, public DetectorGraph::FuturePublisher<Reset>
{
public:
    ResetDetector(DetectorGraph::Graph* graph) : DetectorGraph::Detector(graph)
    {
        Subscribe<EventCount>(this);
        SetupFuturePublishing<Reset>(this);
    }

    void Evaluate(const EventCount& aEventCount)
    {
        if (aEventCount.count >= kMaxCount)
        {
            PublishOnFutureEvaluation(Reset());
        }
    }

    static const int kMaxCount = 5;
};
//![Reset Detector]

//![CounterWithResetGraph]
class CounterWithResetGraph : public DetectorGraph::ProcessorContainer
{
public:
    CounterWithResetGraph()
    : mEventCountDetector(&mGraph)
    , mResetDetector(&mGraph)
    , mEventCountTopic(mGraph.ResolveTopic<EventCount>())
    {
    }

    EventCountDetector mEventCountDetector;
    ResetDetector mResetDetector;
    DetectorGraph::Topic<EventCount>* mEventCountTopic;

    virtual void ProcessOutput()
    {
        if (mEventCountTopic->HasNewValue())
        {
            const EventCount& eventCount = mEventCountTopic->GetNewValue();
            cout << "EventCount.count = " << eventCount.count << endl;
        }
    }
};
//![CounterWithResetGraph]

//![main]
int main()
{
    CounterWithResetGraph counterGraph;
    for (int i = 0; i < 7; ++i)
    {
        counterGraph.ProcessData(EventHappened());
    }
}
//![main]

/// @endcond DO_NOT_DOCUMENT
