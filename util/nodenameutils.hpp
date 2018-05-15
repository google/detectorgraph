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

#ifndef DETECTORGRAPH_PLATFORM_STANDALONE_NODENAMEUTILS_HPP_
#define DETECTORGRAPH_PLATFORM_STANDALONE_NODENAMEUTILS_HPP_

#include <string>

namespace DetectorGraph
{

namespace NodeNameUtils
{

/**
 * @brief Provides utilities for manipulating DetectorGraph node names.
 *
 * DetectorGraph Vertices (Detectors & Topics) and TopicStates have a GetName()
 * API call that returns a compiler-specific string identifying that type.
 *
 * The utilities in this file were created to decode those names for either
 * visualization or code-generation purposes.
 *
 * "You will never find a more wretched hive of scum and villainy. We must be
 * cautious." - this code should not be used for anything serious.
 */

/**
 * @brief Returns the Demangled name as a single-line string.
 *
 * This method uses cxxabi.h's __cxa_demangle to return the original names for
 * objects. This works in the same way as c++filt.
 */
std::string GetDemangledName(const std::string& aMangledString);

/**
 * @brief Returns a readable name with redundant prefixes/suffixes removed.
 *
 * For visualization purposes it may be useful to strip out common prefixes/
 * suffixes on types  (e.g. Detectors::Topics<>, TopicState).
 */
std::string GetMinimalName(const std::string& aMangledString);


/**
 * @brief Adds a \\n to the input name before common suffixes.
 */
std::string WrapOnCommonEndings(const std::string& aNodeName);

/**
 * @brief Removes all elimStrings from aInStr and returns new string.
 */
std::string RemoveSubstrings(const std::string& aInStr, const char* elimStrings[]);

/**
 * @brief Adds a \\n to aInStr before each sub-string in wrapStrings.
 *
 * Note that this will not add leading \\n even if the name starts with a string
 * in wrapStrings.
 */
std::string WrapOnSubStrings(const std::string& aInStr, const char* wrapStrings[]);

}

}

#endif // DETECTORGRAPH_PLATFORM_STANDALONE_NODENAMEUTILS_HPP_
