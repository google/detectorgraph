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

#include <typeinfo>
#include <iostream>

#include "nltest.h"
#include "errortype.hpp"

#include "graph.hpp"
#include "detector.hpp"
#include "futurepublisher.hpp"
#include "topicstate.hpp"

#include "test_futurepublisher.h"

#define SUITE_DECLARATION(name, test_ptr) { #name, test_ptr, setup_##name, teardown_##name }

using namespace DetectorGraph;

/* This is a test for the PublishOnFutureEvaluation capability.
 *
 * For that we use a 'Echo' detector that will re-publish it's input TopicState
 * for a configurable number of times.
 */

// BEGIN FAKE DETECTOR
struct EchoTopicState : public TopicState
{
    enum EchoSource { FUTURE_PUBLISH, EXTERNAL_PUBLISH, NONE };

    EchoTopicState() : mEchoSource(NONE), mLimit(0) { }
    EchoTopicState(EchoSource aEchoSource, int aLimit) : mEchoSource(aEchoSource), mLimit(aLimit) { }
    EchoSource mEchoSource;
    int mLimit;
};

// This mock detector echoes its input EchoTopicState.mLimit number of times.
struct SampleEchoDetector : public Detector,
    public SubscriberInterface<EchoTopicState>,
    public FuturePublisher<EchoTopicState>
{
    SampleEchoDetector(Graph* graph) : Detector(graph), mFutureState(), mEvalCount(0)
    {
        Subscribe<EchoTopicState>(this);
        SetupFuturePublishing<EchoTopicState>(this);
    }

    virtual void Evaluate(const EchoTopicState& aFutureState)
    {
        mFutureState = aFutureState;
        if (mEvalCount < mFutureState.mLimit)
        {
            EchoTopicState newState = mFutureState;
            newState.mEchoSource = EchoTopicState::FUTURE_PUBLISH;
            PublishOnFutureEvaluation(newState);
        }
        mEvalCount++;
    }

    EchoTopicState mFutureState;
    int mEvalCount;
};
// END FAKE DETECTOR

static int setup_futurepublisher(void *inContext)
{
    return 0;
}

static int teardown_futurepublisher(void *inContext)
{
    return 0;
}

static void Test_GraphTopology(nlTestSuite *inSuite, void *inContext)
{
    Graph graph;
    SampleEchoDetector detector(&graph);
    NL_TEST_ASSERT(inSuite, detector.mFutureState.mEchoSource == EchoTopicState::NONE);
    NL_TEST_ASSERT(inSuite, graph.GetVertices().size() == 2);
    NL_TEST_ASSERT(inSuite, graph.ResolveTopic<EchoTopicState>()->GetOutEdges().front() == &detector);
    NL_TEST_ASSERT(inSuite, static_cast<Vertex*>(&detector)->GetOutEdges().size() == 0);
    NL_TEST_ASSERT(inSuite, static_cast<Vertex*>(&detector)->GetFutureOutEdges().front() == graph.ResolveTopic<EchoTopicState>());
    NL_TEST_ASSERT(inSuite, static_cast<Vertex*>(&detector)->GetInEdges().front() == graph.ResolveTopic<EchoTopicState>());
    NL_TEST_ASSERT(inSuite, static_cast<Vertex*>(&detector)->GetFutureInEdges().size() == 0);
}

static void Test_PublishOnFutureEvaluation(nlTestSuite *inSuite, void *inContext)
{
    Graph graph;
    SampleEchoDetector detector(&graph);

    graph.PushData<EchoTopicState>(EchoTopicState(EchoTopicState::EXTERNAL_PUBLISH, 1));

    NL_TEST_ASSERT(inSuite, detector.mFutureState.mEchoSource == EchoTopicState::NONE);
    NL_TEST_ASSERT(inSuite, detector.mEvalCount == 0);

    graph.EvaluateGraph();

    NL_TEST_ASSERT(inSuite, detector.mFutureState.mEchoSource == EchoTopicState::EXTERNAL_PUBLISH);
    NL_TEST_ASSERT(inSuite, detector.mEvalCount == 1);

    graph.EvaluateGraph();

    NL_TEST_ASSERT(inSuite, detector.mFutureState.mEchoSource == EchoTopicState::FUTURE_PUBLISH);
    NL_TEST_ASSERT(inSuite, detector.mEvalCount == 2);
}

static void Test_EvaluateAllPendingWithFuturePublish(nlTestSuite *inSuite, void *inContext)
{
    Graph graph;
    SampleEchoDetector detector(&graph);

    graph.PushData<EchoTopicState>(EchoTopicState(EchoTopicState::EXTERNAL_PUBLISH, 5));

    NL_TEST_ASSERT(inSuite, detector.mFutureState.mEchoSource == EchoTopicState::NONE);
    NL_TEST_ASSERT(inSuite, detector.mEvalCount == 0);

    while (graph.EvaluateIfHasDataPending()) { /* keep going */ }

    NL_TEST_ASSERT(inSuite, detector.mFutureState.mEchoSource == EchoTopicState::FUTURE_PUBLISH);
    NL_TEST_ASSERT(inSuite, detector.mEvalCount == 6);
}

static const nlTest sTests[] = {
    NL_TEST_DEF("Test_GraphTopology", Test_GraphTopology),
    NL_TEST_DEF("Test_PublishOnFutureEvaluation", Test_PublishOnFutureEvaluation),
    NL_TEST_DEF("Test_EvaluateAllPendingWithFuturePublish", Test_EvaluateAllPendingWithFuturePublish),
    NL_TEST_SENTINEL()
};

//This function creates the Suite (i.e: the name of your test and points to the array of test functions)
extern "C"
int futurepublisher_testsuite(void)
{
    nlTestSuite theSuite = SUITE_DECLARATION(futurepublisher, &sTests[0]);
    nlTestRunner(&theSuite, NULL);
    return nlTestRunnerStats(&theSuite);
}
