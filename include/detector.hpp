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

#ifndef DETECTORGRAPH_INCLUDE_DETECTOR_HPP_
#define DETECTORGRAPH_INCLUDE_DETECTOR_HPP_

#include <list>
#include <iostream>

#include "vertex.hpp"
#include "subscriberinterface.hpp"
#include "subscriptiondispatcher.hpp"
#include "topicregistry.hpp"
#include "graph.hpp"
#include "publisher.hpp"
#include "futurepublisher.hpp"
#include "timeoutpublisher.hpp"

namespace DetectorGraph
{

/**
 * @brief A unit of logic in a DetectorGraph
 *
 * Detectors are compartmentalized algorithm with clear inputs & outputs.
 * It has fixed input types (Subscriptions) and fixed output types
 * (Publishing).
 *
 * A new detector is implemented by a new class that:
 * - Sub-classes Detector
 * - Sub-classes [SubscriberInterface<TopicStateX>](@ref SubscriberInterface)
 * for each TopicState it subscribes to.
 * - Sub-classes [Publisher<TopicStateY>](@ref Publisher) for each TopicState
 * it publishes.
 * - Calls to [Subscribe<TopicStateX>(this)](@ref Detector::Subscribe) in
 * the constructor for each TopicState it subscribes to.
 * - Calls to [SetupPublishing<TopicStateY>(this)](@ref Detector::SetupPublishing)
 * (or variants) in constructor for each TopicState it publishes to (see
 * Detector::Detector for more info).
 *
 * Detectors should be designed to be modular & finely grained. Note that
 * there's a trade-off between granularity & practicality/overhead and
 * sometimes it's easier to find the sweet spot by designing TopicStates
 * that provide intuitive intermediary state representations and then design
 * the detectors afterwards.
 *
 * For example, a trivial 'temperature threshold' detector could be:
 * @snippet helloworld.cpp Detector
 * For a complete example, go to [Hello World](@ref helloworld.cpp)
 *
 * More complex Detectors will subscribe & publish to multiple different
 * topics.
 *
 * Detectors can also implement BeginEvaluation() and CompleteEvaluation()
 * methods if performing a summary across multiple Evaluate() calls - with
 * calls to Publish() from within CompleteEvaluation().
 *
 */
class Detector : public Vertex
{
    Graph* mGraph;
public:
    /**
     * @brief Constructor
     *
     * A Detector is always created within a @ref Graph.
     * The detector is "attached" to the graph during construction.
     * Also, during construction a subclass must make the necessary
     * calls to:
     * - [Subscribe(this)](@ref Detector::Subscribe)
     * - [SetupPublishing(this)](@ref Detector::SetupPublishing)
     * - [SetupFuturePublishing(this)](@ref Detector::SetupFuturePublishing)
     * - [SetupTimeoutPublishing(this, TimeoutPublisherService*)](@ref Detector::SetupTimeoutPublishing)
     * - [SetupPeriodicPublishing(aPeriodInMilliseconds, TimeoutPublisherService*)](@ref Detector::SetupPeriodicPublishing)
     */
    Detector(Graph* graph);

    /**
     * @brief Destructor
     *
     * Destruction of a Detector removes it from the graph it's contained.
     * It also removes it's own subscription dispatchers and the 'out' edges
     * pointing to this detector owned by the topics it subscribes to.
     */
    virtual ~Detector();

    /**
     * @brief Returns kDetectorVertex to identify this subclass of @ref Vertex
     */
    virtual VertexType GetVertexType() const { return Vertex::kDetectorVertex; }

    /**
     * @brief Consume data in the topics.
     *
     * Executes the evaluation of this detector. This entails:
     *  - Calling @ref BeginEvaluation()
     *  - Iterating through all the subscription dispatchers firing only the ones
     * with new data. Firing causes the correct SubscriptionInterface::Evaluate method
     * to be called with the new data.
     *  - Calling @ref CompleteEvaluation()
     */
    void ProcessVertex();

protected:
    /**
     * @brief Setup an subscription on a specific topic
     *
     * This method must be called at the constructor of a detector;
     * once per `SubscriberInterface<T>` interfaces it implements.
     */
    template<class TTopic> void Subscribe(SubscriberInterface<TTopic>* aSubscriber)
    {
        Topic<TTopic>* topic = mGraph->ResolveTopic<TTopic>();
        SubscriptionDispatcherInterface* dispacher = new SubscriptionDispatcher<TTopic>(topic, aSubscriber);

        // Keep track of all edges
        topic->InsertEdge(this);
        mInDispatchers.push_back(dispacher);
    }

    /**
     * @brief Setup an advertisement on a specific topic
     *
     * This method must be called at the constructor of a detector;
     * once per `Publisher<T>` implementation it contains.
     */
    template<class TTopic> void SetupPublishing(Publisher<TTopic>* aPublisher)
    {
        aPublisher->SetGraph(mGraph);
        Vertex* topic = mGraph->ResolveTopic<TTopic>();
        InsertEdge(topic);
    }

    /**
     * @brief Setup an future advertisement on a specific topic
     *
     * This method must be called at the constructor of a detector;
     * once per `FuturePublisher<T>` implementation it contains.
     */
    template<class TTopic> void SetupFuturePublishing(FuturePublisher<TTopic>* aFuturePublisher)
    {
        aFuturePublisher->SetGraph(mGraph);
        Vertex* topic = mGraph->ResolveTopic<TTopic>();
        MarkFutureEdge(topic);
    }

    /**
     * @brief Setup an timeout advertisement on a specific topic
     *
     * This method must be called at the constructor of a detector;
     * once per `TimeoutPublisher<T>` implementation it contains.
     */
    template<class TTopic> void SetupTimeoutPublishing(TimeoutPublisher<TTopic>* aTimeoutPublisher, TimeoutPublisherService* aTimeoutPublisherService)
    {
        aTimeoutPublisher->SetTimeoutService(aTimeoutPublisherService);
        Vertex* topic = mGraph->ResolveTopic<TTopic>();
        MarkFutureEdge(topic);
    }

    /**
     * @brief Setup an periodic advertisement on a specific topic
     *
     * This method must be called at the constructor of a detector;
     */
    template<class TTopic> void SetupPeriodicPublishing(const uint64_t aPeriodInMilliseconds, TimeoutPublisherService* aTimeoutPublisherService)
    {
        aTimeoutPublisherService->SchedulePeriodicPublishing<TTopic>(aPeriodInMilliseconds);
        Vertex* topic = mGraph->ResolveTopic<TTopic>();
        MarkFutureEdge(topic);
    }

    /**
     * @brief Called before any calls to SubscriberInterface::Evaluate
     *
     * See more at @ref ProcessVertex()
     */
    virtual void BeginEvaluation();

    /**
     * @brief Called after all calls to SubscriberInterface::Evaluate
     *
     * See more at @ref ProcessVertex()
     */
    virtual void CompleteEvaluation();

private:
    /**
     * @brief Contain dispatchers to manage subscription interfaces
     */
    std::list<SubscriptionDispatcherInterface*> mInDispatchers;
};

} // namespace DetectorGraph

#endif // DETECTORGRAPH_INCLUDE_DETECTOR_HPP_
