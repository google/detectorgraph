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

#include "nltest.h"

#include "test_statictypedallocator.h"

#include "statictypedallocator-lite.hpp"
#include <typeinfo>

#define SUITE_DECLARATION(name, test_ptr) { #name, test_ptr, setup_##name, teardown_##name }

using namespace DetectorGraph;

static int setup_statictypedallocator(void *inContext)
{
    return 0;
}

static int teardown_statictypedallocator(void *inContext)
{
    return 0;
}

// Helper Types used in this tests.
struct SomeBase { virtual ~SomeBase() { } };

template<class T> struct SomeChild : public SomeBase
{
    static int instanceCount;
    SomeChild(const T& d) : data(d) { instanceCount++; }
    SomeChild(const SomeChild& other) : data(other.data) { instanceCount++; }
    ~SomeChild() { instanceCount--; }
    T data;
};

struct TopicStateA { TopicStateA(int arg) : a(arg) {} int a; };
struct TopicStateB { TopicStateB(int arg) : b(arg) {} int b; };
struct TopicStateC { TopicStateC(int arg) : c(arg) {} int c; };

template<>
int SomeChild<TopicStateA>::instanceCount = 0;
template<>
int SomeChild<TopicStateB>::instanceCount = 0;
template<>
int SomeChild<TopicStateC>::instanceCount = 0;

static void Test_StoreOneType(nlTestSuite *inSuite, void *inContext)
{
    {
        StaticTypedAllocator<SomeBase> allocator;

        SomeChild<TopicStateA>* childPtr = allocator.New<SomeChild<TopicStateA>>(TopicStateA(42));
        NL_TEST_ASSERT(inSuite, childPtr->data.a == 42);
        allocator.Delete(childPtr);
    }
    NL_TEST_ASSERT(inSuite, SomeChild<TopicStateA>::instanceCount == 0);
}

static void Test_StoreMultiple(nlTestSuite *inSuite, void *inContext)
{
    {
        StaticTypedAllocator<SomeBase> allocator;

        SomeBase* basePtrA = allocator.New<SomeChild<TopicStateA>>(73);
        SomeBase* basePtrB = allocator.New<SomeChild<TopicStateB>>(83);

        NL_TEST_ASSERT(inSuite, basePtrA != basePtrB);

        SomeChild<TopicStateA>* childPtrA = static_cast< SomeChild<TopicStateA>* >(basePtrA);
        SomeChild<TopicStateB>* childPtrB = static_cast< SomeChild<TopicStateB>* >(basePtrB);

        NL_TEST_ASSERT(inSuite, childPtrA->data.a == 73);
        NL_TEST_ASSERT(inSuite, childPtrB->data.b == 83);
    }
    NL_TEST_ASSERT(inSuite, SomeChild<TopicStateA>::instanceCount == 0);
    NL_TEST_ASSERT(inSuite, SomeChild<TopicStateB>::instanceCount == 0);
}

static void Test_Reuse(nlTestSuite *inSuite, void *inContext)
{
    {
        StaticTypedAllocator<SomeBase> allocator;

        SomeBase* basePtr;

        basePtr = allocator.New<SomeChild<TopicStateA>>(TopicStateA(666));
        allocator.Delete(basePtr);
        basePtr = allocator.New<SomeChild<TopicStateA>>(TopicStateA(999));

        SomeChild<TopicStateA>* childPtr = static_cast< SomeChild<TopicStateA>* >(basePtr);
        NL_TEST_ASSERT(inSuite, childPtr->data.a == 999);
    }
    NL_TEST_ASSERT(inSuite, SomeChild<TopicStateA>::instanceCount == 0);
}

static void Test_DeleteMiddle(nlTestSuite *inSuite, void *inContext)
{
    {
        StaticTypedAllocator<SomeBase> allocator;

        allocator.New<SomeChild<TopicStateA>>(0);
        SomeBase* basePtrB = allocator.New<SomeChild<TopicStateB>>(1000);
        allocator.New<SomeChild<TopicStateC>>(0);

        allocator.Delete(basePtrB);
        basePtrB = allocator.New<SomeChild<TopicStateB>>(1001);
        SomeChild<TopicStateB>* childPtr = static_cast< SomeChild<TopicStateB>* >(basePtrB);

        NL_TEST_ASSERT(inSuite, childPtr->data.b == 1001);
    }
    NL_TEST_ASSERT(inSuite, SomeChild<TopicStateA>::instanceCount == 0);
    NL_TEST_ASSERT(inSuite, SomeChild<TopicStateB>::instanceCount == 0);
    NL_TEST_ASSERT(inSuite, SomeChild<TopicStateC>::instanceCount == 0);
}

static void Test_DeleteLast(nlTestSuite *inSuite, void *inContext)
{
    {
        StaticTypedAllocator<SomeBase> allocator;

        allocator.New<SomeChild<TopicStateA>>(0);
        allocator.New<SomeChild<TopicStateB>>(0);
        SomeBase* basePtrC = allocator.New<SomeChild<TopicStateC>>(1000);

        allocator.Delete(basePtrC);
        basePtrC = allocator.New<SomeChild<TopicStateC>>(1001);
        SomeChild<TopicStateC>* childPtr = static_cast< SomeChild<TopicStateC>* >(basePtrC);

        NL_TEST_ASSERT(inSuite, childPtr->data.c == 1001);
    }
    NL_TEST_ASSERT(inSuite, SomeChild<TopicStateA>::instanceCount == 0);
    NL_TEST_ASSERT(inSuite, SomeChild<TopicStateB>::instanceCount == 0);
    NL_TEST_ASSERT(inSuite, SomeChild<TopicStateC>::instanceCount == 0);
}

static void Test_MultipleInstances(nlTestSuite *inSuite, void *inContext)
{
    {
        struct One {};
        StaticTypedAllocator<SomeBase, One > allocatorOne;
        struct Two {};
        StaticTypedAllocator<SomeBase, Two > allocatorTwo;

        SomeBase* basePtrOne = allocatorOne.New<SomeChild<TopicStateA>>(1);

        SomeBase* basePtrTwo = allocatorTwo.New<SomeChild<TopicStateA>>(2);

        SomeChild<TopicStateA>* childPtrOne = static_cast< SomeChild<TopicStateA>* >(basePtrOne);
        SomeChild<TopicStateA>* childPtrTwo = static_cast< SomeChild<TopicStateA>* >(basePtrTwo);

        NL_TEST_ASSERT(inSuite, childPtrOne != childPtrTwo);
        NL_TEST_ASSERT(inSuite, childPtrOne->data.a == 1);
        NL_TEST_ASSERT(inSuite, childPtrTwo->data.a == 2);
    }
    NL_TEST_ASSERT(inSuite, SomeChild<TopicStateA>::instanceCount == 0);
}

static void Test_Clear(nlTestSuite *inSuite, void *inContext)
{
    StaticTypedAllocator<SomeBase> allocator;

    allocator.New<SomeChild<TopicStateA>>(0);
    allocator.New<SomeChild<TopicStateB>>(0);

    allocator.clear();
    NL_TEST_ASSERT(inSuite, SomeChild<TopicStateA>::instanceCount == 0);
    NL_TEST_ASSERT(inSuite, SomeChild<TopicStateB>::instanceCount == 0);

    allocator.New<SomeChild<TopicStateA>>(1);
    allocator.New<SomeChild<TopicStateB>>(1);

    allocator.clear();
    NL_TEST_ASSERT(inSuite, SomeChild<TopicStateA>::instanceCount == 0);
    NL_TEST_ASSERT(inSuite, SomeChild<TopicStateB>::instanceCount == 0);
}

static void Test_PerfectNew(nlTestSuite *inSuite, void *inContext)
{
    {
        StaticTypedAllocator<SomeBase> allocator;

        SomeBase* basePtr = allocator.New<SomeChild<TopicStateA>>(TopicStateA(42));
        allocator.Delete(basePtr);

        SomeChild<TopicStateA>* childPtr = static_cast< SomeChild<TopicStateA>* >(basePtr);
        NL_TEST_ASSERT(inSuite, childPtr->data.a == 42);
    }
    NL_TEST_ASSERT(inSuite, SomeChild<TopicStateA>::instanceCount == 0);

    {
        StaticTypedAllocator<SomeBase> allocator;

        SomeBase* basePtr = allocator.New<SomeChild<TopicStateA>>(42);
        allocator.Delete(basePtr);

        SomeChild<TopicStateA>* childPtr = static_cast< SomeChild<TopicStateA>* >(basePtr);
        NL_TEST_ASSERT(inSuite, childPtr->data.a == 42);
    }
    NL_TEST_ASSERT(inSuite, SomeChild<TopicStateA>::instanceCount == 0);
}

// Types with different requirements tested on Test_PerfectNew

// Non-Copy-constructible, Non-default-constructible
template<class T> struct NonCopyNonDefault : public SomeBase
{
    static int instanceCount;
    NonCopyNonDefault(const T& d) : data(d) { instanceCount++; }
    ~NonCopyNonDefault() { instanceCount--; }
    T data;
private:
    NonCopyNonDefault(const NonCopyNonDefault&) {}
};

template<>
int NonCopyNonDefault<TopicStateA>::instanceCount = 0;

// Non-Copy-constructible, Default-constructible
template<class T> struct NonCopyButDefault : public SomeBase
{
    static int instanceCount;
    NonCopyButDefault() : data(42) { instanceCount++; }
    ~NonCopyButDefault() { instanceCount--; }
    T data;
private:
    NonCopyButDefault(const NonCopyButDefault& other) { }
};

template<>
int NonCopyButDefault<TopicStateA>::instanceCount = 0;

static void Test_PerfectNewWithRestrictions(nlTestSuite *inSuite, void *inContext)
{
    {
        StaticTypedAllocator<SomeBase> allocator;

        SomeBase* basePtr = allocator.New<NonCopyNonDefault<TopicStateA>>(TopicStateA(42));
        allocator.Delete(basePtr);

        NonCopyNonDefault<TopicStateA>* childPtr = static_cast< NonCopyNonDefault<TopicStateA>* >(basePtr);
        NL_TEST_ASSERT(inSuite, childPtr->data.a == 42);
    }
    NL_TEST_ASSERT(inSuite, NonCopyNonDefault<TopicStateA>::instanceCount == 0);

    {
        StaticTypedAllocator<SomeBase> allocator;

        SomeBase* basePtr = allocator.New<NonCopyNonDefault<TopicStateA>>(42);
        allocator.Delete(basePtr);

        NonCopyNonDefault<TopicStateA>* childPtr = static_cast< NonCopyNonDefault<TopicStateA>* >(basePtr);
        NL_TEST_ASSERT(inSuite, childPtr->data.a == 42);
    }
    NL_TEST_ASSERT(inSuite, NonCopyNonDefault<TopicStateA>::instanceCount == 0);

    {
        StaticTypedAllocator<SomeBase> allocator;

        SomeBase* basePtr = allocator.New<NonCopyNonDefault<TopicStateA>>(42);
        allocator.Delete(basePtr);

        NonCopyNonDefault<TopicStateA>* childPtr = static_cast< NonCopyNonDefault<TopicStateA>* >(basePtr);
        NL_TEST_ASSERT(inSuite, childPtr->data.a == 42);
    }
    NL_TEST_ASSERT(inSuite, NonCopyNonDefault<TopicStateA>::instanceCount == 0);

    {
        StaticTypedAllocator<SomeBase> allocator;

        SomeBase* basePtr = allocator.New<NonCopyButDefault<TopicStateA>>();
        allocator.Delete(basePtr);

        NonCopyButDefault<TopicStateA>* childPtr = static_cast< NonCopyButDefault<TopicStateA>* >(basePtr);
        NL_TEST_ASSERT(inSuite, childPtr->data.a == 42);
    }
    NL_TEST_ASSERT(inSuite, NonCopyButDefault<TopicStateA>::instanceCount == 0);
}

static const nlTest sTests[] = {
    NL_TEST_DEF("Test_StoreOneType", Test_StoreOneType),
    NL_TEST_DEF("Test_StoreMultiple", Test_StoreMultiple),
    NL_TEST_DEF("Test_Reuse", Test_Reuse),
    NL_TEST_DEF("Test_DeleteMiddle", Test_DeleteMiddle),
    NL_TEST_DEF("Test_DeleteLast", Test_DeleteLast),
    NL_TEST_DEF("Test_MultipleInstances", Test_MultipleInstances),
    NL_TEST_DEF("Test_Clear", Test_Clear),
    NL_TEST_DEF("Test_PerfectNew", Test_PerfectNew),
    NL_TEST_DEF("Test_PerfectNewWithRestrictions", Test_PerfectNewWithRestrictions),
    NL_TEST_SENTINEL()
};

//This function creates the Suite (i.e: the name of your test and points to the array of test functions)
extern "C"
int statictypedallocator_testsuite(void)
{
    nlTestSuite theSuite = SUITE_DECLARATION(statictypedallocator, &sTests[0]);
    nlTestRunner(&theSuite, NULL);
    return nlTestRunnerStats(&theSuite);
}
