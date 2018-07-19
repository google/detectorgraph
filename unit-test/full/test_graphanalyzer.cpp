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

#include "test_graphanalyzer.h"

#include "graphanalyzer.hpp"
#include "detector.hpp"
#include "futurepublisher.hpp"
#include "testtimeoutpublisherservice.hpp"

#include <typeinfo>
#include <iostream>
#include <fstream>

#define GRAPHVIZ_DIR               "tmptest/graphviz/"

#define SUITE_DECLARATION(name, test_ptr) { #name, test_ptr, setup_##name, teardown_##name }

using namespace DetectorGraph;

static int setup_graphanalyzer(void *inContext)
{
    int r;

    r = system("mkdir -p " GRAPHVIZ_DIR);

    return r;
}

static int teardown_graphanalyzer(void *inContext)
{
    int r;

    r = system("rm -fR " GRAPHVIZ_DIR "*");

    return r;
}

std::string DoesNothingFilter(const std::string& aInStr)
{
    std::string retString = aInStr;
    return retString;
}

std::string WrapTypeString(const std::string& aInStr)
{
    std::string retString = aInStr;
    return retString;
}

// LCOV_EXCL_START
struct TopicA : public TopicState { };
struct TopicB : public TopicState { };
struct TopicC : public TopicState { };
struct TopicD : public TopicState { };
struct TopicE : public TopicState { };
struct TopicF : public TopicState { };
struct TopicI : public TopicState { };

struct TopicG : public TopicState { DetectorGraph::TopicStateIdType GetId() const { return (DetectorGraph::TopicStateIdType)1; } };
struct TopicH : public TopicState { DetectorGraph::TopicStateIdType GetId() const { return (DetectorGraph::TopicStateIdType)2; } };
struct TopicJ : public TopicState { DetectorGraph::TopicStateIdType GetId() const { return (DetectorGraph::TopicStateIdType)3; } };

class DetectorA : public Detector,
    public SubscriberInterface<TopicA>,
    public SubscriberInterface<TopicB>,
    public SubscriberInterface<TopicC>,
    public Publisher<TopicD>,
    public Publisher<TopicI>,
    public FuturePublisher<TopicB>,
    public TimeoutPublisher<TopicC>
{
public:
    DetectorA(Graph* graph, TimeoutPublisherService* apService) : Detector(graph)
    {
        Subscribe<TopicA>(this);
        Subscribe<TopicB>(this);
        Subscribe<TopicC>(this);
        SetupPublishing<TopicD>(this);
        SetupPublishing<TopicI>(this);
        SetupFuturePublishing<TopicB>(this);
        SetupTimeoutPublishing<TopicC>(this, apService);
    }

    virtual void Evaluate(const TopicA&) { }
    virtual void Evaluate(const TopicB&) { }
    virtual void Evaluate(const TopicC&) { }
};

class DetectorB : public Detector,
    public SubscriberInterface<TopicD>,
    public Publisher<TopicE>
{
public:
    DetectorB(Graph* graph) : Detector(graph)
    {
        Subscribe<TopicD>(this);
        SetupPublishing<TopicE>(this);
    }

    virtual void Evaluate(const TopicD&) { }
};

class DetectorC : public Detector,
    public SubscriberInterface<TopicF>,
    public Publisher<TopicG>
{
public:
    DetectorC(Graph* graph) : Detector(graph)
    {
        Subscribe<TopicF>(this);
        SetupPublishing<TopicG>(this);
    }

    virtual void Evaluate(const TopicF&) { }
};

class DetectorD : public Detector,
    public SubscriberInterface<TopicA>,
    public SubscriberInterface<TopicD>,
    public SubscriberInterface<TopicE>,
    public Publisher<TopicH>
{
public:
    DetectorD(Graph* graph) : Detector(graph)
    {
        Subscribe<TopicA>(this);
        Subscribe<TopicD>(this);
        Subscribe<TopicE>(this);
        SetupPublishing<TopicH>(this);
    }

    virtual void Evaluate(const TopicA&) { }
    virtual void Evaluate(const TopicD&) { }
    virtual void Evaluate(const TopicE&) { }
};

class DetectorE : public Detector,
    public SubscriberInterface<TopicI>,
    public Publisher<TopicJ>
{
public:
    DetectorE(Graph* graph) : Detector(graph)
    {
        Subscribe<TopicI>(this);
        SetupPublishing<TopicJ>(this);
    }

    virtual void Evaluate(const TopicI&) { }
};

// LCOV_EXCL_STOP

static void Test_PrintVertex(nlTestSuite *inSuite, void *inContext)
{
    Graph graph;
    TestTimeoutPublisherService timeoutService(graph);
    DetectorA detectorA(&graph, &timeoutService);
    GraphAnalyzer analyzer(graph);

    analyzer.PrintVertexes();

    NL_TEST_ASSERT(inSuite, true);
}

static void Test_GenerateDotFile(nlTestSuite *inSuite, void *inContext)
{
    Graph graph;
    TestTimeoutPublisherService timeoutService(graph);
    DetectorA detectorA(&graph, &timeoutService);
    DetectorB detectorB(&graph);
    DetectorC detectorC(&graph);
    DetectorD detectorD(&graph);
    DetectorE detectorE(&graph);

    ErrorType r = graph.TopoSortGraph();
    NL_TEST_ASSERT(inSuite, r == ErrorType_Success);

    GraphAnalyzer analyzer(graph);

    analyzer.SetStringFilter(&DoesNothingFilter);
    analyzer.SetLabelWordWrapper(&WrapTypeString);

    // Act to create .dot file
    analyzer.GenerateDotFile(GRAPHVIZ_DIR "detectorgraph.dot");

    // Assert .dot file
    int number_of_lines = 0;
    std::string line;
    std::ifstream createdFile(GRAPHVIZ_DIR "detectorgraph.dot");

    while (std::getline(createdFile, line))
    {
        ++number_of_lines;
    }
    printf("Test_GenerateDotFile number_of_lines==%d\n", number_of_lines);
    NL_TEST_ASSERT(inSuite, number_of_lines == 19 /* legend */ + 37 /* 10*topic, 5*detector, 15*connectors, 2*futureConnector, 5*header/footer */);
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

static void Test_ConflictAcrossDetectors(nlTestSuite *inSuite, void *inContext)
{
    /*
     *
     * To test that, assume the graph:
     *     -            TopicB
     *     ^             ^
     *    / \            |
     *   O   O --->    (TestDetectorA, TestDetectorB)
     *   ^   ^           ^
     *   \   /           |
     *    \_/           TopicA
     *
     *
     * With:
     *
     * @code

void TestDetector::Evaluate(TopicA)
{
    Publish(TopicB());
}

     * @endcode
     *
     * When TopicB is a Public/Named Topic.
     */

    Graph graph;
    GraphAnalyzer analyzer(graph);

    TestDetector detectorA(&graph);

    NL_TEST_ASSERT(inSuite, analyzer.HasPublicConflict() == false);

    TestDetector detectorB(&graph);

    NL_TEST_ASSERT(inSuite, analyzer.HasPublicConflict() == true);
}

static const nlTest sTests[] = {
    NL_TEST_DEF("Test_PrintVertex", Test_PrintVertex),
    NL_TEST_DEF("Test_GenerateDotFile", Test_GenerateDotFile),
    NL_TEST_DEF("Test_ConflictAcrossDetectors", Test_ConflictAcrossDetectors),
    NL_TEST_SENTINEL()
};

//This function creates the Suite (i.e: the name of your test and points to the array of test functions)
extern "C"
int graphanalyzer_testsuite(void)
{
    nlTestSuite theSuite = SUITE_DECLARATION(graphanalyzer, &sTests[0]);
    nlTestRunner(&theSuite, NULL);
    return nlTestRunnerStats(&theSuite);
}
