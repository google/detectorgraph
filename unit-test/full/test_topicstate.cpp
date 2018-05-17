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

#include "test_topicstate.h"

#include "topicstate.hpp"

#include <iostream>

#define SUITE_DECLARATION(name, test_ptr) { #name, test_ptr, setup_##name, teardown_##name }

using namespace DetectorGraph;

static int setup_topicstate(void *inContext)
{
    return 0;
}

static int teardown_topicstate(void *inContext)
{
    return 0;
}

namespace {
    struct AnonymousPacketType : public TopicState
    {
        AnonymousPacketType(int aV = 0) : mV(aV) {}; int mV;
    };

    struct PacketTypeA : public TopicState
    {
        static const TopicStateIdType PacketTypeAId = 42;
        PacketTypeA(int aV = 0) : mV(aV) {}; int mV;

        virtual TopicStateIdType GetId() const
        {
            return PacketTypeAId;
        }
    };
}

static void Test_GetId(nlTestSuite *inSuite, void *inContext)
{
    AnonymousPacketType anonTopicState;
    NL_TEST_ASSERT(inSuite, anonTopicState.GetId() == TopicState::kAnonymousTopicState);

    TopicState* anonTopicStatePointer = &anonTopicState;
    NL_TEST_ASSERT(inSuite, anonTopicStatePointer->GetId() == TopicState::kAnonymousTopicState);

    NL_TEST_ASSERT(inSuite, TopicState::GetId<AnonymousPacketType>() == TopicState::kAnonymousTopicState);

    PacketTypeA typeATopicState;
    NL_TEST_ASSERT(inSuite, typeATopicState.GetId() == PacketTypeA::PacketTypeAId);

    TopicState* typeATopicStatePointer = &typeATopicState;
    NL_TEST_ASSERT(inSuite, typeATopicStatePointer->GetId() == PacketTypeA::PacketTypeAId);

    NL_TEST_ASSERT(inSuite, TopicState::GetId<PacketTypeA>() == PacketTypeA::PacketTypeAId);
}

static const nlTest sTests[] = {
    NL_TEST_DEF("Test_GetId", Test_GetId),
    NL_TEST_SENTINEL()
};

extern "C"
int topicstate_testsuite(void)
{
    nlTestSuite theSuite = SUITE_DECLARATION(topicstate, &sTests[0]);
    nlTestRunner(&theSuite, NULL);
    return nlTestRunnerStats(&theSuite);
}
