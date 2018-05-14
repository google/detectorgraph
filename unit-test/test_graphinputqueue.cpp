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

#include "test_graphinputqueue.h"

#include "graphinputqueue.hpp"
#include "topicstate.hpp"
#include "graph.hpp"

#include "nltest.h"

#define SUITE_DECLARATION(name, test_ptr) { #name, test_ptr, setup_##name, teardown_##name }

using namespace DetectorGraph;

static int setup_graphinputqueue(void *inContext)
{
    return 0;
}

static int teardown_graphinputqueue(void *inContext)
{
    return 0;
}

struct TestTopicStateA : public DetectorGraph::TopicState
{
    TestTopicStateA(int av = 0) : v(av) {}
    int v;
};

struct TestTopicStateB : public DetectorGraph::TopicState
{
    TestTopicStateB(int av = 0) : v(av) {}
    int v;
};

static void Test_IsEmpty(nlTestSuite *inSuite, void *inContext)
{
    GraphInputQueue inputQueue;
    NL_TEST_ASSERT(inSuite, inputQueue.IsEmpty() == true);

    Topic<TestTopicStateA> topic;
    inputQueue.Enqueue(topic, TestTopicStateA(42));
    NL_TEST_ASSERT(inSuite, inputQueue.IsEmpty() == false);

    inputQueue.DequeueAndDispatch();
    NL_TEST_ASSERT(inSuite, inputQueue.IsEmpty() == true);
}


static void Test_DequeueAndDispatch(nlTestSuite *inSuite, void *inContext)
{
    GraphInputQueue inputQueue;
    Topic<TestTopicStateA> topic;

    inputQueue.Enqueue(topic, TestTopicStateA(42));
    bool retValue = inputQueue.DequeueAndDispatch();

    topic.ProcessVertex();
    NL_TEST_ASSERT(inSuite, retValue == true);
    NL_TEST_ASSERT(inSuite, topic.HasNewValue() == true);
    NL_TEST_ASSERT(inSuite, topic.GetNewValue().v == 42);
}

static void Test_DequeueMultiple(nlTestSuite *inSuite, void *inContext)
{
    GraphInputQueue inputQueue;
    Topic<TestTopicStateA> topicA;
    Topic<TestTopicStateB> topicB;

    inputQueue.Enqueue(topicA, TestTopicStateA(42));
    inputQueue.Enqueue(topicB, TestTopicStateB(99));

    bool retValue = inputQueue.DequeueAndDispatch();
    topicA.ProcessVertex();
    topicB.ProcessVertex();
    NL_TEST_ASSERT(inSuite, retValue == true);
    NL_TEST_ASSERT(inSuite, topicA.HasNewValue() == true);
    NL_TEST_ASSERT(inSuite, topicA.GetNewValue().v == 42);
    NL_TEST_ASSERT(inSuite, topicB.HasNewValue() == false);

    topicA.SetState(Vertex::kVertexClear);
    topicB.SetState(Vertex::kVertexClear);
    retValue = inputQueue.DequeueAndDispatch();
    topicA.ProcessVertex();
    topicB.ProcessVertex();
    NL_TEST_ASSERT(inSuite, retValue == true);
    NL_TEST_ASSERT(inSuite, topicA.HasNewValue() == false);
    NL_TEST_ASSERT(inSuite, topicB.HasNewValue() == true);
    NL_TEST_ASSERT(inSuite, topicB.GetNewValue().v == 99);
}

static void Test_Cleanup(nlTestSuite *inSuite, void *inContext)
{
    GraphInputQueue inputQueue;
    Topic<TestTopicStateA> topic;
    inputQueue.Enqueue(topic, TestTopicStateA(42));
}

static const nlTest sTests[] = {
    NL_TEST_DEF("Test_IsEmpty", Test_IsEmpty),
    NL_TEST_DEF("Test_DequeueAndDispatch", Test_DequeueAndDispatch),
    NL_TEST_DEF("Test_DequeueMultiple", Test_DequeueMultiple),
    NL_TEST_DEF("Test_Cleanup", Test_Cleanup),
    NL_TEST_SENTINEL()
};

//This function creates the Suite (i.e: the name of your test and points to the array of test functions)
extern "C"
int graphinputqueue_testsuite(void)
{
    nlTestSuite theSuite = SUITE_DECLARATION(graphinputqueue, &sTests[0]);
    nlTestRunner(&theSuite, NULL);
    return nlTestRunnerStats(&theSuite);
}
