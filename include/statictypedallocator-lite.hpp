// Copyright 2018 Nest Labs, Inc.
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

#ifndef DETECTORGRAPH_INCLUDE_STATICTYPEDALLOCATOR_LITE_HPP_
#define DETECTORGRAPH_INCLUDE_STATICTYPEDALLOCATOR_LITE_HPP_

#include "dgassert.hpp"

#include <new>
#include <stdint.h>
#include <utility>

namespace DetectorGraph
{

/**
 * @brief Internal - Static allocator for child types of arbitrary sizes.
 *
 * Memory-constrained devices often don't provide a general purpose heap (new
 * & delete). In those cases it's desirable to have an application limit its
 * memory allocations purely to statically allocated memory.
 * DetectorGraph uses the `class Child<T> : class Base` pattern a lot to
 * provide it's richly typed APIs. And in some places it relies on runtime
 * allocation of such `Child<T>` objects. This class provides an allocator
 * for such objects that complies with the no-heap requirement.
 *
 * The one limitation is that it's only able to store 1 object per `T` of
 * `Child<T>`.
 *
 * Usage sample:
 * @code

struct SomeBase { virtual ~SomeBase() {} };
template<class T> struct SomeChild : public SomeBase { };

StaticTypedAllocator<SomeBase, 0> allocator;

SomeBase* objT1 = allocator.New<SomeChild<T1>>();
SomeBase* objT2 = allocator.New<SomeChild<T2>>();
allocator.Delete(objT1);
allocator.Delete(objT2);

 * @endcode
 *
 * Different values for the Ctxt template parameter must be used on separate
 * instances if they store the same types:
 * @code
// Given

StaticTypedAllocator<SomeBase, 0> allocatorA;
SomeBase* objA = allocator.New<SomeChild<T1>>();
// SomeBase* objB = allocator.New<SomeChild<T1>>(); // Will throw a 'busy' assert.

StaticTypedAllocator<SomeBase, 1> allocatorB;
SomeBase* objB = allocatorB.New<SomeChild<T1>>(); // ok

 * @endcode
 *
 *
 */
namespace {
	struct DefaultStaticTypedAllocatorCtxt {};
}

template< class TBase, class Ctxt = DefaultStaticTypedAllocatorCtxt>
class StaticTypedAllocator
{
    struct NodeHeader
    {
        NodeHeader(uint8_t* aStorage)
        : storage(aStorage)
        , next(NULL), busy(false)
        {
        }

        uint8_t* storage;
        TBase* GetObjectPtr()
        {
            return reinterpret_cast<TBase*>(storage);
        }
        NodeHeader* next;
        bool busy;
    };
public:
    StaticTypedAllocator() : mHeadNode(NULL)
    {
    }

    /* Below there are two versions of the New() method.
     * The top one uses C++11 variadic template args and rvalue perfect
     * forwarding. This is both more convenient and more efficient as it
     * constructs the type only once vs constructing & copy constructing. */

    /**
     * @brief Constructs & Allocates new object of type TChild with arguments.
     */
    template <class TChild, typename... TChildArgs>
    TBase* New(TChildArgs&&... constructor_args)
    {
        NodeHeader* node = GetNodeHeader<TChild>();

        DG_ASSERT(!node->busy);
        // NOTE: Cannot store more than one object at the same time.

        TBase* newObjPtr =
            new(node->storage) TChild(std::forward<TChildArgs>(constructor_args)...);

        node->busy = true;
        LinkNode(node);
        return newObjPtr;
    }

    /**
     * @brief Copy-Constructs & Allocates new object of type TChild.
     */
    template<class TChild>
    TBase* New(const TChild& aOriginal)
    {
        NodeHeader* node = GetNodeHeader<TChild>();

        DG_ASSERT(!node->busy);
        // NOTE: Cannot store more than one object at the same time.

        TBase* newObjPtr =
            new(node->storage) TChild(aOriginal);

        node->busy = true;
        LinkNode(node);
        return newObjPtr;
    }

    /**
     * @brief Deletes/Frees an allocated object via the base ptr.
     */
    void Delete(TBase* targetObject)
    {
        // O(N) deletion
        DG_ASSERT(mHeadNode); // Not empty

        // A reference to the .next ptr on the previous node
        NodeHeader** fromPtr = &mHeadNode;

        // Traversing node
        NodeHeader* nodeIt = mHeadNode;

        // Target
        TBase* storedObject = nodeIt->GetObjectPtr();

        while(nodeIt && storedObject != targetObject)
        {
            // Save .next ptr address before following it
            fromPtr = &(nodeIt->next);
            nodeIt = nodeIt->next;
            storedObject = nodeIt->GetObjectPtr();
        }

        DG_ASSERT(nodeIt && storedObject);

        // Bypass deleted node
        *fromPtr = nodeIt->next;

        // Destroy deleted node
        storedObject->~TBase();

        // Cleanup deleted node
        nodeIt->busy = false;
        nodeIt->next = NULL;
    }

    /**
     * @brief Deletes all allocated objects.
     */
    void clear()
    {
        // O(N) clear.
        NodeHeader* nodeIt = mHeadNode;
        while(nodeIt)
        {
            TBase* storedObject = nodeIt->GetObjectPtr();
            storedObject->~TBase();
            nodeIt->busy = false;

            NodeHeader* tmp = nodeIt; // only necessary to cleanup
            nodeIt = nodeIt->next;
            tmp->next = NULL;
        }
        mHeadNode = NULL;
    }

    ~StaticTypedAllocator()
    {
        clear();
    }

private:
    void LinkNode(NodeHeader* node)
    {
        // O(N) insertion.

        // Find insertion point
        NodeHeader** nextPtr = &mHeadNode;
        while(*nextPtr)
        {
            nextPtr = &((*nextPtr)->next);
        }

        // Change last ptr to point to new node instead of NULL
        *nextPtr = node;
        node->next = NULL;
    }

private:
    NodeHeader* mHeadNode;

    template<class TChild>
    NodeHeader* GetNodeHeader()
    {
        static uint8_t mNodeStorage[sizeof(TChild)];
        static NodeHeader node = NodeHeader(mNodeStorage);
        return &node;
    }
};

}

#endif // DETECTORGRAPH_INCLUDE_STATICTYPEDALLOCATOR_LITE_HPP_
