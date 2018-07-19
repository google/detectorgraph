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

#include "testtimeoutpublisherservice.hpp"

namespace DetectorGraph
{

#if defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
const TimeOffset TestTimeoutPublisherService::kInvalidMaxOffset = (TimeOffset)-1;
const TimeoutPublisherHandle TestTimeoutPublisherService::kMetronomeId = TimeOffsetsContainer::max_size - 1;
#else
const TimeOffset TestTimeoutPublisherService::kInvalidMaxOffset = std::numeric_limits<TimeOffset>::max();
const TimeoutPublisherHandle TestTimeoutPublisherService::kMetronomeId = kInvalidTimeoutPublisherHandle;
#endif

TestTimeoutPublisherService::TestTimeoutPublisherService(Graph& arGraph)
: TimeoutPublisherService(arGraph), mElapsedTime(0), mWallClockOffset(0), mMetronomeTimerPeriod(0)
{
#if defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
    for (unsigned i = 0; i < TimeOffsetsContainer::max_size; i++)
    {
        mTimerDeadlines.push_back(TimeOffsetPairType(i, kInvalidMaxOffset));
    }
#endif
}

void TestTimeoutPublisherService::SetTimeout(const TimeOffset aMillisecondsFromNow, const TimeoutPublisherHandle aTimerId)
{
#if defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
    mTimerDeadlines[aTimerId] = TimeOffsetPairType(aTimerId, aMillisecondsFromNow + mElapsedTime);
#else
    mTimerDeadlines[aTimerId] = aMillisecondsFromNow + mElapsedTime;
#endif
}

void TestTimeoutPublisherService::Start(const TimeoutPublisherHandle aTimerId)
{
/* not much */
}

void TestTimeoutPublisherService::Cancel(const TimeoutPublisherHandle aTimerId)
{
#if defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
    mTimerDeadlines[aTimerId] = TimeOffsetPairType(aTimerId, kInvalidMaxOffset);
#else
    mTimerDeadlines[aTimerId] = kInvalidMaxOffset;
#endif
}

void TestTimeoutPublisherService::StartMetronome(const TimeOffset aPeriodInMilliseconds)
{
    mMetronomeTimerPeriod = aPeriodInMilliseconds;
    SetTimeout(mMetronomeTimerPeriod, kMetronomeId);
    Start(kMetronomeId);
}

void TestTimeoutPublisherService::CancelMetronome()
{
    Cancel(kMetronomeId);
}

void TestTimeoutPublisherService::SetWallClockOffset(int64_t aWallClockOffset)
{
    mWallClockOffset = aWallClockOffset;
}

bool TestTimeoutPublisherService::FireNextTimeout()
{
    TimeOffsetsIterator minIt = GetNextTimeout();
    if (minIt != mTimerDeadlines.end())
    {
        mElapsedTime = minIt->second;

        if (minIt->first == kMetronomeId)
        {
            MetronomeFired();
            SetTimeout(mMetronomeTimerPeriod, kMetronomeId);
            Start(kMetronomeId);
        }
        else
        {
            TimeoutExpired(minIt->first);
            minIt->second = kInvalidMaxOffset;
        }

        return true;
    }

    return false;
}

bool TestTimeoutPublisherService::ForwardTimeAndEvaluate(TimeOffset aFwdTime, Graph& aGraphToEvaluate)
{
    bool firedAtLeastOne = false;
    TimeOffset finalDeadline = mElapsedTime + aFwdTime;

    // If we're forwarding any amount of time ahead
    // flush input queue first.
    if (aFwdTime > 0)
    {
        while (aGraphToEvaluate.HasDataPending())
        {
            aGraphToEvaluate.EvaluateGraph();
        }
    }

    TimeOffsetsIterator minIt = GetNextTimeout();

    while (minIt != mTimerDeadlines.end())
    {
        TimeOffset nextDeadline = minIt->second;
        if (nextDeadline > finalDeadline)
        {
            break;
        }

        mElapsedTime = nextDeadline;

        if (minIt->first == kMetronomeId)
        {
            MetronomeFired();
            SetTimeout(mMetronomeTimerPeriod, kMetronomeId);
            Start(kMetronomeId);
        }
        else
        {
            TimeoutExpired(minIt->first);
            minIt->second = kInvalidMaxOffset;
        }

        firedAtLeastOne = true;

        while (aGraphToEvaluate.HasDataPending())
        {
            aGraphToEvaluate.EvaluateGraph();

            // If we're already at the targed time, evaluate only once and quit
            // The idea is that finalDeadline is the "moment of interest" for a
            // test; exiting here gives the test an opportunity inspect all
            // outputs created for that particular moment in time.
            if (nextDeadline == finalDeadline)
            {
                break;
            }
        }

        minIt = GetNextTimeout();
    }

    mElapsedTime = finalDeadline;

    return firedAtLeastOne;
}

TimeOffset TestTimeoutPublisherService::GetTime() const
{
    return mElapsedTime + mWallClockOffset;
}

TimeOffset TestTimeoutPublisherService::GetMonotonicTime() const
{
    return mElapsedTime;
}

TestTimeoutPublisherService::TimeOffsetsIterator TestTimeoutPublisherService::GetNextTimeout()
{
    TimeOffset nextDeadline = kInvalidMaxOffset;
    TimeOffsetsIterator minIt = mTimerDeadlines.end();
    TimeOffsetsIterator it = mTimerDeadlines.begin();
    while (it != mTimerDeadlines.end())
    {
        if (it->second < nextDeadline)
        {
            minIt = it;
            nextDeadline = it->second;
        }
        ++it;
    }

    return minIt;
}

TimeOffset TestTimeoutPublisherService::GetMetronomePeriod()
{
    return mMetronomeTimerPeriod;
}

}
