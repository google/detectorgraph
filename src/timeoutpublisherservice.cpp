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

#include "timeoutpublisherservice.hpp"

namespace DetectorGraph
{

TimeoutPublisherService::TimeoutPublisherService(Graph& arGraph) : mrGraph(arGraph), mMetronomePeriodMsec(0), mLastHandleId(0)
{
}

TimeoutPublisherService::~TimeoutPublisherService()
{
// The Lite version's allocator automatically deletes all objects using RAII
#if !defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
    for (TimeoutDispatchersIterator tZombieDataIt = mTimeoutDispatchers.begin();
        tZombieDataIt != mTimeoutDispatchers.end();
        ++tZombieDataIt)
    {
        delete tZombieDataIt->second;
    }

    for (PeriodicSeriesIterator tZombieDataIt = mPeriodicSeries.begin();
        tZombieDataIt != mPeriodicSeries.end();
        ++tZombieDataIt)
    {
        delete tZombieDataIt->mpDispatcher;
    }
#endif
}

TimeoutPublisherHandle TimeoutPublisherService::GetUniqueTimerHandle()
{
    // Type sanity check;
    DG_ASSERT(mLastHandleId != kInvalidTimeoutPublisherHandle);
    return mLastHandleId++;
    // TimeoutPublisherHandle newHandle = (TimeoutPublisherHandle)mTimeoutDispatchers.size();
    // mTimeoutDispatchers.push_back(NULL);
    // return newHandle;
}

void TimeoutPublisherService::ScheduleTimeoutDispatcher(
    DispatcherInterface* aDispatcher,
    const TimeOffset aMillisecondsFromNow,
    const TimeoutPublisherHandle aTimerHandle)
{
    CancelPublishOnTimeout(aTimerHandle);
    mTimeoutDispatchers[aTimerHandle] = aDispatcher;
    SetTimeout(aMillisecondsFromNow, aTimerHandle);
    Start(aTimerHandle);
}

void TimeoutPublisherService::SchedulePeriodicPublishingDispatcher(
    DispatcherInterface* aDispatcher,
    const TimeOffset aPeriodInMilliseconds)
{
    mMetronomePeriodMsec = gcd(aPeriodInMilliseconds, mMetronomePeriodMsec);
    mPeriodicSeries.push_back(
        PeriodicPublishingSeries(aPeriodInMilliseconds, aDispatcher));
}

void TimeoutPublisherService::CancelPublishOnTimeout(const TimeoutPublisherHandle aId)
{
    if (mTimeoutDispatchers.count(aId))
    {
        Cancel(aId);
        TimeoutDispatchersIterator it = mTimeoutDispatchers.find(aId);
        DispatcherInterface* tDispatcher = (*it).second;
#if defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
        mTimeoutDispatchersAllocator.Delete(tDispatcher);
#else
        delete tDispatcher;
#endif
        mTimeoutDispatchers.erase(it);
    }
}

void TimeoutPublisherService::TimeoutExpired(const TimeoutPublisherHandle aId)
{
    if (mTimeoutDispatchers.count(aId))
    {
        TimeoutDispatchersIterator it = mTimeoutDispatchers.find(aId);
        DispatcherInterface* tDispatcher = (*it).second;
        tDispatcher->Dispatch(mrGraph);
#if defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
        mTimeoutDispatchersAllocator.Delete(tDispatcher);
#else
        delete tDispatcher;
#endif
        mTimeoutDispatchers.erase(it);
    }
}

bool TimeoutPublisherService::HasTimeoutExpired(const TimeoutPublisherHandle aId) const
{
    return (mTimeoutDispatchers.count(aId) == 0);
}

void TimeoutPublisherService::StartPeriodicPublishing()
{
    if (mMetronomePeriodMsec > 0)
    {
        StartMetronome(mMetronomePeriodMsec);
    }
}

void TimeoutPublisherService::MetronomeFired()
{
    for (PeriodicSeriesIterator it = mPeriodicSeries.begin();
        it != mPeriodicSeries.end();
        ++it)
    {
        it->mMetronomeCounter++;
        if (it->mMetronomeCounter >= (it->mPublishingPeriodMsec / mMetronomePeriodMsec))
        {
            it->mpDispatcher->Dispatch(mrGraph);
            it->mMetronomeCounter = 0;
        }
    }
}

TimeOffset TimeoutPublisherService::gcd(TimeOffset lhs, TimeOffset rhs)
{
    while (rhs != 0)
    {
        TimeOffset temp = rhs;
        rhs = lhs % rhs;
        lhs = temp;
    }
    return lhs;
}

} // namespace DetectorGraph
