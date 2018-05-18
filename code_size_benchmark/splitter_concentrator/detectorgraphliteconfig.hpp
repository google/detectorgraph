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

#ifndef DETECTORGRAPHCONFIG_H
#define DETECTORGRAPHCONFIG_H

namespace DetectorGraphConfig
{

enum DetectorGraphConfigEnum
{

    // This enum contains the hardcoded config/sizing parameters for a particular
    // graph. This header must be provided by the application. Ideally you'd want
    // this numbers as small as possible. If the numbers are too small an init-time
    // assert will fail pointing you to bump the one that is too small.
    // This numbers DO NOT change for a running graph, so if it starts it's good
    // forever.
    //
    // One can tie this numbers to kNumberOfTopicStateIds & NumberOfDetectors
    // for ideal sizing:
    // kMaxNumberOfVertices = TopicStateId::kNumberOfTopicStateIds + NumberOfDetectors,
    //
    // For the below sizes, using kNumberOfTopicStateIds is the conservative
    // option as it is a theoretical upper bound for those sizes.
    // kMaxNumberOfOutEdges = TopicStateId::kNumberOfTopicStateIds,
    // kMaxNumberOfInEdges = TopicStateId::kNumberOfTopicStateIds,
    // kMaxNumberOfTopics = TopicStateId::kNumberOfTopicStateIds,
    //
    // For the purposes of this example we can also hardcode them to sensible
    // numbers:
    kMaxNumberOfVertices = 22,
    kMaxNumberOfOutEdges = 20,
    kMaxNumberOfInEdges = 20,
    kMaxNumberOfTopicStates = 1,
};

}

#endif // DETECTORGRAPHCONFIG_H