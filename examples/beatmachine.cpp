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

#include <iostream>

#include "graph.hpp"
#include "detector.hpp"
#include "processorcontainer.hpp"
#include "timeoutpublisherservice.hpp"
#include "dglogging.hpp"
#include "graphanalyzer.hpp"

#include <map>
#include <chrono>
#include <thread>
#include <algorithm>

using namespace DetectorGraph;

using std::cout;
using std::endl;

/**
 * @file beatmachine.cpp
 * @brief A Rube Goldbergesque contraption that uses TimeoutPublisherService APIs
 *
 * @section Introduction
 * Writing examples is hard - you have to be creative and stuff. So naturally
 * stuff went wild. This one is inspired by the Na Automata [1] and the Hey
 * Jude Flowchart [2].
 *
 * Jokes aside, this example provides a useful but very basic (C++11 - only)
 * concrete implementation for the DetectorGraph::TimeoutPublisherService class.
 * This shows how Timers & Timeouts can be integrated with Detectors & Topics.
 *
 * In the example, five detectors attempt to sing Hey Jude together in a
 * distributed fashion - not unlike when you try to sing bits of a song in a
 * crowded concert without knowing the song's entire lyrics or melody. Here the
 * four 'line-singing' detectors must act in synchrony to achieve the desired
 * output.
 *
 * @section tps Implementing TimeoutPublisherService
 * When porting the DetectorGraph library to your environment you'll need to
 * provide a concrete subclass of DetectorGraph::TimeoutPublisherService
 * similar to DetectorGraph::SleepBasedTimeoutPublisherService in this example.
 * Note that features and accuracy were sacrificed in order to keep
 * DetectorGraph::SleepBasedTimeoutPublisherService as simple as possible.
 *
 * @section utps Using the Timer APIs
 * The first example of using the Time APIs is on `ThemeDetector` where it sets
 * up a 75 BPM _Periodic Timer_ that publishes `RhytmBeats` every 0.8s and
 * subscribes to it:
 * @code
ThemeDetector(DetectorGraph::Graph* graph, TimeoutPublisherService* timeService) : DetectorGraph::Detector(graph)
{
    SetupPeriodicPublishing<RhytmBeats>(RhytmBeats::kPeriod, timeService);
    Subscribe<RhytmBeats>(this);
    SetupPublishing<SongThemeState>(this);
}
 * @endcode
 *
 * Another example, `ThenYouDetector` is a DetectorGraph::TimeoutPublisher that
 * acts in a _fire and forget_ manner. This allows the detector to schedule the
 * publishing of a `TopicState` to a `Topic` prompting a new graph evaluation
 * in the future for that `TopicState`.
 * The call to `PublishOnTimeout` basically says: "Post this data to its topic
 * in X milliseconds".
 * @code
class ThenYouDetector : public DetectorGraph::Detector
, ...
, public DetectorGraph::TimeoutPublisher<ThenYouState>
{
...
    ThenYouDetector(DetectorGraph::Graph* graph, DetectorGraph::TimeoutPublisherService* timeService)
    : DetectorGraph::Detector(graph)
...
    {
...
        SetupTimeoutPublishing<ThenYouState>(this, timeService);
    }

...
        PublishOnTimeout(ThenYouState(kResponses[mThenYouIndex] + " " + kResponseEnd), RhytmBeats::kPeriod);
 * @endcode
 *
 * A common DetectorGraph::TimeoutPublisher pattern is seen on `DontDetector`;
 * it is both a DetectorGraph::TimeoutPublisher and a
 * DetectorGraph::SubscriberInterface of the same Topic, `DontPause`. This
 * achieves a simple timeout pattern where the detector is "called back" later.
 * @code
class DontDetector : public DetectorGraph::Detector
 ...
, public DetectorGraph::SubscriberInterface<DontPause>
, public DetectorGraph::TimeoutPublisher<DontPause>
{
    DontDetector(DetectorGraph::Graph* graph, DetectorGraph::TimeoutPublisherService* timeService)
 ...
    {
 ...
        Subscribe<DontPause>(this);
        SetupTimeoutPublishing<DontPause>(this, timeService);
    }
 ...
        PublishOnTimeout(DontPause(), RhytmBeats::kPeriod);
 * @endcode
 *
 *
 * @section Architecture
 * Below is the graph representation for this example.
 * @dot "BeatMachine"
digraph GraphAnalyzer {
    rankdir = "LR";
    node[fontname=Helvetica];

    "ThenYouState" [label="0:ThenYouState",style=filled, shape=box, color=limegreen];
    "RememberPause" [label="1:RememberPause",style=filled, shape=box, color=orange];
        "RememberPause" -> "RememberDetector";
    "DontPause" [label="2:DontPause",style=filled, shape=box, color=orange];
        "DontPause" -> "DontDetector";
    "RhytmBeats" [label="3:RhytmBeats",style=filled, shape=box, color=orange];
        "RhytmBeats" -> "ThemeDetector";
    "ThemeDetector" [label="4:ThemeDetector", color=blue];
        "ThemeDetector" -> "SongThemeState";
        "ThemeDetector" -> "RhytmBeats" [style=dotted, color=red];
    "SongThemeState" [label="5:SongThemeState",style=filled, shape=box, color=red];
        "SongThemeState" -> "DontDetector";
        "SongThemeState" -> "RememberDetector";
        "SongThemeState" -> "ThenYouDetector";
        "SongThemeState" -> "TooManyNaNaNasDetector";
    "TooManyNaNaNasDetector" [label="6:TooManyNaNaNasDetector", color=blue];
        "TooManyNaNaNasDetector" -> "PlaybackState";
    "PlaybackState" [label="7:PlaybackState",style=filled, shape=box, color=limegreen];
    "ThenYouDetector" [label="8:ThenYouDetector", color=blue];
        "ThenYouDetector" -> "ThenYouState" [style=dotted, color=red];
    "RememberDetector" [label="9:RememberDetector", color=blue];
        "RememberDetector" -> "RememberState";
        "RememberDetector" -> "RememberPause" [style=dotted, color=red];
    "RememberState" [label="10:RememberState",style=filled, shape=box, color=limegreen];
    "DontDetector" [label="11:DontDetector", color=blue];
        "DontDetector" -> "DontThemeState";
        "DontDetector" -> "DontPause" [style=dotted, color=red];
    "DontThemeState" [label="12:DontThemeState",style=filled, shape=box, color=limegreen];
}
 * @enddot
 *
 * @section on Other Notes
 * Note that this example in contained in a single file for the sake
 * of unity as an example. In real-world scenarios the suggested pattern is to
 * split the code into:
 *
 @verbatim
   detectorgraph/
        include/
            beatmachine.hpp (BeatMachine header)
        src/
            beatmachine.hpp (BeatMachine implementation)
        detectors/
            include/
                ThemeDetector.hpp
                DontDetector.hpp
                RememberDetector.hpp
                ThenYouDetector.hpp
                SondEndDetector.hpp
            src/
                ThemeDetector.cpp
                DontDetector.cpp
                RememberDetector.cpp
                ThenYouDetector.cpp
                SondEndDetector.cpp
        topicstates/
            include/
                RhytmBeats.hpp
                SongLine.hpp
                SongThemeState.hpp
                DontThemeState.hpp
                RememberState.hpp
                ThenYouState.hpp
                PlaybackState.hpp
@endverbatim
 *
 * @section References
 *  - [1] Na Automata - https://xkcd.com/851/
 *  - [2] Hey Jude Flowchart - http://loveallthis.tumblr.com/post/166124704
 *
 * Note that this examples take a bunch of poetic licenses to allow for more
 * compact representation of the code (e.g. omitting namespaces)
 */

/// @cond DO_NOT_DOCUMENT


using WallClock = std::chrono::high_resolution_clock;
using SteadyClock = std::chrono::steady_clock;
using TimePoint = std::chrono::time_point<WallClock>;
using Milliseconds = std::chrono::milliseconds;

class SleepBasedTimeoutPublisherService : public DetectorGraph::TimeoutPublisherService
{
    using TimerMap = std::map<TimeoutPublisherHandle, TimeOffset>;
    using TimerIterator = TimerMap::iterator;
public:
    SleepBasedTimeoutPublisherService(DetectorGraph::Graph& arGraph)
    : DetectorGraph::TimeoutPublisherService(arGraph)
    , mMetronomeId(kInvalidTimeoutPublisherHandle)
    {
    }

    // Sleeps until the next timer goes off.
    bool SleepTillBrooklyn()
    {
        if (mTimerMap.size() > 0)
        {
            TimerIterator minIt = GetNextTimeout();
            TimeOffset deadline = minIt->second;
            TimePoint deadlineTp = TimePoint(Milliseconds(deadline));
            std::this_thread::sleep_until(deadlineTp);

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

    // BEGIN CONCRETE IMPLEMENTATIONS
    // All methods below are necessary for the concrete implementation of TimeoutPublisherService
    TimeOffset GetTime() const
    {
        return std::chrono::duration_cast<Milliseconds>(WallClock::now().time_since_epoch())
            .count();
    }
    TimeOffset GetMonotonicTime() const
    {
        return std::chrono::duration_cast<Milliseconds>(SteadyClock::now().time_since_epoch())
            .count();
    }
protected:
    void SetTimeout(const TimeOffset aMillisecondsFromNow, const TimeoutPublisherHandle aTimerId)
    {
        mTimerMap[aTimerId] = aMillisecondsFromNow + GetMonotonicTime();
    }
    void Start(const TimeoutPublisherHandle aTimerId) { }
    void Cancel(const TimeoutPublisherHandle aTimerId) { mTimerMap.erase(aTimerId); }
    void StartMetronome(const TimeOffset aPeriodInMilliseconds)
    {
        mMetronomeId = GetUniqueTimerHandle();
        mMetronomeTimerPeriod = aPeriodInMilliseconds;
        SetTimeout(mMetronomeTimerPeriod, mMetronomeId);
        Start(mMetronomeId);
    }
    void CancelMetronome() { Cancel(mMetronomeId); }
    // END CONCRETE IMPLEMENTATIONS

private:
    TimerIterator GetNextTimeout()
    {
        return std::min_element(mTimerMap.begin(), mTimerMap.end(),
            [](const TimerMap::value_type& t1, const TimerMap::value_type& t2) -> bool {
                return (t1.second < t2.second);
        });
    }

private:
    std::map<TimeoutPublisherHandle, TimeOffset> mTimerMap;
    TimeoutPublisherHandle mMetronomeId;
    TimeOffset mMetronomeTimerPeriod;
};

struct RhytmBeats : public DetectorGraph::TopicState
{
    static const TimeOffset kPeriod = 60000 / 75; // 75 bpm
};

//! [TopicStates Inheritance Example]
struct SongLine
{
    std::string line;
    SongLine() : line() {}
    SongLine(const std::string& aLine) : line(aLine) {}
    friend std::ostream& operator<<(std::ostream& os, SongLine s) { return os << s.line; }
};

struct SongThemeState : public DetectorGraph::TopicState, public SongLine
{
    SongThemeState() {}
    SongThemeState(const std::string& aLine) : SongLine(aLine) {}
};

struct DontThemeState : public DetectorGraph::TopicState, public SongLine
{
    DontThemeState() {}
    DontThemeState(const std::string& aLine) : SongLine(aLine) {}
};

struct RememberState : public DetectorGraph::TopicState, public SongLine
{
    RememberState() {}
    RememberState(const std::string& aLine) : SongLine(aLine) {}
};

struct ThenYouState : public DetectorGraph::TopicState, public SongLine
{
    ThenYouState() {}
    ThenYouState(const std::string& aLine) : SongLine(aLine) {}
};

struct PlaybackState : public DetectorGraph::TopicState
{
    bool autoplay;
    PlaybackState(bool aAutoplay = false) : autoplay(aAutoplay) {}
};
//! [TopicStates Inheritance Example]

class ThemeDetector : public DetectorGraph::Detector
, public DetectorGraph::SubscriberInterface<RhytmBeats>
, public DetectorGraph::Publisher<SongThemeState>
{
public:
//! [ThemeDetector Constructor]
    ThemeDetector(DetectorGraph::Graph* graph, DetectorGraph::TimeoutPublisherService* timeService)
    : DetectorGraph::Detector(graph)
    , kBeatsPerTheme({
        {"Hey Jude", 2}, // 0
        {"don't", 14}, // 1
        {"Remember to", 6}, // 2
        {"then you", 10}, // 3
        {" ... better faster stronger ... ", 40}, // 4
        {"Na Na Na", 12}, // 5
        {"So let it out and let it in", 2}, // 6
        {"...", 18}, // 7
    })
    , kThemeSequence({0, 1, 2, 3, 0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 6, 0, 7, 0, 7, 5, 0, 1, 2, 5, 5})
    , mBeatsInSectionCount(0)
    , mThemeIt()
    {
        SetupPeriodicPublishing<RhytmBeats>(RhytmBeats::kPeriod, timeService);
        Subscribe<RhytmBeats>(this);
        SetupPublishing<SongThemeState>(this);
        mThemeIt = kThemeSequence.begin();
    }
//! [ThemeDetector Constructor]

    virtual void Evaluate(const RhytmBeats&)
    {
        if (mBeatsInSectionCount++ == 0)
        {
            Publish(SongThemeState(kBeatsPerTheme[*mThemeIt].first));
        }

        if (mBeatsInSectionCount == kBeatsPerTheme[*mThemeIt].second)
        {
            // Prep for next beat
            if (++mThemeIt == kThemeSequence.end()) mThemeIt = kThemeSequence.begin();
            mBeatsInSectionCount = 0;
        }
    }
private:
    using BeatsAndThemePair = std::pair<std::string, int>;
    const std::vector<BeatsAndThemePair> kBeatsPerTheme;
    const std::vector<int> kThemeSequence;
    int mBeatsInSectionCount;
    std::vector<int>::const_iterator mThemeIt;
};

struct DontPause : public DetectorGraph::TopicState {};

class DontDetector : public DetectorGraph::Detector
, public DetectorGraph::SubscriberInterface<SongThemeState>
, public DetectorGraph::Publisher<DontThemeState>
, public DetectorGraph::SubscriberInterface<DontPause>
, public DetectorGraph::TimeoutPublisher<DontPause>
{
public:
    DontDetector(DetectorGraph::Graph* graph, DetectorGraph::TimeoutPublisherService* timeService)
    : DetectorGraph::Detector(graph)
    , mDontIndex(0)
    , kResponses({"make it bad, take a sad song and make it better",
                  "be afraid, you were made to go out and get her",
                  "let me down, you have found her, now go and get her"})
    {
        Subscribe<SongThemeState>(this);
        SetupPublishing<DontThemeState>(this);

        Subscribe<DontPause>(this);
        SetupTimeoutPublishing<DontPause>(this, timeService);
    }

    virtual void Evaluate(const SongThemeState& aSongThemeState)
    {
        // If this is our cue
        if (aSongThemeState.line.find("don't") != std::string::npos)
        {
            PublishOnTimeout(DontPause(), RhytmBeats::kPeriod);
        }
    }
    virtual void Evaluate(const DontPause&)
    {
        Publish(DontThemeState(kResponses[mDontIndex]));
        mDontIndex++;
        mDontIndex %= kResponses.size();
    }
private:
    DontThemeState mDontState;
    int mDontIndex;
    const std::vector<std::string> kResponses;
};

struct RememberPause : public DetectorGraph::TopicState {};

class RememberDetector : public DetectorGraph::Detector
, public DetectorGraph::SubscriberInterface<SongThemeState>
, public DetectorGraph::Publisher<RememberState>
, public DetectorGraph::SubscriberInterface<RememberPause>
, public DetectorGraph::TimeoutPublisher<RememberPause>
{
public:
    RememberDetector(DetectorGraph::Graph* graph, DetectorGraph::TimeoutPublisherService* timeService)
    : DetectorGraph::Detector(graph)
    , mRememberIndex(0)
    , kResponses({"let her into your heart",
                  "let her under your skin"})
    {
        Subscribe<SongThemeState>(this);
        SetupPublishing<RememberState>(this);

        Subscribe<RememberPause>(this);
        SetupTimeoutPublishing<RememberPause>(this, timeService);
    }

    virtual void Evaluate(const SongThemeState& aSongThemeState)
    {
        // If this is our cue
        if (aSongThemeState.line.find("Remember") != std::string::npos)
        {
            PublishOnTimeout(RememberPause(), RhytmBeats::kPeriod);
        }
    }
    virtual void Evaluate(const RememberPause&)
    {
        Publish(RememberState(kResponses[mRememberIndex]));
        mRememberIndex++;
        mRememberIndex %= kResponses.size();
    }
private:
    int mRememberIndex;
    const std::vector<std::string> kResponses;
};

class ThenYouDetector : public Detector
, public DetectorGraph::SubscriberInterface<SongThemeState>
, public DetectorGraph::TimeoutPublisher<ThenYouState>
{
public:
    ThenYouDetector(DetectorGraph::Graph* graph, DetectorGraph::TimeoutPublisherService* timeService)
    : DetectorGraph::Detector(graph)
    , mThenYouIndex(0)
    , kResponses({"can start", "begin"})
    , kResponseEnd("to make it better")
    {
        Subscribe<SongThemeState>(this);
        SetupTimeoutPublishing<ThenYouState>(this, timeService);
    }

    virtual void Evaluate(const SongThemeState& aSongThemeState)
    {
        // If this is our cue
        if (aSongThemeState.line.find("then you") != std::string::npos)
        {
            PublishOnTimeout(ThenYouState(kResponses[mThenYouIndex] + " " + kResponseEnd), RhytmBeats::kPeriod);
            mThenYouIndex++;
            mThenYouIndex %= kResponses.size();
        }
    }
private:
    int mThenYouIndex;
    const std::vector<std::string> kResponses;
    const std::string kResponseEnd;
};

class TooManyNaNaNasDetector : public DetectorGraph::Detector
, public DetectorGraph::SubscriberInterface<SongThemeState>
, public DetectorGraph::Publisher<PlaybackState>
{
public:
    TooManyNaNaNasDetector(DetectorGraph::Graph* graph) : DetectorGraph::Detector(graph), nananaCount(0)
    {
        Subscribe<SongThemeState>(this);
        SetupPublishing<PlaybackState>(this);
    }

    virtual void Evaluate(const SongThemeState& aSongThemeState)
    {
        if (aSongThemeState.line.find("Na") != std::string::npos)
        {
            if (++nananaCount == kTotalNa)
            {
                Publish(PlaybackState(false));
            }
        }
    }

private:
    int nananaCount;
    const int kTotalNa = 4;

};

class BeatMachine : public DetectorGraph::ProcessorContainer
{
public:
    BeatMachine()
    : mTimeService(mGraph)
    , mThemeDetector(&mGraph, &mTimeService)
    , mDontDetector(&mGraph, &mTimeService)
    , mRememberDetector(&mGraph, &mTimeService)
    , mThenYouDetector(&mGraph, &mTimeService)
    , mTooManyNaNaNasDetector(&mGraph)
    , mCurrentPlaybackState()
    {
    }

    SleepBasedTimeoutPublisherService mTimeService;
    ThemeDetector mThemeDetector;
    DontDetector mDontDetector;
    RememberDetector mRememberDetector;
    ThenYouDetector mThenYouDetector;
    TooManyNaNaNasDetector mTooManyNaNaNasDetector;

    PlaybackState mCurrentPlaybackState;

    void StartLoop()
    {
        mTimeService.StartPeriodicPublishing();
        while(mCurrentPlaybackState.autoplay && mTimeService.SleepTillBrooklyn())
        {
            ProcessGraph();
        }
    }

    virtual void ProcessOutput()
    {
        auto themeTopic = mGraph.ResolveTopic<SongThemeState>();
        if (themeTopic->HasNewValue())
        {
            cout << themeTopic->GetNewValue() << endl;
        }

        auto dontsTopic = mGraph.ResolveTopic<DontThemeState>();
        if (dontsTopic->HasNewValue())
        {
            cout << dontsTopic->GetNewValue() << endl;
        }

        auto rememberTopic = mGraph.ResolveTopic<RememberState>();
        if (rememberTopic->HasNewValue())
        {
            cout << rememberTopic->GetNewValue() << endl;
        }

        auto thenYouTopic = mGraph.ResolveTopic<ThenYouState>();
        if (thenYouTopic->HasNewValue())
        {
            cout << thenYouTopic->GetNewValue() << endl;
        }

        auto playbackTopic = mGraph.ResolveTopic<PlaybackState>();
        if (playbackTopic->HasNewValue())
        {
            mCurrentPlaybackState = playbackTopic->GetNewValue();
        }
    }
};

int main()
{
    BeatMachine beatMachine;

    beatMachine.ProcessData(PlaybackState(true));
    beatMachine.StartLoop();

    DetectorGraph::GraphAnalyzer analyzer(beatMachine.mGraph);
    analyzer.GenerateDotFile("beat_machine.dot");
}

/// @endcond DO_NOT_DOCUMENT
