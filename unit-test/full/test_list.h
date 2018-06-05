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
#include "test_detector.h"
#include "test_graph.h"
#include "test_graphanalyzer.h"
#include "test_graphstatestore.h"
#include "test_graphtestutils.h"
#include "test_nodenameutils.h"
#include "test_testsplitterdetector.h"
#include "test_testtimeoutpublisherservice.h"
#include "test_topicstate.h"

typedef int (*test_fp)(void);

/* (2) ADD THE FUNCTION TO CALL INTO YOUR TEST HERE */
#define UNIT_TEST_LIST {\
    COMMON_TEST_LIST \
    detector_testsuite, \
    graph_testsuite, \
    graphanalyzer_testsuite, \
    graphstatestore_testsuite, \
    graphtestutils_testsuite, \
    nodenameutils_testsuite, \
    testsplitterdetector_testsuite, \
    testtimeoutpublisherservice_testsuite, \
    topicstate_testsuite, \
}

#endif
