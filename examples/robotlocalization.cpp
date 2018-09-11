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

#include "graph.hpp"
#include "detector.hpp"
#include "graphanalyzer.hpp"
#include "processorcontainer.hpp"
#include "lag.hpp"

#include <iostream>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <string>
#include <Eigen/Dense>

using namespace std;
using namespace DetectorGraph;
using namespace Eigen;

/**
 * @file robotlocalization.cpp
 * @brief A localization system for a mobile robot with Lag-based feedback loop.
 *
 * @section Introduction
 * Back in the 2000s all [_citation needed_] robots were _differential wheeled
 * robots_ [1] and localization with sensor fusion was a popular problem.
 * This example shows a simple solution to that problem using DetectorGraph.
 * It uses an Extended Kalman Filter [2] to continuously predict and correct
 * the robot's Pose (and associated uncertainty) from two information sources:
 * the input to the wheels and a GPS-like system.
 *
 * @section fpvslag FuturePublisher vs. Lag\<T\>
 * This localization algorithm depends on a feedback-loop in that the output of
 * one graph evaluation (i.e. `LocalizationBelief`) is used as an input for the
 * next one (i.e. `Lagged<LocalizationBelief>`):
 @snippet robotlocalization.cpp KalmanPoseCorrector Feedback Loop
 * Note that in this graph the TopicState where the feedback loop closes is a
 * legitimate output in itself and it's very likely that new Detectors in the
 * graph would subscribe to it. In cases like this the use of
 * DetectorGraph::Lag is preferred as it is more extensible and it preserves
 * the normal TopicState guarantee for the TopicState in question as well as it
 * makes the Lagged version of the TopicState clearly documented and also
 * available. Lag allows even for detectors that subscribes to both the immediate
 * and the Lagged version of a TopicState - this could be useful for things like
 * differentiation etc.
 *
 * @section ssc Configuration TopicStates
 * It also shows how TopicStates can be used for static/configuration data
 * (e.g. `RobotConfig`). This allows for easy dependency tracking, visualization
 * and testing at pretty much no runtime cost.
 *
 * @section la Localization Algorithm
 * Regarding the Localization algorithm itself, it's mostly taken from [3]
 * except for the correction model where this example uses a different input
 * (i.e. `GPSPosition`) with much simpler transfer function to the Pose vector.
 *
 * @section Architecture
 * The graph below shows the relationships between the topics (rectangles) and
 * detectors (ellipses). Note that this graph can be automatically generated
 * for any instance of DetectorGraph::Graph using DetectorGraph::GraphAnalyzer.
 *  @dot "RobotBrain"
digraph GraphAnalyzer {
    rankdir = "LR";
    node[fontname=Helvetica];
    size="12,5";

    "RobotConfig" [label="0:RobotConfig",style=filled, shape=box, color=lightblue];
        "RobotConfig" -> "ExtendedKalmanPosePredictor";
    "InitialPose" [label="1:InitialPose",style=filled, shape=box, color=lightblue];
        "InitialPose" -> "ExtendedKalmanPosePredictor";
    "WheelSpeeds" [label="2:WheelSpeeds",style=filled, shape=box, color=lightblue];
        "WheelSpeeds" -> "ExtendedKalmanPosePredictor";
    "GPSPosition" [label="3:GPSPosition",style=filled, shape=box, color=lightblue];
        "GPSPosition" -> "KalmanPoseCorrector";
    "LaggedLocalizationBelief" [label="4:Lagged<LocalizationBelief>",style=filled, shape=box, color=lightblue];
        "LaggedLocalizationBelief" -> "ExtendedKalmanPosePredictor";
        "LaggedLocalizationBelief" -> "KalmanPoseCorrector";
    "KalmanPoseCorrector" [label="6:KalmanPoseCorrector", color=blue];
        "KalmanPoseCorrector" -> "LocalizationBelief";
    "ExtendedKalmanPosePredictor" [label="5:ExtendedKalmanPosePredictor", color=blue];
        "ExtendedKalmanPosePredictor" -> "LocalizationBelief";
    "LocalizationBelief" [label="7:LocalizationBelief",style=filled, shape=box, color=red];
        "LocalizationBelief" -> "LagLocalizationBelief";
    "LagLocalizationBelief" [label="8:Lag<LocalizationBelief>", color=blue];
        "LagLocalizationBelief" -> "LaggedLocalizationBelief" [style=dotted, color=red, constraint=false];
}
 *  @enddot
 *
 *
 * @section on Other Notes
 * This example also uses the Eigen library [4] for matrix/vector arithmetic
 * and linear-algebraic operations.
 *
 * Note that this entire algorithm is contained in a single file for the sake
 * of unity as an example. In real-world scenarios the suggested pattern is to
 * split the code into:
 *
 @verbatim
   detectorgraph/
        include/
            robotbrain.hpp (RobotBrain header)
        src/
            robotbrain.hpp (RobotBrain implementation)
        detectors/
            include/
                ExtendedKalmanPosePredictor.hpp
                KalmanPoseCorrector.hpp
            src/
                ExtendedKalmanPosePredictor.cpp
                KalmanPoseCorrector.cpp
        topicstates/
            include/
                RobotConfig.hpp
                InitialPose.hpp
                WheelSpeeds.hpp
                LocalizationBelief.hpp
                GPSPosition.hpp
@endverbatim
 *
 * @section References
 *  - [1] Differential Wheeled Robots - https://en.wikipedia.org/wiki/Differential_wheeled_robot
 *  - [2] Extended Kalman Filter - https://en.wikipedia.org/wiki/Extended_Kalman_filter
 *  - [3] EKF applied to Mobile Robot’s Localization (Section 2.4.4, page 17) - http://cpscotti.com/pdf/pfc_scotti.pdf
 *  - [4] Eigen - http://eigen.tuxfamily.org/dox/index.html
 */

/// @cond DO_NOT_DOCUMENT

// Contains the configuration parameters of this differential mobile Robot
// or vehicle.
struct RobotConfig : public TopicState
{
    RobotConfig() : r(), b(), e() {}
    RobotConfig(double aR, double aB, double aE)
        : r(aR), b(aB), e(aE) {}
    double r; // wheels r
    double b; // displacement between wheels 2*l (l=d(wheel,center))

    double e; // relative wheel slippage/error
};

struct InitialPose : public TopicState
{
    InitialPose() : timestampMs(0), pose(Vector3d::Zero()) {}
    InitialPose(Vector3d aPose) : timestampMs(0), pose(aPose) {}
    InitialPose(uint64_t aTimestampMs, Vector3d aPose) : timestampMs(aTimestampMs), pose(aPose) {}
    uint64_t timestampMs;
    Vector3d pose;
};

// The measured (via encoders or other odometry method) wheel speeds
struct WheelSpeeds : public TopicState
{
    WheelSpeeds() : timestampMs(), phi(Vector2d::Zero()) {}
    WheelSpeeds(uint64_t ts, Vector2d aPhi) : timestampMs(ts), phi(aPhi) {}
    WheelSpeeds(uint64_t ts, double r, double l) : timestampMs(ts), phi(r, l) {}
    uint64_t timestampMs;

    // phi_r, phi_l in radians
    Vector2d phi;
};

//! [TopicStates Composition Example]
struct KalmanState
{
    KalmanState() : pose(Vector3d::Zero()), error(Matrix3d::Zero()) {}
    KalmanState(Vector3d aPose, Matrix3d aError) : pose(aPose), error(aError) {}
    Vector3d pose;
    Matrix3d error;
};

//! [Mutually Atomic Variables]
struct LocalizationBelief : public TopicState
{
    LocalizationBelief() : timestampMs(), state() {}
    LocalizationBelief(uint64_t ts, const KalmanState& aState) : timestampMs(ts), state(aState) {}
    LocalizationBelief(uint64_t ts, const Vector3d& aPose, const Matrix3d& aError) : timestampMs(ts), state(aPose, aError) {}
    uint64_t timestampMs;
    KalmanState state;
};
//! [Mutually Atomic Variables]

struct GPSPosition : public TopicState
{
    GPSPosition() : timestampMs(), state() {}
    GPSPosition(uint64_t ts, const KalmanState& aState) : timestampMs(ts), state(aState) {}
    uint64_t timestampMs;
    KalmanState state;
};
//! [TopicStates Composition Example]

double WrapAngle(double th)
{
    // remainder requires C++11
    return std::remainder(th, 2. * M_PI);
}

class ExtendedKalmanPosePredictor : public Detector,
    public SubscriberInterface<RobotConfig>,
    public SubscriberInterface<InitialPose>,
    public SubscriberInterface< Lagged<LocalizationBelief> >,
    public SubscriberInterface<WheelSpeeds>,
    public Publisher<LocalizationBelief>
{
public:
    ExtendedKalmanPosePredictor(Graph* graph) : Detector(graph), mCurrentBelief(), mConfig()
    {
        Subscribe<RobotConfig>(this);
        Subscribe<InitialPose>(this);
        Subscribe< Lagged<LocalizationBelief> >(this);
        Subscribe<WheelSpeeds>(this);
        SetupPublishing<LocalizationBelief>(this);
    }

    virtual void Evaluate(const RobotConfig& aConfig)
    {
        mConfig = aConfig;
    }

    virtual void Evaluate(const InitialPose& aInitialPose)
    {
        mCurrentBelief.state = KalmanState(aInitialPose.pose, Matrix3d::Zero());
        mCurrentBelief.timestampMs = aInitialPose.timestampMs;
    }

    virtual void Evaluate(const Lagged<LocalizationBelief>& aLaggedBelief)
    {
        mCurrentBelief.state = aLaggedBelief.data.state;
        mCurrentBelief.timestampMs = aLaggedBelief.data.timestampMs;
    }

    virtual void Evaluate(const WheelSpeeds& aWheelSpeeds)
    {
        // [\Delta S_{r}, \Delta S_{l}] from phi
        Vector2d wheelLinearSpeed = aWheelSpeeds.phi * mConfig.r;

        // Eq. 2.36
        double d_th = (wheelLinearSpeed.x() - wheelLinearSpeed.y()) / mConfig.b;
        // Eq. 2.37
        double d_travel = (wheelLinearSpeed.x() + wheelLinearSpeed.y()) / 2.;

        double theta = mCurrentBelief.state.pose.z();

        // Used on most equations below
        double th_dth2 = theta + d_th / 2.;

        // Eq. 2.33 on [3]
        Vector3d cartesianSpeed;
        cartesianSpeed << d_travel * cos(th_dth2),
                          d_travel * sin(th_dth2),
                                             d_th;


        // Compute Time Delta
        double tDelta = (aWheelSpeeds.timestampMs - mCurrentBelief.timestampMs) * 1e-3;

        // Eq. 2.33 (i.e. 2.23) on [3]
        mCurrentBelief.state.pose = mCurrentBelief.state.pose + (cartesianSpeed * tDelta);
        mCurrentBelief.state.pose.z() = WrapAngle(mCurrentBelief.state.pose.z());

        // Eq. 2.34 on [3]
        Matrix3d G;
        G << 1, 0, -d_travel * sin(th_dth2),
             0, 1,  d_travel * cos(th_dth2),
             0, 0,                        1;

        // Eq. 2.35 on [3]
        Matrix<double, 3, 2> V;
        V << 0.5 * cos(th_dth2) - (d_travel / (2. * mConfig.b)) * sin(th_dth2), 0.5 * cos(th_dth2) + (d_travel / (2. * mConfig.b)) * sin(th_dth2),
             0.5 * sin(th_dth2) + (d_travel / (2. * mConfig.b)) * cos(th_dth2), 0.5 * sin(th_dth2) - (d_travel / (2. * mConfig.b)) * cos(th_dth2),
                                                                  1./mConfig.b,                                                     -1./mConfig.b;

        // Eq. 2.38 on [3]
        Matrix2d M;
        M << mConfig.e * abs(wheelLinearSpeed.x()),                                     0,
                                                 0, mConfig.e * abs(wheelLinearSpeed.y());

        // Eq. 2.24 on [3]
        mCurrentBelief.state.error = G * mCurrentBelief.state.error * G.transpose() + V * M * V.transpose();

        mCurrentBelief.timestampMs = aWheelSpeeds.timestampMs;
        Publish(mCurrentBelief);
    }

    LocalizationBelief mCurrentBelief;
    RobotConfig mConfig;
};

///! [KalmanPoseCorrector Feedback Loop]
class KalmanPoseCorrector : public Detector,
    public SubscriberInterface< Lagged<LocalizationBelief> >,
    public SubscriberInterface<GPSPosition>,
    public Publisher<LocalizationBelief>
{
public:
    KalmanPoseCorrector(Graph* graph) : Detector(graph), mCurrentBelief()
    {
        Subscribe< Lagged<LocalizationBelief> >(this);
        Subscribe<GPSPosition>(this);
        SetupPublishing<LocalizationBelief>(this);
    }
///! [KalmanPoseCorrector Feedback Loop]

    virtual void Evaluate(const Lagged<LocalizationBelief>& aLaggedBelief)
    {
        mCurrentBelief.state = aLaggedBelief.data.state;
        mCurrentBelief.timestampMs = aLaggedBelief.data.timestampMs;
    }

    virtual void Evaluate(const GPSPosition& aCorrectionInput)
    {
        // C = Identity(3)

        // Eq. 2.15 on [3]
        Matrix3d K = mCurrentBelief.state.error * (mCurrentBelief.state.error + aCorrectionInput.state.error).inverse();

        // Eq. 2.13 on [3]
        mCurrentBelief.state.pose = mCurrentBelief.state.pose + K * (aCorrectionInput.state.pose - mCurrentBelief.state.pose);

        // Eq. 2.14 on [3]
        mCurrentBelief.state.error = (Matrix3d::Identity() - K) * mCurrentBelief.state.error;

        Publish(mCurrentBelief);
    }

    LocalizationBelief mCurrentBelief;
};

//![RobotBrain Static]
class RobotBrain : public ProcessorContainer
{
public:
    RobotBrain()
    : mPosePredictor(&mGraph)
    , mKalmanPoseCorrector(&mGraph)
    , mBeliefFeedback(&mGraph)
    {
    }

    ExtendedKalmanPosePredictor mPosePredictor;
    KalmanPoseCorrector mKalmanPoseCorrector;
    Lag<LocalizationBelief> mBeliefFeedback;
    //![RobotBrain Static]

    virtual void ProcessOutput()
    {
        Topic<LocalizationBelief>* correctedStateTopic = mGraph.ResolveTopic<LocalizationBelief>();
        if (correctedStateTopic->HasNewValue())
        {
            const LocalizationBelief& belief = correctedStateTopic->GetNewValue();
            const auto& pose = belief.state.pose;
            cout << "Pose = [" << pose.x() << ", " << pose.y() << ", " << pose.z() << "], ";
            const auto& error = belief.state.error.diagonal();
            cout << "Error = [" << error.x() << ", " << error.y() << ", " << error.z() << "]";
            cout << endl;
        }
    }
};

int main()
{
    RobotBrain robot;

    uint64_t kSimStep = 1000;
    uint64_t dataIndex = 0;

    robot.ProcessData(RobotConfig(1, 1, 0.1));
    robot.ProcessData(InitialPose(Vector3d(10.0, 10.0, 0.0)));

    // Circular Movement
    // for (int i=0;i<100;i++) robot.ProcessData(WheelSpeeds((dataIndex++)*kSimStep, 1., 0.5));

    // Fwd, Rotate ~180, Fwd, GPS Update
    robot.ProcessData(WheelSpeeds((dataIndex++)*kSimStep, 1, 1));
    robot.ProcessData(WheelSpeeds((dataIndex++)*kSimStep, 1, 1));
    robot.ProcessData(WheelSpeeds((dataIndex++)*kSimStep, 1, 1));
    robot.ProcessData(WheelSpeeds((dataIndex++)*kSimStep, 1, 1));
    robot.ProcessData(WheelSpeeds((dataIndex++)*kSimStep, 1, 1));
    robot.ProcessData(WheelSpeeds((dataIndex++)*kSimStep, 1, -1));
    robot.ProcessData(WheelSpeeds((dataIndex++)*kSimStep, 1, -1));
    robot.ProcessData(WheelSpeeds((dataIndex++)*kSimStep, 1, 1));
    robot.ProcessData(WheelSpeeds((dataIndex++)*kSimStep, 1, 1));
    robot.ProcessData(WheelSpeeds((dataIndex++)*kSimStep, 1, 1));
    robot.ProcessData(WheelSpeeds((dataIndex++)*kSimStep, 1, 1));
    robot.ProcessData(WheelSpeeds((dataIndex++)*kSimStep, 1, 1));
    robot.ProcessData(GPSPosition((dataIndex++)*kSimStep,
        KalmanState(Vector3d(10., 10., 4),  // Position
                    Vector3d(0.1,0.1,300000).asDiagonal()  // Error
            )));

    cout << "---- Done ----" << endl;

    GraphAnalyzer analyzer(robot.mGraph);
    analyzer.GenerateDotFile("robot_localization.dot");

    return 0;
}

/// @endcond DO_NOT_DOCUMENT
