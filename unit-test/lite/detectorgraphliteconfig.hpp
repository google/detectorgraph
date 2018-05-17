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

#ifndef UNIT_TEST_DETECTORGRAPHLITECONFIG_HPP_
#define UNIT_TEST_DETECTORGRAPHLITECONFIG_HPP_

namespace DetectorGraphConfig
{

enum DetectorGraphConfigEnum
{

    // This enum contains the hard-coded config/sizing parameters for a
    // particular graph. This header must be provided by the application. In a
    // constrained environment you'd want these constants to be as small as
    // possible. If the constants are too small an initialization-time assert
    // will fail pointing you to bump the one that is too small.
    // These sizes DO NOT change for a running graph, so if the graph is
    // initialized correctly then it's good forever.
    //
    // For the purposes of this example we can also hard-code them to sensible
    // numbers:

    kMaxNumberOfVertices = 22,
    kMaxNumberOfOutEdges = 20,
    kMaxNumberOfInEdges = 20,
    kMaxNumberOfTopicStates = 2,
};

}

#endif // UNIT_TEST_DETECTORGRAPHLITECONFIG_HPP_