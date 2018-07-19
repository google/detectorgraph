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

#include "dglogging.hpp"

#if defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
// LITE_BEGIN
#include "detectorgraphliteconfig.hpp"
#include "sequencecontainer-lite.hpp"
// LITE_END
#else
// FULL_BEGIN
#include <list>
#include <typeinfo>
// FULL_END
#endif

namespace DetectorGraph
{
/**
 * @brief Define behaviors of a vertex in a graph
 */
class Vertex
{
public:
#if defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
    typedef SequenceContainer<Vertex*, DetectorGraphConfig::kMaxNumberOfOutEdges> VertexPtrContainer;
#else
    typedef std::list<Vertex*> VertexPtrContainer;
#endif

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

    VertexSearchState GetState() const
    {
        return mState;
    }

    void SetState(VertexSearchState aNewState)
    {
        mState = aNewState;
    }

    void InsertEdge(Vertex* aVertex)
    {
        mOutEdges.push_back(aVertex);
#if !defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
        aVertex->mInEdges.push_back(this);
#endif
#if defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_INSTRUMENT_RESOURCE_USAGE)
    DG_LOG("Added Edge (total=%u)", mOutEdges.size());
#endif
    }

    void RemoveEdge(Vertex* aVertex)
    {
#if !defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
        mOutEdges.remove(aVertex);
        aVertex->mInEdges.remove(this);
#endif
    }

    VertexPtrContainer& GetOutEdges()
    {
        return mOutEdges;
    }

    void MarkFutureEdge(Vertex* aVertex)
    {
#if !defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
        mFutureOutEdges.push_back(aVertex);
        aVertex->mFutureInEdges.push_back(this);
#endif
    }

#if !defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
    // FULL_BEGIN
public:
    VertexPtrContainer& GetInEdges()
    {
        return mInEdges;
    }

    VertexPtrContainer& GetFutureOutEdges()
    {
        return mFutureOutEdges;
    }

    VertexPtrContainer& GetFutureInEdges()
    {
        return mFutureInEdges;
    }

    // This uses RTTI only for clarity purposes. And could potentially be removed.
    // LCOV_EXCL_START
    const char * GetName() const
    {
        return typeid(*this).name();
    }
    // LCOV_EXCL_STOP
    // FULL_END
#endif

protected:
    VertexSearchState mState;
    VertexPtrContainer mOutEdges;

#if !defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
    VertexPtrContainer mInEdges;
    VertexPtrContainer mFutureOutEdges;
    VertexPtrContainer mFutureInEdges;
#endif
};

} // namespace DetectorGraph

#endif // DETECTORGRAPH_INCLUDE_VERTEX_HPP_
