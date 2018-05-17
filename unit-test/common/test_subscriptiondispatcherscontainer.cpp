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

#include "test_subscriptiondispatcherscontainer.h"

#include "subscriptiondispatcherscontainer.hpp"
#include "topicstate.hpp"
#include "graph.hpp"

#include "nltest.h"

#define SUITE_DECLARATION(name, test_ptr) { #name, test_ptr, setup_##name, teardown_##name }

using namespace DetectorGraph;

static int setup_subscriptiondispatcherscontainer(void *inContext)
{
    return 0;
}

static int teardown_subscriptiondispatcherscontainer(void *inContext)
{
    return 0;
}

struct TestTopicStateA : public TopicState
{
    TestTopicStateA(int av = 0) : v(av) {}
    int v;
};

struct MockSubscriber : public SubscriberInterface<TestTopicStateA>
{
    void Evaluate(const TestTopicStateA& aTestA)
    {
        testA = aTestA;
    }
    TestTopicStateA testA;
};

static void Test_GetSize(nlTestSuite *inSuite, void *inContext)
{
    SubscriptionDispatchersContainer container;
    NL_TEST_ASSERT(inSuite, container.GetSize() == 0);

    Topic<TestTopicStateA>* fakeTopic = NULL;
    SubscriberInterface<TestTopicStateA>* fakeSubscriber = NULL;
    container.CreateDispatcher(fakeTopic, fakeSubscriber);
    NL_TEST_ASSERT(inSuite, container.GetSize() == 1);
}


static void Test_CreateAndDispatch(nlTestSuite *inSuite, void *inContext)
{
    SubscriptionDispatchersContainer container;

    Topic<TestTopicStateA> topic;
    MockSubscriber subscriber;

    container.CreateDispatcher(&topic, &subscriber);
    NL_TEST_ASSERT(inSuite, container.GetSize() == 1);
    NL_TEST_ASSERT(inSuite, container.GetDispatchers()[0]->GetTopicVertex() == static_cast<Vertex*>(&topic));

    topic.Publish(TestTopicStateA(42));
    topic.ProcessVertex();
    container.GetDispatchers()[0]->Dispatch();

    NL_TEST_ASSERT(inSuite, subscriber.testA.v == 42);
}

static const nlTest sTests[] = {
    NL_TEST_DEF("Test_GetSize", Test_GetSize),
    NL_TEST_DEF("Test_CreateAndDispatch", Test_CreateAndDispatch),
    NL_TEST_SENTINEL()
};

//This function creates the Suite (i.e: the name of your test and points to the array of test functions)
extern "C"
int subscriptiondispatcherscontainer_testsuite(void)
{
    nlTestSuite theSuite = SUITE_DECLARATION(subscriptiondispatcherscontainer, &sTests[0]);
    nlTestRunner(&theSuite, NULL);
    return nlTestRunnerStats(&theSuite);
}
