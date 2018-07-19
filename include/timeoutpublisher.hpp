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

#ifndef DETECTORGRAPH_INCLUDE_TIMEOUTPUBLISHER_HPP_
#define DETECTORGRAPH_INCLUDE_TIMEOUTPUBLISHER_HPP_

#include "graph.hpp"
#include "topicstate.hpp"
#include "timeoutpublisherservice.hpp"

namespace DetectorGraph
{

/**
 * @brief Push data to a topic when timer expires
 *
 * TimeoutPublisher provides a mechanism to schedule the publishing of a
 * TopicState to the graph in the future. Similarly to FuturePublisher,
 * the published TopicState goes into a separate (future) evaluation and so
 * toposort constraints do not apply; this allows a detector to publish to
 * itself.
 *
 * Additionally the detector can cancel/reset a scheduled job.
 *
 * Example:
 * @code
class HeartbeatDetector : public Detector,
    public SubscriberInterface<HeartBeat>,
    public TimeoutPublisher<HeartBeat>
{
    HeartbeatDetector(Graph* graph, TimeoutPublisherService* apTimeoutService) : Detector(graph)
    {
        Subscribe<HeartBeat>(this);
        SetupTimeoutPublishing<HeartBeat>(this, apTimeoutService);
    }

    void Evaluate(const HeartBeat&)
    {
        // This will be evaluated at 1Hz
        PublishOnTimeout(HeartBeat(), 1000);
    }
}
 * @endcode
 */
template<class T>
class TimeoutPublisher
{
public:
    /**
     * @brief Basic Constructor
     *
     * This constructor does not fully initialize the TimeoutPublisher as
     * a TimeoutPublisherService is needed. That's done by SetTimeoutService.
     */
    TimeoutPublisher()
    : mpTimeoutPublisherService(NULL), mDefaultHandle(kInvalidTimeoutPublisherHandle)
    {
    }

    /**
     * @brief Sets the timeout service and acquires a TimerHandle to be used
     by the default/simple API.
     */
    void SetTimeoutService(TimeoutPublisherService* apTimeoutPublisherService)
    {
        mpTimeoutPublisherService = apTimeoutPublisherService;
        mDefaultHandle = mpTimeoutPublisherService->GetUniqueTimerHandle();
    }

    /**
     * @brief Empty Virtual Destructor
     */
    virtual ~TimeoutPublisher() { }

    /**
     * @brief Schedules a TopicState for Publishing after a timeout
     *
     * This method is analogous to Publish() in the sense that it's a Detector
     * output but with two big differences:
     * - @param aData is only published after @param aMillisecondsFromNow
     * - @param aData is published to the Graph Input queue instead of another
     * topic so TopoSort constraints do not apply; this allows a detector to
     * publish and subscribe to the same topic.
     * @param aTimerId (optional) allows detectors to control multiple concurrent timers.
     * TimeoutPublisherHandles are vended through TimeoutPublisherService::GetUniqueTimerHandle
     */
    void PublishOnTimeout(const T& aData, const uint64_t aMillisecondsFromNow, TimeoutPublisherHandle aTimerId = kInvalidTimeoutPublisherHandle)
    {
        if (aTimerId == kInvalidTimeoutPublisherHandle) { aTimerId = mDefaultHandle; }

        mpTimeoutPublisherService->ScheduleTimeout<T>(aData, aMillisecondsFromNow, aTimerId);
    }

    /**
     * @brief Cancels the Scheduled PublishOnTimeout
     */
    void CancelPublishOnTimeout(TimeoutPublisherHandle aTimerId = kInvalidTimeoutPublisherHandle)
    {
        if (aTimerId == kInvalidTimeoutPublisherHandle) { aTimerId = mDefaultHandle; }

        mpTimeoutPublisherService->CancelPublishOnTimeout(aTimerId);
    }

    /**
     * @brief Returns weather a timeout has expired or not.
     */
    bool HasTimeoutExpired(TimeoutPublisherHandle aTimerId = kInvalidTimeoutPublisherHandle) const
    {
        if (aTimerId == kInvalidTimeoutPublisherHandle) { aTimerId = mDefaultHandle; }

        return mpTimeoutPublisherService->HasTimeoutExpired(aTimerId);
    }

protected:
    // Keep reference to Graph in order to push data to graph in future
    TimeoutPublisherService* mpTimeoutPublisherService;
    TimeoutPublisherHandle mDefaultHandle;
};

} // namespace DetectorGraph

#endif // DETECTORGRAPH_INCLUDE_TIMEOUTPUBLISHER_HPP_
