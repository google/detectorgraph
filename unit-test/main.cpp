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

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "nltest.h" /*sw/nestlabs/lib/test-framework/nltest.h */
#include "test_list.h"

#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))

typedef int (*test_fp)(void);

const test_fp tests[] = UNIT_TEST_LIST;

int main(void)
{
    int i, retval;
    int suiteFailed = 0;
    int numTests = ARRAY_SIZE(tests);

    nl_test_set_output_style(OUTPUT_CSV);

    // Run tests
    for (i = 0; i < numTests; i++)
    {
        retval = tests[i]();
        if (retval != 0) // LCOV_EXCL_START
        {
           suiteFailed++;
        }
        // LCOV_EXCL_STOP
    }

    printf("%d/%d test suites failed\n", suiteFailed, numTests);

    return suiteFailed;
}
