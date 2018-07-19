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

#include "detectorgraphliteconfig.hpp"
#include "graph.hpp"
#include "detector.hpp"
#include "dglogging.hpp"
#include "dgassert.hpp"
#include "processorcontainer.hpp"

using namespace DetectorGraph;

// #define SIZE_BENCHMARK_DT
#define SIZE_BENCHMARK_DD
// #define SIZE_BENCHMARK_1_ON_1_DETECTOR

struct InputTopic : public TopicState
{
    int v;
    InputTopic(int aV) : v(aV) {}
};

struct OutputTopic : public TopicState
{
    int v;
    OutputTopic(int aV) : v(aV) {}
};

class SisoDetector
: public Detector
, public SubscriberInterface<InputTopic>
, public Publisher<OutputTopic>
{
public:
    SisoDetector(Graph* graph) : Detector(graph)
    {
        Subscribe<InputTopic>(this);
        SetupPublishing<OutputTopic>(this);
    }

    virtual void Evaluate(const InputTopic& t)
    {
        Publish(OutputTopic(t.v));
    }

};

class SisoGraph : public ProcessorContainer
{
public:
    SisoGraph()
    : mInputTopic(mGraph.ResolveTopic<InputTopic>())
    , mSisoDetector(&mGraph)
    , mOutputTopic(mGraph.ResolveTopic<OutputTopic>())
    {
        DG_ASSERT(mGraph.IsGraphSorted());
    }

    void ProcessOutput()
    {
    }

    Topic<InputTopic>* mInputTopic;
    SisoDetector mSisoDetector;
    Topic<OutputTopic>* mOutputTopic;
};

int main()
{
    // Same example as above but using purely stack allocations:
    SisoGraph dg;

    dg.ProcessData<InputTopic>(InputTopic(42));
    DG_LOG("OutputTopic = %d", dg.mOutputTopic->GetNewValue().v);

    return 0;
}
