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

#ifndef DETECTORGRAPH_INCLUDE_TOPIC_HPP_
#define DETECTORGRAPH_INCLUDE_TOPIC_HPP_

#include "vertex.hpp"
#include "subscriberinterface.hpp"
#include "topicstate.hpp"
#include "dgassert.hpp"

#if defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
// LITE_BEGIN
#include "detectorgraphliteconfig.hpp"
#include "sequencecontainer-lite.hpp"
// LITE_END
#else
// FULL_BEGIN
#include "sharedptr.hpp"
#include <vector>
#include <typeinfo>
// FULL_END
#endif

#include "dgstdincludes.hpp"

namespace DetectorGraph
{

class Graph;

/**
 * @brief Provide interface for a topic
 */
class BaseTopic : public Vertex
{
public:
#if !defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
    virtual std::list< ptr::shared_ptr<const TopicState> > GetCurrentTopicStates() const = 0;
    virtual TopicStateIdType GetId() const = 0;
#endif

    virtual VertexType GetVertexType() const { return Vertex::kTopicVertex; }

protected:
    void MarkChildrenState(VertexSearchState aNewState)
    {
        for (VertexPtrContainer::iterator vIt = GetOutEdges().begin();
            vIt != GetOutEdges().end();
            ++vIt)
        {
            (*vIt)->SetState(aNewState);
        }
    }
};
/**
 * @brief Manage data and its handler
 *
 * It is a data transport system with publish / subscribe semantics.
 *
 * # Internals #
 * `std::vector<T> mCurrentValues` is the holder for current values in the topic.
 *
 * This vector is cleared once per evaluation at either:
 * - The first Publish call during an evaluation pass (cleared
 * before adding new value)
 * - Topic is processed, and no further data will be Published to it
 *
 * The intended behavior is to have this vector always contain all the
 * data for a single evaluation pass - or nothing if that's the case.
 */
template<class T>
class Topic : public BaseTopic
{
    // Post-C++11 type checking
#if defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_STATIC_ASSERTS)
    static_assert(std::is_base_of<TopicState, T>::value, "T must inherit from TopicState");
#endif

public:

#if defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
    typedef SequenceContainer<T, DetectorGraphConfig::kMaxNumberOfTopicStates> ValuesContainer;
#else
    typedef std::vector<T> ValuesContainer;
#endif

    Topic()
    {
        // Pre-C++11 type checking.
#if __cplusplus < 201103L
        (void)static_cast<TopicState*>((T*)0);
#endif
    }

#if defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
    Topic(Graph* registry);
#endif

    /**
     * @brief Append data to its vector
     */
    void Publish(const T& arPayload)
    {
        if (Vertex::GetState() != kVertexProcessing)
        {
            mCurrentValues.clear();
            Vertex::SetState(kVertexProcessing);
        }

        mCurrentValues.push_back(arPayload);
    }

    virtual void ProcessVertex()
    {
        // TODO(DGRAPH-5): This maintains the old detector graph state style
        // if we want to preserve topic states until they're touched we can
        // remove this clearing here.
        if (Vertex::GetState() == kVertexClear)
        {
            mCurrentValues.clear();
        }

        if (Vertex::GetState() == kVertexProcessing)
        {
            Vertex::SetState(kVertexDone);
            MarkChildrenState(kVertexProcessing);
        }
    }

    /**
     * @brief Pass pending data to its handler
     */
    void DispatchIntoSubscriber(SubscriberInterface<T>* aSubscriber)
    {
        if (Vertex::GetState() == kVertexDone)
        {
            // TODO(DGRAPH-35): Both implementations below work fine. Sometimes
            // the iterator generates smaller code, sometimes not. Sort this out.
            for (typename ValuesContainer::const_iterator valueIt = mCurrentValues.begin();
                valueIt != mCurrentValues.end();
                ++valueIt)
            {
                aSubscriber->Evaluate(*valueIt);
            }
            // for (unsigned valueIdx = 0; valueIdx < mCurrentValues.size(); ++valueIdx)
            // {
            //     aSubscriber->Evaluate(mCurrentValues[valueIdx]);
            // }
        }
    }

    /**
     * @brief Returns true if the new Data is available.
     *
     * New data is signaled by the Topic having been Processed and containing
     * in storage.
     */
    inline bool HasNewValue() const
    {
        return (Vertex::GetState() == DetectorGraph::Vertex::kVertexDone);
    }

    /**
     * @brief Returns reference to the new/latest value in the topic.
     *
     * This method is a direct forwarding of the internal STL container and as
     * such, calling GetNewValue on an empty Topic is undefined.
     *
     * On debug builds, a DG_ASSERT will check that the Topic contains a value.
     */
    inline const T& GetNewValue() const
    {
        DG_ASSERT(HasNewValue());
        return mCurrentValues.back();
    }

    const ValuesContainer& GetCurrentValues() const
    {
        return mCurrentValues;
    }

    virtual ~Topic()
    {}

#if !defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)
// FULL_BEGIN
    virtual TopicStateIdType GetId() const
    {
        return TopicState::GetId<T>();
    }

    virtual std::list<ptr::shared_ptr<const TopicState> > GetCurrentTopicStates() const
    {
        std::list<ptr::shared_ptr<const TopicState> > tCurrentTopicStates;

        for (typename std::vector<T>::const_iterator valueIt = mCurrentValues.begin();
                valueIt != mCurrentValues.end();
                ++valueIt)
        {
            tCurrentTopicStates.push_back(ptr::shared_ptr<TopicState>(new T(*valueIt)));
        }

        return tCurrentTopicStates;
    }
// FULL_END
#endif

private:
    /*
     * @brief List of current data in topic. @sa Topic.
     */
    ValuesContainer mCurrentValues;
};

} // namespace DetectorGraph

#endif // DETECTORGRAPH_INCLUDE_TOPIC_HPP_
