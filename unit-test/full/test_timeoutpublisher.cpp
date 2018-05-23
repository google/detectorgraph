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

#include <map>

#include "nltest.h"
#include "errortype.hpp"

#include "graph.hpp"
#include "detector.hpp"
#include "timeoutpublisherservice.hpp"
#include "timeoutpublisher.hpp"
#include "topicstate.hpp"
#include "testtimeoutpublisherservice.hpp"

#include "test_timeoutpublisher.h"

#define SUITE_DECLARATION(name, test_ptr) { #name, test_ptr, setup_##name, teardown_##name }

using namespace DetectorGraph;

static int setup_timeoutpublisher(void *inContext)
{
    return 0;
}

static int teardown_timeoutpublisher(void *inContext)
{
    return 0;
}

namespace {
    struct MockTimeoutPublisherService : public TimeoutPublisherService
    {
        MockTimeoutPublisherService(Graph& arGraph) : TimeoutPublisherService(arGraph) {}

        virtual void SetTimeout(const uint64_t aMillisecondsFromNow, const TimeoutPublisherHandle aTimerId) { mTimerMap[aTimerId] = false; /* running = true */ }
        virtual void Start(const TimeoutPublisherHandle aTimerId) { mTimerMap[aTimerId] = true; /* running = true */ }
        virtual void Cancel(const TimeoutPublisherHandle aTimerId) { mTimerMap[aTimerId] = false; /* running = false */ }
        virtual void StartMetronome(const TimeOffset aPeriodInMilliseconds) {};
        virtual void CancelMetronome() {};

        virtual TimeOffset GetTime() const { return 0; } // LCOV_EXCL_LINE
        virtual TimeOffset GetMonotonicTime() const { return 0; }; // LCOV_EXCL_LINE

        void FireMockTimeout(const TimeoutPublisherHandle aTimerId)
        {
            TimeoutExpired(aTimerId);
            mTimerMap[aTimerId] = false;
        }

        std::map<TimeoutPublisherHandle, bool> mTimerMap;
    };

    struct TriggerTopicState : public TopicState { TriggerTopicState(int aV = 0) : mV(aV) {}; int mV; };
    struct TimeoutTopicState : public TopicState { TimeoutTopicState(int aV = 0) : mV(aV) {}; int mV; };

    struct SampleTimeoutDetector : public Detector,
        public SubscriberInterface<TriggerTopicState>,
        public TimeoutPublisher<TimeoutTopicState>
    {
        SampleTimeoutDetector(Graph* graph, TimeoutPublisherService* apService)
        : Detector(graph)
        , mEvalCount(0)
        {
            Subscribe<TriggerTopicState>(this);
            SetupTimeoutPublishing<TimeoutTopicState>(this, apService);
        }

        virtual void Evaluate(const TriggerTopicState&)
        {
            PublishOnTimeout(TimeoutTopicState(9999), 1000);
            mEvalCount++;
        }

        TimeoutPublisherHandle GetDefaultTimeoutPublisherHandle()
        {
            return TimeoutPublisher<TimeoutTopicState>::mDefaultHandle;
        }

        int mEvalCount;
    };
}

static void Test_Lifecycle(nlTestSuite *inSuite, void *inContext)
{
    Graph graph;
    MockTimeoutPublisherService timeoutPublisherService(graph);
    SampleTimeoutDetector detector(&graph, &timeoutPublisherService);
    NL_TEST_ASSERT(inSuite, graph.GetVertices().size() == 3);
    NL_TEST_ASSERT(inSuite, graph.ResolveTopic<TriggerTopicState>() != NULL);
    NL_TEST_ASSERT(inSuite, graph.ResolveTopic<TimeoutTopicState>() != NULL);

    NL_TEST_ASSERT(inSuite, graph.ResolveTopic<TriggerTopicState>()->GetOutEdges().front() == &detector);
    NL_TEST_ASSERT(inSuite, static_cast<Vertex*>(&detector)->GetInEdges().front() == graph.ResolveTopic<TriggerTopicState>());
    NL_TEST_ASSERT(inSuite, static_cast<Vertex*>(&detector)->GetFutureOutEdges().front() == graph.ResolveTopic<TimeoutTopicState>());
    NL_TEST_ASSERT(inSuite, graph.ResolveTopic<TimeoutTopicState>()->GetFutureInEdges().front() == &detector);
}

static void Test_PublishOnTimeoutEvaluation(nlTestSuite *inSuite, void *inContext)
{
    // Arrange - graph with detector
    Graph graph;
    MockTimeoutPublisherService timeoutPublisherService(graph);
    SampleTimeoutDetector detector(&graph, &timeoutPublisherService);


    // Act - trigger timer start
    graph.PushData<TriggerTopicState>(TriggerTopicState(44));
    graph.EvaluateGraph();

    // Assert - detector is evaluted once and doesnt emit any further outputs
    NL_TEST_ASSERT(inSuite, detector.mEvalCount == 1);
    NL_TEST_ASSERT(inSuite, timeoutPublisherService.mTimerMap[detector.GetDefaultTimeoutPublisherHandle()] == true);
    NL_TEST_ASSERT(inSuite, detector.HasTimeoutExpired() == false);
    NL_TEST_ASSERT(inSuite, graph.GetOutputList().size() == 1);

    // Act - re-evaluate graph
    graph.EvaluateGraph();

    // Assert - nothing changes
    NL_TEST_ASSERT(inSuite, graph.GetOutputList().size() == 0);
    NL_TEST_ASSERT(inSuite, detector.HasTimeoutExpired() == false);
    NL_TEST_ASSERT(inSuite, timeoutPublisherService.mTimerMap[detector.GetDefaultTimeoutPublisherHandle()] == true);

    // Act - fire timeout
    timeoutPublisherService.FireMockTimeout(detector.GetDefaultTimeoutPublisherHandle());

    // Assert - no TimeoutData emitted until evaluation (but timer stops)
    NL_TEST_ASSERT(inSuite, graph.GetOutputList().size() == 0);
    NL_TEST_ASSERT(inSuite, detector.HasTimeoutExpired() == true);
    NL_TEST_ASSERT(inSuite, timeoutPublisherService.mTimerMap[detector.GetDefaultTimeoutPublisherHandle()] == false);

    // Act - evaluate graph
    graph.EvaluateGraph();

    // Assert - TimeoutData emitted and conforming with mock
    NL_TEST_ASSERT(inSuite, graph.GetOutputList().size() == 1);
    NL_TEST_ASSERT(inSuite, detector.HasTimeoutExpired() == true);
    NL_TEST_ASSERT(inSuite, ptr::dynamic_pointer_cast<const TimeoutTopicState>(graph.GetOutputList().front()) != NULL);
    NL_TEST_ASSERT(inSuite, ptr::dynamic_pointer_cast<const TimeoutTopicState>(graph.GetOutputList().front())->mV == 9999);

    // Act - evaluate graph
    graph.EvaluateGraph();

    // Nothing changes further down
    NL_TEST_ASSERT(inSuite, graph.GetOutputList().size() == 0);

    // Act - trigger timer start again
    graph.PushData<TriggerTopicState>(TriggerTopicState(55));
    graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, detector.HasTimeoutExpired() == false);

    // Leaving scope without firing or cancelling timer.
}

namespace {
    struct MockButtonUpdate : public TopicState { MockButtonUpdate(char aKey = ' ', bool aDown = false) : mKey(aKey), mDown(aDown) {}; char mKey; bool mDown; };
    struct ButtonStuckEvent : public TopicState { ButtonStuckEvent(char aKey = 0) : mKey(aKey) {}; char mKey; };

    struct MockButtonStuckDetector : public Detector,
        public SubscriberInterface<MockButtonUpdate>,
        public TimeoutPublisher<ButtonStuckEvent>
    {
        MockButtonStuckDetector(Graph* graph, TimeoutPublisherService* apTimeoutService)
        : Detector(graph)
        , mpTimeoutService(apTimeoutService)
        {
            Subscribe<MockButtonUpdate>(this);
            SetupTimeoutPublishing<ButtonStuckEvent>(this, apTimeoutService);
        }

        virtual void Evaluate(const MockButtonUpdate& aInPacket)
        {
            if (mKeyToTimerMap.count(aInPacket.mKey) == 0)
            {
                mKeyToTimerMap[aInPacket.mKey] = mpTimeoutService->GetUniqueTimerHandle();
            }

            if (aInPacket.mDown)
            {
                PublishOnTimeout(ButtonStuckEvent(aInPacket.mKey), 5000, mKeyToTimerMap[aInPacket.mKey]);
            }
            else // Up
            {
                CancelPublishOnTimeout(mKeyToTimerMap[aInPacket.mKey]);
            }
        }
        TimeoutPublisherService* mpTimeoutService;
        std::map<char, TimeoutPublisherHandle> mKeyToTimerMap;
    };
}

static void Test_MultipleTimerPublishOnTimeoutEvaluation(nlTestSuite *inSuite, void *inContext)
{
    Graph graph;
    MockTimeoutPublisherService timeoutPublisherService(graph);
    MockButtonStuckDetector detector(&graph, &timeoutPublisherService);

    MockButtonUpdate aDown('a', true);
    MockButtonUpdate aUp('a', false);

    MockButtonUpdate bDown('b', true);
    MockButtonUpdate bUp('b', false);

    /*
     * This tests a 'mock' "ButtonStuckChecker"
     * Suppose two buttons' states pressed = _ release = ''
     *
     * a   |'''''''___'''''''''''''''''''''''''''
     * b   |'''''''''''''''''''''''______________
     *
     * TmrA|       \______/
     * TmrB|                       \______/
     * Stuck Event ________________________A_____
     *
     * Button A is pressed and released before timer goes off.
     * Button B is pressed and kept down until timer goes off triggering Stuck Event
     */


    // Act
    graph.PushData<MockButtonUpdate>(aDown);
    graph.EvaluateGraph();

    // Assert
    NL_TEST_ASSERT(inSuite, graph.GetOutputList().size() == 1); // just MockButtonUpdate
    NL_TEST_ASSERT(inSuite, timeoutPublisherService.mTimerMap[detector.mKeyToTimerMap['a']] == true); // Running

    // Act
    graph.EvaluateGraph();

    // Assert
    NL_TEST_ASSERT(inSuite, graph.GetOutputList().size() == 0);
    NL_TEST_ASSERT(inSuite, timeoutPublisherService.mTimerMap[detector.mKeyToTimerMap['a']] == true); // Running


    // Act
    graph.PushData<MockButtonUpdate>(aUp); // This will test cancelling a timer
    graph.EvaluateGraph();

    // Assert
    NL_TEST_ASSERT(inSuite, graph.GetOutputList().size() == 1); // just MockButtonUpdate
    NL_TEST_ASSERT(inSuite, timeoutPublisherService.mTimerMap[detector.mKeyToTimerMap['a']] == false); // Running


    // Act - Suppose the timerExpired event was already in the queue.
    timeoutPublisherService.FireMockTimeout(detector.mKeyToTimerMap['a']);
    graph.EvaluateGraph();

    // Assert Nothing changes.
    NL_TEST_ASSERT(inSuite, graph.GetOutputList().size() == 0); // nothing. Timer was cancelled
    NL_TEST_ASSERT(inSuite, timeoutPublisherService.mTimerMap[detector.mKeyToTimerMap['a']] == false); // Running


    // -----------

    // Act
    graph.PushData<MockButtonUpdate>(bDown);
    graph.EvaluateGraph();

    // Assert
    NL_TEST_ASSERT(inSuite, graph.GetOutputList().size() == 1); // just MockButtonUpdate
    NL_TEST_ASSERT(inSuite, timeoutPublisherService.mTimerMap[detector.mKeyToTimerMap['a']] == false); // Running
    NL_TEST_ASSERT(inSuite, timeoutPublisherService.mTimerMap[detector.mKeyToTimerMap['b']] == true); // Running

    // Act - Timer goes off
    timeoutPublisherService.FireMockTimeout(detector.mKeyToTimerMap['b']);
    graph.EvaluateGraph();

    // Assert
    NL_TEST_ASSERT(inSuite, graph.GetOutputList().size() == 1);
    NL_TEST_ASSERT(inSuite, timeoutPublisherService.mTimerMap[detector.mKeyToTimerMap['a']] == false); // Running
    NL_TEST_ASSERT(inSuite, timeoutPublisherService.mTimerMap[detector.mKeyToTimerMap['b']] == false); // Running
    NL_TEST_ASSERT(inSuite, ptr::dynamic_pointer_cast<const ButtonStuckEvent>(graph.GetOutputList().front()) != NULL);
    NL_TEST_ASSERT(inSuite, ptr::dynamic_pointer_cast<const ButtonStuckEvent>(graph.GetOutputList().front())->mKey == 'b');

    graph.EvaluateGraph();
    NL_TEST_ASSERT(inSuite, graph.GetOutputList().size() == 0);
}

namespace {
    struct TimerRequest : public TopicState {
        enum RequestType { kStartATimer, kCancelATimer, kRestartATimer, kStartBTimer, kCancelBTimer, kRestartBTimer, kNone} mRequest;
        TimerRequest(RequestType aType = kNone) : mRequest(aType) {};
    };

    struct TimeoutTopicStateB : public TopicState { TimeoutTopicStateB(int aV = 0) : mV(aV) {}; int mV; };

    struct MultipleTimeoutTypesDetector : public Detector,
        public SubscriberInterface<TimerRequest>,
        public TimeoutPublisher<TimeoutTopicState>,
        public TimeoutPublisher<TimeoutTopicStateB>
    {
        typedef TimeoutPublisher<TimeoutTopicState> PublisherA;
        typedef TimeoutPublisher<TimeoutTopicStateB> PublisherB;

        MultipleTimeoutTypesDetector(Graph* graph, TimeoutPublisherService* apTimeoutService)
        : Detector(graph)
        {
            Subscribe<TimerRequest>(this);
            SetupTimeoutPublishing<TimeoutTopicState>(this, apTimeoutService);
            SetupTimeoutPublishing<TimeoutTopicStateB>(this, apTimeoutService);
            mPacketAId = apTimeoutService->GetUniqueTimerHandle();
            mPacketBId = apTimeoutService->GetUniqueTimerHandle();
        }

        virtual void Evaluate(const TimerRequest& aInPacket)
        {
            switch (aInPacket.mRequest)
            {
                case TimerRequest::kStartATimer:
                    PublisherA::PublishOnTimeout(TimeoutTopicState(11111), 5000, mPacketAId);
                break;

                case TimerRequest::kCancelATimer:
                    PublisherA::CancelPublishOnTimeout(mPacketAId);
                break;

                case TimerRequest::kRestartATimer:
                    PublisherA::PublishOnTimeout(TimeoutTopicState(11112), 5000, mPacketAId);
                break;

                case TimerRequest::kStartBTimer:
                    PublisherB::PublishOnTimeout(TimeoutTopicStateB(22222), 5000, mPacketBId);
                break;

                case TimerRequest::kCancelBTimer:
                    PublisherB::CancelPublishOnTimeout(mPacketBId);
                break;

                case TimerRequest::kRestartBTimer:
                    PublisherB::PublishOnTimeout(TimeoutTopicStateB(22223), 5000, mPacketBId);
                break;

                case TimerRequest::kNone: // LCOV_EXCL_LINE
                break; // LCOV_EXCL_LINE
            }
        }
        TimeoutPublisherHandle mPacketAId, mPacketBId;
    };
}

static void Test_MultipleTimeoutTypes(nlTestSuite *inSuite, void *inContext)
{
    Graph graph;
    MockTimeoutPublisherService timeoutPublisherService(graph);
    MultipleTimeoutTypesDetector detector(&graph, &timeoutPublisherService);

    // Assert 4 vertices: Topic<TimerRequest>, Detector, Topic<TimeoutPacket>, Topic<TimeoutPacketB>
    NL_TEST_ASSERT(inSuite, graph.GetVertices().size() == 4);

    // Act
    graph.PushData<TimerRequest>(TimerRequest(TimerRequest::kStartATimer));
    graph.EvaluateGraph();

    // Assert - only the request
    NL_TEST_ASSERT(inSuite, graph.GetOutputList().size() == 1);

    // Act
    graph.EvaluateGraph();

    // Assert - nothing
    NL_TEST_ASSERT(inSuite, graph.GetOutputList().size() == 0);

    // Act
    timeoutPublisherService.FireMockTimeout(detector.mPacketAId);
    graph.EvaluateGraph();

    // Assert - timeout A, with normal value
    NL_TEST_ASSERT(inSuite, graph.GetOutputList().size() == 1);
    NL_TEST_ASSERT(inSuite, ptr::dynamic_pointer_cast<const TimeoutTopicState>(graph.GetOutputList().front()) != NULL);
    NL_TEST_ASSERT(inSuite, ptr::dynamic_pointer_cast<const TimeoutTopicState>(graph.GetOutputList().front())->mV == 11111);

    // Act
    graph.PushData<TimerRequest>(TimerRequest(TimerRequest::kRestartATimer));
    graph.EvaluateGraph();
    timeoutPublisherService.FireMockTimeout(detector.mPacketAId);
    graph.EvaluateGraph();

    // Assert - timeout A, with reset
    NL_TEST_ASSERT(inSuite, graph.GetOutputList().size() == 1);
    NL_TEST_ASSERT(inSuite, ptr::dynamic_pointer_cast<const TimeoutTopicState>(graph.GetOutputList().front()) != NULL);
    NL_TEST_ASSERT(inSuite, ptr::dynamic_pointer_cast<const TimeoutTopicState>(graph.GetOutputList().front())->mV == 11112);

    // Act Start and Cancel Timer B
    graph.PushData<TimerRequest>(TimerRequest(TimerRequest::kStartBTimer));
    graph.EvaluateGraph();
    graph.PushData<TimerRequest>(TimerRequest(TimerRequest::kCancelBTimer));
    graph.EvaluateGraph();

    // Assert Output is only the cancellation request:
    NL_TEST_ASSERT(inSuite, graph.GetOutputList().size() == 1);
    NL_TEST_ASSERT(inSuite, ptr::dynamic_pointer_cast<const TimerRequest>(graph.GetOutputList().front()) != NULL);

    // Act
    graph.PushData<TimerRequest>(TimerRequest(TimerRequest::kRestartATimer));
    graph.EvaluateGraph();
    graph.PushData<TimerRequest>(TimerRequest(TimerRequest::kCancelATimer));
    graph.EvaluateGraph();

    // Assert Output is only the cancellation request:
    NL_TEST_ASSERT(inSuite, graph.GetOutputList().size() == 1);
    NL_TEST_ASSERT(inSuite, ptr::dynamic_pointer_cast<const TimerRequest>(graph.GetOutputList().front()) != NULL);

    // Act
    graph.PushData<TimerRequest>(TimerRequest(TimerRequest::kRestartBTimer));
    graph.EvaluateGraph();
    graph.PushData<TimerRequest>(TimerRequest(TimerRequest::kCancelBTimer));
    graph.EvaluateGraph();

    // Assert Output is only the cancellation request:
    NL_TEST_ASSERT(inSuite, graph.GetOutputList().size() == 1);
    NL_TEST_ASSERT(inSuite, ptr::dynamic_pointer_cast<const TimerRequest>(graph.GetOutputList().front()) != NULL);

}

static const nlTest sTests[] = {
    NL_TEST_DEF("Test_Lifecycle", Test_Lifecycle),
    NL_TEST_DEF("Test_PublishOnTimeoutEvaluation", Test_PublishOnTimeoutEvaluation),
    NL_TEST_DEF("Test_MultipleTimerPublishOnTimeoutEvaluation", Test_MultipleTimerPublishOnTimeoutEvaluation),
    NL_TEST_DEF("Test_MultipleTimeoutTypes", Test_MultipleTimeoutTypes),
    NL_TEST_SENTINEL()
};

//This function creates the Suite (i.e: the name of your test and points to the array of test functions)
extern "C"
int timeoutpublisher_testsuite(void)
{
    nlTestSuite theSuite = SUITE_DECLARATION(timeoutpublisher, &sTests[0]);
    nlTestRunner(&theSuite, NULL);
    return nlTestRunnerStats(&theSuite);
}
