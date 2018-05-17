// Copyright 2018 Nest Labs, Inc.
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

#include "test_topicregistry.h"
#include "topicregistry.hpp"
#include "nltest.h"

#define SUITE_DECLARATION(name, test_ptr) { #name, test_ptr, setup_##name, teardown_##name }

using namespace DetectorGraph;

static int setup_topicregistry(void *inContext)
{
    return 0;
}

static int teardown_topicregistry(void *inContext)
{
    return 0;
}

namespace {
    struct TopicStateA : public TopicState {};
    struct TopicStateB : public TopicState {};
}

static void Test_ResolveUnregistered(nlTestSuite *inSuite, void *inContext)
{
    TopicRegistry registry;
    Topic<TopicStateA>* resolvedPtr;
    Topic<TopicStateB> other;

    resolvedPtr = registry.Resolve<TopicStateA>();
    NL_TEST_ASSERT(inSuite, resolvedPtr == NULL);

    registry.Register(&other);

    resolvedPtr = registry.Resolve<TopicStateA>();
    NL_TEST_ASSERT(inSuite, resolvedPtr == NULL);
}

static void Test_ResolveRegistered(nlTestSuite *inSuite, void *inContext)
{
    TopicRegistry registry;
    Topic<TopicStateA> topicA;
    Topic<TopicStateB> topicB;

    registry.Register(&topicA);
    registry.Register(&topicB);

    Topic<TopicStateA>* topicAPtr = registry.Resolve<TopicStateA>();
    NL_TEST_ASSERT(inSuite, topicAPtr == &topicA);

    Topic<TopicStateB>* topicBPtr = registry.Resolve<TopicStateB>();
    NL_TEST_ASSERT(inSuite, topicBPtr == &topicB);
}

static void Test_CleanupWithScope(nlTestSuite *inSuite, void *inContext)
{
    {
        TopicRegistry registry;
        Topic<TopicStateA> topicA;

        registry.Register(&topicA);
    }
    {
        TopicRegistry registry;
        Topic<TopicStateA>* topicAPtr = registry.Resolve<TopicStateA>();
        NL_TEST_ASSERT(inSuite, topicAPtr == NULL);
    }
}

static const nlTest sTests[] = {
    NL_TEST_DEF("Test_ResolveUnregistered", Test_ResolveUnregistered),
    NL_TEST_DEF("Test_ResolveRegistered", Test_ResolveRegistered),
    NL_TEST_DEF("Test_CleanupWithScope", Test_CleanupWithScope),
    NL_TEST_SENTINEL()
};

extern "C"
int topicregistry_testsuite(void)
{
    nlTestSuite theSuite = SUITE_DECLARATION(topicregistry, &sTests[0]);
    nlTestRunner(&theSuite, NULL);
    return nlTestRunnerStats(&theSuite);
}
