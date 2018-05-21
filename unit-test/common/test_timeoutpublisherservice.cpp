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

#include "test_timeoutpublisherservice.h"
#include "timeoutpublisherservice.hpp"
#include "nltest.h"

#define SUITE_DECLARATION(name, test_ptr) { #name, test_ptr, setup_##name, teardown_##name }

using namespace DetectorGraph;

static int setup_timeoutpublisherservice(void *inContext)
{
    return 0;
}

static int teardown_timeoutpublisherservice(void *inContext)
{
    return 0;
}

namespace {
    class _TimeoutPublisherService : public TimeoutPublisherService
    {
        public:
            _TimeoutPublisherService(Graph& graph)
            : TimeoutPublisherService(graph)
            {
            }

            TimeoutPublisherHandle GetUniqueTimerHandle()
            {
                static TimeoutPublisherHandle handle = TimeoutPublisherHandle();
                return handle++;
            }

            TimeOffset GetTime() const { return TimeOffset(); }

            TimeOffset GetMonotonicTime() const { return TimeOffset(); }

            void SetTimeout(const TimeOffset, const TimeoutPublisherHandle) {}
            void Start(const TimeoutPublisherHandle) {}
            void Cancel(const TimeoutPublisherHandle) {}
            void StartMetronome(const TimeOffset aPeriodInMilliseconds) {}
            void CancelMetronome() {}

            void TimeoutExpired(const TimeoutPublisherHandle aTimerHandle)
            {
                TimeoutPublisherService::TimeoutExpired(aTimerHandle);
            }

            void MetronomeFired()
            {
                TimeoutPublisherService::MetronomeFired();
            }
    };

    struct TopicStateA : public TopicState
    {
         int v;
        TopicStateA(int av) : v(av) {};
#if !defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
        TopicStateA() : v() {}
#endif
    };

    struct TopicStateB : public TopicState
    {
         int v;
        TopicStateB(int av) : v(av) {};
#if !defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
        TopicStateB() : v() {}
#endif
    };
}

static void Test_Lifetime(nlTestSuite *inSuite, void *inContext)
{
    Graph graph;
    _TimeoutPublisherService timeoutPublisherService(graph);
}

static void Test_DispatchSingleTS(nlTestSuite *inSuite, void *inContext)
{
    Graph graph;
    _TimeoutPublisherService timeoutPublisherService(graph);

#if defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
    Topic<TopicStateA> topicInstance(&graph);
    graph.AddVertex(&topicInstance);
#endif

    Topic<TopicStateA>* topicAPtr = graph.ResolveTopic<TopicStateA>();

    TimeoutPublisherHandle handle = timeoutPublisherService.GetUniqueTimerHandle();

    timeoutPublisherService.ScheduleTimeout<TopicStateA>(TopicStateA(42),
                                                         /* not used */0,
                                                         handle);

    graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, !topicAPtr->HasNewValue());

    timeoutPublisherService.TimeoutExpired(handle);

    graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, topicAPtr->HasNewValue());
}

static void Test_DispatchMultiple(nlTestSuite *inSuite, void *inContext)
{
    Graph graph;
    _TimeoutPublisherService timeoutPublisherService(graph);

#if defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
    Topic<TopicStateA> topicAInstance(&graph);
    Topic<TopicStateB> topicBInstance(&graph);
    graph.AddVertex(&topicAInstance);
    graph.AddVertex(&topicBInstance);
#endif

    Topic<TopicStateA>* topicAPtr = graph.ResolveTopic<TopicStateA>();
    Topic<TopicStateB>* topicBPtr = graph.ResolveTopic<TopicStateB>();

    TimeoutPublisherHandle handleForA = timeoutPublisherService.GetUniqueTimerHandle();
    TimeoutPublisherHandle handleForB = timeoutPublisherService.GetUniqueTimerHandle();

    timeoutPublisherService.ScheduleTimeout<TopicStateA>(TopicStateA(42),
                                                         /* not used */0,
                                                         handleForA);
    timeoutPublisherService.ScheduleTimeout<TopicStateB>(TopicStateB(99),
                                                         /* not used */0,
                                                         handleForB);

    graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, !topicAPtr->HasNewValue());
    NL_TEST_ASSERT(inSuite, !topicBPtr->HasNewValue());

    timeoutPublisherService.TimeoutExpired(handleForB);

    graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, !topicAPtr->HasNewValue());
    NL_TEST_ASSERT(inSuite, topicBPtr->HasNewValue());

    timeoutPublisherService.TimeoutExpired(handleForA);

    graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, topicAPtr->HasNewValue());
    NL_TEST_ASSERT(inSuite, !topicBPtr->HasNewValue());
}

static void Test_PeriodicOne(nlTestSuite *inSuite, void *inContext)
{
    struct PeriodicTopicState : public TopicState {};

    Graph graph;
    _TimeoutPublisherService timeoutPublisherService(graph);

#if defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
    Topic<PeriodicTopicState> topicInstance(&graph);
    graph.AddVertex(&topicInstance);
#endif

    Topic<PeriodicTopicState>* topicPtr = graph.ResolveTopic<PeriodicTopicState>();

    timeoutPublisherService.SchedulePeriodicPublishing<PeriodicTopicState>(5000);
    timeoutPublisherService.StartPeriodicPublishing();

    graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, !topicPtr->HasNewValue());

    timeoutPublisherService.MetronomeFired();
    graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, topicPtr->HasNewValue());

    timeoutPublisherService.MetronomeFired();
    graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, topicPtr->HasNewValue());
}

static void Test_PeriodicMultiple(nlTestSuite *inSuite, void *inContext)
{

    struct TopicState9ms : public TopicState {};
    struct TopicState15ms : public TopicState {};

    Graph graph;
    _TimeoutPublisherService timeoutPublisherService(graph);

#if defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
    Topic<TopicState9ms> topic9Instance(&graph);
    Topic<TopicState15ms> topic15Instance(&graph);
    graph.AddVertex(&topic9Instance);
    graph.AddVertex(&topic15Instance);
#endif

    Topic<TopicState9ms>* topic9Ptr = graph.ResolveTopic<TopicState9ms>();
    Topic<TopicState15ms>* topic15Ptr = graph.ResolveTopic<TopicState15ms>();

    /* 9ms and 15ms. gcd is 3ms */
    timeoutPublisherService.SchedulePeriodicPublishing<TopicState9ms>(9);
    timeoutPublisherService.SchedulePeriodicPublishing<TopicState15ms>(15);
    timeoutPublisherService.StartPeriodicPublishing();

    graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, !topic9Ptr->HasNewValue());
    NL_TEST_ASSERT(inSuite, !topic15Ptr->HasNewValue());

    // t = 3ms
    timeoutPublisherService.MetronomeFired();
    graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, !topic9Ptr->HasNewValue());
    NL_TEST_ASSERT(inSuite, !topic15Ptr->HasNewValue());

    // t = 6ms
    timeoutPublisherService.MetronomeFired();
    graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, !topic9Ptr->HasNewValue());
    NL_TEST_ASSERT(inSuite, !topic15Ptr->HasNewValue());

    // t = 9ms
    timeoutPublisherService.MetronomeFired();
    graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, topic9Ptr->HasNewValue());
    NL_TEST_ASSERT(inSuite, !topic15Ptr->HasNewValue());

    // t = 12ms
    timeoutPublisherService.MetronomeFired();
    graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, !topic9Ptr->HasNewValue());
    NL_TEST_ASSERT(inSuite, !topic15Ptr->HasNewValue());

    // t = 15ms
    timeoutPublisherService.MetronomeFired();
    graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, !topic9Ptr->HasNewValue());
    NL_TEST_ASSERT(inSuite, topic15Ptr->HasNewValue());

    // t = 18ms
    timeoutPublisherService.MetronomeFired();
    graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, topic9Ptr->HasNewValue());
    NL_TEST_ASSERT(inSuite, !topic15Ptr->HasNewValue());
}

static const nlTest sTests[] = {
    NL_TEST_DEF("Test_Lifetime", Test_Lifetime),
    NL_TEST_DEF("Test_DispatchSingleTS", Test_DispatchSingleTS),
    NL_TEST_DEF("Test_DispatchMultiple", Test_DispatchMultiple),
    NL_TEST_DEF("Test_PeriodicOne", Test_PeriodicOne),
    NL_TEST_DEF("Test_PeriodicMultiple", Test_PeriodicMultiple),
    NL_TEST_SENTINEL()
};

extern "C"
int timeoutpublisherservice_testsuite(void)
{
    nlTestSuite theSuite = SUITE_DECLARATION(timeoutpublisherservice, &sTests[0]);
    nlTestRunner(&theSuite, NULL);
    return nlTestRunnerStats(&theSuite);
}
