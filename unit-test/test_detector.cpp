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

#include "nltest.h"
#include "errortype.hpp"

#include "test_detector.h"

#include "graph.hpp"
#include "detector.hpp"
#include "vertex.hpp"
#include "topicstate.hpp"

#include <typeinfo>
#include <iostream>
#include <algorithm>

#define SUITE_DECLARATION(name, test_ptr) { #name, test_ptr, setup_##name, teardown_##name }

using namespace DetectorGraph;

static int setup_detector(void *inContext)
{
    return 0;
}

static int teardown_detector(void *inContext)
{
    return 0;
}

static void Test_VertexType(nlTestSuite *inSuite, void *inContext)
{
    struct TestDetector : public Detector
    {
        TestDetector(Graph* graph) : Detector(graph)
        {
        }
    };

    Graph graph;
    TestDetector* detector = new TestDetector(&graph);
    Vertex* vtxPtr = static_cast<Vertex*>(detector);

    NL_TEST_ASSERT(inSuite, vtxPtr->GetVertexType() == Vertex::kDetectorVertex);
}

namespace {
    // MOCK detector. A transparent multiple-use int-to-uint pass-through detector
    struct PacketTypeA : public TopicState { PacketTypeA(int aV = 0) : mV(aV) {}; int mV; };
    struct PacketTypeB : public TopicState { PacketTypeB(unsigned int aV = 0) : mV(aV) {}; unsigned int mV; };
    struct TestDetector : public Detector, public SubscriberInterface<PacketTypeA>, public Publisher<PacketTypeB>
    {
        TestDetector(Graph* graph) : Detector(graph), mEvalCount(0), mInData(0), mOutData(0)
        {
            Subscribe<PacketTypeA>(this);
            SetupPublishing<PacketTypeB>(this);
        }
        virtual void Evaluate(const PacketTypeA& aInData)
        {
            mInData = aInData;
            mOutData = PacketTypeB(mInData.mV);
            Publish(mOutData);
            mEvalCount++;
        }
        int mEvalCount;
        PacketTypeA mInData;
        PacketTypeB mOutData;
    };
}

static void Test_ConstructionDestruction(nlTestSuite *inSuite, void *inContext)
{
    /*
     * Detectors add themselves to a Graph on construction
     * and remove themselves on destruction.
     *
     *   -            TopicB
     *   ^            ^
     *   |            |
     *   O    --->    TestDetector
     *   ^            ^
     *   |            |
     *   -            TopicA
     */

    // Arrange empty graph.
    Graph graph;
    NL_TEST_ASSERT(inSuite, graph.GetVertices().size() == 0);

    // Act
    TestDetector* detector = new TestDetector(&graph);

    // Assert
    NL_TEST_ASSERT(inSuite, graph.GetVertices().size() == 3);
    NL_TEST_ASSERT(inSuite, graph.ResolveTopic<PacketTypeA>()->GetOutEdges().front() == detector);
    NL_TEST_ASSERT(inSuite, static_cast<Vertex*>(detector)->GetOutEdges().front() == graph.ResolveTopic<PacketTypeB>());
    NL_TEST_ASSERT(inSuite, graph.ResolveTopic<PacketTypeB>()->GetOutEdges().size() == 0);

    NL_TEST_ASSERT(inSuite, graph.ResolveTopic<PacketTypeB>()->GetInEdges().front() == detector);
    NL_TEST_ASSERT(inSuite, static_cast<Vertex*>(detector)->GetInEdges().front() == graph.ResolveTopic<PacketTypeA>());
    NL_TEST_ASSERT(inSuite, graph.ResolveTopic<PacketTypeA>()->GetInEdges().size() == 0);

    // Act
    delete detector;

    // Topics are loaded lazily and are kept in the graph afterwards.
    // Assert
    NL_TEST_ASSERT(inSuite, graph.GetVertices().size() == 2);
    NL_TEST_ASSERT(inSuite, graph.ResolveTopic<PacketTypeA>()->GetOutEdges().size() == 0);
    NL_TEST_ASSERT(inSuite, graph.ResolveTopic<PacketTypeB>()->GetOutEdges().size() == 0);
}

static void Test_DetectorInOutConnections(nlTestSuite *inSuite, void *inContext)
{
    /*
     * Detectors add themselves to a Graph on construction
     * and remove themselves on destruction.
     *
     *
     *   -            TopicB
     *   ^            ^
     *   |            |
     *   O    --->    TestDetector
     *   ^            ^
     *   |            |
     *   -            TopicA
     *
     *
     */

    // Arrange
    Graph graph;
    TestDetector* detector = new TestDetector(&graph);
    Topic<PacketTypeA>* ta = graph.ResolveTopic<PacketTypeA>();
    Topic<PacketTypeB>* tb = graph.ResolveTopic<PacketTypeB>();

    PacketTypeA dataIn;

    // Act
    ta->Publish(dataIn);
    ta->ProcessVertex();
    detector->ProcessVertex();
    tb->ProcessVertex();

    // Assert
    NL_TEST_ASSERT(inSuite, tb->GetCurrentValues().size() == 1);

    // Act
    delete detector;

    // Arrange
    ta->SetState(Vertex::kVertexClear);
    tb->SetState(Vertex::kVertexClear);

    // Act
    ta->Publish(dataIn);
    ta->ProcessVertex();
    tb->ProcessVertex();

    // Assert
    NL_TEST_ASSERT(inSuite, tb->GetCurrentValues().size() == 0);
}

static void Test_SingleEvaluate(nlTestSuite *inSuite, void *inContext)
{
    Graph graph;
    TestDetector detector(&graph);
    PacketTypeA mockData;

    mockData.mV = 111111;

    NL_TEST_ASSERT(inSuite, detector.mInData.mV == 0);
    NL_TEST_ASSERT(inSuite, detector.mEvalCount == 0);

    graph.PushData<PacketTypeA>(mockData);

    NL_TEST_ASSERT(inSuite, detector.mInData.mV == 0);
    NL_TEST_ASSERT(inSuite, detector.mEvalCount == 0);

    graph.EvaluateGraph();

    NL_TEST_ASSERT(inSuite, detector.mInData.mV == 111111);
    NL_TEST_ASSERT(inSuite, detector.mEvalCount == 1);

    graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, detector.mEvalCount == 1);

    // TODO(DGRAPH-4): This test used to push multiple PacketTypeAs and
    // evaluate later.

    graph.PushData<PacketTypeA>(mockData);
    graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, detector.mEvalCount == 2);

    graph.PushData<PacketTypeA>(mockData);
    graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, detector.mEvalCount == 3);
}

namespace
{
    struct PacketTypeC : public TopicState { PacketTypeC(unsigned int aV = 0) : mV(aV) {}; unsigned int mV; };
    // Call order test detector. A transparent multiple-use int-to-uint pass-through detector
    struct TestEvaluateCallsDetector
    : public Detector
    , public SubscriberInterface<PacketTypeA>
    , public SubscriberInterface<PacketTypeB>
    , public Publisher<PacketTypeC>
    {
        TestEvaluateCallsDetector(Graph* graph, bool reverseSubscribes = false)
        : Detector(graph)
        , mEvalACount(0)
        , mEvalBCount(0)
        , mPreProcCount(0)
        , mPostProcCount(0)
        , mEvalAOrder(-1)
        , mEvalBOrder(-1)
        , mInDataA(0)
        , mInDataB(0)
        {
            if (!reverseSubscribes)
            {
                Subscribe<PacketTypeA>(this);
                Subscribe<PacketTypeB>(this);
            }
            else
            {
                Subscribe<PacketTypeB>(this);
                Subscribe<PacketTypeA>(this);
            }
            SetupPublishing<PacketTypeC>(this);
        }

        virtual void BeginEvaluation()
        {
            mPreProcCount++;
            mEvalAOrder = -1;
            mEvalBOrder = -1;
        }

        virtual void Evaluate(const PacketTypeA& aInData)
        {
            mInDataA = aInData;
            mEvalACount++;
            mEvalAOrder = std::max(mEvalAOrder, mEvalBOrder)+1;
        }

        virtual void Evaluate(const PacketTypeB& aInData)
        {
            mInDataB = aInData;
            mEvalBCount++;
            mEvalBOrder = std::max(mEvalAOrder, mEvalBOrder)+1;
        }

        virtual void CompleteEvaluation()
        {
            mPostProcCount++;
        }

        int mEvalACount;
        int mEvalBCount;
        int mPreProcCount;
        int mPostProcCount;
        int mEvalAOrder;
        int mEvalBOrder;

        PacketTypeA mInDataA;
        PacketTypeB mInDataB;
    };
}

static void Test_BeginEvaluationEvaluateCompleteEvaluation(nlTestSuite *inSuite, void *inContext)
{
    Graph graph;
    TestEvaluateCallsDetector detector(&graph);

    // Assert init conditions
    NL_TEST_ASSERT(inSuite, detector.mPreProcCount == 0);
    NL_TEST_ASSERT(inSuite, detector.mEvalACount == 0);
    NL_TEST_ASSERT(inSuite, detector.mEvalBCount == 0);
    NL_TEST_ASSERT(inSuite, detector.mPostProcCount == 0);

    // Act with unrelated data
    graph.PushData<PacketTypeC>(PacketTypeC());
    graph.EvaluateGraph();

    // Assert nothing was called
    NL_TEST_ASSERT(inSuite, detector.mPreProcCount == 0);
    NL_TEST_ASSERT(inSuite, detector.mEvalACount == 0);
    NL_TEST_ASSERT(inSuite, detector.mEvalBCount == 0);
    NL_TEST_ASSERT(inSuite, detector.mPostProcCount == 0);
    NL_TEST_ASSERT(inSuite, graph.GetOutputList().size() == 1);

    // Act with one of the subscribing types
    graph.PushData<PacketTypeA>(PacketTypeA());
    graph.EvaluateGraph();

    // Assert Pre/proc and evalA were called once
    NL_TEST_ASSERT(inSuite, detector.mPreProcCount == 1);
    NL_TEST_ASSERT(inSuite, detector.mEvalACount == 1);
    NL_TEST_ASSERT(inSuite, detector.mEvalBCount == 0);
    NL_TEST_ASSERT(inSuite, detector.mPostProcCount == 1);
    NL_TEST_ASSERT(inSuite, graph.GetOutputList().size() == 1);

    // Act with one of the subscribing types
    graph.PushData<PacketTypeB>(PacketTypeB());
    graph.EvaluateGraph();

    // Assert Pre/proc and evalA were called once
    NL_TEST_ASSERT(inSuite, detector.mPreProcCount == 2);
    NL_TEST_ASSERT(inSuite, detector.mEvalACount == 1);
    NL_TEST_ASSERT(inSuite, detector.mEvalBCount == 1);
    NL_TEST_ASSERT(inSuite, detector.mPostProcCount == 2);
    NL_TEST_ASSERT(inSuite, graph.GetOutputList().size() == 1);
}

namespace
{
    struct SplitterTrigger : public TopicState { SplitterTrigger(unsigned int aV = 0) : mV(aV) {}; unsigned int mV; };
    struct SplitterDetector
    : public Detector
    , public SubscriberInterface<SplitterTrigger>
    , public Publisher<PacketTypeA>
    , public Publisher<PacketTypeB>
    {
        SplitterDetector(Graph* graph, bool aReverse = false)
        : Detector(graph)
        , mReverseOrders(aReverse)
        {
            Subscribe<SplitterTrigger>(this);
            if (!mReverseOrders)
            {
                SetupPublishing<PacketTypeA>(this);
                SetupPublishing<PacketTypeB>(this);
            }
            else
            {
                SetupPublishing<PacketTypeB>(this);
                SetupPublishing<PacketTypeA>(this);
            }
        }

        virtual void Evaluate(const SplitterTrigger&)
        {
            if (!mReverseOrders)
            {
                Publisher<PacketTypeA>::Publish(PacketTypeA());
                Publisher<PacketTypeB>::Publish(PacketTypeB());
            }
            else
            {
                Publisher<PacketTypeB>::Publish(PacketTypeB());
                Publisher<PacketTypeA>::Publish(PacketTypeA());
            }
        }

        bool mReverseOrders;
    };
}

static void Test_SplitterPublisher(nlTestSuite *inSuite, void *inContext)
{
    Graph graph;
    SplitterDetector splitterDetector(&graph);
    TestEvaluateCallsDetector detector(&graph);

    // Assert init conditions
    NL_TEST_ASSERT(inSuite, detector.mPreProcCount == 0);
    NL_TEST_ASSERT(inSuite, detector.mEvalACount == 0);
    NL_TEST_ASSERT(inSuite, detector.mEvalBCount == 0);
    NL_TEST_ASSERT(inSuite, detector.mPostProcCount == 0);

    // Act with one single datapoint
    graph.PushData<SplitterTrigger>(SplitterTrigger());
    graph.EvaluateGraph();

    // Assert nothing was called
    NL_TEST_ASSERT(inSuite, detector.mPreProcCount == 1);
    NL_TEST_ASSERT(inSuite, detector.mEvalACount == 1);
    NL_TEST_ASSERT(inSuite, detector.mEvalBCount == 1);
    NL_TEST_ASSERT(inSuite, detector.mPostProcCount == 1);
    NL_TEST_ASSERT(inSuite, graph.GetOutputList().size() == 3);

    // Act with one of the subscribing types
    graph.PushData<PacketTypeC>(PacketTypeC());
    graph.EvaluateGraph();

    // Assert Pre/proc and evalA were not called anymore
    NL_TEST_ASSERT(inSuite, detector.mPreProcCount == 1);
    NL_TEST_ASSERT(inSuite, detector.mEvalACount == 1);
    NL_TEST_ASSERT(inSuite, detector.mEvalBCount == 1);
    NL_TEST_ASSERT(inSuite, detector.mPostProcCount == 1);
    NL_TEST_ASSERT(inSuite, graph.GetOutputList().size() == 1);

    // Act with one of the subscribing types
    graph.PushData<PacketTypeB>(PacketTypeB());
    graph.EvaluateGraph();

    // Assert Pre/proc and evalA were called once
    NL_TEST_ASSERT(inSuite, detector.mPreProcCount == 2);
    NL_TEST_ASSERT(inSuite, detector.mEvalACount == 1);
    NL_TEST_ASSERT(inSuite, detector.mEvalBCount == 2);
    NL_TEST_ASSERT(inSuite, detector.mPostProcCount == 2);
    NL_TEST_ASSERT(inSuite, graph.GetOutputList().size() == 1);
}

static void Test_EvalsInSubscribeOrder(nlTestSuite *inSuite, void *inContext)
{
    Graph* pGraph = new Graph();
    SplitterDetector* pSplitter = new SplitterDetector(pGraph);
    TestEvaluateCallsDetector* pFwdDetector = new TestEvaluateCallsDetector(pGraph);
    TestEvaluateCallsDetector* pRevDetector = new TestEvaluateCallsDetector(pGraph, true);

    // Assert init conditions
    NL_TEST_ASSERT(inSuite, pFwdDetector->mEvalAOrder == -1);
    NL_TEST_ASSERT(inSuite, pFwdDetector->mEvalBOrder == -1);
    NL_TEST_ASSERT(inSuite, pRevDetector->mEvalBOrder == -1);
    NL_TEST_ASSERT(inSuite, pRevDetector->mEvalBOrder == -1);

    // Act with one single datapoint
    pGraph->PushData<SplitterTrigger>(SplitterTrigger());
    pGraph->EvaluateGraph();

    // Assert Evaluate(A) was called before Evaluate(B) on normal Detector
    NL_TEST_ASSERT(inSuite, pFwdDetector->mEvalAOrder == 0);
    NL_TEST_ASSERT(inSuite, pFwdDetector->mEvalBOrder == 1);

    // Assert Evaluate(B) was called before Evaluate(A) on reversed Detector
    NL_TEST_ASSERT(inSuite, pRevDetector->mEvalAOrder == 1);
    NL_TEST_ASSERT(inSuite, pRevDetector->mEvalBOrder == 0);

    delete pRevDetector;
    delete pFwdDetector;
    delete pSplitter;
    delete pGraph;

    pGraph = new Graph();
    pSplitter = new SplitterDetector(pGraph, true);
    pFwdDetector = new TestEvaluateCallsDetector(pGraph);
    pRevDetector = new TestEvaluateCallsDetector(pGraph, true);

    // Assert init conditions
    NL_TEST_ASSERT(inSuite, pFwdDetector->mEvalAOrder == -1);
    NL_TEST_ASSERT(inSuite, pFwdDetector->mEvalBOrder == -1);
    NL_TEST_ASSERT(inSuite, pRevDetector->mEvalBOrder == -1);
    NL_TEST_ASSERT(inSuite, pRevDetector->mEvalBOrder == -1);

    // Act with one single datapoint
    pGraph->PushData<SplitterTrigger>(SplitterTrigger());
    pGraph->EvaluateGraph();

    // Assert Evaluate(A) was called before Evaluate(B) on normal Detector
    NL_TEST_ASSERT(inSuite, pFwdDetector->mEvalAOrder == 0);
    NL_TEST_ASSERT(inSuite, pFwdDetector->mEvalBOrder == 1);

    // Assert Evaluate(B) was called before Evaluate(A) on reversed Detector
    NL_TEST_ASSERT(inSuite, pRevDetector->mEvalAOrder == 1);
    NL_TEST_ASSERT(inSuite, pRevDetector->mEvalBOrder == 0);

    delete pRevDetector;
    delete pFwdDetector;
    delete pSplitter;
    delete pGraph;
}

static const nlTest sTests[] = {
    NL_TEST_DEF("Test_VertexType", Test_VertexType),
    NL_TEST_DEF("Test_ConstructionDestruction", Test_ConstructionDestruction),
    NL_TEST_DEF("Test_DetectorInOutConnections", Test_DetectorInOutConnections),
    NL_TEST_DEF("Test_SingleEvaluate", Test_SingleEvaluate),
    NL_TEST_DEF("Test_BeginEvaluationEvaluateCompleteEvaluation", Test_BeginEvaluationEvaluateCompleteEvaluation),
    NL_TEST_DEF("Test_SplitterPublisher", Test_SplitterPublisher),
    NL_TEST_DEF("Test_EvalsInSubscribeOrder", Test_EvalsInSubscribeOrder),
    NL_TEST_SENTINEL()
};

//This function creates the Suite (i.e: the name of your test and points to the array of test functions)
extern "C"
int detector_testsuite(void)
{
    nlTestSuite theSuite = SUITE_DECLARATION(detector, &sTests[0]);
    nlTestRunner(&theSuite, NULL);
    return nlTestRunnerStats(&theSuite);
}
