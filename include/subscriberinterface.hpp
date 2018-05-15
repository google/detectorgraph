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

#ifndef DETECTORGRAPH_INCLUDE_SUBSCRIBERINTERFACE_HPP_
#define DETECTORGRAPH_INCLUDE_SUBSCRIBERINTERFACE_HPP_

namespace DetectorGraph
{

/**
 * @brief A Pure interface that declares the Subscriber behavior
 *
 * Detectors inherit from a number of versions of this interface
 * to declare their input set.
 *
 * A class `FooDetector` acquires the "Subscriber of BarTopicState"
 * behavior by inheriting `SubscriberInterface` templated to `BarTopicState`.
 *
 * @code
class FooDetector :
    public Detector,
    public SubscriberInterface<BarTopicState>
{
    // ...
}
 * @endcode
 *
 * This interface serves a tiny purpose:
 * - Abstracts the interface of Subscriber to a Topic into a type-safe method.
 * - Document in a clear way the Subscriptions of a Detector.
 *
 * To enable the subscription Detectors must call Detector::Subscribe from the inheriting
 * Detector's constructor:
 * @code
class FooDetector :
    public Detector,
    public SubscriberInterface<BarTopicState>
{
    FooDetector(Graph* graph) : Detector(graph)
    {
        Subscribe<BarTopicState>(this);
    }
}
 * @endcode
 *
 */
template<class T>
class SubscriberInterface
{
public:
    /**
     * @brief Pure-virtual method that should Evaluate a piece of input data.
     */
    virtual void Evaluate(const T&) = 0;
};

} // namespace DetectorGraph

#endif // DETECTORGRAPH_INCLUDE_SUBSCRIBERINTERFACE_HPP_
