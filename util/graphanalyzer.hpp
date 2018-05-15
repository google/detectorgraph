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

#ifndef DETECTORGRAPH_INCLUDE_DETECTORGRAPHANALYZER_HPP_
#define DETECTORGRAPH_INCLUDE_DETECTORGRAPHANALYZER_HPP_


#include "graph.hpp"

#include <string>

namespace DetectorGraph
{

using namespace std;

/**
 * @brief Class that provides debugging/diagnostics to a DetectorGraph detector graph
 */
class GraphAnalyzer
{
public:
    GraphAnalyzer(const Graph& aGraph);

    /**
     * @brief Sets a filter to produce readable names from mangle C++ ones.
     *
     * If not set, NodeNameUtils::GetMinimalName will be used.
     */
    void SetStringFilter(std::string (*aStringFilter)(const std::string&));

    /**
     * @brief Sets a filter to insert \n at specific points.
     *
     * If not set NodeNameUtils::WrapOnCommonEndings
     */
    void SetLabelWordWrapper(std::string (*aStringFilter)(const std::string&));

    /**
     * @brief Print to aOutFilePath a graphviz visualization of the graph
     */
    void GenerateDotFile(const std::string& aOutFilePath) const;

    /**
     * @brief Prints to stdout the names of all vertices in the current order
     */
    void PrintVertexes() const;

    /**
     * @brief Prints whether two detectors post to the same public Topic
     */
    bool HasPublicConflict() const;

private:
    std::string GetLegend() const;
    std::string GenerateNodeName(const char* aCompilerName) const;
    std::string GenerateNodeLabel(const std::string& aNodeName, int aEvaluationIndex) const;

private:
    const Graph& mGraph;
    std::string (*mStringFilter)(const std::string&);
    std::string (*mLabelWordWrapper)(const std::string&);
};

}

#endif // DETECTORGRAPH_INCLUDE_DETECTORGRAPHANALYZER_HPP_
