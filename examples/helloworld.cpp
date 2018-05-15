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
#include "sharedptr.hpp"
#include "processorcontainer.hpp"

using std::cout;
using std::endl;

/**
 * @file helloworld.cpp
 * @brief The basics - A trivial Graph with a single Detector.
 *
 * @section Introduction
 * This examples cover the basics of using the DetectorGraph framework - a "Hello
 * World" of sorts.
 * For the purposes of this example imagine a temperature sensor that produces
 * samples regularly and that we're tasked to signal other parts of the system
 * whenever that temperature crosses a threshold.
 *
 * We start by splitting the problem, within the scope of our task, in three
 * parts:
 *  - What's our input data? _The input Topic_
 *  - What's our output data? _The output Topic_
 *  - How do we get from one to the other? _The Detector what will do the job_
 *
 * @section Topics
 * Topics are defined by the DetectorGraph::TopicState type they carry. A
 * TopicState is any C++ struct/class that inherits from
 * DetectorGraph::TopicState.
 *
 * For the 'input' to our system we'll declare a `TemperatureSample` TopicState:
 @snippet helloworld.cpp Input Topic
 * The important here is that this `struct` inherits TopicState and that it has
 * data field(s).
 *
 * Next we define the 'output'; another struct that also inherits
 * DetectorGraph::TopicState and has data fields:
 @snippet helloworld.cpp Output Topic
 *
 * @section Detector
 * Now we just need to fill in the gap; the Detector that creates
 * `OverheatingState` from `TemperatureSample`:
 * two.
 @snippet helloworld.cpp Detector
 *
 * @section Graph
 * The final plumbing is to add our newly created Detector to a
 * DetectorGraph::Graph.
 * Depending on the situation this can be done in different ways but here's the
 * simplest:
 @snippet helloworld.cpp Adding to a Graph
 *
 * With that the Graph instance will internally create the following graph:
 *
 *  @dot "HelloWorldGraph"
digraph GraphAnalyzer {
    rankdir = "LR";
    node[fontname=Helvetica];
    size="12,5";
    "TemperatureSample" [label="0:TemperatureSample",style=filled, shape=box, color=lightblue];
        "TemperatureSample" -> "OverheatingDetector";
        "OverheatingDetector" -> "OverheatingState";
    "OverheatingState" [label="2:OverheatingState",style=filled, shape=box, color=limegreen];
    "OverheatingDetector" [label="1:OverheatingDetector", color=blue];
}
 *  @enddot
 *
 * Topics are represented by rectangles, Detectors by the oval shapes and
 * arrows follow the flow of data - this is the standard representation for
 * DetectorGraph graphs. The numeric prefix to the names is each node's order
 * in the Topological sort of the graph. Output topics are painted Lime Green
 * and input ones Light Blue.
 *
 * @section Usage
 * Finally, using the graph is done by Pushing data in, evaluating the graph
 * and checking outputs:
 @snippet helloworld.cpp Basic Graph Usage
 *
 * Running the program then gives:
 \verbatim
DetectorGraph: Graph Initialized
IsOverheating = true
 \endverbatim
 *
 * An alternative way to do things is to subclass the
 * DetectorGraph::ProcessorContainer utility that streamlines the
 * DetectorGraph::Graph::PushData, DetectorGraph::Graph::EvaluateGraph and
 * subsequent inspection steps.
 * For the example above, the DetectorGraph::ProcessorContainer implementation
 * would be:
 @snippet helloworld.cpp ProcessorContainer
 *
 * And then usage would be like:
 @snippet helloworld.cpp Using ProcessorContainer
 *
 *
 \verbatim
DetectorGraph: Graph Initialized
OverheatingState.isOverheating = false
OverheatingState.isOverheating = false
OverheatingState.isOverheating = false
OverheatingState.isOverheating = true
OverheatingState.isOverheating = true
 \endverbatim
 *
 * This example can be built with:
 \code
g++ -std=c++11 -I./include/ -I./platform_standalone/ src/graph.cpp src/detector.cpp platform_standalone/dglogging.cpp examples/helloworld.cpp -o helloworld.out
 \endcode
 *
 * @cond DO_NOT_DOCUMENT
 */

//! [Input Topic]
struct TemperatureSample : public DetectorGraph::TopicState
{
    TemperatureSample(int aTemp = 0) : temperature(aTemp) {}
    int temperature;
};
//! [Input Topic]

//! [Output Topic]
struct OverheatingState : public DetectorGraph::TopicState
{
    OverheatingState(bool aState = false) : isOverheating(aState) {}
    bool isOverheating;
};
//! [Output Topic]

//![Detector]
class OverheatingDetector : public DetectorGraph::Detector
, public DetectorGraph::SubscriberInterface<TemperatureSample>
, public DetectorGraph::Publisher<OverheatingState>
{
public:
    OverheatingDetector(DetectorGraph::Graph* graph) : DetectorGraph::Detector(graph)
    {
        Subscribe<TemperatureSample>(this);
        SetupPublishing<OverheatingState>(this);
    }

    virtual void Evaluate(const TemperatureSample& sample)
    {
        if (sample.temperature > kThreshold)
        {
            Publish(OverheatingState(true));
        }
        else
        {
            Publish(OverheatingState(false));
        }
    }

    static const int kThreshold = 100;
};
//![Detector]

//![ProcessorContainer]
class HelloWorldGraph : public DetectorGraph::ProcessorContainer
{
public:
    HelloWorldGraph()
    : mOverheatingDetector(&mGraph)
    {
    }

    OverheatingDetector mOverheatingDetector;

    virtual void ProcessOutput()
    {
        DetectorGraph::Topic<OverheatingState>* overheatingStateTopic = mGraph.ResolveTopic<OverheatingState>();
        if (overheatingStateTopic->HasNewValue())
        {
            const OverheatingState& overheatingState = overheatingStateTopic->GetNewValue();
            cout << "OverheatingState.isOverheating = " << ((overheatingState.isOverheating) ? "true" : "false") << endl;
        }
    }
};
//![ProcessorContainer]

//![main]
int main()
{
    // Below are examples of two different ways of using/composing
    // DetectorGraph graphs.

    //![Adding to a Graph]
    DetectorGraph::Graph graph;
    OverheatingDetector detector(&graph);
    //![Adding to a Graph]

    //![Basic Graph Usage]
    graph.PushData(TemperatureSample(110));
    graph.EvaluateGraph();

    const OverheatingState& output =
        graph.ResolveTopic<OverheatingState>()->GetNewValue();
    cout << "IsOverheating = " << ((output.isOverheating) ? "true" : "false") << endl;
    //![Basic Graph Usage]

    //![Using ProcessorContainer]
    HelloWorldGraph thermostat;
    thermostat.ProcessData(TemperatureSample(70));
    thermostat.ProcessData(TemperatureSample(90));
    thermostat.ProcessData(TemperatureSample(100));
    thermostat.ProcessData(TemperatureSample(110));
    thermostat.ProcessData(TemperatureSample(120));
    //![Using ProcessorContainer]
}
//![main]

/// @endcond DO_NOT_DOCUMENT
