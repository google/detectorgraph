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

#include "test_graphstatestore.h"

#include "graphstatestore.hpp"
#include "topicstate.hpp"

#include <iostream>

#define SUITE_DECLARATION(name, test_ptr) { #name, test_ptr, setup_##name, teardown_##name }

using namespace DetectorGraph;

static int setup_graphstatestore(void *inContext)
{
    return 0;
}

static int teardown_graphstatestore(void *inContext)
{
    return 0;
}

static void Test_Lifetime(nlTestSuite *inSuite, void *inContext)
{
    GraphStateStore stateStore;

    // Assert that it returns a valid (empty state snapshot)
    NL_TEST_ASSERT(inSuite, stateStore.GetLastState());
}

namespace {
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

static void Test_StoreSimple(nlTestSuite *inSuite, void *inContext)
{
    GraphStateStore stateStore;

    int packetAValue = 49;
    {
        // Local scope
        std::list< ptr::shared_ptr<const TopicState> > mockList;

        mockList.push_back(ptr::shared_ptr<const TopicState>(new PacketTypeA(packetAValue)));

        stateStore.TakeNewSnapshot(mockList);
    }

    NL_TEST_ASSERT(inSuite, stateStore.GetLastState()->GetState(PacketTypeA::PacketTypeAId));
    NL_TEST_ASSERT(inSuite, stateStore.GetLastState()->GetMapSize() == 1);
    NL_TEST_ASSERT(inSuite, stateStore.GetLastState()->GetState(PacketTypeA::PacketTypeAId)->GetId() == PacketTypeA::PacketTypeAId);
    NL_TEST_ASSERT(inSuite, ptr::static_pointer_cast<const PacketTypeA>(stateStore.GetLastState()->GetState(PacketTypeA::PacketTypeAId))->mV == packetAValue);
}

static void Test_GetTopicStates(nlTestSuite *inSuite, void *inContext)
{
    GraphStateStore stateStore;

    int packetAValue = 6454;
    {
        // Local scope
        std::list< ptr::shared_ptr<const TopicState> > inputList;

        inputList.push_back(ptr::shared_ptr<const TopicState>(new PacketTypeA(packetAValue)));

        stateStore.TakeNewSnapshot(inputList);
    }

    std::list< ptr::shared_ptr<const TopicState> > outputList;
    stateStore.GetLastState()->GetTopicStates(outputList);

    NL_TEST_ASSERT(inSuite, outputList.size() == 1);
    NL_TEST_ASSERT(inSuite, outputList.front()->GetId() == PacketTypeA::PacketTypeAId);
    NL_TEST_ASSERT(inSuite, ptr::static_pointer_cast<const PacketTypeA>(outputList.front())->mV == packetAValue);
}

static void Test_TemplatedGetState(nlTestSuite *inSuite, void *inContext)
{
    GraphStateStore stateStore;

    int packetAValue = 49;
    {
        // Local scope
        std::list< ptr::shared_ptr<const TopicState> > mockList;

        mockList.push_back(ptr::shared_ptr<const TopicState>(new PacketTypeA(packetAValue)));

        stateStore.TakeNewSnapshot(mockList);
    }

    NL_TEST_ASSERT(inSuite, stateStore.GetLastState()->GetState<PacketTypeA>());
    NL_TEST_ASSERT(inSuite, stateStore.GetLastState()->GetState<PacketTypeA>()->GetId() == PacketTypeA::PacketTypeAId);
    NL_TEST_ASSERT(inSuite, stateStore.GetLastState()->GetState<PacketTypeA>()->mV == packetAValue);
}

namespace {
    struct PacketTypeB : public TopicState
    {
        static const TopicStateIdType PacketTypeBId = 43;
        PacketTypeB(int aV = 0) : mV(aV) {}; int mV;

        virtual TopicStateIdType GetId() const
        {
            return PacketTypeBId;
        }
    };
}

static void Test_StoreAccumulation(nlTestSuite *inSuite, void *inContext)
{
    GraphStateStore stateStore;

    int packetAValue = 49;
    {
        std::list< ptr::shared_ptr<const TopicState> > firstEvaluationOutput;

        firstEvaluationOutput.push_back(ptr::shared_ptr<const TopicState>(new PacketTypeA(packetAValue)));

        // First "evaluation" output
        stateStore.TakeNewSnapshot(firstEvaluationOutput);
    }

    int packetBValue = 1545;
    {
        std::list< ptr::shared_ptr<const TopicState> > secondEvaluationOutput;

        secondEvaluationOutput.push_back(ptr::shared_ptr<const TopicState>(new PacketTypeB(packetBValue)));

        // First "evaluation" output
        stateStore.TakeNewSnapshot(secondEvaluationOutput);
    }

    NL_TEST_ASSERT(inSuite, stateStore.GetLastState()->GetMapSize() == 2);
    NL_TEST_ASSERT(inSuite, stateStore.GetLastState()->GetState(PacketTypeA::PacketTypeAId));
    NL_TEST_ASSERT(inSuite, stateStore.GetLastState()->GetState(PacketTypeB::PacketTypeBId));
    NL_TEST_ASSERT(inSuite, ptr::static_pointer_cast<const PacketTypeA>(stateStore.GetLastState()->GetState(PacketTypeA::PacketTypeAId))->mV == packetAValue);
    NL_TEST_ASSERT(inSuite, ptr::static_pointer_cast<const PacketTypeB>(stateStore.GetLastState()->GetState(PacketTypeB::PacketTypeBId))->mV == packetBValue);
}

static void Test_StoreReplacement(nlTestSuite *inSuite, void *inContext)
{
    GraphStateStore stateStore;

    // Arrange an StateSnapshot that contains both A{49} and B{1545}
    int packetAValue = 49;
    int packetBValue = 1545;
    {
        std::list< ptr::shared_ptr<const TopicState> > firstEvaluationOutput;

        firstEvaluationOutput.push_back(ptr::shared_ptr<const TopicState>(new PacketTypeA(packetAValue)));
        firstEvaluationOutput.push_back(ptr::shared_ptr<const TopicState>(new PacketTypeB(packetBValue)));

        // Arrange
        stateStore.TakeNewSnapshot(firstEvaluationOutput);
    }


    // Act with a new value that should replace A{49}
    int secondPacketAValue = 505;
    {
        std::list< ptr::shared_ptr<const TopicState> > secondEvaluationOutput;

        secondEvaluationOutput.push_back(ptr::shared_ptr<const TopicState>(new PacketTypeA(secondPacketAValue)));

        // Act! This should substitute A{49} with A{505}
        stateStore.TakeNewSnapshot(secondEvaluationOutput);
    }

    // Assert
    NL_TEST_ASSERT(inSuite, stateStore.GetLastState()->GetMapSize() == 2);
    NL_TEST_ASSERT(inSuite, stateStore.GetLastState()->GetState(PacketTypeA::PacketTypeAId));
    NL_TEST_ASSERT(inSuite, stateStore.GetLastState()->GetState(PacketTypeB::PacketTypeBId));
    NL_TEST_ASSERT(inSuite, ptr::static_pointer_cast<const PacketTypeA>(stateStore.GetLastState()->GetState(PacketTypeA::PacketTypeAId))->mV == secondPacketAValue);
    NL_TEST_ASSERT(inSuite, ptr::static_pointer_cast<const PacketTypeB>(stateStore.GetLastState()->GetState(PacketTypeB::PacketTypeBId))->mV == packetBValue);
}

static void Test_StateSnapshotLifetime(nlTestSuite *inSuite, void *inContext)
{
    GraphStateStore stateStore;

    // Arrange an StateSnapshot that contains just A{0}
    int packetValue = 0;
    {
        std::list< ptr::shared_ptr<const TopicState> > firstEvaluationOutput;

        firstEvaluationOutput.push_back(ptr::shared_ptr<const TopicState>(new PacketTypeA(packetValue)));

        // Arrange
        stateStore.TakeNewSnapshot(firstEvaluationOutput);
    }

    // Act Retrieve the last state
    ptr::shared_ptr<const StateSnapshot> consumerStateSnapshot = stateStore.GetLastState();

    // Arrange a lot of state rolling; with adding new states.
    for (packetValue = 1; packetValue < 10; ++packetValue)
    {
        std::list< ptr::shared_ptr<const TopicState> > nEvaluationOutput;

        nEvaluationOutput.push_back(ptr::shared_ptr<const TopicState>(new PacketTypeA(packetValue)));
        nEvaluationOutput.push_back(ptr::shared_ptr<const TopicState>(new PacketTypeB(packetValue)));

        // Arrange
        stateStore.TakeNewSnapshot(nEvaluationOutput);
    }

    // Assert the previously saved state only contains A
    NL_TEST_ASSERT(inSuite, consumerStateSnapshot->GetMapSize() == 1);
    NL_TEST_ASSERT(inSuite, consumerStateSnapshot->GetState(PacketTypeA::PacketTypeAId));
    NL_TEST_ASSERT(inSuite, !(consumerStateSnapshot->GetState(PacketTypeB::PacketTypeBId)));
    NL_TEST_ASSERT(inSuite, ptr::static_pointer_cast<const PacketTypeA>(consumerStateSnapshot->GetState(PacketTypeA::PacketTypeAId))->mV == 0);

    // Assert last state contains both A and B
    NL_TEST_ASSERT(inSuite, stateStore.GetLastState()->GetMapSize() == 2);
}

static void Test_DuplicatePublicTopicPublish(nlTestSuite *inSuite, void *inContext)
{
    GraphStateStore stateStore;

    // Arrange an StateSnapshot that contains just A{0}
    {
        std::list< ptr::shared_ptr<const TopicState> > firstEvaluationOutput;

        firstEvaluationOutput.push_back(ptr::shared_ptr<const TopicState>(new PacketTypeA(1)));

        // Arrange
        stateStore.TakeNewSnapshot(firstEvaluationOutput);

        stateStore.TakeNewSnapshot(firstEvaluationOutput);
    }
}

static void Test_GetStateVersion(nlTestSuite *inSuite, void *inContext)
{
    GraphStateStore stateStore;

    // Arrange an StateSnapshot that contains just A{}
    {
        std::list< ptr::shared_ptr<const TopicState> > firstEvaluationOutput;

        firstEvaluationOutput.push_back(ptr::shared_ptr<const TopicState>(new PacketTypeA()));

        // Arrange
        stateStore.TakeNewSnapshot(firstEvaluationOutput);
    }

    // Act Retrieve the last state
    ptr::shared_ptr<const StateSnapshot> firstStateSnapshot = stateStore.GetLastState();

    // Arrange a lot of state rolling; with adding new states.
    for (int i = 1; i < 10; ++i)
    {
        std::list< ptr::shared_ptr<const TopicState> > nEvaluationOutput;

        nEvaluationOutput.push_back(ptr::shared_ptr<const TopicState>(new PacketTypeA()));

        // Arrange
        stateStore.TakeNewSnapshot(nEvaluationOutput);
    }

    ptr::shared_ptr<const StateSnapshot> lastStateSnapshot = stateStore.GetLastState();

    // Assert the previously saved state only contains A
    NL_TEST_ASSERT(inSuite, firstStateSnapshot->GetStateVersion() == 1);
    NL_TEST_ASSERT(inSuite, lastStateSnapshot->GetStateVersion() == 10);
}

static const nlTest sTests[] = {
    NL_TEST_DEF("Test_Lifetime", Test_Lifetime),
    NL_TEST_DEF("Test_StoreSimple", Test_StoreSimple),
    NL_TEST_DEF("Test_GetTopicStates", Test_GetTopicStates),
    NL_TEST_DEF("Test_TemplatedGetState", Test_TemplatedGetState),
    NL_TEST_DEF("Test_StoreAccumulation", Test_StoreAccumulation),
    NL_TEST_DEF("Test_StoreReplacement", Test_StoreReplacement),
    NL_TEST_DEF("Test_StateSnapshotLifetime", Test_StateSnapshotLifetime),
    NL_TEST_DEF("Test_DuplicatePublicTopicPublish", Test_DuplicatePublicTopicPublish),
    NL_TEST_DEF("Test_GetStateVersion", Test_GetStateVersion),
    NL_TEST_SENTINEL()
};

extern "C"
int graphstatestore_testsuite(void)
{
    nlTestSuite theSuite = SUITE_DECLARATION(graphstatestore, &sTests[0]);
    nlTestRunner(&theSuite, NULL);
    return nlTestRunnerStats(&theSuite);
}
