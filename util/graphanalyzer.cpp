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

#include "graphanalyzer.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <list>
#include <typeinfo>

#include "nodenameutils.hpp"
#include "dglogging.hpp"

namespace DetectorGraph
{

GraphAnalyzer::GraphAnalyzer(const Graph& aGraph)
:   mGraph(aGraph)
,   mStringFilter(NodeNameUtils::GetMinimalName)
,   mLabelWordWrapper(NodeNameUtils::WrapOnCommonEndings)
{
}

void GraphAnalyzer::SetStringFilter(std::string (*aStringFilter)(const std::string&))
{
    mStringFilter = aStringFilter;
}

void GraphAnalyzer::SetLabelWordWrapper(std::string (*aStringFilter)(const std::string&))
{
    mLabelWordWrapper = aStringFilter;
}

void GraphAnalyzer::GenerateDotFile(const std::string& aOutFilePath) const
{
    ofstream dotFile;
    dotFile.open(aOutFilePath.c_str());

    unsigned int evaluationIndex = 0;
    if (dotFile.is_open())
    {
        dotFile << "digraph GraphAnalyzer {" << endl;
        dotFile << "\trankdir = \"LR\";" << endl;
        dotFile << "\tnode[fontname=Helvetica];" << endl;
        dotFile << "\t//ratio=fill; size=\"17,11\";" << endl;
        dotFile << GetLegend() << endl;
        for (std::list< Vertex* >::const_iterator it = mGraph.GetVertices().begin();
            it != mGraph.GetVertices().end();
            ++it)
        {
            std::string nodeName(GenerateNodeName((*it)->GetName()));
            std::string nodeLabel(GenerateNodeLabel(nodeName, evaluationIndex++));

            if ( (*it)->GetVertexType() == Vertex::kTopicVertex)
            {
                std::string exposureStyleOverride;
                BaseTopic* tTopic = static_cast<BaseTopic*>(*it);
                if (tTopic->GetId() != TopicState::kAnonymousTopicState)
                {
                    exposureStyleOverride = "peripheries=2, ";
                }
                else
                {
                    exposureStyleOverride = "";
                }

                /* If timer topic */
                if ((*it)->GetFutureInEdges().size() > 0 && (*it)->GetFutureInEdges() == (*it)->GetOutEdges())
                {
                    dotFile << "\t\"" << nodeName << "\" [label=\"" << nodeLabel << "\",style=filled, shape=box, " << exposureStyleOverride << "color=orange];" << endl;
                }
                /* if output topic */
                else if ((*it)->GetOutEdges().size() == 0)
                {
                    dotFile << "\t\"" << nodeName << "\" [label=\"" << nodeLabel << "\",style=filled, shape=box, " << exposureStyleOverride << "color=limegreen];" << endl;
                }
                /* if input topic */
                else if ((*it)->GetInEdges().size() == 0)
                {
                    dotFile << "\t\"" << nodeName << "\" [label=\"" << nodeLabel << "\",style=filled, shape=box, " << exposureStyleOverride << "color=lightblue];" << endl;
                }
                else
                {
                    dotFile << "\t\"" << nodeName << "\" [label=\"" << nodeLabel << "\",style=filled, shape=box, " << exposureStyleOverride << "color=red];" << endl;
                }
            }
            else if ( (*it)->GetVertexType() == Vertex::kDetectorVertex)
            {
                dotFile << "\t\"" << nodeName << "\" [label=\"" <<  nodeLabel << "\", color=blue];" << endl;
            }

            for (std::list<Vertex*>::const_iterator outIt = (*it)->GetOutEdges().begin();
                outIt != (*it)->GetOutEdges().end();
                ++outIt)
            {
                dotFile <<  "\t\t\"" << nodeName << "\" -> \"" << GenerateNodeName((*outIt)->GetName())  << "\";" << endl;
            }

            for (std::list<Vertex*>::const_iterator outIt = (*it)->GetFutureOutEdges().begin();
                outIt != (*it)->GetFutureOutEdges().end();
                ++outIt)
            {
                dotFile <<  "\t\t\"" << nodeName << "\" -> \"" << GenerateNodeName((*outIt)->GetName())  << "\" [style=dotted, color=red, constraint=false];" << endl;
            }
        }

        dotFile << "}" << endl;

        dotFile.close();

        DG_LOG("GraphViz DOT file created at: %s", aOutFilePath.c_str());
    }
}

void GraphAnalyzer::PrintVertexes() const
{
    DG_LOG("--- VERTICES START --- size = %d", mGraph.GetVertices().size());
    for (std::list<Vertex*>::const_iterator vertexIt = mGraph.GetVertices().begin();
        vertexIt != mGraph.GetVertices().end();
        ++vertexIt)
    {
        DG_LOG("Vertex %s", GenerateNodeName((*vertexIt)->GetName()).c_str());
    }
    DG_LOG("--- VERTICES END ---");
}

bool GraphAnalyzer::HasPublicConflict() const
{
    for (std::list< Vertex* >::const_iterator it = mGraph.GetVertices().begin();
        it != mGraph.GetVertices().end();
        ++it)
    {
        if ( (*it)->GetVertexType() == Vertex::kTopicVertex)
        {
            if ((*it)->GetInEdges().size() > 1)
            {
                BaseTopic* tTopic = static_cast<BaseTopic*>(*it);
                if (tTopic->GetId() != TopicState::kAnonymousTopicState)
                {
                    DG_LOG("Topic %s has two inputs and is public", (*it)->GetName());
                    return true;
                }
            } // LCOV_EXCL_LINE
        }
    }
    return false;
}

std::string GraphAnalyzer::GenerateNodeName(const char* aCompilerName) const
{
    std::string retString = std::string(aCompilerName);
    if (mStringFilter)
    {
        retString = mStringFilter(retString);
    }

    return retString;
}

std::string GraphAnalyzer::GenerateNodeLabel(const std::string& aNodeName, int aEvaluationIndex) const
{
    std::string nodeLabel(aNodeName);

    if (mLabelWordWrapper)
    {
        nodeLabel = mLabelWordWrapper(nodeLabel);
    }

    std::ostringstream nodeLabelStream;
    nodeLabelStream << aEvaluationIndex << ":" << nodeLabel;

    return nodeLabelStream.str();
}

std::string GraphAnalyzer::GetLegend() const
{
    std::string legend = "\n"
    "{\n"
      "node [shape=plaintext]\n"
      "subgraph cluster_02\n"
      "{\n"
        "label = \"Legend\";\n"
        "\"Input Topic\" [label=\"[i] Input Topic\",style=filled, shape=box, color=lightblue];\n"
        "\"Normal Topic\" [label=\"[i] Normal Topic\",style=filled, shape=box, color=red];\n"
        "\"Public Topic\" [label=\"[i] Public Topic\",style=filled, shape=box, color=grey, peripheries=2];\n"
        "\"Output Topic\" [label=\"[i] Output Topic\",style=filled, shape=box, color=limegreen];\n"
        "\"Timeout Topic\" [label=\"[i] Timeout Topic\",style=filled, shape=box, color=orange];\n"
        "\"Detector\" [label=\"[i] Detector\", shape=ellipse, color=blue];\n"
        "\"hidden1\" [label=\"\"]\n"
        "\"hidden2\" [label=\"\"]\n"
        "\"Publish() dependency\" -> \"hidden1\";\n"
        "\"FuturePublish() dependency\" -> \"hidden2\" [style=dotted, color=red];\n"
      "}\n"
    "}\n";

    return legend;
}

}
