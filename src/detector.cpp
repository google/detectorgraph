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
    // Remove self as out edge on topics
    for (std::list<SubscriptionDispatcherInterface*>::iterator it = mInDispatchers.begin(); it != mInDispatchers.end(); ++it)
    {
        (*it)->GetTopicVertex()->RemoveEdge(this);

        // Remove subscription dispatcher
        delete *it;
    }
    mInDispatchers.clear();
    mOutEdges.clear();
    mGraph->RemoveVertex(this);
}

void Detector::ProcessVertex()
{
    if (Vertex::GetState() == kVertexProcessing)
    {
        this->BeginEvaluation();
        // cout << "ProcessVertex " << GetName() << endl;
        for (std::list<SubscriptionDispatcherInterface*>::iterator it = mInDispatchers.begin(); it != mInDispatchers.end(); ++it)
        {
            (*it)->Dispatch();
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
