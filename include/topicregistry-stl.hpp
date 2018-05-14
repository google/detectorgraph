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

#ifndef DETECTORGRAPH_INCLUDE_TOPICREGISTRY_STL_HPP_
#define DETECTORGRAPH_INCLUDE_TOPICREGISTRY_STL_HPP_

#include <map>
#include <typeinfo>

namespace DetectorGraph
{
/**
 * @brief _Internal_ - A Registry of available Topics
 *
 * Graphs use a TopicRegistry to register and resolve (i.e. retrieve) Topics
 * using a Type-aware API.
 * This uses RTTI to retrieve an 'index' for each class.
 *
 * Right now using 'name' because it's convenient for debugging..
 * But could use other stuff too.
 */
class TopicRegistry
{
    /* TODO(DGRAPH-18): Use std::type_index (from <typeindex>) as the
     * registry's index when on C++11. */
    std::map<const char*, BaseTopic*> registry;
public:
    template<class TTopicState> Topic<TTopicState>* Resolve()
    {
#if __cplusplus >= 201103L
        static_assert(std::is_base_of<TopicState, TTopicState>::value,
            "Trying to Resolve non-Topic type.");
#endif
        const char* typeKey = typeid(TTopicState).name();
        if (registry.count(typeKey) != 0)
        {
            return static_cast<Topic<TTopicState>*>(registry[typeKey]);
        }
        return NULL;
    }

    template<class TTopicState> void Register(Topic<TTopicState>* obj)
    {
#if __cplusplus >= 201103L
        static_assert(std::is_base_of<TopicState, TTopicState>::value,
            "Trying to Register non-Topic type.");
#endif
        registry[typeid(TTopicState).name()] = obj;
    }
};

} // namespace DetectorGraph

#endif // DETECTORGRAPH_INCLUDE_TOPICREGISTRY_STL_HPP_
