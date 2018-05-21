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

TimeoutPublisherService::TimeoutPublisherService(Graph& arGraph) : mrGraph(arGraph), mMetronomePeriodMsec(0)
{
}

TimeoutPublisherService::~TimeoutPublisherService()
{
    for (MapIterator tZombieDataIt = mScheduledTopicStatesMap.begin();
        tZombieDataIt != mScheduledTopicStatesMap.end();
        ++tZombieDataIt)
    {
        delete tZombieDataIt->second;
    }

    for (DispatcherIterator tZombieDataIt = mScheduledPeriodicTopicStatesList.begin();
        tZombieDataIt != mScheduledPeriodicTopicStatesList.end();
        ++tZombieDataIt)
    {
        delete tZombieDataIt->mpTopicStateDispatcher;
    }

}

void TimeoutPublisherService::ScheduleTimeoutDispatcher(
    TopicStateDispatcherInterface* aDispatcher,
    const TimeOffset aMillisecondsFromNow,
    const TimeoutPublisherHandle aTimerHandle)
{
    CancelPublishOnTimeout(aTimerHandle);
    mScheduledTopicStatesMap[aTimerHandle] = aDispatcher;
    SetTimeout(aMillisecondsFromNow, aTimerHandle);
    Start(aTimerHandle);
}

void TimeoutPublisherService::SchedulePeriodicPublishingDispatcher(
    TopicStateDispatcherInterface* aDispatcher,
    const TimeOffset aPeriodInMilliseconds)
{
    mMetronomePeriodMsec = gcd(aPeriodInMilliseconds, mMetronomePeriodMsec);
    mScheduledPeriodicTopicStatesList.push_back(
        PeriodicTopicStateDispatcher(aPeriodInMilliseconds, aDispatcher));
}

void TimeoutPublisherService::CancelPublishOnTimeout(const TimeoutPublisherHandle aId)
{
    if (mScheduledTopicStatesMap.count(aId))
    {
        Cancel(aId);
        MapIterator it = mScheduledTopicStatesMap.find(aId);
        TopicStateDispatcherInterface* tDispatcher = (*it).second;
        delete tDispatcher;
        mScheduledTopicStatesMap.erase(it);
    }
}

void TimeoutPublisherService::TimeoutExpired(const TimeoutPublisherHandle aId)
{
    if (mScheduledTopicStatesMap.count(aId))
    {
        MapIterator it = mScheduledTopicStatesMap.find(aId);
        TopicStateDispatcherInterface* tDispatcher = (*it).second;
        tDispatcher->Dispatch(mrGraph);
        delete tDispatcher;
        mScheduledTopicStatesMap.erase(it);
    }
}

bool TimeoutPublisherService::HasTimeoutExpired(const TimeoutPublisherHandle aId) const
{
    return (mScheduledTopicStatesMap.count(aId) == 0);
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
    for (DispatcherIterator it = mScheduledPeriodicTopicStatesList.begin();
        it != mScheduledPeriodicTopicStatesList.end();
        ++it)
    {
        it->mMetronomeCounter++;
        if (it->mMetronomeCounter >= (it->mPublishingPeriodMsec / mMetronomePeriodMsec))
        {
            it->mpTopicStateDispatcher->Dispatch(mrGraph);
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
