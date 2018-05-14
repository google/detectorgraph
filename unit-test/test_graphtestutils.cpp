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

#include "test_graphtestutils.h"

#include "graph.hpp"
#include "detector.hpp"
#include "vertex.hpp"
#include "topicstate.hpp"
#include "graphtestutils.hpp"

#include <typeinfo>
#include <iostream>

#define SUITE_DECLARATION(name, test_ptr) { #name, test_ptr, setup_##name, teardown_##name }

using namespace DetectorGraph;

static int setup_graphtestutils(void *inContext)
{
    return 0;
}

static int teardown_graphtestutils(void *inContext)
{
    return 0;
}

namespace
{
    struct TopicTypeA : public TopicState { TopicTypeA(int aV = 0) : mV(aV) {}; int mV; };
    struct TopicTypeB : public TopicState { TopicTypeB(int aV = 0) : mV(aV) {}; int mV; };
}

static void Test_Flush(nlTestSuite *inSuite, void *inContext)
{
    Graph graph;

    Topic<TopicTypeA>* outA = graph.ResolveTopic<TopicTypeA>();
    NL_TEST_ASSERT(inSuite, outA->GetCurrentValues().size() == 0);
    Topic<TopicTypeB>* outB = graph.ResolveTopic<TopicTypeB>();
    NL_TEST_ASSERT(inSuite, outB->GetCurrentValues().size() == 0);

    // Push two TS to the inputQueue
    graph.PushData<TopicTypeA>(TopicTypeA());
    graph.PushData<TopicTypeB>(TopicTypeB());

    // Evaluate first
    graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, outA->GetCurrentValues().size() == 1);
    NL_TEST_ASSERT(inSuite, outB->GetCurrentValues().size() == 0);

    // Evaluate second
    graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, outA->GetCurrentValues().size() == 0);
    NL_TEST_ASSERT(inSuite, outB->GetCurrentValues().size() == 1);

    // nothing left to Evaluate
    graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, outA->GetCurrentValues().size() == 0);
    NL_TEST_ASSERT(inSuite, outB->GetCurrentValues().size() == 0);

    // Push two TS to the inputQueue
    graph.PushData<TopicTypeA>(TopicTypeA());
    graph.PushData<TopicTypeB>(TopicTypeB());

    // Flush Everything
    GraphTestUtils::Flush(graph);
    NL_TEST_ASSERT(inSuite, outA->GetCurrentValues().size() == 0);
    NL_TEST_ASSERT(inSuite, outB->GetCurrentValues().size() == 1);

    // Nothing left to Evaluate
    graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, outA->GetCurrentValues().size() == 0);
    NL_TEST_ASSERT(inSuite, outB->GetCurrentValues().size() == 0);
}

static void Test_FlushAndPush(nlTestSuite *inSuite, void *inContext)
{
    Graph graph;

    Topic<TopicTypeA>* outA = graph.ResolveTopic<TopicTypeA>();

    // Push two TS to the inputQueue
    GraphTestUtils::FlushAndPush<TopicTypeA>(graph, TopicTypeA());
    GraphTestUtils::FlushAndPush<TopicTypeA>(graph, TopicTypeA());
    GraphTestUtils::FlushAndPush<TopicTypeA>(graph, TopicTypeA());
    GraphTestUtils::FlushAndPush<TopicTypeA>(graph, TopicTypeA());

    graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, outA->GetCurrentValues().size() == 1);

    // FlushAndPush doesn't let TSs accumulate
    graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, outA->GetCurrentValues().size() == 0);
}

static void Test_PrintOutputs(nlTestSuite *inSuite, void *inContext)
{
    Graph graph;

    // Push two TS to the inputQueue
    graph.PushData<TopicTypeA>(TopicTypeA());
    graph.EvaluateGraph();
    GraphTestUtils::PrintOutputs(graph);
}

static const nlTest sTests[] = {
    NL_TEST_DEF("Test_Flush", Test_Flush),
    NL_TEST_DEF("Test_FlushAndPush", Test_FlushAndPush),
    NL_TEST_DEF("Test_PrintOutputs", Test_PrintOutputs),
    NL_TEST_SENTINEL()
};

//This function creates the Suite (i.e: the name of your test and points to the array of test functions)
extern "C"
int graphtestutils_testsuite(void)
{
    nlTestSuite theSuite = SUITE_DECLARATION(graphtestutils, &sTests[0]);
    nlTestRunner(&theSuite, NULL);
    return nlTestRunnerStats(&theSuite);
}
