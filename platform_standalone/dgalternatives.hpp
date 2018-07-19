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

#ifndef DETECTORGRAPH_INCLUDE_DETECTOR_GRAPH_DGALTERNATIVES_HPP_
#define DETECTORGRAPH_INCLUDE_DETECTOR_GRAPH_DGALTERNATIVES_HPP_


#if defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_NO_64BIT_REMAINDER)
template<typename T>
T mod64(T lhs, T rhs)
{
    // Taken from https://stackoverflow.com/a/2566570/435007
    T x = rhs;
    while (x <= (lhs>>1))
    {
        x <<= 1;
    }
    while (lhs >= rhs)
    {
        if (lhs >= x)
        {
            lhs -= x;
        }
        x >>= 1;
    }
    return lhs;
}

#define DG_TIMEOFFSET_REMAINDER(A,B) mod64((A),(B))
#endif

#endif // DETECTORGRAPH_INCLUDE_DETECTOR_GRAPH_DGALTERNATIVES_HPP_
