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

#ifndef DETECTORGRAPH_INCLUDE_SHARED_PTR_HPP_
#define DETECTORGRAPH_INCLUDE_SHARED_PTR_HPP_

// This is just a detection of C++11
// to fall back to TR1 if that's not the case.
//
// Thus, if the detectorgraph library is compiled using c++11,
// then so must everyone who uses the library
#if __cplusplus >= 201103L

#include <memory>
namespace ptr = std;

# else

#include <tr1/memory>
namespace ptr = std::tr1;

#endif

#endif // DETECTORGRAPH_INCLUDE_SHARED_PTR_HPP_
