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

struct InputTopic : public TopicState { };

struct InnerData0 : public TopicState { };
struct InnerData1 : public TopicState { };
struct InnerData2 : public TopicState { };
struct InnerData3 : public TopicState { };
struct InnerData4 : public TopicState { };
struct InnerData5 : public TopicState { };
struct InnerData6 : public TopicState { };
struct InnerData7 : public TopicState { };
struct InnerData8 : public TopicState { };
struct InnerData9 : public TopicState { };
struct InnerDataA : public TopicState { };
struct InnerDataB : public TopicState { };
struct InnerDataC : public TopicState { };
struct InnerDataD : public TopicState { };
struct InnerDataE : public TopicState { };
struct InnerDataF : public TopicState { };
struct InnerDataG : public TopicState { };

struct OutputTopic : public TopicState
{
    int v;
    OutputTopic() : v() {}
    OutputTopic(int aV) : v(aV) {}
};

class SplitterDetector
: public Detector
, public SubscriberInterface<InputTopic>
, public Publisher<InnerData0>
, public Publisher<InnerData1>
, public Publisher<InnerData2>
, public Publisher<InnerData3>
, public Publisher<InnerData4>
, public Publisher<InnerData5>
, public Publisher<InnerData6>
, public Publisher<InnerData7>
, public Publisher<InnerData8>
, public Publisher<InnerData9>
, public Publisher<InnerDataA>
, public Publisher<InnerDataB>
, public Publisher<InnerDataC>
, public Publisher<InnerDataD>
, public Publisher<InnerDataE>
, public Publisher<InnerDataF>
, public Publisher<InnerDataG>
{
public:
    SplitterDetector(Graph* graph) : Detector(graph)
    {
        Subscribe<InputTopic>(this);
        SetupPublishing<InnerData0>(this);
        SetupPublishing<InnerData1>(this);
        SetupPublishing<InnerData2>(this);
        SetupPublishing<InnerData3>(this);
        SetupPublishing<InnerData4>(this);
        SetupPublishing<InnerData5>(this);
        SetupPublishing<InnerData6>(this);
        SetupPublishing<InnerData7>(this);
        SetupPublishing<InnerData8>(this);
        SetupPublishing<InnerData9>(this);
        SetupPublishing<InnerDataA>(this);
        SetupPublishing<InnerDataB>(this);
        SetupPublishing<InnerDataC>(this);
        SetupPublishing<InnerDataD>(this);
        SetupPublishing<InnerDataE>(this);
        SetupPublishing<InnerDataF>(this);
        SetupPublishing<InnerDataG>(this);
    }

    virtual void Evaluate(const InputTopic&)
    {
        Publisher<InnerData0>::Publish(InnerData0());
        Publisher<InnerData1>::Publish(InnerData1());
        Publisher<InnerData2>::Publish(InnerData2());
        Publisher<InnerData3>::Publish(InnerData3());
        Publisher<InnerData4>::Publish(InnerData4());
        Publisher<InnerData5>::Publish(InnerData5());
        Publisher<InnerData6>::Publish(InnerData6());
        Publisher<InnerData7>::Publish(InnerData7());
        Publisher<InnerData8>::Publish(InnerData8());
        Publisher<InnerData9>::Publish(InnerData9());
        Publisher<InnerDataA>::Publish(InnerDataA());
        Publisher<InnerDataB>::Publish(InnerDataB());
        Publisher<InnerDataC>::Publish(InnerDataC());
        Publisher<InnerDataD>::Publish(InnerDataD());
        Publisher<InnerDataE>::Publish(InnerDataE());
        Publisher<InnerDataF>::Publish(InnerDataF());
        Publisher<InnerDataG>::Publish(InnerDataG());
    }

};

class ConcentratorDetector
: public Detector
, public SubscriberInterface<InnerData0>
, public SubscriberInterface<InnerData1>
, public SubscriberInterface<InnerData2>
, public SubscriberInterface<InnerData3>
, public SubscriberInterface<InnerData4>
, public SubscriberInterface<InnerData5>
, public SubscriberInterface<InnerData6>
, public SubscriberInterface<InnerData7>
, public SubscriberInterface<InnerData8>
, public SubscriberInterface<InnerData9>
, public SubscriberInterface<InnerDataA>
, public SubscriberInterface<InnerDataB>
, public SubscriberInterface<InnerDataC>
, public SubscriberInterface<InnerDataD>
, public SubscriberInterface<InnerDataE>
, public SubscriberInterface<InnerDataF>
, public SubscriberInterface<InnerDataG>
, public Publisher<OutputTopic>
{
public:
    ConcentratorDetector(Graph* graph) : Detector(graph), v()
    {
        Subscribe<InnerData0>(this);
        Subscribe<InnerData1>(this);
        Subscribe<InnerData2>(this);
        Subscribe<InnerData3>(this);
        Subscribe<InnerData4>(this);
        Subscribe<InnerData5>(this);
        Subscribe<InnerData6>(this);
        Subscribe<InnerData7>(this);
        Subscribe<InnerData8>(this);
        Subscribe<InnerData9>(this);
        Subscribe<InnerDataA>(this);
        Subscribe<InnerDataB>(this);
        Subscribe<InnerDataC>(this);
        Subscribe<InnerDataD>(this);
        Subscribe<InnerDataE>(this);
        Subscribe<InnerDataF>(this);
        Subscribe<InnerDataG>(this);
        SetupPublishing<OutputTopic>(this);
    }

    virtual void Evaluate(const InnerData0&) { v++; }
    virtual void Evaluate(const InnerData1&) { v++; }
    virtual void Evaluate(const InnerData2&) { v++; }
    virtual void Evaluate(const InnerData3&) { v++; }
    virtual void Evaluate(const InnerData4&) { v++; }
    virtual void Evaluate(const InnerData5&) { v++; }
    virtual void Evaluate(const InnerData6&) { v++; }
    virtual void Evaluate(const InnerData7&) { v++; }
    virtual void Evaluate(const InnerData8&) { v++; }
    virtual void Evaluate(const InnerData9&) { v++; }
    virtual void Evaluate(const InnerDataA&) { v++; }
    virtual void Evaluate(const InnerDataB&) { v++; }
    virtual void Evaluate(const InnerDataC&) { v++; }
    virtual void Evaluate(const InnerDataD&) { v++; }
    virtual void Evaluate(const InnerDataE&) { v++; }
    virtual void Evaluate(const InnerDataF&) { v++; }
    virtual void Evaluate(const InnerDataG&) { v++; }
    virtual void CompleteEvaluation() { Publish(OutputTopic(v)); }
    int v;
};

class ConcentratorGraph : public ProcessorContainer
{
public:
    ConcentratorGraph()
#if defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
    : mInputTopic(&mGraph)
    , mInnerData0(&mGraph)
    , mInnerData1(&mGraph)
    , mInnerData2(&mGraph)
    , mInnerData3(&mGraph)
    , mInnerData4(&mGraph)
    , mInnerData5(&mGraph)
    , mInnerData6(&mGraph)
    , mInnerData7(&mGraph)
    , mInnerData8(&mGraph)
    , mInnerData9(&mGraph)
    , mInnerDataA(&mGraph)
    , mInnerDataB(&mGraph)
    , mInnerDataC(&mGraph)
    , mInnerDataD(&mGraph)
    , mInnerDataE(&mGraph)
    , mInnerDataF(&mGraph)
    , mInnerDataG(&mGraph)
    , mOutputTopic(&mGraph)
#endif
    , mSplitter(&mGraph)
    , mConcentrator(&mGraph)
    {

#if defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
        mGraph.AddVertex(&mInputTopic);
        mGraph.AddVertex(&mSplitter);
        mGraph.AddVertex(&mInnerData0);
        mGraph.AddVertex(&mInnerData1);
        mGraph.AddVertex(&mInnerData2);
        mGraph.AddVertex(&mInnerData3);
        mGraph.AddVertex(&mInnerData4);
        mGraph.AddVertex(&mInnerData5);
        mGraph.AddVertex(&mInnerData6);
        mGraph.AddVertex(&mInnerData7);
        mGraph.AddVertex(&mInnerData8);
        mGraph.AddVertex(&mInnerData9);
        mGraph.AddVertex(&mInnerDataA);
        mGraph.AddVertex(&mInnerDataB);
        mGraph.AddVertex(&mInnerDataC);
        mGraph.AddVertex(&mInnerDataD);
        mGraph.AddVertex(&mInnerDataE);
        mGraph.AddVertex(&mInnerDataF);
        mGraph.AddVertex(&mInnerDataG);
        mGraph.AddVertex(&mConcentrator);
        mGraph.AddVertex(&mOutputTopic);
        DG_ASSERT(mGraph.IsGraphSorted());
#endif
    }

    void ProcessOutput()
    {
    }

#if defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
    Topic<InputTopic> mInputTopic;
    Topic<InnerData0> mInnerData0;
    Topic<InnerData1> mInnerData1;
    Topic<InnerData2> mInnerData2;
    Topic<InnerData3> mInnerData3;
    Topic<InnerData4> mInnerData4;
    Topic<InnerData5> mInnerData5;
    Topic<InnerData6> mInnerData6;
    Topic<InnerData7> mInnerData7;
    Topic<InnerData8> mInnerData8;
    Topic<InnerData9> mInnerData9;
    Topic<InnerDataA> mInnerDataA;
    Topic<InnerDataB> mInnerDataB;
    Topic<InnerDataC> mInnerDataC;
    Topic<InnerDataD> mInnerDataD;
    Topic<InnerDataE> mInnerDataE;
    Topic<InnerDataF> mInnerDataF;
    Topic<InnerDataG> mInnerDataG;
    Topic<OutputTopic> mOutputTopic;
#endif
    SplitterDetector mSplitter;
    ConcentratorDetector mConcentrator;
};

int main()
{
    // Same example as above but using purely stack allocations:
    ConcentratorGraph sdg;

    sdg.ProcessData<InputTopic>(InputTopic());
    DG_LOG("OutputTopic = %d", sdg.mOutputTopic.GetNewValue().v);

    return 0;
}
