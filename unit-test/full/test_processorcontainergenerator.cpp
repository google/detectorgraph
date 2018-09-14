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
#include "errortype.hpp"

#include "test_processorcontainergenerator.h"

#include "processorcontainergenerator.hpp"
#include "graph.hpp"
#include "detector.hpp"
#include "nodenameutils.hpp"

#include <typeinfo>
#include <iostream>
#include <fstream>

#define HPP_DIR               "tmptest/cpp/"

#define SUITE_DECLARATION(name, test_ptr) { #name, test_ptr, setup_##name, teardown_##name }

using namespace DetectorGraph;

static int setup_processorcontainergenerator(void *inContext)
{
    int r;

    r = system("mkdir -p " HPP_DIR);

    return r;
}

static int teardown_processorcontainergenerator(void *inContext)
{
    int r = 0;

    r = system("rm -fR " HPP_DIR "*");

    return r;
}

// TODO(DGRAPH-3): do actual stuff here.
static void Test_EmptyGraph(nlTestSuite *inSuite, void *inContext)
{
    Graph graph;
    ProcessorContainerGenerator processorContainerGenerator(graph);
    processorContainerGenerator.GenerateClass(HPP_DIR "emptygraph.hpp");
}

struct TestTopicStateA : public DetectorGraph::TopicState {};
struct TestTopicStateB : public DetectorGraph::TopicState {};
class TestDetectorA : public DetectorGraph::Detector
, public SubscriberInterface<TestTopicStateA>
, public Publisher<TestTopicStateB>
{
public:
    TestDetectorA(Graph* graph)
    : DetectorGraph::Detector(graph)
    {
        Subscribe<TestTopicStateA>(this);
        SetupPublishing<TestTopicStateB>(this);
    }

    void Evaluate(const TestTopicStateA& aTestTopicStateA)
    {

        Publisher<TestTopicStateB>::Publish(TestTopicStateB());
    }
};

// TODO(DGRAPH-3): do actual stuff here.
static void Test_GetVertexMeta(nlTestSuite *inSuite, void *inContext)
{
    Graph graph;
    TestDetectorA detectorA(&graph);

    ProcessorContainerGenerator processorContainerGenerator(graph);
    processorContainerGenerator.SetStringFilter(&NodeNameUtils::GetDemangledName);
    std::vector<ProcessorContainerGenerator::VertexMeta> vertices = processorContainerGenerator.GetVerticesData();

    for (unsigned i = 0; i < vertices.size(); i++)
    {
        const ProcessorContainerGenerator::VertexMeta& vertexMeta = vertices[i];
        printf("Vertex %d, Type %d, Class=%s, Instance=%s\n", i, int(vertexMeta.type),
            vertexMeta.className.c_str(), vertexMeta.instanceName.c_str());
    }
}

// TODO(DGRAPH-3): do actual stuff here.
static void Test_SingleDetector(nlTestSuite *inSuite, void *inContext)
{
    Graph graph;
    TestDetectorA detectorA(&graph);

    ProcessorContainerGenerator processorContainerGenerator(graph);
    processorContainerGenerator.SetStringFilter(&NodeNameUtils::GetDemangledName);
    processorContainerGenerator.GenerateClass(HPP_DIR "singledetector.hpp");
}

static const nlTest sTests[] = {
    NL_TEST_DEF("Test_EmptyGraph", Test_EmptyGraph),
    NL_TEST_DEF("Test_GetVertexMeta", Test_GetVertexMeta),
    NL_TEST_DEF("Test_SingleDetector", Test_SingleDetector),
    NL_TEST_SENTINEL()
};

//This function creates the Suite (i.e: the name of your test and points to the array of test functions)
extern "C"
int processorcontainergenerator_testsuite(void)
{
    nlTestSuite theSuite = SUITE_DECLARATION(processorcontainergenerator, &sTests[0]);
    nlTestRunner(&theSuite, NULL);
    return nlTestRunnerStats(&theSuite);
}
