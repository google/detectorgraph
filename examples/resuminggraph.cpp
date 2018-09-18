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
#include "graphstatestore.hpp"
#include "resumefromsnapshottopicstate.hpp"

#include <iostream>
#include <fstream>

using std::cout;
using std::endl;

/**
 * @file resuminggraph.cpp
 * @brief Basic Counter Graph with persistent storage.
 *
 * @section ex-rg-intro Introduction
 * This examples cover the most basic way to preserve state across Graph
 * instances.
 *
 * This covers how to resume state and a neat way to define initial state.
 *
 * @section ex-rg-arch Graph Architecture
 *
 * The example uses a minimal Graph with a single Detector and two topics.
 * The Detector counts `EventHappened` and publishes the total in `EventCount`.
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
 * @section ex-rg-state-persistence State Persistence; Snapshots & ResumeFromSnapshotTopicState
 *
 * This example shows how [GraphStateStore](@ref DetectorGraph::GraphStateStore
 * ) and [StateSnapshot](@ref DetectorGraph::StateSnapshot) can be used in
 * conjunction with [ResumeFromSnapshotTopicState](@ref DetectorGraph::ResumeFromSnapshotTopicState)
 * to provide an extensible, robust and transparent state persistence
 * mechanism.
 *
 * The diagram below shows the life cycle of StateSnapshots during boot.
 *  @dot "State Lifetime FSM"
 digraph StateLifetime {
    node[fontname=Helvetica];
    edge[lp="r", fontname="Times-Italic", fontcolor=blue];
    "first_ever" [label="", shape=none];
    "first_ever" -> "PrimeSnapshot" [label="Start"];
    "PrimeSnapshot" [fontname=Monospace];
    "ResumeSnapshot" [fontname=Monospace];
    "PrimeSnapshot" -> "ResumeSnapshot" [label="Reads from Storage"];
    "ResumeSnapshot" -> "LatestSnapshot" [label="First Graph Evaluation"];
    subgraph clusterrunning {
        style=filled;
        color=lightskyblue;
        labeljust="r";
        label="Running & Updating GraphStateStore";
        "LatestSnapshot" [fontname=Monospace];
        "LatestSnapshot" -> "LatestSnapshot" [label="Subsequent Evaluations\n+\n Write to Storage"];
    }
 }
 *  @enddot
 *
 * This life cycle can be seen in the `main` of this example:
 @snippetlineno resuminggraph.cpp main
 *
 * `PrimeSnapshot` is a StateSnapshot that contains the initial state/data that
 * should always be used - regardless of file-based state persistence.
 * In this example we synthesize `PrimeSnapshot` this way:
 @snippetlineno resuminggraph.cpp PrimeSnapshot

 * `ResumeSnapshot` is the aggregation of TopicStates contained in
 * `PrimeSnapshot` and what is deserialized from storage in `ReadSnapshot()`:
 @snippetlineno resuminggraph.cpp Deserialization

 * `ResumeSnapshot` is then used to construct a `ResumeFromSnapshotTopicState`
 * which is then posted to the graph to allow Detectors to resume/initialize
 * their state.
 @snippetlineno resuminggraph.cpp Evaluate-ResumeFromSnapshot

 * From then on `mStateStore` in `ResumingGraph` is continually updated
 * from within `ResumingGraph::ProcessOutput()`:
 @snippetlineno resuminggraph.cpp UpdateStateStore

 * The up-to-date `latestSnapshot` can then be flushed to disk as necessary
 * with `WriteSnapshot()`:
 @snippetlineno resuminggraph.cpp Serialization

 @note
Different applications may chose how to do this step differently. Depending on
your chosen Serialization technology it can be too expensive to call
`WriteSnapshot()` for each new processed input. Some applications may choose
to only call `WriteSnapshot()` during shutdown. A more sophisticated approach
is to call `WriteSnapshot()` only when a `TopicState` in a set of _critical_
ones changes. This can be done by iterating through the list in
`mGraph.GetOutputList()` as it only contains the changed `TopicStates`.

 *
 * @section ex-rg-instance Running the Program
 * Running the program produces:
 \verbatim
$ ./resuminggraph.out
DetectorGraph: Graph Initialized
EventCount = 1001
EventCount = 1002
EventCount = 1003
EventCount = 1004
EventCount = 1005
EventCount = 1006
EventCount = 1007

$ ./resuminggraph.out
DetectorGraph: Graph Initialized
EventCount = 1008
EventCount = 1009
EventCount = 1010
EventCount = 1011
EventCount = 1012
EventCount = 1013
EventCount = 1014

$ cat resumingGraphSnapshot.txt
1014
 \endverbatim
 *
 *
 * @cond DO_NOT_DOCUMENT
 */

enum class ResumingGraphTopicStateIds{
    kEventCount = 0,
};

struct EventHappened : public DetectorGraph::TopicState
{
};

//! [NamedEventCount]
struct EventCount : public DetectorGraph::TopicState
{
    EventCount(int aCount = 0) : count(aCount) {}
    int count;

    DetectorGraph::TopicStateIdType GetId() const
    {
        return static_cast<DetectorGraph::TopicStateIdType>(
            ResumingGraphTopicStateIds::kEventCount);
    }
};
//! [NamedEventCount]

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

    //![Evaluate-ResumeFromSnapshot]
    void Evaluate(const DetectorGraph::ResumeFromSnapshotTopicState& aResumeFrom)
    {
        const auto previousEventCount = aResumeFrom.snapshot.GetState<EventCount>();
        if (previousEventCount)
        {
            mEventCount = *previousEventCount;
        }
    }
    //![Evaluate-ResumeFromSnapshot]

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
    ResumingGraph(const DetectorGraph::StateSnapshot& aInitialSnapshot)
    : mResumingCountDetector(&mGraph)
    {
        ProcessData(DetectorGraph::ResumeFromSnapshotTopicState(aInitialSnapshot));
    }

    ResumingCountDetector mResumingCountDetector;

    virtual void ProcessOutput()
    {
        //![UpdateStateStore]
        mStateStore.TakeNewSnapshot(mGraph.GetOutputList());
        //![UpdateStateStore]
    }

    template<class TTopicState> ptr::shared_ptr<const TTopicState> GetLastTopicState() const
    {
        DG_ASSERT(DetectorGraph::TopicState::GetId<TTopicState>()
            != DetectorGraph::TopicState::kAnonymousTopicState);

        return ptr::static_pointer_cast<const TTopicState>(
            mStateStore.GetLastState()->GetState<TTopicState>());
    }
    ptr::shared_ptr<const DetectorGraph::StateSnapshot> GetLastState() const
    {
        return mStateStore.GetLastState();
    }

private:
    DetectorGraph::GraphStateStore mStateStore;
};

//![ResumingGraph]

constexpr char kSavedSnapshotFileName[] = "resumingGraphSnapshot.txt";

//![PrimeSnapshot]
DetectorGraph::StateSnapshot GetPrimeSnapshot()
{
    std::list< ptr::shared_ptr<const DetectorGraph::TopicState> > topicStatesList;
    topicStatesList.push_back(ptr::shared_ptr<const DetectorGraph::TopicState>(new EventCount(1000)));
    // ... add any other TopicStates that need Prime states.
    return DetectorGraph::StateSnapshot(topicStatesList);
}
//![PrimeSnapshot]

//![Serialization]
void WriteSnapshot(const DetectorGraph::StateSnapshot& aSnapshot)
{
    std::ofstream snapshotOutStream(kSavedSnapshotFileName);

    // Generally here you'd have a loop through a list of TopicState
    // serializers giving each one a chance to flush that topic's contents to
    // disk. And you'd normally use something fancier for Serialization (e.g.
    // protobufs).
    // For the purposes of this demo we'll mock that in a single method:
    {
        ptr::shared_ptr<const EventCount> countPtr = aSnapshot.GetState<EventCount>();
        if (countPtr)
        {
            snapshotOutStream << countPtr->count << endl;
        }
        // ... add serialization of any other TopicStates of interest.
    }
}
//![Serialization]

//![Deserialization]
DetectorGraph::StateSnapshot ReadSnapshot(const DetectorGraph::StateSnapshot& primeSnapshot)
{
    std::list< ptr::shared_ptr<const DetectorGraph::TopicState> > topicStatesList;

    std::ifstream snapshotInStream(kSavedSnapshotFileName);
    if (snapshotInStream.is_open())
    {
        // Generally here you'd have a loop through the TopicStates found on
        // Storage and adding each one to `topicStatesList`
        {
            int count;
            snapshotInStream >> count;
            topicStatesList.push_back(ptr::shared_ptr<const DetectorGraph::TopicState>(new EventCount(count)));
        }
        // ... add deserialization of any other TopicStates of interest.
    }

    return DetectorGraph::StateSnapshot(primeSnapshot, topicStatesList);
}
//![Deserialization]

//![main]
int main()
{
    DetectorGraph::StateSnapshot primeSnapshot = GetPrimeSnapshot()
    DetectorGraph::StateSnapshot resumeSnapshot = ReadSnapshot(primeSnapshot);

    ResumingGraph resumingGraph = ResumingGraph(resumeSnapshot);
    for (int i = 0; i < 7; ++i)
    {
        resumingGraph.ProcessData(EventHappened());
        cout << "EventCount = " << resumingGraph.GetLastTopicState<EventCount>()->count << endl;

        const auto latestSnapshot = resumingGraph.GetLastState();
        WriteSnapshot(*latestSnapshot);
    }
}
//![main]

/// @endcond DO_NOT_DOCUMENT
