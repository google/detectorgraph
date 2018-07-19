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

#ifndef DETECTORGRAPH_TEST_UTIL_MARKIITESTUTILS_HPP_
#define DETECTORGRAPH_TEST_UTIL_MARKIITESTUTILS_HPP_

#include "graph.hpp"
#include "topicstate.hpp"
#include "detector.hpp"

namespace DetectorGraph
{

namespace GraphTestUtils
{

    /*
     * These are quickhand utilities for composing unit tests
     * for Detectors implemented using DetectorGraph
     */

    void Flush(Graph& aGraph);

    template<class TTopic> void FlushAndPush(Graph& aGraph, const TTopic& aTopicState)
    {
        Flush(aGraph);
        aGraph.PushData<TTopic>(aTopicState);
    }

#if !defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
    void PrintOutputs(Graph& aGraph);
#endif

} // namespace GraphTestUtils

} // namespace DetectorGraph

#endif // DETECTORGRAPH_TEST_UTIL_MARKIITESTUTILS_HPP_
