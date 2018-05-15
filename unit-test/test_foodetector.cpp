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

#include <typeinfo>
#include <iostream>

#include "nltest.h"
#include "errortype.hpp"

#include "graph.hpp"
#include "detector.hpp"
#include "topicstate.hpp"
#include "dglogging.hpp"

#include "test_foodetector.h"

#define SUITE_DECLARATION(name, test_ptr) { #name, test_ptr, setup_##name, teardown_##name }

/* This is not a real Detector unit test nor is it a DetectorGraph unit test.
 * This is instead a sample of what a real Detector and it's unit test might
 * look like. The below detector and structures would be defined somewhere in
 * the application code. */
// BEGIN FAKE DETECTOR
struct CoinInserted : public DetectorGraph::TopicState
{
};

struct BuyButtonClicked : public DetectorGraph::TopicState
{
};

struct SaleCompleted : public DetectorGraph::TopicState
{
};

struct Balance : public DetectorGraph::TopicState
{
    Balance() : numberOfCoins(0) {}
    int numberOfCoins;
};

class SaleDetector : public DetectorGraph::Detector
, public DetectorGraph::SubscriberInterface<CoinInserted>
, public DetectorGraph::SubscriberInterface<BuyButtonClicked>
, public DetectorGraph::Publisher<SaleCompleted>
, public DetectorGraph::Publisher<Balance>
{
public:
    SaleDetector(DetectorGraph::Graph* graph) : DetectorGraph::Detector(graph), balance()
    {
        Subscribe<CoinInserted>(this);
        Subscribe<BuyButtonClicked>(this);
        SetupPublishing<SaleCompleted>(this);
        SetupPublishing<Balance>(this);
    }

    void Evaluate(const CoinInserted&)
    {
        balance.numberOfCoins++;
    }
    void Evaluate(const BuyButtonClicked&)
    {
        if (balance.numberOfCoins > 0)
        {
            DG_LOG("Making Sale");
            balance.numberOfCoins--;
            Publisher<SaleCompleted>::Publish(SaleCompleted());
        }
        else
        {
            DG_LOG("Funds not available; no sale made.");
        }
    }
    void CompleteEvaluation()
    {
        Publisher<Balance>::Publish(balance);
    }

private:
    Balance balance;
};
// END FAKE DETECTOR

static int setup_foodetector(void *inContext)
{

    return 0;
}

static int teardown_foodetector(void *inContext)
{
    return 0;
}

static void Test_Balance(nlTestSuite *inSuite, void *inContext)
{
    DetectorGraph::Graph graph;
    SaleDetector detector(&graph);
    DetectorGraph::Topic<Balance>* balanceTopic = graph.ResolveTopic<Balance>();

    graph.PushData(CoinInserted());
    graph.EvaluateGraph();

    NL_TEST_ASSERT(inSuite, balanceTopic->HasNewValue());
    NL_TEST_ASSERT(inSuite, balanceTopic->GetNewValue().numberOfCoins == 1);
}

static void Test_SaleCompleted(nlTestSuite *inSuite, void *inContext)
{
    DetectorGraph::Graph graph;
    SaleDetector detector(&graph);
    DetectorGraph::Topic<SaleCompleted>* saleCompletedTopic = graph.ResolveTopic<SaleCompleted>();

    graph.PushData(CoinInserted());
    graph.EvaluateGraph();

    graph.PushData(BuyButtonClicked());
    graph.EvaluateGraph();

    NL_TEST_ASSERT(inSuite, saleCompletedTopic->HasNewValue());
}

static void Test_NotEnoughFunds(nlTestSuite *inSuite, void *inContext)
{
    DetectorGraph::Graph graph;
    SaleDetector detector(&graph);
    DetectorGraph::Topic<SaleCompleted>* saleCompletedTopic = graph.ResolveTopic<SaleCompleted>();

    graph.PushData(BuyButtonClicked());
    graph.EvaluateGraph();

    NL_TEST_ASSERT(inSuite, saleCompletedTopic->HasNewValue() == false);
}

static void Test_SalesDeductedFromBalance(nlTestSuite *inSuite, void *inContext)
{
    DetectorGraph::Graph graph;
    SaleDetector detector(&graph);
    DetectorGraph::Topic<Balance>* balanceTopic = graph.ResolveTopic<Balance>();

    graph.PushData(CoinInserted());
    graph.EvaluateGraph();

    graph.PushData(CoinInserted());
    graph.EvaluateGraph();

    graph.PushData(BuyButtonClicked());
    graph.EvaluateGraph();

    graph.PushData(BuyButtonClicked());
    graph.EvaluateGraph();

    NL_TEST_ASSERT(inSuite, balanceTopic->HasNewValue());
    NL_TEST_ASSERT(inSuite, balanceTopic->GetNewValue().numberOfCoins == 0);
}

static const nlTest sTests[] = {
    NL_TEST_DEF("Test_Balance", Test_Balance),
    NL_TEST_DEF("Test_SaleCompleted", Test_SaleCompleted),
    NL_TEST_DEF("Test_NotEnoughFunds", Test_NotEnoughFunds),
    NL_TEST_DEF("Test_SalesDeductedFromBalance", Test_SalesDeductedFromBalance),
    NL_TEST_SENTINEL()
};

//This function creates the Suite (i.e: the name of your test and points to the array of test functions)
extern "C"
int foodetector_testsuite(void)
{
    nlTestSuite theSuite = SUITE_DECLARATION(foodetector, &sTests[0]);
    nlTestRunner(&theSuite, NULL);
    return nlTestRunnerStats(&theSuite);
}
