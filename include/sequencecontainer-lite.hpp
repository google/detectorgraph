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

#ifndef DETECTORGRAPH_INCLUDE_SEQUENCECONTAINER_LITE_HPP_
#define DETECTORGRAPH_INCLUDE_SEQUENCECONTAINER_LITE_HPP_

#include "dgassert.hpp"

#include <new>

#if __cplusplus >= 201103L
#include <type_traits>
#endif

namespace DetectorGraph
{

// Used for actually storing a number of stuff - stuff can be anything pretty
// much. This may be nice for storing TopicStates as it doesn't impose default
// constructibility (which ends up adding cost for nothing).
// But god knows how safe this is.
template <typename T, unsigned N>
class SequenceContainer
{
#if __cplusplus >= 201103L
    static_assert(sizeof(T) % sizeof(uint32_t) == 0, "T must be uint32_t aligned");
    static_assert(std::is_copy_constructible<T>::value, "Generic SequenceContainer copy-constructs T");
#endif
public:
    SequenceContainer() : mNumElements()
    {
    }

    ~SequenceContainer()
    {
        clear();
    }

    void push_back(const T& v)
    {
        DG_ASSERT(mNumElements < N);

        uint8_t* storagePtr = &(mStorage[mNumElements * sizeof(T)]);
        T* newElement = new(storagePtr) T(v); // Copy constructor
        (void)newElement; // Otherwise it's unused in release builds.
        DG_ASSERT((void*)newElement == (void*)&(Items()[mNumElements]));
        mNumElements++;
    }

    T (& Items())[N]
    {
        return reinterpret_cast<T(&)[N]>(mStorage);
    }

    const T (& Items() const)[N]
    {
        return reinterpret_cast<const T(&)[N]>(mStorage);
    }

    T& operator [](unsigned idx) {
        DG_ASSERT(idx < N);
        return Items()[idx];
    }

    const T& operator [](unsigned idx) const {
        DG_ASSERT(idx < N);
        return Items()[idx];
    }

    const size_t size() const
    {
        return mNumElements;
    }

    void clear()
    {
        for (unsigned idx = 0; idx < mNumElements; ++idx)
        {
            Items()[idx].~T();
        }
        mNumElements = 0;
    }

    const T& back() const
    {
        return Items()[mNumElements-1];
    }


    typedef T * iterator;
    typedef const T * const_iterator;
    const_iterator begin() const { return &(Items()[0]); }
    const_iterator end() const { return &(Items()[mNumElements]); }
    iterator begin() { return &(Items()[0]); }
    iterator end() { return &(Items()[mNumElements]); }
    // Or if that makes anyone nervous:
    // const_iterator end() const { return &(Items()[0]) + mNumElements; }
private:
    uint8_t mStorage[N * sizeof(T)];
    unsigned mNumElements;
};


// // This may be useful for storing single TopicStates (clobbering) but keeping
// // the same interface.
// template <typename T>
// class ClobberingStorage<T, 1>
// {
//     static_assert(std::is_default_constructible<T>::value, "ClobberingStorage requires default constructor");
//     static_assert(std::is_copy_constructible<T>::value, "ClobberingStorage requires copying");
// public:
//     ClobberingStorage()
//     {
//     }

//     void push_back(const T& v)
//     {
//         mStorage[0] = v;
//     }

//     T (& Items())[1]
//     {
//         return reinterpret_cast<T(&)[1]>(mStorage);
//     }

//     const size_t GetSize() const
//     {
//         return 1;
//     }
// private:
//     T mStorage[1];
// };

} // namespace DetectorGraph

#endif // DETECTORGRAPH_INCLUDE_SEQUENCECONTAINER_LITE_HPP_
