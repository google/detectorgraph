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

#include "detector.hpp"

namespace DetectorGraph
{

Detector::Detector(Graph* graph) : mGraph(graph)
{
    mGraph->AddVertex(this);
}

Detector::~Detector()
{
#if !defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
    // Remove self as out edge on topics
    for (unsigned idx = 0; idx != mDispatchersContainer.GetSize(); ++idx)
    {
        mDispatchersContainer.GetDispatchers()[idx]->GetTopicVertex()->RemoveEdge(this);
    }
    mOutEdges.clear();
    mGraph->RemoveVertex(this);
#endif
}

void Detector::ProcessVertex()
{
    if (Vertex::GetState() == kVertexProcessing)
    {
        this->BeginEvaluation();
        for (unsigned idx = 0; idx != mDispatchersContainer.GetSize(); ++idx)
        {
            mDispatchersContainer.GetDispatchers()[idx]->Dispatch();
        }
        this->CompleteEvaluation();

        Vertex::SetState(kVertexDone);
    }
}

void Detector::BeginEvaluation()
{
    // Empty
}

void Detector::CompleteEvaluation()
{
    // Empty
}

}
