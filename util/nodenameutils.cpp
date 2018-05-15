// Copyright 2018 Nest Labs, Inc.
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

#include "nodenameutils.hpp"

#include "dgassert.hpp"
#include <cxxabi.h>
#include <string.h>
#include <stdlib.h>

std::string DetectorGraph::NodeNameUtils::GetDemangledName(const std::string& aMangledString)
{
    int demanglingStatus;
    char* demangledChars = abi::__cxa_demangle(aMangledString.c_str(), 0, 0, &demanglingStatus);
    std::string demangledName = demangledChars;
    free(demangledChars);

    return demangledName;
}

std::string DetectorGraph::NodeNameUtils::GetMinimalName(const std::string& aMangledString)
{
    static const char* elimStrings[] = {
        "DetectorGraph::",
        "TopicState",
        "Topic",
        "<",
        ">",
        "\n",
        NULL };

    std::string demangledName = GetDemangledName(aMangledString);
    std::string minimalName = RemoveSubstrings(demangledName, elimStrings);
    return minimalName;
}

std::string DetectorGraph::NodeNameUtils::WrapOnCommonEndings(const std::string& aNodeName)
{
    static const char* wrapStrings[] = {
        "State",
        "Detector",
        "Set",
        "Request",
        "Response",
        "Sample",
        "Vote",
        "Timeout",
        "Update",
        NULL };

    std::string wrappedName = WrapOnSubStrings(aNodeName, wrapStrings);

    return wrappedName;
}

std::string DetectorGraph::NodeNameUtils::RemoveSubstrings(const std::string& aInStr, const char* elimStrings[])
{
    std::string retString = aInStr;

    size_t elimStrIt = 0;
    while (elimStrings[elimStrIt] != NULL)
    {
        std::string::size_type i = retString.find(elimStrings[elimStrIt]);

        if (i != std::string::npos)
        {
            retString.erase(i, strlen(elimStrings[elimStrIt]));
            elimStrIt = 0; // restart again
        }
        else
        {
            elimStrIt++;
        }
    }

    return retString;
}

std::string DetectorGraph::NodeNameUtils::WrapOnSubStrings(const std::string& aInStr, const char* wrapStrings[])
{
    std::string retString = aInStr;

    size_t wrapStrIt = 0;
    while (wrapStrings[wrapStrIt] != NULL)
    {
        std::string::size_type wrapPos = retString.rfind(wrapStrings[wrapStrIt]);

        if (wrapPos != std::string::npos && wrapPos != 0)
        {
            retString.insert(wrapPos, "\\n");
        }

        wrapStrIt++;
    }

    return retString;
}

