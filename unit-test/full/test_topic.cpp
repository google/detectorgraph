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

#include "test_topic.h"

#include "vertex.hpp"
#include "topic.hpp"

#include <string.h>

#define SUITE_DECLARATION(name, test_ptr) { #name, test_ptr, setup_##name, teardown_##name }

using namespace DetectorGraph;

static int setup_topic(void *inContext)
{
    return 0;
}

static int teardown_topic(void *inContext)
{
    return 0;
}

namespace
{
    struct TopicTypeA : public TopicState {}; // LCOV_EXCL_LINE all we need is the type
}

static void Test_Lifetime(nlTestSuite *inSuite, void *inContext)
{
    Topic<TopicTypeA>* topic = new Topic<TopicTypeA>();
    NL_TEST_ASSERT(inSuite, topic->GetCurrentValues().size() == 0);
    delete topic;
}

static void Test_VertexType(nlTestSuite *inSuite, void *inContext)
{
    Topic<TopicTypeA>* topic = new Topic<TopicTypeA>();
    Vertex* vtxPtr = static_cast<Vertex*>(topic);

    NL_TEST_ASSERT(inSuite, vtxPtr->GetVertexType() == Vertex::kTopicVertex);
    delete topic;
}

namespace {
    struct PacketTypeA : public TopicState { PacketTypeA(int aV = 0) : mV(aV) {}; int mV; };
    struct MockSubscriber : public SubscriberInterface<PacketTypeA>
    {
        MockSubscriber() : mEvalCounter(0), mLastValue(0) {}
        void Evaluate(const PacketTypeA& aValue) { mLastValue = aValue; mEvalCounter++; }
        int mEvalCounter;
        PacketTypeA mLastValue;
    };
}

static void Test_PublishProcessDispatch(nlTestSuite *inSuite, void *inContext)
{
    Topic<PacketTypeA>* topic = new Topic<PacketTypeA>();

    MockSubscriber subscriber;

    // Act: Accumulate one packet
    topic->SetState(Vertex::kVertexClear);
    topic->Publish(PacketTypeA(10001));
    topic->ProcessVertex();
    topic->DispatchIntoSubscriber(&subscriber);

    // Assert: only one exists and only one was delivered/eval'd
    NL_TEST_ASSERT(inSuite, topic->GetState() == Vertex::kVertexDone);
    NL_TEST_ASSERT(inSuite, topic->GetCurrentValues().size() == 1);
    NL_TEST_ASSERT(inSuite, subscriber.mEvalCounter == 1);
    NL_TEST_ASSERT(inSuite, subscriber.mLastValue.mV == 10001);

    // Act: Accumulate two packets
    topic->SetState(Vertex::kVertexClear);
    topic->Publish(PacketTypeA(10002));
    topic->Publish(PacketTypeA(10003));
    topic->ProcessVertex();
    topic->DispatchIntoSubscriber(&subscriber);

    // Assert: two exists and eval was called twice and with values in-order
    NL_TEST_ASSERT(inSuite, topic->GetState() == Vertex::kVertexDone);
    NL_TEST_ASSERT(inSuite, topic->GetCurrentValues().size() == 2);
    NL_TEST_ASSERT(inSuite, subscriber.mEvalCounter == 3);
    NL_TEST_ASSERT(inSuite, subscriber.mLastValue.mV == 10003);

    // Act: Don't accumulate any packets
    topic->SetState(Vertex::kVertexClear);
    topic->ProcessVertex();
    topic->DispatchIntoSubscriber(&subscriber);

    // Assert no current values exist and nothing has changed
    NL_TEST_ASSERT(inSuite, topic->GetState() == Vertex::kVertexClear);
    NL_TEST_ASSERT(inSuite, subscriber.mEvalCounter == 3);
    NL_TEST_ASSERT(inSuite, subscriber.mLastValue.mV == 10003);
    NL_TEST_ASSERT(inSuite, topic->GetCurrentValues().size() == 0);

    delete topic;
}

static void Test_GetCurrentTopicStates(nlTestSuite *inSuite, void *inContext)
{
    Topic<PacketTypeA>* topic = new Topic<PacketTypeA>();
    BaseTopic* itopic = static_cast<BaseTopic*>(topic);

    // Act: Accumulate one packet
    topic->SetState(Vertex::kVertexClear);
    topic->Publish(PacketTypeA(10001));
    topic->ProcessVertex();

    // Assert: one only was delivered
    NL_TEST_ASSERT(inSuite, itopic->GetState() == Vertex::kVertexDone);
    NL_TEST_ASSERT(inSuite, itopic->GetCurrentTopicStates().size() == 1);
    NL_TEST_ASSERT(inSuite, ptr::dynamic_pointer_cast< const PacketTypeA >(itopic->GetCurrentTopicStates().front()) != NULL);
    NL_TEST_ASSERT(inSuite, ptr::dynamic_pointer_cast< const PacketTypeA >(itopic->GetCurrentTopicStates().front())->mV == 10001);

    // Act: Accumulate two packets
    topic->SetState(Vertex::kVertexClear);
    topic->Publish(PacketTypeA(10002));
    topic->Publish(PacketTypeA(10003));
    topic->ProcessVertex();

    // Assert: eval was called twice and with values in-order
    NL_TEST_ASSERT(inSuite, itopic->GetState() == Vertex::kVertexDone);
    NL_TEST_ASSERT(inSuite, itopic->GetCurrentTopicStates().size() == 2);
    NL_TEST_ASSERT(inSuite, ptr::dynamic_pointer_cast< const PacketTypeA >(itopic->GetCurrentTopicStates().front()) != NULL);
    NL_TEST_ASSERT(inSuite, ptr::dynamic_pointer_cast< const PacketTypeA >(itopic->GetCurrentTopicStates().front())->mV == 10002);
    NL_TEST_ASSERT(inSuite, ptr::dynamic_pointer_cast< const PacketTypeA >(itopic->GetCurrentTopicStates().back()) != NULL);
    NL_TEST_ASSERT(inSuite, ptr::dynamic_pointer_cast< const PacketTypeA >(itopic->GetCurrentTopicStates().back())->mV == 10003);

    // Act: Don't accumulate any packets
    topic->SetState(Vertex::kVertexClear);
    topic->ProcessVertex();

    // Assert nothing has changed
    NL_TEST_ASSERT(inSuite, itopic->GetCurrentTopicStates().size() == 0);

    delete topic;
}

namespace {
    struct TopicGetNameA : public TopicState { };
    struct TopicGetNameB : public TopicState { };
}

static void Test_GetName(nlTestSuite *inSuite, void *inContext)
{
    Topic<TopicGetNameA>* topicA = new Topic<TopicGetNameA>();
    Topic<TopicGetNameB>* topicB = new Topic<TopicGetNameB>();

    Vertex* ivertexA = static_cast<Vertex*>(topicA);
    Vertex* ivertexB = static_cast<Vertex*>(topicB);

    NL_TEST_ASSERT(inSuite, strcmp(ivertexA->GetName(), ivertexB->GetName()) != 0);

    Topic<TopicGetNameB>* topicBv2 = new Topic<TopicGetNameB>();
    Vertex* ivertexBv2 = static_cast<Vertex*>(topicBv2);

    NL_TEST_ASSERT(inSuite, strcmp(ivertexB->GetName(), ivertexBv2->GetName()) == 0);

    TopicGetNameA stateA;
    TopicGetNameB stateB;
    TopicGetNameB stateBv2;

    NL_TEST_ASSERT(inSuite, strcmp(stateA.GetName(), stateB.GetName()) != 0);
    NL_TEST_ASSERT(inSuite, strcmp(stateB.GetName(), stateBv2.GetName()) == 0);

    delete ivertexA;
    delete ivertexB;
    delete ivertexBv2;
}

static const nlTest sTests[] = {
    NL_TEST_DEF("Test_Lifetime", Test_Lifetime),
    NL_TEST_DEF("Test_VertexType", Test_VertexType),
    NL_TEST_DEF("Test_PublishProcessDispatch", Test_PublishProcessDispatch),
    NL_TEST_DEF("Test_GetCurrentTopicStates", Test_GetCurrentTopicStates),
    NL_TEST_DEF("Test_GetName", Test_GetName),
    NL_TEST_SENTINEL()
};

//This function creates the Suite (i.e: the name of your test and points to the array of test functions)
extern "C"
int topic_testsuite(void)
{
    nlTestSuite theSuite = SUITE_DECLARATION(topic, &sTests[0]);
    nlTestRunner(&theSuite, NULL);
    return nlTestRunnerStats(&theSuite);
}
