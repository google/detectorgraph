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

#include <dglogging.hpp>

#include <cstdio>
#include <cstdarg>

#define LINE_BUFFER_SIZE 512

void DG_LOG(const char* aLogString, ...)
{
    char buff[LINE_BUFFER_SIZE];
    va_list args;
    va_start(args, aLogString);
    // vsnprintf only exists in c++11; without "n" this is a security problem
    // but I don't care since this is just test code.
    vsprintf(buff, aLogString, args);
    va_end(args);

    printf("DetectorGraph: %s\n", buff);
}
