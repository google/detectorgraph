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

TestTimeoutPublisherService::TestTimeoutPublisherService(Graph& arGraph)
: TimeoutPublisherService(arGraph), mElapsedTime(0), mWallClockOffset(0), mMetronomeId(kInvalidTimeoutPublisherHandle), mMetronomeTimerPeriod(0)
{
}

void TestTimeoutPublisherService::SetTimeout(const TimeOffset aMillisecondsFromNow, const TimeoutPublisherHandle aTimerId)
{
    mTimerMap[aTimerId] = aMillisecondsFromNow + mElapsedTime;
}

void TestTimeoutPublisherService::Start(const TimeoutPublisherHandle aTimerId)
{
/* not much */
}

void TestTimeoutPublisherService::Cancel(const TimeoutPublisherHandle aTimerId)
{
    MapIterator it = mTimerMap.find(aTimerId);
    if (it != mTimerMap.end())
    {
        mTimerMap.erase(it);
    }
}

void TestTimeoutPublisherService::StartMetronome(const TimeOffset aPeriodInMilliseconds)
{
    mMetronomeId = GetUniqueTimerHandle();
    mMetronomeTimerPeriod = aPeriodInMilliseconds;
    SetTimeout(mMetronomeTimerPeriod, mMetronomeId);
    Start(mMetronomeId);
}

void TestTimeoutPublisherService::CancelMetronome()
{
    Cancel(mMetronomeId);
}

void TestTimeoutPublisherService::SetWallClockOffset(int64_t aWallClockOffset)
{
    mWallClockOffset = aWallClockOffset;
}

bool TestTimeoutPublisherService::FireNextTimeout()
{
    if (mTimerMap.size() > 0)
    {
        MapIterator minIt = GetNextTimeout();

        mElapsedTime = minIt->second;

        if (minIt->first == mMetronomeId)
        {
            MetronomeFired();
            SetTimeout(mMetronomeTimerPeriod, mMetronomeId);
            Start(mMetronomeId);
        }
        else
        {
            TimeoutExpired(minIt->first);
            mTimerMap.erase(minIt);
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

    MapIterator minIt = GetNextTimeout();

    while (minIt != mTimerMap.end())
    {
        TimeOffset nextDeadline = minIt->second;
        if (nextDeadline > finalDeadline)
        {
            break;
        }

        mElapsedTime = nextDeadline;

        if (minIt->first == mMetronomeId)
        {
            MetronomeFired();
            SetTimeout(mMetronomeTimerPeriod, mMetronomeId);
            Start(mMetronomeId);
        }
        else
        {
            TimeoutExpired(minIt->first);
            mTimerMap.erase(minIt);
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

TestTimeoutPublisherService::MapIterator TestTimeoutPublisherService::GetNextTimeout()
{
    TimeOffset nextDeadline = std::numeric_limits<TimeOffset>::max();
    MapIterator minIt = mTimerMap.end();
    MapIterator it = mTimerMap.begin();
    while (it != mTimerMap.end())
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
