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

#ifndef DETECTORGRAPH_INCLUDE_TIMEOUTPUBLISHERSERVICE_HPP_
#define DETECTORGRAPH_INCLUDE_TIMEOUTPUBLISHERSERVICE_HPP_

#include "graph.hpp"
#include "topicstate.hpp"

#if defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
// LITE_BEGIN
#include "detectorgraphliteconfig.hpp"
#include "statictypedallocator-lite.hpp"
// LITE_END
#else
// FULL_BEGIN
#include "dglogging.hpp"
#include <vector>
// FULL_END
#endif


namespace DetectorGraph
{

typedef int TimeoutPublisherHandle;
enum { kInvalidTimeoutPublisherHandle = -1 };

typedef uint64_t TimeOffset;

/**
 * @brief A service that provides Timer function to DetectorGraph Detectors
 *
 * TimeoutPublisherService is used/shared among many TimeoutPublishers
 * (Detectors) to add the notion of timed publications to DetectorGraph.
 */
class TimeoutPublisherService
{
    // Types and classes used internally
    /**
     * @brief Internal DispatcherInterface for dispatching any scheduled
     TopicState to Graph::PushData<T>
     */
    struct DispatcherInterface
    {
        virtual void Dispatch(Graph& aGraph) = 0;
        virtual ~DispatcherInterface() {}
    };

    /**
     * @brief Internal Dispatcher for dispatching a particular scheduled
     TopicState to Graph::PushData<T>
     */
    template<class T>
    struct Dispatcher : public DispatcherInterface
    {
        Dispatcher() : mData() {}
        Dispatcher(const T& aData) : mData(aData) {}
        virtual void Dispatch(Graph& aGraph)
        {
            aGraph.PushData<T>(mData);
        }
        const T mData;
    };

    /**
     * @brief Internal Dispatcher for periodically dispatching TopicState to Graph::PushData<T>
     *
     * This internal data structure holds a periodically-triggered dispatcher. It is also used to track
     * the time between triggers, so that different dispatchers can use the same synchronized metronome timer.
     */
    struct PeriodicPublishingSeries
    {
        TimeOffset mPublishingPeriodMsec;
        TimeOffset mMetronomeCounter;
        DispatcherInterface* mpDispatcher;

        PeriodicPublishingSeries(TimeOffset aPublishingPeriodMsec,
            DispatcherInterface* aDispatcher)
        : mPublishingPeriodMsec(aPublishingPeriodMsec)
        , mMetronomeCounter(0)
        , mpDispatcher(aDispatcher) {}
    };


#if defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
    typedef SequenceContainer<DispatcherInterface*,
        DetectorGraphConfig::kMaxNumberOfTimeouts> TimeoutDispatchersContainer;
    typedef SequenceContainer<PeriodicPublishingSeries,
        DetectorGraphConfig::kMaxNumberOfPeriodicTimers> PeriodicPublishingSeriesContainer;
    struct TimeoutCtxt {};
    typedef StaticTypedAllocator<DispatcherInterface, TimeoutCtxt> TimeoutDispatchersAllocator;
    struct PeriodicCtxt {};
    typedef StaticTypedAllocator<DispatcherInterface, PeriodicCtxt> PeriodicDispatchersAllocator;
#else
    typedef std::vector<DispatcherInterface*> TimeoutDispatchersContainer;
    typedef std::vector<PeriodicPublishingSeries> PeriodicPublishingSeriesContainer;
#endif

public:
    /**
     * @brief Constructor that initializes the service connected to a graph
     *
     * @param[in] graph The graph to which timed out TopicStates will be posted
     */
    TimeoutPublisherService(Graph& graph);

    /**
     * @brief Destructor
     *
     * Deletes all dynamically allocated pending TopicStates
     */
    virtual ~TimeoutPublisherService();

    /**
     * @brief Starts a Metronome to publish scheduled TopicStates
     *
     * Calling this method starts a metronome(periodic timer).
     * TimeoutPublisherService will start publishing scheduled TopicStates periodically to graph.
     */
    void StartPeriodicPublishing();

    /**
     * @brief Returns a unique id/handle for a new timer
     *
     * Different TimeoutPublishers will call this to 'acquire' a timer. The
     * handle is then used throughout the API to refer to any individual timer.
     * Note that this will never return kInvalidTimeoutPublisherHandle.
     */
    TimeoutPublisherHandle GetUniqueTimerHandle();

    /**
     * @brief Schedules a TopicState for publishing periodically
     *
     * This is called by different Detectors with a TopicState and a publishing period.
     * This method updates the metronome period based on the GCD of the requested publishing period.
     * Calling 'StartPeriodicPublishing' will start publishing `T` to the graph periodically
     * with interval @param aPeriodInMilliseconds .
     *
     * @param aPeriodInMilliseconds The regular period at which T should be published.
     */
    template<class T>
    void SchedulePeriodicPublishing(const TimeOffset aPeriodInMilliseconds)
    {
#if defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
        SchedulePeriodicPublishingDispatcher(mPeriodicDispatchersAllocator.New<Dispatcher<T>>(), aPeriodInMilliseconds);
#else
        SchedulePeriodicPublishingDispatcher(new Dispatcher<T>(), aPeriodInMilliseconds);
#endif
    }

    /**
     * @brief Schedules a TopicState for Publishing after a timeout
     *
     * This is called internally by TimeoutPublishers. It starts a timer set to
     * expire at a given deadline. When the deadline is reached @param aData is
     * Published to the graph.
     * Calling this method on an pending @param aTimerHandle resets it
     * (canceling any previous timeouts)
     *
     * @param aData The TopicState to be published when the deadline expires.
     * @param aMillisecondsFromNow The deadline relative to now.
     * @param aTimerHandle A unique handle for this timer.
     */
    template<class T>
    void ScheduleTimeout(const T& aData, const TimeOffset aMillisecondsFromNow, const TimeoutPublisherHandle aTimerHandle)
    {
#if defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
        ScheduleTimeoutDispatcher(mTimeoutDispatchersAllocator.New<Dispatcher<T>>(aData), aMillisecondsFromNow, aTimerHandle);
#else
        DG_LOG("Schedulling Timeout for %s in %d milliseconds\n", aData.GetName(), aMillisecondsFromNow);
        ScheduleTimeoutDispatcher(new Dispatcher<T>(aData), aMillisecondsFromNow, aTimerHandle);
#endif
    }

    /**
     * @brief Cancels a timeout and deletes the stored TopicState
     *
     * This is called by different TimeoutPublishers when a timeout must be
     * canceled.
     */
    void CancelPublishOnTimeout(const TimeoutPublisherHandle aTimerHandle);

    /**
     * @brief Returns weather the timeout for a given handle has expired/fired
     already.
     *
     * This will also return true if the referred timer never existed.
     */
    bool HasTimeoutExpired(const TimeoutPublisherHandle aTimerHandle) const;

    /**
     * @brief Should return the time offset to Epoch
     *
     * This must be implemented by subclasses. Different detectors may call
     * this to acquire a timestamp - usually used to "stamp" a TopicState.
     * This clock may jump back & forth due to time sync.
     */
    virtual TimeOffset GetTime() const = 0;

    /**
     * @brief Should return monotonic time since some unspecified starting point.
     *
     * This must be implemented by subclasses.
     * Returns the time offset to an unspecified point back in time that should
     * not change for the duration of this instance.
     * Different detectors may call this to acquire a consistent, strictly
     * increasing, time offset valid for the duration of this object's instance.
     */
    virtual TimeOffset GetMonotonicTime() const = 0;

protected:
    /**
     * @brief Fires/Dispatches a TopicState that was pending on a timeout
     *
     * This method should be called by a particular subclasses of
     * TimeoutPublisherService to notify the service that the actual
     * internal timers have expired/fired.
     */
    void TimeoutExpired(const TimeoutPublisherHandle aTimerHandle);

    /**
     * @brief Should setup a timeout for the given handle.
     *
     * This must be implemented by subclasses. This should initialize a unique
     * timer for that handle (if it doesn't already exist) and set it's timeout
     * accordingly.
     */
    virtual void SetTimeout(const TimeOffset aMillisecondsFromNow, const TimeoutPublisherHandle) = 0;

    /**
     * @brief Should start a timer for the given handle.
     *
     * This must be implemented by subclasses. This should start the timer.
     */
    virtual void Start(const TimeoutPublisherHandle) = 0;

    /**
     * @brief Should cancel the timer the given handle.
     *
     * This must be implemented by subclasses. This should cancel the timer.
     */
    virtual void Cancel(const TimeoutPublisherHandle) = 0;

    /**
     * @brief Update metronome counters and Fires/Dispatches TopicStates that was pending on scheduled period.
     *
     * This method should be called by a particular subclasses of
     * TimeoutPublisherService to notify the service that the actual
     * internal period timer has fired.
     */
    void MetronomeFired();

    /**
     * @brief Should start the metronome (periodic timer) for the given period.
     *
     * This must be implemented by subclasses. This should start the periodic timer.
     */
    virtual void StartMetronome(const TimeOffset aPeriodInMilliseconds) = 0;

    /**
     * @brief Should stop the metronome.
     *
     * This must be implemented by subclasses. This should stop the periodic timer.
     */
    virtual void CancelMetronome() = 0;

private:
    /**
     * @brief Internal type-agnostic method to schedule timeouts
     */
    void ScheduleTimeoutDispatcher(DispatcherInterface* aDispatcher, const TimeOffset aMillisecondsFromNow, const TimeoutPublisherHandle aTimerHandle);

    /**
     * @brief Internal type-agnostic method to schedule periodic timers
     */
    void SchedulePeriodicPublishingDispatcher(DispatcherInterface* aDispatcher, const TimeOffset aPeriodInMilliseconds);

    /**
     * @brief Euclidean algorithm to compute great common divisor(GCD)
     */
    TimeOffset gcd(TimeOffset lhs, TimeOffset rhs);

    /**
     * @brief Reference to the graph to which timed out TopicStates will be
     pushed/posted.
     */
    Graph& mrGraph;

    /**
     * @brief Metronome period which used for periodic Topicstates publishing
     */
    TimeOffset mMetronomePeriodMsec;

    /**
     * @brief Map of pending TopicStates per Handle
     */
    TimeoutDispatchersContainer mTimeoutDispatchers;

    /**
     * @brief List of scheduled periodic TopicStates dispatcher
     */
    PeriodicPublishingSeriesContainer mPeriodicSeries;

#if defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
    TimeoutDispatchersAllocator mTimeoutDispatchersAllocator;
    PeriodicDispatchersAllocator mPeriodicDispatchersAllocator;
#endif
};

} // namespace DetectorGraph

#endif // DETECTORGRAPH_INCLUDE_TIMEOUTPUBLISHERSERVICE_HPP_
