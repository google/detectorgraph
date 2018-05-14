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

#ifndef DETECTORGRAPH_INCLUDE_GRAPHINPUTQUEUE_HPP_
#define DETECTORGRAPH_INCLUDE_GRAPHINPUTQUEUE_HPP_

// #define BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE
#define BUILD_FEATURE_DETECTORGRAPH_CONFIG_VANILLLA


#if defined(BUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE)

#include "graphinputqueue-lite.hpp"

#else

#include "graphinputqueue-stl.hpp"

#endif

#endif // DETECTORGRAPH_INCLUDE_GRAPHINPUTQUEUE_HPP_
