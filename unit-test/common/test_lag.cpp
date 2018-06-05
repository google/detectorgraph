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

#include "test_lag.h"

#include "graph.hpp"
#include "lag.hpp"

#define SUITE_DECLARATION(name, test_ptr) { #name, test_ptr, setup_##name, teardown_##name }

using namespace DetectorGraph;

static int setup_lag(void *inContext)
{
    return 0;
}

static int teardown_lag(void *inContext)
{
    return 0;
}

namespace {

    struct StartTopicState : public TopicState { StartTopicState(int aV = 0) : mV(aV) {}; int mV; };

    struct LoopTopicState : public TopicState { LoopTopicState(int aV = 0) : mV(aV) {}; int mV; };

    struct LoopTestDetector
    : public Detector
    , public SubscriberInterface<StartTopicState>
    , public SubscriberInterface< Lagged<LoopTopicState> >
    , public Publisher<LoopTopicState>
    {
        LoopTestDetector(Graph* graph) : Detector(graph), mState(0)
        {
            Subscribe<StartTopicState>(this);
            Subscribe< Lagged<LoopTopicState> >(this);
            SetupPublishing<LoopTopicState>(this);
        }
        virtual void Evaluate(const StartTopicState& aInData)
        {
            mState = LoopTopicState(1);
            Publish(mState);
        }

        virtual void Evaluate(const Lagged<LoopTopicState>& aLoopData)
        {
            mState = aLoopData.data;
            mState.mV++;
            if (mState.mV < 5)
            {
                Publish(mState);
            }
        }

        LoopTopicState mState;
    };
}

static void Test_LaggedDataConstructor(nlTestSuite *inSuite, void *inContext)
{
    Lagged<LoopTopicState> dummy;
    NL_TEST_ASSERT(inSuite, dummy.data.mV == 0);
}

static void Test_FeedbackLoop(nlTestSuite *inSuite, void *inContext)
{
    Graph graph;
    graph.ResolveTopic<StartTopicState>();
    Topic< Lagged<LoopTopicState> >* outputTopic = graph.ResolveTopic< Lagged<LoopTopicState> >();
    LoopTestDetector detector(&graph);
    graph.ResolveTopic<LoopTopicState>();
    Lag<LoopTopicState> delayDetector(&graph);

    graph.PushData<StartTopicState>(0);

    NL_TEST_ASSERT(inSuite, detector.mState.mV == 0);

    graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, detector.mState.mV == 1);
    NL_TEST_ASSERT(inSuite, !outputTopic->HasNewValue());

    graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, detector.mState.mV == 2);
    NL_TEST_ASSERT(inSuite, outputTopic->HasNewValue());
    NL_TEST_ASSERT(inSuite, outputTopic->GetNewValue().data.mV == 1);

    graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, detector.mState.mV == 3);
    NL_TEST_ASSERT(inSuite, outputTopic->HasNewValue());
    NL_TEST_ASSERT(inSuite, outputTopic->GetNewValue().data.mV == 2);

    graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, detector.mState.mV == 4);
    NL_TEST_ASSERT(inSuite, outputTopic->HasNewValue());
    NL_TEST_ASSERT(inSuite, outputTopic->GetNewValue().data.mV == 3);

    graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, detector.mState.mV == 5);
    NL_TEST_ASSERT(inSuite, outputTopic->HasNewValue());
    NL_TEST_ASSERT(inSuite, outputTopic->GetNewValue().data.mV == 4);

    graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, detector.mState.mV == 5);
    NL_TEST_ASSERT(inSuite, !outputTopic->HasNewValue());
}

static const nlTest sTests[] = {
    NL_TEST_DEF("Test_LaggedDataConstructor", Test_LaggedDataConstructor),
    NL_TEST_DEF("Test_FeedbackLoop", Test_FeedbackLoop),
    NL_TEST_SENTINEL()
};

extern "C"
int lag_testsuite(void)
{
    nlTestSuite theSuite = SUITE_DECLARATION(lag, &sTests[0]);
    nlTestRunner(&theSuite, NULL);
    return nlTestRunnerStats(&theSuite);
}
