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

#include "test_lite.h"

#include "graph.hpp"
#include "detector.hpp"
#include "vertex.hpp"
#include "topicstate.hpp"

#include <algorithm>

#define SUITE_DECLARATION(name, test_ptr) { #name, test_ptr, setup_##name, teardown_##name }

using namespace DetectorGraph;

static int setup_lite(void *inContext)
{
    return 0;
}

static int teardown_lite(void *inContext)
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
    TestDetector detector = TestDetector(&graph);
    Vertex* vtxPtr = static_cast<Vertex*>(&detector);

    NL_TEST_ASSERT(inSuite, vtxPtr->GetVertexType() == Vertex::kDetectorVertex);
}

namespace {
    namespace TopicStateId
    {

    enum TestDetectorTopicStateIds
    {
        kPacketTypeA = 0,
        kPacketTypeB,
        kPacketTypeC,
        kSplitterTrigger,

        // Keep as last one
        kNumberOfTopicStateIds,
    };

    }

    // MOCK detector. A transparent multiple-use int-to-uint pass-through detector
    struct PacketTypeA : public TopicState { PacketTypeA(int aV = 0) : mV(aV) {}; int mV; TopicStateIdType GetId() const { return TopicStateId::kPacketTypeA; }; };
    struct PacketTypeB : public TopicState { PacketTypeB(int aV = 0) : mV(aV) {}; int mV; TopicStateIdType GetId() const { return TopicStateId::kPacketTypeB; }; };
    struct PacketTypeC : public TopicState { PacketTypeC(int aV = 0) : mV(aV) {}; int mV; TopicStateIdType GetId() const { return TopicStateId::kPacketTypeC; }; };


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

    // Instantiate Graph Elements
    Topic<PacketTypeA> topicPacketA(&graph);
    Topic<PacketTypeB> topicPacketB(&graph);
    TestDetector detector(&graph);

    // Order Vertices
    graph.AddVertex(&topicPacketA);
    graph.AddVertex(&detector);
    graph.AddVertex(&topicPacketB);

    // Assert
    NL_TEST_ASSERT(inSuite, graph.GetVertices().size() == 3);
    NL_TEST_ASSERT(inSuite, graph.GetTopicRegistry().Resolve<PacketTypeA>() != NULL);

    NL_TEST_ASSERT(inSuite, static_cast<Vertex*>(&detector)->GetOutEdges()[0] == graph.GetTopicRegistry().Resolve<PacketTypeB>());
    NL_TEST_ASSERT(inSuite, graph.GetTopicRegistry().Resolve<PacketTypeB>()->GetOutEdges().size() == 0);
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
    Topic<PacketTypeA> ta(&graph);
    Topic<PacketTypeB> tb(&graph);
    TestDetector da(&graph);

    // Order Vertices (unnecessary for this test)
    graph.AddVertex(&ta);
    graph.AddVertex(&da);
    graph.AddVertex(&tb);

    PacketTypeA dataIn;

    // Act
    ta.Publish(dataIn);
    ta.ProcessVertex();
    da.ProcessVertex();
    tb.ProcessVertex();

    // Assert
    NL_TEST_ASSERT(inSuite, ta.GetState() == Vertex::kVertexDone);
    NL_TEST_ASSERT(inSuite, da.GetState() == Vertex::kVertexDone);
    NL_TEST_ASSERT(inSuite, tb.GetState() == Vertex::kVertexDone);

    // Arrange
    ta.SetState(Vertex::kVertexClear);
    da.SetState(Vertex::kVertexClear);
    tb.SetState(Vertex::kVertexClear);

    // Act
    ta.Publish(dataIn);
    ta.ProcessVertex();
    // DOES NOT PROCESS da
    tb.ProcessVertex();

    // Assert that no data shows up on tb regardless
    NL_TEST_ASSERT(inSuite, tb.GetState() == Vertex::kVertexClear);
}

static void Test_SingleEvaluate(nlTestSuite *inSuite, void *inContext)
{
    Graph graph;
    Topic<PacketTypeA> ta(&graph);
    Topic<PacketTypeB> tb(&graph);
    TestDetector detector(&graph);

    // Order Vertices (unnecessary for this test)
    graph.AddVertex(&ta);
    graph.AddVertex(&detector);
    graph.AddVertex(&tb);

    PacketTypeA mockData;

    mockData.mV = 111111;

    NL_TEST_ASSERT(inSuite, detector.mInData.mV == 0);
    NL_TEST_ASSERT(inSuite, detector.mEvalCount == 0);

    graph.PushData<PacketTypeA>(mockData);

    // Pushing data does not cause evaluation
    NL_TEST_ASSERT(inSuite, detector.mInData.mV == 0);
    NL_TEST_ASSERT(inSuite, detector.mEvalCount == 0);

    // Act with new data
    graph.EvaluateGraph();

    NL_TEST_ASSERT(inSuite, detector.mInData.mV == 111111);
    NL_TEST_ASSERT(inSuite, detector.mEvalCount == 1);

    // Act without new data
    graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, detector.mEvalCount == 1);
}

// static void Test_InputQueue(nlTestSuite *inSuite, void *inContext)
// {
//     Graph graph;
//     Topic<PacketTypeA> ta(&graph);
//     Topic<PacketTypeB> tb(&graph);
//     TestDetector detector(&graph);

//     // Order Vertices (unnecessary for this test)
//     graph.AddVertex(&ta);
//     graph.AddVertex(&detector);
//     graph.AddVertex(&tb);

//     PacketTypeA mockData;

//     graph.PushData<PacketTypeA>(mockData);
//     graph.PushData<PacketTypeA>(mockData);

//     graph.EvaluateGraph();
//     NL_TEST_ASSERT(inSuite, detector.mEvalCount == 1);

//     graph.EvaluateGraph();
//     NL_TEST_ASSERT(inSuite, detector.mEvalCount == 2);
// }

namespace
{
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
            Publish(PacketTypeC(mInDataA.mV+mInDataB.mV));
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
    Topic<PacketTypeA> ta(&graph);
    Topic<PacketTypeB> tb(&graph);
    Topic<PacketTypeC> tc(&graph);
    TestEvaluateCallsDetector detector(&graph);

    graph.AddVertex(&ta);
    graph.AddVertex(&tb);
    graph.AddVertex(&detector);
    graph.AddVertex(&tc);

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

    NL_TEST_ASSERT(inSuite, ta.GetState() == Vertex::kVertexClear);
    NL_TEST_ASSERT(inSuite, tb.GetState() == Vertex::kVertexClear);
    NL_TEST_ASSERT(inSuite, detector.GetState() == Vertex::kVertexClear);
    NL_TEST_ASSERT(inSuite, tc.GetState() == Vertex::kVertexDone);

    // Act with one of the subscribing types
    graph.PushData<PacketTypeA>(PacketTypeA());
    graph.EvaluateGraph();

    // Assert Pre/proc and evalA were called once
    NL_TEST_ASSERT(inSuite, detector.mPreProcCount == 1);
    NL_TEST_ASSERT(inSuite, detector.mEvalACount == 1);
    NL_TEST_ASSERT(inSuite, detector.mEvalBCount == 0);
    NL_TEST_ASSERT(inSuite, detector.mPostProcCount == 1);
    NL_TEST_ASSERT(inSuite, ta.GetState() == Vertex::kVertexDone);
    NL_TEST_ASSERT(inSuite, tb.GetState() == Vertex::kVertexClear);
    NL_TEST_ASSERT(inSuite, detector.GetState() == Vertex::kVertexDone);
    NL_TEST_ASSERT(inSuite, tc.GetState() == Vertex::kVertexDone);

    // Act with one of the subscribing types
    graph.PushData<PacketTypeB>(PacketTypeB());
    graph.EvaluateGraph();

    // Assert Pre/proc and evalA were called once
    NL_TEST_ASSERT(inSuite, detector.mPreProcCount == 2);
    NL_TEST_ASSERT(inSuite, detector.mEvalACount == 1);
    NL_TEST_ASSERT(inSuite, detector.mEvalBCount == 1);
    NL_TEST_ASSERT(inSuite, detector.mPostProcCount == 2);
    NL_TEST_ASSERT(inSuite, ta.GetState() == Vertex::kVertexClear);
    NL_TEST_ASSERT(inSuite, tb.GetState() == Vertex::kVertexDone);
    NL_TEST_ASSERT(inSuite, detector.GetState() == Vertex::kVertexDone);
    NL_TEST_ASSERT(inSuite, tc.GetState() == Vertex::kVertexDone);
}

namespace
{
    struct SplitterTrigger : public TopicState { SplitterTrigger(int aV = 0) : mV(aV) {}; int mV; TopicStateIdType GetId() const { return TopicStateId::kSplitterTrigger; }; };
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
    Topic<PacketTypeA> ta(&graph);
    Topic<PacketTypeB> tb(&graph);
    Topic<PacketTypeC> tc(&graph);
    Topic<SplitterTrigger> ts(&graph);
    SplitterDetector splitter(&graph);
    TestEvaluateCallsDetector detector(&graph);

    graph.AddVertex(&ts);
    graph.AddVertex(&splitter);
    graph.AddVertex(&ta);
    graph.AddVertex(&tb);
    graph.AddVertex(&detector);
    graph.AddVertex(&tc);

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
    NL_TEST_ASSERT(inSuite, ts.GetState() == Vertex::kVertexDone);
    NL_TEST_ASSERT(inSuite, splitter.GetState() == Vertex::kVertexDone);
    NL_TEST_ASSERT(inSuite, ta.GetState() == Vertex::kVertexDone);
    NL_TEST_ASSERT(inSuite, tb.GetState() == Vertex::kVertexDone);
    NL_TEST_ASSERT(inSuite, detector.GetState() == Vertex::kVertexDone);
    NL_TEST_ASSERT(inSuite, tc.GetState() == Vertex::kVertexDone);

    // Act with one of the subscribing types
    graph.PushData<PacketTypeC>(PacketTypeC());
    graph.EvaluateGraph();

    // Assert Pre/proc and evalA were not called anymore
    NL_TEST_ASSERT(inSuite, detector.mPreProcCount == 1);
    NL_TEST_ASSERT(inSuite, detector.mEvalACount == 1);
    NL_TEST_ASSERT(inSuite, detector.mEvalBCount == 1);
    NL_TEST_ASSERT(inSuite, detector.mPostProcCount == 1);
    NL_TEST_ASSERT(inSuite, ts.GetState() == Vertex::kVertexClear);
    NL_TEST_ASSERT(inSuite, splitter.GetState() == Vertex::kVertexClear);
    NL_TEST_ASSERT(inSuite, ta.GetState() == Vertex::kVertexClear);
    NL_TEST_ASSERT(inSuite, tb.GetState() == Vertex::kVertexClear);
    NL_TEST_ASSERT(inSuite, detector.GetState() == Vertex::kVertexClear);
    NL_TEST_ASSERT(inSuite, tc.GetState() == Vertex::kVertexDone);

    // Act with one of the subscribing types
    graph.PushData<PacketTypeB>(PacketTypeB());
    graph.EvaluateGraph();

    // Assert Pre/proc and evalA were called once
    NL_TEST_ASSERT(inSuite, detector.mPreProcCount == 2);
    NL_TEST_ASSERT(inSuite, detector.mEvalACount == 1);
    NL_TEST_ASSERT(inSuite, detector.mEvalBCount == 2);
    NL_TEST_ASSERT(inSuite, detector.mPostProcCount == 2);
    NL_TEST_ASSERT(inSuite, ts.GetState() == Vertex::kVertexClear);
    NL_TEST_ASSERT(inSuite, splitter.GetState() == Vertex::kVertexClear);
    NL_TEST_ASSERT(inSuite, ta.GetState() == Vertex::kVertexClear);
    NL_TEST_ASSERT(inSuite, tb.GetState() == Vertex::kVertexDone);
    NL_TEST_ASSERT(inSuite, detector.GetState() == Vertex::kVertexDone);
    NL_TEST_ASSERT(inSuite, tc.GetState() == Vertex::kVertexDone);
}

static void Test_EvalsInSubscribeOrder(nlTestSuite *inSuite, void *inContext)
{
    Graph graph;
    Topic<PacketTypeA> ta(&graph);
    Topic<PacketTypeB> tb(&graph);
    Topic<PacketTypeC> tc(&graph);
    Topic<SplitterTrigger> ts(&graph);
    SplitterDetector splitter(&graph);
    TestEvaluateCallsDetector fwd_detector(&graph);
    TestEvaluateCallsDetector rev_detector(&graph, true);

    graph.AddVertex(&ts);
    graph.AddVertex(&splitter);
    graph.AddVertex(&ta);
    graph.AddVertex(&tb);
    graph.AddVertex(&fwd_detector);
    graph.AddVertex(&rev_detector);
    graph.AddVertex(&tc);

    // Assert init conditions
    NL_TEST_ASSERT(inSuite, fwd_detector.mEvalAOrder == -1);
    NL_TEST_ASSERT(inSuite, fwd_detector.mEvalBOrder == -1);
    NL_TEST_ASSERT(inSuite, rev_detector.mEvalBOrder == -1);
    NL_TEST_ASSERT(inSuite, rev_detector.mEvalBOrder == -1);

    // Act with one single datapoint
    graph.PushData<SplitterTrigger>(SplitterTrigger());
    graph.EvaluateGraph();

    // Assert Evaluate(A) was called before Evaluate(B) on normal Detector
    NL_TEST_ASSERT(inSuite, fwd_detector.mEvalAOrder == 0);
    NL_TEST_ASSERT(inSuite, fwd_detector.mEvalBOrder == 1);

    // Assert Evaluate(B) was called before Evaluate(A) on reversed Detector
    NL_TEST_ASSERT(inSuite, rev_detector.mEvalAOrder == 1);
    NL_TEST_ASSERT(inSuite, rev_detector.mEvalBOrder == 0);
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
int lite_testsuite(void)
{
    nlTestSuite theSuite = SUITE_DECLARATION(lite, &sTests[0]);
    nlTestRunner(&theSuite, NULL);
    return nlTestRunnerStats(&theSuite);
}
