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

#ifndef DETECTORGRAPH_TEST_UTIL_SPLITTERDETECTOR_HPP_
#define DETECTORGRAPH_TEST_UTIL_SPLITTERDETECTOR_HPP_

#include "graph.hpp"
#include "topicstate.hpp"
#include "detector.hpp"

#include <utility>

namespace DetectorGraph
{

/* This is a utility test tool - this is meant to help with unit tests.
 * It's a "utility" detector and TopicState to stress the publishing to two
 * different topicStates in the same evaluation pass.
 *
 *
 * Topic<T1>   Topic<T2>
 *      ^         ^
 *       \       /
 *        \     /
 *         \   /
 *          \ /
 *           O  TestSplitterDetector<T1,T2>
 *           ^
 *           |
 *           -  Topic< TestSplitterTrigger<T1,T2> >
 *
 *
 */
template <class OutA, class OutB>
struct TestSplitterTrigger : public TopicState, public std::pair<OutA, OutB>
{
    TestSplitterTrigger() : std::pair<OutA, OutB>()
    {
    }

    TestSplitterTrigger(const OutA& a, const OutB& b) : std::pair<OutA, OutB>(a,b)
    {
    }
};

template<class OutA, class OutB>
class TestSplitterDetector : public Detector,
    public SubscriberInterface< TestSplitterTrigger<OutA, OutB> >,
    public Publisher<OutA>,
    public Publisher<OutB>
{
public:
    TestSplitterDetector(Graph* graph) : Detector(graph)
    {
        Subscribe< TestSplitterTrigger<OutA, OutB> >(this);
        SetupPublishing<OutA>(this);
        SetupPublishing<OutB>(this);
    }

    virtual void Evaluate(const TestSplitterTrigger<OutA, OutB>& pair)
    {
        Publisher<OutA>::Publish(pair.first);
        Publisher<OutB>::Publish(pair.second);
    }
};

} // namespace DetectorGraph

#endif // DETECTORGRAPH_TEST_UTIL_SPLITTERDETECTOR_HPP_
