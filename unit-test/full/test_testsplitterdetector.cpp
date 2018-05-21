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

#include "test_testsplitterdetector.h"

#include "graph.hpp"
#include "detector.hpp"
#include "vertex.hpp"
#include "topicstate.hpp"
#include "testsplitterdetector.hpp"
#include "graphtestutils.hpp"

#define SUITE_DECLARATION(name, test_ptr) { #name, test_ptr, setup_##name, teardown_##name }

using namespace DetectorGraph;

static int setup_testsplitterdetector(void *inContext)
{
    return 0;
}

static int teardown_testsplitterdetector(void *inContext)
{
    return 0;
}

namespace
{
    struct TopicTypeA : public TopicState { TopicTypeA(int aV = 0) : mV(aV) {}; int mV; };
    struct TopicTypeB : public TopicState { TopicTypeB(int aV = 0) : mV(aV) {}; int mV; };
}

static void Test_Splitting(nlTestSuite *inSuite, void *inContext)
{
    Graph graph;
    TestSplitterDetector<TopicTypeA, TopicTypeB> splitter(&graph);

    graph.PushData< TestSplitterTrigger<TopicTypeA, TopicTypeB> >(
        TestSplitterTrigger<TopicTypeA, TopicTypeB>(TopicTypeA(12345), TopicTypeB(54321)));
    graph.EvaluateGraph();

    Topic<TopicTypeA>* outA = graph.ResolveTopic<TopicTypeA>();
    Topic<TopicTypeB>* outB = graph.ResolveTopic<TopicTypeB>();

    NL_TEST_ASSERT(inSuite, outA->GetCurrentValues().front().mV == 12345);
    NL_TEST_ASSERT(inSuite, outB->GetCurrentValues().front().mV == 54321);

    GraphTestUtils::PrintOutputs(graph);
}

static const nlTest sTests[] = {
    NL_TEST_DEF("Test_Splitting", Test_Splitting),
    NL_TEST_SENTINEL()
};

//This function creates the Suite (i.e: the name of your test and points to the array of test functions)
extern "C"
int testsplitterdetector_testsuite(void)
{
    nlTestSuite theSuite = SUITE_DECLARATION(testsplitterdetector, &sTests[0]);
    nlTestRunner(&theSuite, NULL);
    return nlTestRunnerStats(&theSuite);
}
