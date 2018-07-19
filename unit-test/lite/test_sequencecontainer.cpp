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

#include "test_sequencecontainer.h"

#include "sequencecontainer-lite.hpp"

#define SUITE_DECLARATION(name, test_ptr) { #name, test_ptr, setup_##name, teardown_##name }

using namespace DetectorGraph;

static int setup_sequencecontainer(void *inContext)
{
    return 0;
}

static int teardown_sequencecontainer(void *inContext)
{
    return 0;
}

struct SimpleObject
{
    SimpleObject() : v() {}
    SimpleObject(int av) : v(av) {}
    int v;
};

static void Test_CanonicalCase(nlTestSuite *inSuite, void *inContext)
{

    SequenceContainer<SimpleObject, 3> container;
    container.push_back(SimpleObject(10));
    container.push_back(SimpleObject(20));
    container.push_back(SimpleObject(30));

    NL_TEST_ASSERT(inSuite, container[0].v == 10);
    NL_TEST_ASSERT(inSuite, container[1].v == 20);
    NL_TEST_ASSERT(inSuite, container[2].v == 30);

    NL_TEST_ASSERT(inSuite, container.size() == 3);
    NL_TEST_ASSERT(inSuite, container.back().v == 30);

    SequenceContainer<SimpleObject, 3>::iterator it = container.begin();
    NL_TEST_ASSERT(inSuite, it != container.end());
    NL_TEST_ASSERT(inSuite, it++->v == 10);

    NL_TEST_ASSERT(inSuite, it != container.end());
    NL_TEST_ASSERT(inSuite, it++->v == 20);

    NL_TEST_ASSERT(inSuite, it != container.end());
    NL_TEST_ASSERT(inSuite, it++->v == 30);

    NL_TEST_ASSERT(inSuite, it == container.end());

    const SequenceContainer<SimpleObject, 3>& const_container = container;
    SequenceContainer<SimpleObject, 3>::const_iterator cit = const_container.begin();
    NL_TEST_ASSERT(inSuite, cit != const_container.end());
    NL_TEST_ASSERT(inSuite, cit++->v == 10);

    NL_TEST_ASSERT(inSuite, cit != const_container.end());
    NL_TEST_ASSERT(inSuite, cit++->v == 20);

    NL_TEST_ASSERT(inSuite, cit != const_container.end());
    NL_TEST_ASSERT(inSuite, cit++->v == 30);

    NL_TEST_ASSERT(inSuite, cit == const_container.end());
}

struct NoDefaultConstructor
{
    NoDefaultConstructor(int av) : v(av) {}
    int v;
};

template <typename T>
static void Test_ParametrizedType(nlTestSuite *inSuite, void *inContext)
{
    typedef SequenceContainer<T, 3> SequenceContainerType;
    SequenceContainerType container;
    container.push_back(T(10));
    container.push_back(T(20));
    container.push_back(T(30));

    NL_TEST_ASSERT(inSuite, container[0].v == 10);
    NL_TEST_ASSERT(inSuite, container[1].v == 20);
    NL_TEST_ASSERT(inSuite, container[2].v == 30);

    NL_TEST_ASSERT(inSuite, container.size() == 3);
    NL_TEST_ASSERT(inSuite, container.back().v == 30);

    typename SequenceContainerType::iterator it = container.begin();
    NL_TEST_ASSERT(inSuite, it != container.end());
    NL_TEST_ASSERT(inSuite, it++->v == 10);

    NL_TEST_ASSERT(inSuite, it != container.end());
    NL_TEST_ASSERT(inSuite, it++->v == 20);

    NL_TEST_ASSERT(inSuite, it != container.end());
    NL_TEST_ASSERT(inSuite, it++->v == 30);

    NL_TEST_ASSERT(inSuite, it == container.end());

    const SequenceContainerType& const_container = container;
    typename SequenceContainerType::const_iterator cit = const_container.begin();
    NL_TEST_ASSERT(inSuite, cit != const_container.end());
    NL_TEST_ASSERT(inSuite, cit++->v == 10);

    NL_TEST_ASSERT(inSuite, cit != const_container.end());
    NL_TEST_ASSERT(inSuite, cit++->v == 20);

    NL_TEST_ASSERT(inSuite, cit != const_container.end());
    NL_TEST_ASSERT(inSuite, cit++->v == 30);

    NL_TEST_ASSERT(inSuite, cit == const_container.end());
}

// TODO(cscotti): confirm destructors are called

static const nlTest sTests[] = {
    NL_TEST_DEF("Test_CanonicalCase", Test_CanonicalCase),
    NL_TEST_DEF("Test_ParametrizedType<SimpleObject>", Test_ParametrizedType<SimpleObject>),
    NL_TEST_DEF("Test_ParametrizedType<NoDefaultConstructor>", Test_ParametrizedType<NoDefaultConstructor>),
    NL_TEST_SENTINEL()
};

//This function creates the Suite (i.e: the name of your test and points to the array of test functions)
extern "C"
int sequencecontainer_testsuite(void)
{
    nlTestSuite theSuite = SUITE_DECLARATION(sequencecontainer, &sTests[0]);
    nlTestRunner(&theSuite, NULL);
    return nlTestRunnerStats(&theSuite);
}
