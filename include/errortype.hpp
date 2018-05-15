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

#ifndef DETECTORGRAPH_INCLUDE_ERROR_HPP_
#define DETECTORGRAPH_INCLUDE_ERROR_HPP_

namespace DetectorGraph
{

enum ErrorType
{
    /* ok */

    ErrorType_Success                     = 0,

    /* general */

    ErrorType_Failure             = -1,
    ErrorType_NoMemory            = -2,
    ErrorType_Init                = -3,
    ErrorType_Lookup              = -4,
    ErrorType_BadInput            = -5,
    ErrorType_OutOfBounds         = -6,
    ErrorType_NoResource          = -7,
    ErrorType_Parse               = -8,
    ErrorType_BadConfiguration    = -9,
    ErrorType_Unused              = -10
};

}

#endif // DETECTORGRAPH_INCLUDE_ERROR_HPP_
