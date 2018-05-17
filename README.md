# DetectorGraph: Formal C++ applications with logic/data separation, automatic dependency injection and data passing.

[![Build Status](https://travis-ci.org/google/detectorgraph.svg?branch=master)](https://travis-ci.org/google/detectorgraph)

DetectorGraph is a framework for writing programs in a formal graph topology.
This can be used to write applications with multiple interdependent algorithms, applications' data models, general business logic or all of that combined.
The framework uses a formal distinction between data (Topics) and transformations/logic (Detectors).
It natively provides dependency injection, strict type-safety and provides loose coupling between Detectors by formalizing the touch points as Topics.
It forces an intuitive (albeit unusual) programming paradigm that results in highly readable, maintainable & testable code.

This is not an officially supported Google product.

Note that the cross-reference links in this page are only rendered in the Doxygen version of the documentation (see [Building](#building)).
You can also navigate the web version of the documentation hosted at https://google.github.io/detectorgraph/.

## Getting Started

## Usage

Applications are written as a combination of *Detectors* and *Topics*.

*Topics* carry a particular signal/data-type:

    struct SensorData : public DetectorGraph::TopicState
    {
        int x;
    }

*Detectors* encode a logical unit that describes the transformation from any number of *Topics* to any number of other *Topics*:

    class ThresholdDetector : public DetectorGraph::Detector,
        public DetectorGraph::SubscriberInterface<SensorData>,
        public DetectorGraph::Publisher<ThresholdCrossing>
    {
        ThresholdDetector(DetectorGraph::Graph* graph) : DetectorGraph::Detector(graph)
        {
            Subscribe<SensorData>(this);
            SetupPublishing<ThresholdCrossing>(this);
        }
        virtual void Evaluate(const SensorData& data)
        {
            if (data.x > 100)
            {
                Publish(ThresholdCrossing(data.x));
            }
        }
    }

*DetectorGraphs* are created by adding any number of *Detectors* to a *Graph*. All necessary *Topics* are created on demand and supplied via Dependency Injection.

    DetectorGraph::Graph graph;
    ThresholdDetector detector(&graph);

*Detectors* and *Topics* are kept sorted in topological order.

Graph *Evaluations* start after data is posted to a *Topic*. This causes all Detector's [`Evaluate()`](@ref DetectorGraph::SubscriberInterface::Evaluate) methods for that *Topic* to be called with the new piece of data which in turn may result in new data being posted to subsequent *Topics*. That may then trigger the `Evaluate()` of other *Detectors*. This process continues following the topological order until the end of the *Graph* is reached or until no more *Topics* with subscribers have new data.

## User Guide

Below are a number of examples showcasing different aspects & usage patterns of the framework (these are in the `./examples/` folder).
 - Basic
    - [Hello World](@ref helloworld.cpp)
    - [Trivial Vending Machine](@ref trivialvendingmachine.cpp)

 - Feedback Loops
    - [Counter With Reset](@ref counterwithreset.cpp)
    - [Robot Localization](@ref robotlocalization.cpp)
    - [Fancy Vending Machine](@ref fancyvendingmachine.cpp)

 - Timeouts & Timers
    - [Beat Machine](@ref beatmachine.cpp)

 - Dealing with Large TopicStates
    - [Portuguese Translator](@ref portuguesetranslator.cpp)

## Porting

The library has no dependencies beyond C++ & STL (min C++0x) and was designed to be fully portable to any environment supporting those dependencies.

### Platform Implementations

The library uses abstractions for basic things like asserts & logging to allow for project & platform specific customization.
The library is shipped with a basic set of implementations for those basic functions in `./platform_standalone`.

### Timers / Time Integration

To enable time-aware functionality (e.g. [`PublishOnTimeout`](@ref DetectorGraph::TimeoutPublisher), [`GetTime`](@ref DetectorGraph::TimeoutPublisherService::GetTime), [`SetupPeriodicPublishing`](@ref DetectorGraph::Detector::SetupPeriodicPublishing)) you must provide a concrete implementation of [`TimeoutPublisherService`](@ref DetectorGraph::TimeoutPublisherService) and pass that to your *Detectors* upon construction.

### Runtime Integration

There are multiple ways of integrating [`Graph`](@ref DetectorGraph::Graph) into your application. A good place to start is sub-classing [`ProcessorContainer`](@ref DetectorGraph::ProcessorContainer), adding:
    - Your Detectors
    - Plumbing that connects your data sources (e.g. drivers, buttons, network) to calls to [`ProcessData()`](@ref DetectorGraph::ProcessorContainer::ProcessData) for specific _input Topics_
    - An implementation of [`ProcessOutput()`](@ref DetectorGraph::ProcessorContainer::ProcessOutput) that links specific _output Topics_ to your application's outputs (e.g. LEDs, actuators, network)

A more powerful & flexible option is to implement your own container to hold the [`Graph`](@ref DetectorGraph::Graph) and [`Detector`](@ref DetectorGraph::Detector) objects and orchestrate calls to [`PushData<T>()`](@ref DetectorGraph::Graph::PushData), [`EvaluateGraph()`](@ref DetectorGraph::Graph::EvaluateGraph) and either inspect particular Topics (retrievable via [`ResolveTopic<T>()`](@ref DetectorGraph::Graph::ResolveTopic)) or iterate through all modified Topics using [`GetOutputList()`](@ref DetectorGraph::Graph::GetOutputList).

 <a name="building"></a>
## Building

### Tests, Docs and Examples

The library is shipped with a bare-bones makefile that can be used to build & run all of the examples, unit tests, documentation and coverage report.
 - Dependencies:
    - Gcc and/or clang with c++11
    - gcov & lcov (optional - for building test_coverage)
    - doxygen (optional - for building docs)
    - Eigen3 (optional - for building examples/robotlocalization)

 - Building:

        # Hello World
        ~/detectorgraph$ make examples/helloworld

        # Build/Run unit tests
        ~/detectorgraph$ make unit-test/test_lite
        ~/detectorgraph$ make unit-test/test_full

        # Or simply:
        ~/detectorgraph$ make unit-test/test_all

        # Build Docs. Results in /docs
        ~/detectorgraph$ make docs

        # Build/Run Coverage test. Results in /coverage
        ~/detectorgraph$ make unit-test/test_coverage

### For your project

The library is mostly header-only (only 3 core compilation units) and has a trivial compilation process. So instead of providing a binary or a complete build system we recommend that users use their build system of choice to build the library in whatever way fits their needs better. For examples on how to do that, please check [`makefile`](@ref makefile).

## Why another Graph compute framework?

DetectorGraph shares a lot of its core concepts with other frameworks based in computation graphs (e.g. ROS, TensorFlow etc) but has also many differences.
Some of its most unique features are:
- Strongly typed subscription & publishing - guaranteed at compile time.
- Per-Subscription call site at compute nodes.
- Graph topology is dictated by combining the requirements and offerings of each compute node; no separate graph configuration/description is needed.
- Explicit declaration of Topics provide formal interface between compute nodes.
- Timers integration.
- Flat design - the graph has only two types of constructs; Topics and Detectors.
- Small & Simple - it doesn't do anything else.

## Naming

The DetectorGraph library had a little naming problem growing up. From birth it came to replace nlDetectorGraph and so it pretended to be called that way. As an adolescent it decided it wanted to be called MarkII.. but no one cared - for years now the world continues to call it simply DetectorGraph. Now as it approaches adulthood it has finally accepted its popular name: DetectorGraph.

## In-depth Docs & API Reference

For in depth documentation of the library, [start here](@ref core_introduction) - these are provided by the [auto-generated docs](https://google.github.io/detectorgraph/).
