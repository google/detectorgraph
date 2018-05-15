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

#include <iostream>

#include "graph.hpp"
#include "detector.hpp"
#include "processorcontainer.hpp"
#include "graphstatestore.hpp"
#include "dglogging.hpp"
#include "graphanalyzer.hpp"

using std::cout;
using std::endl;

/**
 * @file trivialvendingmachine.cpp
 * @brief Vending Machine with Named TopicStates and GraphAnalyzer
 *
 * @section Introduction
 * This example gives a single-detector solution for a hypothetical one-item,
 * one-coin vending machine. It shows how a single detector can be used to
 * synchronize combine signals - coins being inserted and the "Buy" button being
 * clicked - to produce a third - SaleCompleted. It also introduces the concepts
 * of _Trivial TopicStates_,  _Named TopicStates_ and using GraphAnalyzer.
 *
 * @section tts Trivial TopicStates
 * A _Trivial TopicState_ has no data fields:
 @snippet trivialvendingmachine.cpp Trivial TopicState
 * These are sometimes useful to represent parameter-less signals, timeouts etc.
 *
 * @section nts Named TopicStates
 * _Named TopicStates_, as opposed to Anonymous ones, are TopicStates that
 * override the [TopicState::GetId](@ref DetectorGraph::TopicState::GetId)
 * method. This allows for code that deals with sequences of different
 * [TopicStates](@ref DetectorGraph::TopicState) in a general way. This is used
 * primarily outside (or at the boundary) of the DetectorGraph portion of the
 * application in applications that necessitate any need of generic/transparent
 * treatment of TopicStates. An example of this is
 * [GraphStateStore](@ref DetectorGraph::GraphStateStore) &
 * [StateSnapshot](@ref DetectorGraph::StateSnapshot). They are often also
 * called _Public_ TopicStates as they're what's exposed to the outside world
 * from the point of view of a DetectorGraph graph.
 *
 * TopicState "names" come from an application-specific enumeration value
 * for your named TopicStates - values must be cast-able to
 * [TopicStateIdType](@ref DetectorGraph::TopicStateIdType) and be different than
 * [TopicState::kAnonymousTopicState](@ref DetectorGraph::TopicState::kAnonymousTopicState)
 * (-1). An example would be:
 @snippet trivialvendingmachine.cpp Application-Specific Enum for Named TopicStates
 (`C++03` enums are fine too)
 *
 * Which then can be used in the overriding of TopicState::GetId for the desired
 * named TopicState:
 @snippet trivialvendingmachine.cpp Named TopicState
 *
 * That then enables things like:
 @snippet trivialvendingmachine.cpp Inspecting Graph Output with Named TopicStates
 *
 * @section dga GraphAnalyzer
 * Finally, this example also shows how to use DetectorGraph::GraphAnalyzer to
 * generate a GraphViz-compatible `.dot` representation of the graph:
 @snippet trivialvendingmachine.cpp Using GraphAnalyzer to Create dot file
 *
 * That .dot file when rendered generates the following graph:
 @dot "Trivial Vending Machine"
digraph GraphAnalyzer {
    rankdir = "LR";
    node[fontname=Helvetica];
    "BuyButtonClicked" [label="0:BuyButtonClicked",style=filled, shape=box, color=lightblue];
        "BuyButtonClicked" -> "SaleDetector";
    "CoinInserted" [label="1:CoinInserted",style=filled, shape=box, color=lightblue];
        "CoinInserted" -> "SaleDetector";
    "SaleDetector" [label="2:SaleDetector", color=blue];
        "SaleDetector" -> "SaleCompleted";
        "SaleDetector" -> "Balance";
    "Balance" [label="3:Balance",style=filled, shape=box, peripheries=2, color=limegreen];
    "SaleCompleted" [label="4:SaleCompleted",style=filled, shape=box, peripheries=2, color=limegreen];
}
 @enddot
 * Note that the _Named TopicStates_ are represented by outlined rectangles.
 *
 *
 * For a more sophisticated solution to a similar problem, see @ref fancyvendingmachine.cpp .
 */

/// @cond DO_NOT_DOCUMENT

//![Trivial TopicState]
struct CoinInserted : public DetectorGraph::TopicState
{
};
//![Trivial TopicState]

struct BuyButtonClicked : public DetectorGraph::TopicState
{
};

//![Application-Specific Enum for Named TopicStates]
enum class VendingMachineTopicStateIds{
    kSaleCompleted = 0,
    kBalance
};
//![Application-Specific Enum for Named TopicStates]

//![Named TopicState]
struct SaleCompleted : public DetectorGraph::TopicState
{
    DetectorGraph::TopicStateIdType GetId() const
    {
        return static_cast<DetectorGraph::TopicStateIdType>(
            VendingMachineTopicStateIds::kSaleCompleted);
    }
};
//![Named TopicState]

struct Balance : public DetectorGraph::TopicState
{
    Balance() : numberOfCoins(0) {}

    DetectorGraph::TopicStateIdType GetId() const
    {
        return static_cast<DetectorGraph::TopicStateIdType>(
            VendingMachineTopicStateIds::kBalance);
    }

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

//![VendingMachine ProcessorContainer]
class VendingMachine : public DetectorGraph::ProcessorContainer
{
public:
    VendingMachine()
    : mSaleDetector(&mGraph)
    , mSaleTopic(mGraph.ResolveTopic<SaleCompleted>())
    , mBalanceTopic(mGraph.ResolveTopic<Balance>())
    , mStateStore()
    {
    }

    SaleDetector mSaleDetector;
    DetectorGraph::Topic<SaleCompleted>* mSaleTopic;
    DetectorGraph::Topic<Balance>* mBalanceTopic;
    DetectorGraph::GraphStateStore mStateStore;

    //![Inspecting Graph Output with Named TopicStates]
    void ProcessOutput()
    {
        // Now we can iterate exclusively through newly updated TopicStates
        // instead of checking through every Topic of interest.
        for (const auto topicState : mGraph.GetOutputList())
        {
            switch(static_cast<VendingMachineTopicStateIds>(
                topicState->GetId()))
            {
                case VendingMachineTopicStateIds::kSaleCompleted:
                    cout << "Sale Made" << endl;
                break;

                case VendingMachineTopicStateIds::kBalance:
                {
                    const std::shared_ptr<const Balance> balance =
                        std::static_pointer_cast<const Balance>(topicState);

                    cout << "Balance: " << balance->numberOfCoins;
                    cout << " coins" << endl;
                }
                break;
            }
        }

        // Or update a GraphStateStore that can be used for persistence or for
        // a State querying API to be used from outside of the graph.
        mStateStore.TakeNewSnapshot(mGraph.GetOutputList());
    }
    //![Inspecting Graph Output with Named TopicStates]
};
//![VendingMachine ProcessorContainer]

int main()
{
    VendingMachine vendingMachine;
    vendingMachine.ProcessData(CoinInserted());
    vendingMachine.ProcessData(BuyButtonClicked());

    vendingMachine.ProcessData(BuyButtonClicked());


    vendingMachine.ProcessData(CoinInserted());
    vendingMachine.ProcessData(CoinInserted());
    vendingMachine.ProcessData(BuyButtonClicked());
    vendingMachine.ProcessData(BuyButtonClicked());

//![Using GraphAnalyzer to Create dot file]
    DetectorGraph::GraphAnalyzer analyzer(vendingMachine.mGraph);
    analyzer.GenerateDotFile("trivial_vending_machine.dot");
//![Using GraphAnalyzer to Create dot file]
}

/// @endcond DO_NOT_DOCUMENT
