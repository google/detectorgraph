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

#ifndef DETECTORGRAPH_INCLUDE_LAG_HPP_
#define DETECTORGRAPH_INCLUDE_LAG_HPP_

#include "graph.hpp"
#include "topicstate.hpp"
#include "detector.hpp"
#include "futurepublisher.hpp"

namespace DetectorGraph
{

/* This is a utility detector and topic template that performs a 1-lag on a
 * TopicState. It's meant to enable closing feedback loops in a graph in a
 * expressive and transparent instead of an explicit ad-hoc way.
 *
 *
 *           -  Lagged< Topic< T > >
 *           ^
 *           |
 *
 *           |  (FuturePublish)
 *
 *           |
 *           O  Lag< T >
 *           ^
 *           |
 *           -  Topic< T >
 *
 *
 */


/// @brief A templated TopicState used as the output of Lag
template <class T>
struct Lagged : public TopicState
{
    Lagged() : data()
    {
    }

    Lagged(const T& a) : data(a)
    {
    }

    T data;
};

/**
 * @brief Produces a [Lagged<T>](@ref Lagged) for T
 *
 * This is a utility detector (and topic template) that performs a 1-lag on a
 * TopicState. It's meant to enable closing feedback loops in a graph in an
 * expressive and transparent way. It creates a Detector with the following
 * relationships:
 *
 *  @dot "Lag"
digraph GraphAnalyzer {
    rankdir = "LR";
    node[fontname=Helvetica];
    size="12,5";
    "T" [label="0:T",style=filled, shape=box, color=lightblue];
        "T" -> "Lag";
        "Lag" -> "Lagged" [style=dotted, color=red];
    "Lag" [label="1:Lag<T>", color=blue];
    "Lagged" [label="2:Lagged<T>",style=filled, shape=box, color=limegreen];
}
 *  @enddot
 *
 * Adding a Lag\<T\> Detector to a graph is done simply by adding it to a
 * graph as shown ([from this example](@ref robotlocalization.cpp)) below:
 @snippet robotlocalization.cpp RobotBrain Static
 *
 * Lagged\<T\> can then be used in any detector. In the same [example](
 * @ref robotlocalization.cpp):
 @snippet robotlocalization.cpp KalmanPoseCorrector Feedback Loop
 *
 */
template<class T>
class Lag : public Detector,
    public SubscriberInterface< T >,
    public FuturePublisher< Lagged< T > >
{
public:
    Lag(Graph* graph) : Detector(graph)
    {
        Subscribe< T >(this);
        SetupFuturePublishing< Lagged< T > >(this);
    }

    virtual void Evaluate(const T& currentTopicState)
    {
        FuturePublisher< Lagged<T> >::PublishOnFutureEvaluation(Lagged<T>(currentTopicState));
    }
};

} // namespace DetectorGraph

#endif // DETECTORGRAPH_INCLUDE_LAG_HPP_
