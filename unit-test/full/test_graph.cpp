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

#include "test_graph.h"

#include "graph.hpp"
#include "vertex.hpp"
#include "topicstate.hpp"
#include "detector.hpp"
#include "dglogging.hpp"

#define SUITE_DECLARATION(name, test_ptr) { #name, test_ptr, setup_##name, teardown_##name }

using namespace DetectorGraph;

static int setup_graph(void *inContext)
{
    return 0;
}

static int teardown_graph(void *inContext)
{
    return 0;
}

static void Test_Lifetime(nlTestSuite *inSuite, void *inContext)
{
    Graph* graph = new Graph();

    int tDestructorCounter = 0;

    class TestVertex : public Vertex
    {
    public:
        TestVertex(int& aDestructorCounter) : mDestructorCounter(aDestructorCounter) {}
        virtual ~TestVertex() { mDestructorCounter++; }
        void ProcessVertex() { }
        VertexType GetVertexType() const { return Vertex::kTestVertex; }

        int& mDestructorCounter;
    };

    graph->AddVertex(new TestVertex(tDestructorCounter));
    graph->EvaluateGraph();

    delete graph;

    NL_TEST_ASSERT(inSuite, tDestructorCounter == 1);
}

static void Test_Toposort(nlTestSuite *inSuite, void *inContext)
{
    ErrorType r = ErrorType_Success;

    class TestVertex : public Vertex
    {
    public:
        TestVertex() { }
        virtual ~TestVertex() { }
        void ProcessVertex() { } // LCOV_EXCL_LINE
        VertexType GetVertexType() const { return Vertex::kTestVertex; } // LCOV_EXCL_LINE
        int mSortOrder;
    };

    /*   O
     *   | \
     *   |  -
     *   -   \
     *   |    O
     *   |    |
     *   O --\
     */

    /*   V3            V3
     *   | \           | \
     *   |  -          |  T3
     *   -   \      =  T2   \
     *   |   V2        |    V2
     *   |    |        |    |
     *   V1 --\        V1 --T1
     *
     *
     * Edges:   V1->T1
     *          V1->T2
     *          T1->V2
     *          V2->T3
     *          T2->V3
     *          T3->V3
     */
    TestVertex V1, V2, V3;
    TestVertex T1, T2, T3;

    V1.InsertEdge(&T1);
    V1.InsertEdge(&T2);
    T1.InsertEdge(&V2);
    V2.InsertEdge(&T3);
    T2.InsertEdge(&V3);
    T3.InsertEdge(&V3);

    Graph graph;

    // Adding vertices in 'random' order
    graph.AddVertex(&T1);
    graph.AddVertex(&T2);
    graph.AddVertex(&T3);
    graph.AddVertex(&V3);
    graph.AddVertex(&V2);
    graph.AddVertex(&V1);

    r = graph.TopoSortGraph();
    NL_TEST_ASSERT(inSuite, r == ErrorType_Success);

    int mSortOrder = 0;
    for (Graph::VertexPtrContainer::const_iterator vIt = graph.GetVertices().begin();
        vIt != graph.GetVertices().end(); ++vIt)
    {
        (static_cast<TestVertex*>(*vIt))->mSortOrder = mSortOrder++;
    }

    NL_TEST_ASSERT(inSuite, V3.mSortOrder > T2.mSortOrder);
    NL_TEST_ASSERT(inSuite, V3.mSortOrder > T3.mSortOrder);
    NL_TEST_ASSERT(inSuite, T3.mSortOrder > V2.mSortOrder);
    NL_TEST_ASSERT(inSuite, T2.mSortOrder > V1.mSortOrder);
    NL_TEST_ASSERT(inSuite, V2.mSortOrder > T1.mSortOrder);
    NL_TEST_ASSERT(inSuite, T1.mSortOrder > V1.mSortOrder);

    graph.RemoveVertex(&V1);
    graph.RemoveVertex(&V2);
    graph.RemoveVertex(&V3);
    graph.RemoveVertex(&T1);
    graph.RemoveVertex(&T2);
    graph.RemoveVertex(&T3);
}

static void Test_SelfLoopDetection(nlTestSuite *inSuite, void *inContext)
{
    ErrorType r = ErrorType_Success;

    class TestVertex : public Vertex
    {
    public:
        TestVertex() { }
        virtual ~TestVertex() {}
        void ProcessVertex() { } // LCOV_EXCL_LINE
        VertexType GetVertexType() const { return Vertex::kTestVertex; } // LCOV_EXCL_LINE
    };

    /*    O
     *   ^ \
     *   \ v
     *    -
     */

    /*    V1
     *   ^ \
     *   \ v
     *    T1
     *
     *
     * Edges:   V1->T1
     *          T1->V1
     */
    TestVertex V1, T1;

    V1.InsertEdge(&T1);
    T1.InsertEdge(&V1);

    Graph graph;

    // Adding vertices in 'random' order
    graph.AddVertex(&V1);
    graph.AddVertex(&T1);

    r = graph.TopoSortGraph();
    NL_TEST_ASSERT(inSuite, r == ErrorType_BadConfiguration);

    graph.RemoveVertex(&V1);
    graph.RemoveVertex(&T1);
}

static void Test_LoopDetection(nlTestSuite *inSuite, void *inContext)
{
    ErrorType r = ErrorType_Success;

    class TestVertex : public Vertex
    {
    public:
        TestVertex() { }
        virtual ~TestVertex() {}
        void ProcessVertex() { } // LCOV_EXCL_LINE
        VertexType GetVertexType() const { return Vertex::kTestVertex; } // LCOV_EXCL_LINE
    };

    /*   O            O
     *   ^            ^ \
     *   |            |  \
     *   -    --->    -   -
     *   |            |  /
     *   |            | V
     *   O            O
     */

    /*   V2           V2
     *   ^            ^ \
     *   |            |  \
     *   T1   --->    T1  T2
     *   |            |  /
     *   |            | V
     *  V1           V1
     *
     *
     * Edges:   V1->T1
     *          T1->V2
     *
     *          V2->T2
     *          T2->V1
     */
    TestVertex V1, V2;
    TestVertex T1, T2;

    V1.InsertEdge(&T1);
    T1.InsertEdge(&V2);

    Graph graph;

    // Adding vertices in 'random' order
    graph.AddVertex(&V1);
    graph.AddVertex(&T1);
    graph.AddVertex(&V2);

    r = graph.TopoSortGraph();
    NL_TEST_ASSERT(inSuite, r == ErrorType_Success);

    V2.InsertEdge(&T2);
    T2.InsertEdge(&V1);
    graph.AddVertex(&T2);

    r = graph.TopoSortGraph();
    NL_TEST_ASSERT(inSuite, r == ErrorType_BadConfiguration);

    r = graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, r == ErrorType_BadConfiguration);

    T2.RemoveEdge(&V1);

    r = graph.TopoSortGraph();
    NL_TEST_ASSERT(inSuite, r == ErrorType_Success);

    r = graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, r == ErrorType_Success);

    graph.RemoveVertex(&V1);
    graph.RemoveVertex(&V2);
    graph.RemoveVertex(&T1);
    graph.RemoveVertex(&T2);
}

static void Test_LoopLoopDetection(nlTestSuite *inSuite, void *inContext)
{
    ErrorType r = ErrorType_Success;

    class TestVertex : public Vertex
    {
    public:
        TestVertex() { }
        virtual ~TestVertex() {}
        void ProcessVertex() { } // LCOV_EXCL_LINE
        VertexType GetVertexType() const { return Vertex::kTestVertex; } // LCOV_EXCL_LINE
    };

    /*   O            O
     *   ^            ^ \
     *   |            |  \
     *   -    --->    -   -
     *   |            |  /
     *   |            | V
     *   O      O <-- O
     */

    /*   V2           V2
     *   ^            ^ \
     *   |            |  \
     *   T1   --->    T1  T2
     *   |            |  /
     *   |            | V
     *  V1     D1 <-- V1
     *
     *
     * Edges:   V1->T1
     *          V1->D1 (Dummy vertex)
     *          T1->V2
     *
     *          V2->T2
     *          T2->V1
     */
    TestVertex V1, V2, D1;
    TestVertex T1, T2;

    V1.InsertEdge(&T1);
    V1.InsertEdge(&D1);
    T1.InsertEdge(&V2);

    Graph graph;

    // Adding vertices in 'random' order
    graph.AddVertex(&V1);
    graph.AddVertex(&D1);
    graph.AddVertex(&T1);
    graph.AddVertex(&V2);

    r = graph.TopoSortGraph();
    NL_TEST_ASSERT(inSuite, r == ErrorType_Success);

    V2.InsertEdge(&T2);
    T2.InsertEdge(&V1);
    graph.AddVertex(&T2);

    r = graph.TopoSortGraph();
    NL_TEST_ASSERT(inSuite, r == ErrorType_BadConfiguration);

    r = graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, r == ErrorType_BadConfiguration);

    T2.RemoveEdge(&V1);

    r = graph.TopoSortGraph();
    NL_TEST_ASSERT(inSuite, r == ErrorType_Success);

    r = graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, r == ErrorType_Success);

    graph.RemoveVertex(&V1);
    graph.RemoveVertex(&D1);
    graph.RemoveVertex(&V2);
    graph.RemoveVertex(&T1);
    graph.RemoveVertex(&T2);
}

static void Test_BadlyFormedGraph(nlTestSuite *inSuite, void *inContext)
{
    ErrorType r = ErrorType_Success;

    class TestVertex : public Vertex
    {
    public:
        TestVertex() { }
        virtual ~TestVertex() {}
        void ProcessVertex() { } // LCOV_EXCL_LINE
        VertexType GetVertexType() const { return Vertex::kTestVertex; } // LCOV_EXCL_LINE
    };

    /*             Graph
     *             ._____.
     *   O         |  O  |,>O
     *   ^         |  ^  /
     *   |         |  | /|
     *   -    ---> |  -  |
     *   |         |  |  |
     *   |         |  |  |
     *   O         |  O  |
     *             *-----*
     */

    /*             Graph
     *             ._____.
     *  V2         | V2  |,>V3
     *   ^         |  ^  /
     *   |         |  | /|
     *  T1    ---> | T1  |
     *   |         |  |  |
     *   |         |  |  |
     *  V1         | V1  |
     *             *-----*
     *
     *
     *
     * Edges:   V1->T1
     *          T1->V2
     *
     *          T1->V3
     */
    TestVertex V1, V2, V3;
    TestVertex T1;

    V1.InsertEdge(&T1);
    T1.InsertEdge(&V2);

    Graph graph;

    // Adding vertices in 'random' order
    graph.AddVertex(&V1);
    graph.AddVertex(&T1);
    graph.AddVertex(&V2);

    r = graph.TopoSortGraph();
    NL_TEST_ASSERT(inSuite, r == ErrorType_Success);

    T1.InsertEdge(&V3);

    r = graph.TopoSortGraph();
    DG_LOG("vertices.size after sort %u\n", graph.GetVertices().size());
    NL_TEST_ASSERT(inSuite, r == ErrorType_BadConfiguration);

    graph.AddVertex(&V3);

    r = graph.TopoSortGraph();
    NL_TEST_ASSERT(inSuite, r == ErrorType_Success);

    graph.RemoveVertex(&V1);
    graph.RemoveVertex(&V2);
    graph.RemoveVertex(&V3);
    graph.RemoveVertex(&T1);
}

namespace {
    // MOCK detector. A transparent multiple-use int-to-uint pass-through detector
    enum TestTopicStateTypes
    {
        kPacketTypeA,
        kPacketTypeB
    };

    struct PacketTypeAnonymous : public TopicState { PacketTypeAnonymous(int aV = 0) : mV(aV) {}; int mV; };
    struct PacketTypeA : public TopicState { PacketTypeA(int aV = 0) : mV(aV) {}; int mV; TopicStateIdType GetId() const { return (TopicStateIdType)kPacketTypeA; } };
    struct PacketTypeB : public TopicState { PacketTypeB(unsigned int aV = 0) : mV(aV) {}; unsigned int mV; TopicStateIdType GetId() const { return (TopicStateIdType)kPacketTypeB; } };
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

static void Test_EvaluateGraph(nlTestSuite *inSuite, void *inContext)
{
    /*
     * Evaluating the graph consists in:
     * - Pop TopicState from mInputQueue
     * - Dispatch that TopicState to its topic
     * - Traverse all vertices
     * -    In the future, maybe:
     * -        * from that topic onwards
     * -        * until no data changes
     * - Compose output list containing ptrs to all new values
     *
     * To test that, assume the graph:
     *   -            TopicB
     *   ^            ^
     *   |            |
     *   O    --->    TestDetector
     *   ^            ^
     *   |            |
     *   -            TopicA
     *
     * This test assures the flow from PushData<>() until the outputList.
     *
     */

    Graph graph;
    TestDetector detector(&graph);

    // Assert
    NL_TEST_ASSERT(inSuite, graph.HasDataPending() == false);

    // Act
    graph.PushData<PacketTypeA>(PacketTypeA(11));

    // Assert
    NL_TEST_ASSERT(inSuite, graph.HasDataPending() == true);

    // Act
    ErrorType r = graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, r == ErrorType_Success);

    // Assert
    NL_TEST_ASSERT(inSuite, graph.GetOutputList().size() == 2);
    NL_TEST_ASSERT(inSuite, graph.GetOutputList().front()->GetId() == kPacketTypeA);
    NL_TEST_ASSERT(inSuite, ptr::dynamic_pointer_cast<const PacketTypeA>(graph.GetOutputList().front()) != NULL);
    NL_TEST_ASSERT(inSuite, ptr::dynamic_pointer_cast<const PacketTypeA>(graph.GetOutputList().front())->mV == 11);

    NL_TEST_ASSERT(inSuite, graph.GetOutputList().back()->GetId() == kPacketTypeB);
    NL_TEST_ASSERT(inSuite, ptr::dynamic_pointer_cast<const PacketTypeB>(graph.GetOutputList().back()) != NULL);
    NL_TEST_ASSERT(inSuite, ptr::dynamic_pointer_cast<const PacketTypeB>(graph.GetOutputList().back())->mV == 11);

    // Assert
    NL_TEST_ASSERT(inSuite, graph.HasDataPending() == false);

    // Act (with no data)
    r = graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, r == ErrorType_Success);
    NL_TEST_ASSERT(inSuite, graph.GetOutputList().size() == 0);

    // Assert
    NL_TEST_ASSERT(inSuite, graph.HasDataPending() == false);
}

static void Test_HasDataPending(nlTestSuite *inSuite, void *inContext)
{
    Graph graph;
    TestDetector detector(&graph);

    // Assert
    NL_TEST_ASSERT(inSuite, graph.HasDataPending() == false);


    graph.PushData<PacketTypeA>(PacketTypeA(11));
    NL_TEST_ASSERT(inSuite, graph.HasDataPending() == true);


    ErrorType r = graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, r == ErrorType_Success);
    NL_TEST_ASSERT(inSuite, graph.HasDataPending() == false);
}

static void Test_EvaluateIfHasDataPending(nlTestSuite *inSuite, void *inContext)
{
    Graph graph;
    TestDetector detector(&graph);

    graph.PushData<PacketTypeA>(PacketTypeA(11));

    bool evaluated;

    evaluated = graph.EvaluateIfHasDataPending();
    NL_TEST_ASSERT(inSuite, evaluated == true);

    // TODO(DGRAPH-4): This test used to push multiple PacketTypeAs and
    // evaluate them sequentially later.

    graph.PushData<PacketTypeA>(PacketTypeA(22));

    evaluated = graph.EvaluateIfHasDataPending();
    NL_TEST_ASSERT(inSuite, evaluated == true);

    evaluated = graph.EvaluateIfHasDataPending();
    NL_TEST_ASSERT(inSuite, evaluated == false);

    NL_TEST_ASSERT(inSuite, detector.mEvalCount == 2);
}

static void Test_TopicResolution(nlTestSuite *inSuite, void *inContext)
{
    Graph* graph = new Graph();

    // Assert that auto-generated objects are bounded
    Topic<PacketTypeA>* topicAPtrA = graph->ResolveTopic<PacketTypeA>();
    Topic<PacketTypeA>* topicAPtrB = graph->ResolveTopic<PacketTypeA>();
    NL_TEST_ASSERT(inSuite, topicAPtrA == topicAPtrB);

    Topic<PacketTypeA>* topicAPtrC = graph->GetTopicRegistry().Resolve<PacketTypeA>();
    NL_TEST_ASSERT(inSuite, topicAPtrA == topicAPtrC);

    Topic<PacketTypeB> staticTopicB;
    graph->GetTopicRegistry().Register<PacketTypeB>(&staticTopicB);
    Topic<PacketTypeB>* topicBPtrB = graph->ResolveTopic<PacketTypeB>();
    NL_TEST_ASSERT(inSuite, &staticTopicB == topicBPtrB);

    delete graph;
}

// Tests deleting a graph with a non-empty InputQueue
static void Test_NonEmptyQueues(nlTestSuite *inSuite, void *inContext)
{
    Graph* graph = new Graph();

    graph->PushData<PacketTypeA>(PacketTypeA(99));

    delete graph;
}

static void Test_TopicDataTypes(nlTestSuite *inSuite, void *inContext)
{
    Graph* graph = new Graph();

    graph->PushData<PacketTypeA>(PacketTypeA(99));
    graph->PushData<PacketTypeAnonymous>(PacketTypeAnonymous(101));

    ErrorType r = graph->EvaluateGraph();
    NL_TEST_ASSERT(inSuite, r == ErrorType_Success);
    NL_TEST_ASSERT(inSuite, graph->GetOutputList().size() == 1);
    NL_TEST_ASSERT(inSuite, graph->GetOutputList().front()->GetId() == kPacketTypeA);

    r = graph->EvaluateGraph();
    NL_TEST_ASSERT(inSuite, r == ErrorType_Success);
    NL_TEST_ASSERT(inSuite, graph->GetOutputList().size() == 1);
    NL_TEST_ASSERT(inSuite, graph->GetOutputList().front()->GetId() == TopicState::kAnonymousTopicState);

    delete graph;
}

static const nlTest sTests[] = {
    NL_TEST_DEF("Test_Lifetime", Test_Lifetime),
    NL_TEST_DEF("Test_Toposort", Test_Toposort),
    NL_TEST_DEF("Test_SelfLoopDetection", Test_SelfLoopDetection),
    NL_TEST_DEF("Test_LoopDetection", Test_LoopDetection),
    NL_TEST_DEF("Test_LoopLoopDetection", Test_LoopLoopDetection),
    NL_TEST_DEF("Test_BadlyFormedGraph", Test_BadlyFormedGraph),
    NL_TEST_DEF("Test_HasDataPending", Test_HasDataPending),
    NL_TEST_DEF("Test_EvaluateIfHasDataPending", Test_EvaluateIfHasDataPending),
    NL_TEST_DEF("Test_TopicResolution", Test_TopicResolution),
    NL_TEST_DEF("Test_EvaluateGraph", Test_EvaluateGraph),
    NL_TEST_DEF("Test_NonEmptyQueues", Test_NonEmptyQueues),
    NL_TEST_DEF("Test_TopicDataTypes", Test_TopicDataTypes),
    NL_TEST_SENTINEL()
};

//This function creates the Suite (i.e: the name of your test and points to the array of test functions)
extern "C"
int graph_testsuite(void)
{
    nlTestSuite theSuite = SUITE_DECLARATION(graph, &sTests[0]);
    nlTestRunner(&theSuite, NULL);
    return nlTestRunnerStats(&theSuite);
}
