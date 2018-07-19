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

#include "test_nodenameutils.h"

#include "nodenameutils.hpp"
#include "graph.hpp"
#include "topicstate.hpp"
#include "detector.hpp"

#include <string>
#include <typeinfo>
#include <iostream>
#include <fstream>

#define SUITE_DECLARATION(name, test_ptr) { #name, test_ptr, setup_##name, teardown_##name }

using namespace DetectorGraph;

static int setup_nodenameutils(void *inContext)
{
    return 0;
}

static int teardown_nodenameutils(void *inContext)
{
    return 0;
}

static void Test_RemoveSubstrings(nlTestSuite *inSuite, void *inContext)
{
    const char* qbfElimStrings[] = {"the", "wall", "brown", NULL};
    std::string qbf = NodeNameUtils::RemoveSubstrings(
        "thequickbrownfoxjumpedoverthewall",
        qbfElimStrings);
    NL_TEST_ASSERT(inSuite, qbf == "quickfoxjumpedover");

    const char* noluckElimStrings[] = {"wall", NULL};
    std::string noluck = NodeNameUtils::RemoveSubstrings(
        "thequickbrownfoxjumpedoverthefence",
        noluckElimStrings);
    NL_TEST_ASSERT(inSuite, noluck == "thequickbrownfoxjumpedoverthefence");
}

static void Test_WrapOnSubStrings(nlTestSuite *inSuite, void *inContext)
{
    const char* qbfElimStrings[] = {"jumped", NULL};
    std::string qbf = NodeNameUtils::WrapOnSubStrings(
        "thequickbrownfoxjumpedoverthewall",
        qbfElimStrings);
    NL_TEST_ASSERT(inSuite, qbf == "thequickbrownfox\\njumpedoverthewall");

    const char* noluckElimStrings[] = {"the", "fox", NULL};
    std::string noluck = NodeNameUtils::WrapOnSubStrings(
        "thequickbrownfoxjumpedoverthefence",
        noluckElimStrings);
    NL_TEST_ASSERT(inSuite, noluck == "thequickbrown\\nfoxjumpedover\\nthefence");
}

struct SampleTopicState : public DetectorGraph::TopicState {};

struct OtherSampleTopicState : public DetectorGraph::TopicState {};

class SampleDetector : public DetectorGraph::Detector
, public SubscriberInterface<SampleTopicState>
, public Publisher<OtherSampleTopicState>
{
public:
    SampleDetector(Graph* graph)
    : DetectorGraph::Detector(graph)
    {
        Subscribe<SampleTopicState>(this);
        SetupPublishing<OtherSampleTopicState>(this);
    }

    void Evaluate(const SampleTopicState& aSampleTopicState)
    {
    }
};

static void Test_GetDemangledName(nlTestSuite *inSuite, void *inContext)
{
    SampleTopicState topicState;
    std::string sampleTopicStateName = NodeNameUtils::GetDemangledName(topicState.GetName());
    NL_TEST_ASSERT(inSuite, sampleTopicStateName == "SampleTopicState");

    Topic<SampleTopicState> topic;
    std::string sampleTopicName = NodeNameUtils::GetDemangledName(topic.GetName());
    NL_TEST_ASSERT(inSuite, sampleTopicName == "DetectorGraph::Topic<SampleTopicState>");

    Graph graph;
    SampleDetector aDetector(&graph);
    std::string detectorName = NodeNameUtils::GetDemangledName(aDetector.GetName());
    NL_TEST_ASSERT(inSuite, detectorName == "SampleDetector");
}

static void Test_GetMinimalName(nlTestSuite *inSuite, void *inContext)
{
    SampleTopicState topicState;
    std::string sampleTopicStateName = NodeNameUtils::GetMinimalName(topicState.GetName());
    printf("%s\n", sampleTopicStateName.c_str());
    NL_TEST_ASSERT(inSuite, sampleTopicStateName == "Sample");

    Topic<SampleTopicState> topic;
    std::string sampleTopicName = NodeNameUtils::GetMinimalName(topic.GetName());
    printf("%s\n", sampleTopicName.c_str());
    NL_TEST_ASSERT(inSuite, sampleTopicName == "Sample");

    Graph graph;
    SampleDetector aDetector(&graph);
    std::string detectorName = NodeNameUtils::GetMinimalName(aDetector.GetName());
    NL_TEST_ASSERT(inSuite, detectorName == "SampleDetector");
}

struct FooBarState : public DetectorGraph::TopicState {};
struct FooRequest : public DetectorGraph::TopicState {};
struct FooStateTimeout : public DetectorGraph::TopicState {};

static void Test_WrapOnCommonEndings(nlTestSuite *inSuite, void *inContext)
{
    SampleTopicState topicState;
    std::string sampleTopicStateName = NodeNameUtils::WrapOnCommonEndings(
        NodeNameUtils::GetMinimalName(topicState.GetName()));
    NL_TEST_ASSERT(inSuite, sampleTopicStateName == "Sample");

    Topic<SampleTopicState> topic;
    std::string sampleTopicName = NodeNameUtils::WrapOnCommonEndings(
        NodeNameUtils::GetMinimalName(topic.GetName()));
    NL_TEST_ASSERT(inSuite, sampleTopicName == "Sample");

    Graph graph;
    SampleDetector aDetector(&graph);
    std::string detectorName = NodeNameUtils::WrapOnCommonEndings(
        NodeNameUtils::GetMinimalName(aDetector.GetName()));
    NL_TEST_ASSERT(inSuite, detectorName == "Sample\\nDetector");

    Topic<FooBarState> fooBarStateTopic;
    std::string fooBarStateTopicName = NodeNameUtils::WrapOnCommonEndings(
        NodeNameUtils::GetMinimalName(fooBarStateTopic.GetName()));
    NL_TEST_ASSERT(inSuite, fooBarStateTopicName == "FooBar\\nState");

    Topic<FooRequest> fooRequestTopic;
    std::string fooRequestTopicName = NodeNameUtils::WrapOnCommonEndings(
        NodeNameUtils::GetMinimalName(fooRequestTopic.GetName()));
    NL_TEST_ASSERT(inSuite, fooRequestTopicName == "Foo\\nRequest");

    Topic<FooStateTimeout> fooStateTimeoutTopic;
    std::string fooStateTimeoutTopicName = NodeNameUtils::WrapOnCommonEndings(
        NodeNameUtils::GetMinimalName(fooStateTimeoutTopic.GetName()));
    NL_TEST_ASSERT(inSuite, fooStateTimeoutTopicName == "Foo\\nState\\nTimeout");
}

static const nlTest sTests[] = {
    NL_TEST_DEF("Test_RemoveSubstrings", Test_RemoveSubstrings),
    NL_TEST_DEF("Test_WrapOnSubStrings", Test_WrapOnSubStrings),
    NL_TEST_DEF("Test_GetDemangledName", Test_GetDemangledName),
    NL_TEST_DEF("Test_GetMinimalName", Test_GetMinimalName),
    NL_TEST_DEF("Test_WrapOnCommonEndings", Test_WrapOnCommonEndings),
    NL_TEST_SENTINEL()
};

//This function creates the Suite (i.e: the name of your test and points to the array of test functions)
extern "C"
int nodenameutils_testsuite(void)
{
    nlTestSuite theSuite = SUITE_DECLARATION(nodenameutils, &sTests[0]);
    nlTestRunner(&theSuite, NULL);
    return nlTestRunnerStats(&theSuite);
}
