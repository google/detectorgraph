/* -*- Mode: C++; tab-width: 4 indent-tabs-mode: nil -*-
 *
 *    Copyright (c) 2016 Nest Labs, Inc.
 *    All rights reserved.
 *
 *    This document is the property of Nest. It is considered
 *    confidential and proprietary information.
 *
 *    This document may not be reproduced or transmitted in any form,
 *    in whole or in part, without the express written permission of
 *    Nest.
 *
 *    Description:
 *      TopicState that contains an entire StateSnapshot passed to
 *    all resuming detectors on startup.
 *      Note that this is a private TopicState (otherwise this
 *    object would grow indefinitely mirror vs. mirror style)
 *
 */

#ifndef RESUMEFROMSNAPSHOTTOPICSTATE_H
#define RESUMEFROMSNAPSHOTTOPICSTATE_H

#include <topicstate.hpp>
#include "statesnapshot.hpp"

namespace DetectorGraph
{

struct ResumeFromSnapshotTopicState : public DetectorGraph::TopicState
{
    DetectorGraph::StateSnapshot snapshot;
};

}

#endif // RESUMEFROMSNAPSHOTTOPICSTATE_H
