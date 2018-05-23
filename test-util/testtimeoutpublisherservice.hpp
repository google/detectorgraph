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

#ifndef DETECTORGRAPH_TEST_UTIL_TESTTIMEOUTPUBLISHERSERVICE_HPP_
#define DETECTORGRAPH_TEST_UTIL_TESTTIMEOUTPUBLISHERSERVICE_HPP_

#include "graph.hpp"
#include "topicstate.hpp"
#include "timeoutpublisherservice.hpp"

namespace DetectorGraph
{

/**
 * @brief Push data to a topic when timer expires
 *
 * TimeoutPublisher provides a mechanism to schedule the publish of data to a topic.
 * Additionally the detector can cancel/reset a scheduled job.
 */
class TestTimeoutPublisherService : public TimeoutPublisherService
{
    typedef std::map<TimeoutPublisherHandle, TimeOffset>::iterator MapIterator;
public:
    // Implementing TimeoutPublisherService derived class
    TestTimeoutPublisherService(Graph& arGraph);

    virtual TimeOffset GetTime() const;
    virtual TimeOffset GetMonotonicTime() const;

protected:
    virtual void SetTimeout(const TimeOffset aMillisecondsFromNow, const TimeoutPublisherHandle aTimerId);
    virtual void Start(const TimeoutPublisherHandle aTimerId);
    virtual void Cancel(const TimeoutPublisherHandle aTimerId);
    virtual void StartMetronome(const TimeOffset aPeriodInMilliseconds);
    virtual void CancelMetronome();

public:
    void SetWallClockOffset(int64_t aWallClockOffset);
    bool FireNextTimeout();
    bool ForwardTimeAndEvaluate(TimeOffset aFwdTime, Graph& aGraphToEvaluate);
    TimeOffset GetMetronomePeriod();

private:
    MapIterator GetNextTimeout();

private:
    // Mock Inspection state
    // This could be optimized for time with a queue on the next deadline to
    // remove the O(N) search for next deadline. But ffs, this is a mock class!
    std::map<TimeoutPublisherHandle, TimeOffset> mTimerMap;

    // Equivalent to monotonic time
    TimeOffset mElapsedTime;
    // Offset summed with monotonic time to produce GetTime()
    int64_t mWallClockOffset;

    TimeoutPublisherHandle mMetronomeId;
    TimeOffset mMetronomeTimerPeriod;
};

} // namespace DetectorGraph

#endif // DETECTORGRAPH_TEST_UTIL_TESTTIMEOUTPUBLISHERSERVICE_HPP_
