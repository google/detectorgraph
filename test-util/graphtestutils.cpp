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

#include "graphtestutils.hpp"
#include "dglogging.hpp"

namespace DetectorGraph
{

namespace GraphTestUtils
{

    /*
     * These are quickhand utilities for composing unit tests
     * for Detectors implemented using DetectorGraph
     */

    void Flush(Graph& aGraph)
    {
        while (aGraph.HasDataPending()) { aGraph.EvaluateGraph(); }
    }

#if !defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
    void PrintOutputs(Graph& aGraph)
    {
        DG_LOG("-----Graph::GetOutputList() contains:-----");
        const std::list<ptr::shared_ptr<const TopicState> > outputs = aGraph.GetOutputList();
        for (std::list<ptr::shared_ptr<const TopicState> >::const_iterator it = outputs.begin();
            it != outputs.end();
            ++it)
        {
            DG_LOG("Output contains %s\n", (*it)->GetName());
        }
        DG_LOG("---------------------DONE------------------------");
    }
#endif

} // namespace GraphTestUtils

} // namespace DetectorGraph
