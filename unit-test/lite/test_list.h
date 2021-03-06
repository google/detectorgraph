/*
 * Copyright 2017 Nest Labs, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __TEST_LIST_H__
#define __TEST_LIST_H__

#include "common_test_list.h"

/* (1) INCLUDE YOUR TEST HERE */
#include "test_lite.h"
#include "test_sequencecontainer.h"
#include "test_statictypedallocator.h"

typedef int (*test_fp)(void);

/* (2) ADD THE FUNCTION TO CALL INTO YOUR TEST HERE */
#define UNIT_TEST_LIST {\
    COMMON_TEST_LIST \
    lite_testsuite, \
    sequencecontainer_testsuite, \
    statictypedallocator_testsuite, \
}

#endif
