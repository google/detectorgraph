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

#include "processorcontainergenerator.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <list>
#include <typeinfo>

#include "dglogging.hpp"

namespace DetectorGraph
{

ProcessorContainerGenerator::ProcessorContainerGenerator(Graph& aGraph)
:   mGraph(aGraph)
,   mStringFilter(NULL)
,   mProcessorContainerName("GeneratedProcessorContainer")
{
}

void ProcessorContainerGenerator::SetStringFilter(std::string (*aStringFilter)(const std::string&))
{
    mStringFilter = aStringFilter;
}

void ProcessorContainerGenerator::SetOutputClassName(const std::string& aClassName)
{
    mProcessorContainerName = aClassName;
}

std::vector<ProcessorContainerGenerator::VertexMeta> ProcessorContainerGenerator::GetVerticesData()
{
    std::vector<VertexMeta> vertexMetaVector;
    DetectorGraph::ErrorType r;
    r = mGraph.TopoSortGraph();
    if (r != DetectorGraph::ErrorType_Success)
    {
        DG_LOG("Failed to get graph's Topological Sort.");
        DG_ASSERT(false);
    }

    for (std::list< Vertex* >::const_iterator vertexIt = mGraph.GetVertices().begin();
        vertexIt != mGraph.GetVertices().end();
        ++vertexIt)
    {
        const Vertex* vertex = (*vertexIt);
        VertexMeta vertexMetadata;
        vertexMetadata.className = GetClassName(vertex->GetName());
        vertexMetadata.type = vertex->GetVertexType();
        vertexMetadata.instanceName = GetInstanceName(vertexMetadata.className, vertexMetadata.type);

        vertexMetaVector.push_back(vertexMetadata);
    }

    return vertexMetaVector;
}

void ProcessorContainerGenerator::GenerateClass(const std::string& aOutFilePath)
{
    ofstream cppHeaderFile;
    cppHeaderFile.open(aOutFilePath.c_str());

    if (cppHeaderFile.is_open())
    {
        std::vector<ProcessorContainerGenerator::VertexMeta> vertices = GetVerticesData();

        cppHeaderFile << "#include \"graph.hpp\"" << endl;
        cppHeaderFile << "#include \"detector.hpp\"" << endl;
        cppHeaderFile << "#include \"processorcontainer.hpp\"" << endl;

        cppHeaderFile << endl;

        cppHeaderFile << "class " << mProcessorContainerName << " : public DetectorGraph::ProcessorContainer" << endl;
        cppHeaderFile << "{" << endl;
        cppHeaderFile << "public:" << endl;
        cppHeaderFile << "    " << mProcessorContainerName << "()" << endl;

        for (unsigned nodeIndex = 0; nodeIndex < vertices.size(); nodeIndex++)
        {
            if (nodeIndex == 0)
            {
                cppHeaderFile << "    : ";
            }
            else
            {
                cppHeaderFile << "    , ";
            }

            if (vertices[nodeIndex].type == DetectorGraph::Vertex::kDetectorVertex)
            {
                cppHeaderFile << vertices[nodeIndex].instanceName << "(&mGraph)" << endl;
            }
            else if (vertices[nodeIndex].type == DetectorGraph::Vertex::kTopicVertex)
            {
                cppHeaderFile << vertices[nodeIndex].instanceName << "(mGraph.ResolveTopic<";
                cppHeaderFile << vertices[nodeIndex].className << ">())" << endl;
            }
        }
        cppHeaderFile << "    {" << endl;
        cppHeaderFile << "    }" << endl;

        for (unsigned nodeIndex = 0; nodeIndex < vertices.size(); nodeIndex++)
        {
            if (vertices[nodeIndex].type == DetectorGraph::Vertex::kDetectorVertex)
            {
                cppHeaderFile << "    " << vertices[nodeIndex].className << " ";
                cppHeaderFile << vertices[nodeIndex].instanceName << ";" << endl;
            }
            else if (vertices[nodeIndex].type == DetectorGraph::Vertex::kTopicVertex)
            {
                cppHeaderFile << "    " << vertices[nodeIndex].className << "* ";
                cppHeaderFile << vertices[nodeIndex].instanceName << ";" << endl;
            }
        }
        cppHeaderFile << "};" << endl;

        cppHeaderFile.close();

        DG_LOG("Sorted DetectorGraph::ProcessorContainer header file created at: %s", aOutFilePath.c_str());
    }
}

std::string ProcessorContainerGenerator::GetClassName(const char* aCompilerName) const
{
    std::string retString = std::string(aCompilerName);
    if (mStringFilter)
    {
        retString = mStringFilter(retString);
    }

    return retString;
}

std::string ProcessorContainerGenerator::GetInstanceName(const std::string& aClassName, DetectorGraph::Vertex::VertexType type) const
{
    std::string instanceName = aClassName;

    std::string::size_type lastNamespaceDelimiterPos;
    lastNamespaceDelimiterPos = instanceName.rfind("::");
    if (lastNamespaceDelimiterPos != std::string::npos)
    {
        lastNamespaceDelimiterPos += 2; // "::".size()
        instanceName.erase(0, lastNamespaceDelimiterPos);
    }

    if (type == DetectorGraph::Vertex::kDetectorVertex)
    {
        return "m" + instanceName;
    }
    else if (type == DetectorGraph::Vertex::kTopicVertex)
    {
        instanceName.erase(0, 6); // "Topic<".size()
        instanceName.erase(instanceName.size()-1, 1); // ">"
        instanceName.append("Topic");
        return "mp" + instanceName;
    }
    else
    {
        return instanceName;
    }
}


}
