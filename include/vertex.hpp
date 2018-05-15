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

#ifndef DETECTORGRAPH_INCLUDE_VERTEX_HPP_
#define DETECTORGRAPH_INCLUDE_VERTEX_HPP_

#include <list>
#include <typeinfo>

namespace DetectorGraph
{
/**
 * @brief Define behaviors of a vertex in a graph
 */
class Vertex
{
public:
    Vertex() : mState(kVertexClear) {}
    virtual ~Vertex() {}
    virtual void ProcessVertex() = 0;
    /**
     * @brief Enum used for topological sort & traverse context keeping
     */
    enum VertexSearchState
    {
        kVertexClear,
        kVertexProcessing,
        kVertexDone
    };

    /**
     * @ brief Enum used to identify the type of Vertex on a Detector Graph
     */
    enum VertexType
    {
        kTopicVertex,
        kDetectorVertex,
        kTestVertex
    };

    virtual VertexType GetVertexType() const = 0;

    // This uses RTTI only for clarity purposes. And could potentially be removed.
    // LCOV_EXCL_START
    const char * GetName() const
    {
        return typeid(*this).name();
    }
    // LCOV_EXCL_STOP

    void InsertEdge(Vertex* aVertex)
    {
        mOutEdges.push_back(aVertex);
        aVertex->mInEdges.push_back(this);
    }

    void RemoveEdge(Vertex* aVertex)
    {
        mOutEdges.remove(aVertex);
        aVertex->mInEdges.remove(this);
    }

    std::list<Vertex*>& GetOutEdges()
    {
        return mOutEdges;
    }

    std::list<Vertex*>& GetInEdges()
    {
        return mInEdges;
    }

    void MarkFutureEdge(Vertex* aVertex)
    {
        mFutureOutEdges.push_back(aVertex);
        aVertex->mFutureInEdges.push_back(this);
    }

    std::list<Vertex*>& GetFutureOutEdges()
    {
        return mFutureOutEdges;
    }

    std::list<Vertex*>& GetFutureInEdges()
    {
        return mFutureInEdges;
    }

    VertexSearchState GetState() const
    {
        return mState;
    }

    void SetState(VertexSearchState aNewState)
    {
        mState = aNewState;
    }

protected:
    VertexSearchState mState;
    std::list<Vertex*> mInEdges;
    std::list<Vertex*> mOutEdges;
    std::list<Vertex*> mFutureInEdges;
    std::list<Vertex*> mFutureOutEdges;
};

} // namespace DetectorGraph

#endif // DETECTORGRAPH_INCLUDE_VERTEX_HPP_
