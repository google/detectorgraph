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

#ifndef DETECTORGRAPH_UTIL_PROCESSORCONTAINERGENERATOR_HPP_
#define DETECTORGRAPH_UTIL_PROCESSORCONTAINERGENERATOR_HPP_


#include "graph.hpp"

#include <string>
#include <utility>
#include <vector>

namespace DetectorGraph
{

using namespace std;

/**
 * @brief Class that provides debugging/diagnostics to a DetectorGraph detector graph
 *
 * This is just an early prototype.
 *
 * TODO(DGRAPH-3):
 * - Add Lite vs. Full style options
 * - Add Generate Topics bool?
 * - Upstream Lite TopicContainer API.
 */
class ProcessorContainerGenerator
{
public:
    struct VertexMeta
    {
        std::string className;
        std::string instanceName;
        DetectorGraph::Vertex::VertexType type;
    };

    ProcessorContainerGenerator(Graph& aGraph);

    /**
     * @brief Sets a string-filter to cleanup compiler-generated names
     * (through typeid(*this).name()).
     */
    void SetStringFilter(std::string (*aStringFilter)(const std::string&));

    void SetOutputClassName(const std::string& aClassName);

    /**
     * @brief Print a C++ header for an ordered ProcessContainer
     */
    void GenerateClass(const std::string& aOutFilePath);

    std::vector<VertexMeta> GetVerticesData();

private:
    std::string GetClassName(const char* aCompilerName) const;
    std::string GetInstanceName(const std::string& aNodeName, DetectorGraph::Vertex::VertexType type) const;

private:
    Graph& mGraph;
    std::string (*mStringFilter)(const std::string&);
    std::string mProcessorContainerName;
};

}

#endif // DETECTORGRAPH_UTIL_PROCESSORCONTAINERGENERATOR_HPP_
