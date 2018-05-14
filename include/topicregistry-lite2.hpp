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

#ifndef DETECTORGRAPH_INCLUDE_TOPICREGISTRY_LITE2_HPP_
#define DETECTORGRAPH_INCLUDE_TOPICREGISTRY_LITE2_HPP_

#include "topic.hpp"
#include "topicstate.hpp"
#include "dgassert.hpp"

namespace DetectorGraph
{
/**
 * @brief _Internal_ - A statically and automatically sized registry for Topics
 *
 * This TopicRegistry needs no explicit sizing, does not depend on RTTI nor STL
 * and puts no requirements on TopicStates. It's downside is that its storage is
 * static and thus behaves like a singleton - this means only one instance of
 * TopicRegistry can be alive at any time.
 *
 * This implementation stores BaseTopic* on TTopicState-templated methods. The
 * pointer can be set by calling ResolveOrRegister with a non-NULL argument and
 * retrieved by calling the same method with a NULL argument.
 *
 * The type stored in the templated methods - RegistryNode - is a single node
 * for a linked-list stack. Each node holds a BaseTopic* and a pointer to
 * another RegistryNode. This allows TopicRegistry to perform cleanup of the
 * static/singleton nodes per topic at the destructor.
 */
class TopicRegistry
{
    /**
     * @brief _Internal_ - The node stored at each templated ResolveOrRegister
     */
    struct RegistryNode
    {
        RegistryNode()
        : storedPtr(NULL), next(NULL)
        {}
        BaseTopic* storedPtr;
        RegistryNode* next;
    };

public:
    TopicRegistry() : mTopNode(NULL)
    {
    }

    ~TopicRegistry()
    {
        Cleanup();
    }

    /**
     * @brief _Internal_ - Retrieves a Topic pointer for a given TopicState.
     */
    template<class TTopicState>
    Topic<TTopicState>* Resolve()
    {
#if __cplusplus >= 201103L
        static_assert(std::is_base_of<TopicState, TTopicState>::value,
            "Trying to Resolve non-Topic type.");
#endif
        RegistryNode* nodePtr = ResolveOrRegister<TTopicState>(NULL);

        // Here one may feel tempted to check that there's a valid pointer
        // for this type - but remember that in some versions of this graph
        // a mechanism relies on this returning NULL to then create and
        // register the missing topic.

        return static_cast<Topic<TTopicState>*>(nodePtr->storedPtr);
    }

    /**
     * @brief _Internal_ - Registers a Topic pointer for a given TopicState.
     */
    template<class TTopicState>
    void Register(Topic<TTopicState>* topicPtr)
    {
#if __cplusplus >= 201103L
        static_assert(std::is_base_of<TopicState, TTopicState>::value,
            "Trying to Register non-Topic type.");
#endif
        // Checks that this is the first registration for this type.
        DG_ASSERT(ResolveOrRegister<TTopicState>(NULL)->storedPtr == NULL);

        RegistryNode* nodePtr = ResolveOrRegister<TTopicState>(topicPtr);
        RegisterNode(nodePtr);
    }

private:
    template<class TTopicState>
    RegistryNode* ResolveOrRegister(Topic<TTopicState>* inTopicPtr)
    {
        static RegistryNode topicNode;
        if (inTopicPtr)
        {
            topicNode.storedPtr = static_cast<BaseTopic*>(inTopicPtr);
        }
        return &topicNode;
    }

    void RegisterNode(RegistryNode* node)
    {
        // adds to the end. If empty, next points to null.
        node->next = mTopNode;
        mTopNode = node;
    }

    void Cleanup()
    {
        RegistryNode* node = mTopNode;
        while (node)
        {
            RegistryNode* tmp = node->next;
            node->storedPtr = NULL;
            node->next = NULL;
            node = tmp;
        }
    }

    RegistryNode* mTopNode;
};

} // namespace DetectorGraph

#endif // DETECTORGRAPH_INCLUDE_TOPICREGISTRY_LITE2_HPP_
