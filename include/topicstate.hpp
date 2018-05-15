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

#ifndef DETECTORGRAPH_INCLUDE_TOPICSTATE_HPP_
#define DETECTORGRAPH_INCLUDE_TOPICSTATE_HPP_

#include <list>
#include <typeinfo>

namespace DetectorGraph
{

typedef int TopicStateIdType;

/**
 * @brief Base struct for topic data types
 *
 * This should be seen like a POD data container.
 * A simple example could be:
 * @code
struct AccelerometerData : public TopicState {
    int32_t x;
    int32_t y;
    int32_t z;
};
 * @endcode
 *
 * Ideally all TopicStates are self-explanatory and self-contained;
 * a subscriber shouldn't need anything else to act on it.
 * The rationale for calling it 'struct' instead of 'class':
 * - This is supposed to be a POD-like data container
 * - All methods here are only to support the data-carrying
 * functionality of TopicStates.
 * - This struct and its subclasses should not have any
 * private members.
 *
 */
struct TopicState
{
    TopicState() { }
    virtual ~TopicState() { }

    /**
     * @brief Get type name of itself
     *
     * This method is a convenience method that can be used for
     * graph debugging. It uses RTTI to generate a descriptive
     * string that identifies the child of TopicState. This strings
     * are compiler-specific but mostly based on the actual name of
     * struct that inherits from TopicState.
     */
    const char * GetName() const
    {
        return typeid(*this).name(); // LCOV_EXCL_LINE
    }

    /**
     * @brief Default @ref GetId() return value.
     */
    enum { kAnonymousTopicState = -1 };

    /**
     * @brief Returns the TopicStateId for this TopicState - to be implemented.
     *
     * If not implemented on a derived class it returns -1 (not implemented)
     * meaning this TopicState is an anonymous TopicState (only used within the
     * graph). This refers to a public number-space defined by the application
     * for types that are intended to be provided to (or from) the outside (of
     * the graph).
     *
     * For a full discussion/example of how to use _Named TopicStates_ see
     * [Trivial Vending Machine Example](@ref trivialvendingmachine.cpp)
     */
    virtual TopicStateIdType GetId() const
    {
        return (TopicStateIdType)kAnonymousTopicState;
    }

    /**
     * @brief Convenience templated static method to retrieve the ID for a
     * Type when an instance is not available.
     */
    template<class TTopic>
    static TopicStateIdType GetId()
    {
        static const TTopic dummy = TTopic();
        return dummy.GetId();
    } // LCOV_EXCL_LINE
};

} // namespace DetectorGraph

#endif // DETECTORGRAPH_INCLUDE_TOPICSTATE_HPP_
