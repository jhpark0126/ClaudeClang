/**
 * @file    test_safety_arbiter.cpp
 * @brief   safety_arbiter의 TDD 테스트 (단위 테스트와 별개). SSR-001~007, 009~017, 021, 021a, 023,
 * 024
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
#include <gtest/gtest.h>

extern "C"
{
#include "safety_arbiter.h"
}

namespace
{

cl_validated_t AllValid()
{
    cl_validated_t in = {};
    for (int i = 0; i < CL_IN_COUNT; ++i)
    {
        in.validity[i] = CL_VALID;
        in.value[i] = 0;
    }
    in.value[CL_IN_IGNITION] = 1;
    return in;
}

class SafetyArbiterTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        ASSERT_EQ(CL_OK, safety_arbiter_init(&state_));
    }
    cl_status_t Decide(const cl_validated_t &in)
    {
        return safety_arbiter_decide(&state_, &in, &out_);
    }
    safety_arbiter_state_t state_ = {};
    safety_decision_t out_ = {};
};

TEST_F(SafetyArbiterTest, NullArgumentsAreRejected)
{
    cl_validated_t in = AllValid();
    EXPECT_EQ(CL_ERR_NULL_ARG, safety_arbiter_init(nullptr));
    EXPECT_EQ(CL_ERR_NULL_ARG, safety_arbiter_decide(nullptr, &in, &out_));
    EXPECT_EQ(CL_ERR_NULL_ARG, safety_arbiter_decide(&state_, nullptr, &out_));
    EXPECT_EQ(CL_ERR_NULL_ARG, safety_arbiter_decide(&state_, &in, nullptr));
}

TEST_F(SafetyArbiterTest, NormalInputsGiveFreeAndNormalHealth)
{
    ASSERT_EQ(CL_OK, Decide(AllValid()));
    EXPECT_EQ(CL_DIR_FREE, out_.directive[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_DIR_FREE, out_.directive[CL_DOOR_RIGHT]);
    EXPECT_EQ(CL_HEALTH_NORMAL, out_.health);
    EXPECT_EQ(0U, out_.reason_mask);
}

TEST_F(SafetyArbiterTest, CrashConfirmedReleasesBothDoors)
{
    cl_validated_t in = AllValid();
    in.value[CL_IN_CRASH] = CL_CRASH_CONFIRMED;
    ASSERT_EQ(CL_OK, Decide(in));
    EXPECT_EQ(CL_DIR_RELEASE, out_.directive[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_DIR_RELEASE, out_.directive[CL_DOOR_RIGHT]);
}

TEST_F(SafetyArbiterTest, CrashConfirmedBeatsApproachRisk)
{
    cl_validated_t in = AllValid();
    in.value[CL_IN_CRASH] = CL_CRASH_CONFIRMED;
    in.value[CL_IN_APPROACH_LEFT] = 1;
    in.value[CL_IN_APPROACH_RIGHT] = 1;
    ASSERT_EQ(CL_OK, Decide(in));
    EXPECT_EQ(CL_DIR_RELEASE, out_.directive[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_DIR_RELEASE, out_.directive[CL_DOOR_RIGHT]);
}

TEST_F(SafetyArbiterTest, CrashLatchHoldsAfterInputReturnsToNone)
{
    cl_validated_t in = AllValid();
    in.value[CL_IN_CRASH] = CL_CRASH_CONFIRMED;
    ASSERT_EQ(CL_OK, Decide(in));
    cl_validated_t after = AllValid();
    after.value[CL_IN_APPROACH_LEFT] = 1;
    ASSERT_EQ(CL_OK, Decide(after));
    EXPECT_EQ(CL_DIR_RELEASE, out_.directive[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_DIR_RELEASE, out_.directive[CL_DOOR_RIGHT]);
}

TEST_F(SafetyArbiterTest, InitClearsCrashLatch)
{
    cl_validated_t in = AllValid();
    in.value[CL_IN_CRASH] = CL_CRASH_CONFIRMED;
    ASSERT_EQ(CL_OK, Decide(in));
    ASSERT_EQ(CL_OK, safety_arbiter_init(&state_));
    ASSERT_EQ(CL_OK, Decide(AllValid()));
    EXPECT_EQ(CL_DIR_FREE, out_.directive[CL_DOOR_LEFT]);
}

TEST_F(SafetyArbiterTest, InvalidOrStaleCrashDoesNotLatch)
{
    cl_validated_t in = AllValid();
    in.validity[CL_IN_CRASH] = CL_STALE;
    in.value[CL_IN_CRASH] = 0;
    ASSERT_EQ(CL_OK, Decide(in));
    cl_validated_t again = AllValid();
    ASSERT_EQ(CL_OK, Decide(again));
    EXPECT_EQ(CL_DIR_FREE, out_.directive[CL_DOOR_LEFT]);
}

TEST_F(SafetyArbiterTest, CrashPendingKeepsDirectivesAndReportsReason)
{
    cl_validated_t in = AllValid();
    in.value[CL_IN_CRASH] = CL_CRASH_PENDING;
    ASSERT_EQ(CL_OK, Decide(in));
    EXPECT_EQ(CL_DIR_FREE, out_.directive[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_REASON_CRASH_PENDING, out_.reason_mask);
}

TEST_F(SafetyArbiterTest, ConfirmedAppliesWhileDegradedAndFault)
{
    cl_validated_t degraded = AllValid();
    degraded.validity[CL_IN_IGNITION] = CL_STALE;
    degraded.value[CL_IN_IGNITION] = 0;
    degraded.value[CL_IN_CRASH] = CL_CRASH_CONFIRMED;
    ASSERT_EQ(CL_OK, Decide(degraded));
    EXPECT_EQ(CL_HEALTH_DEGRADED, out_.health);
    EXPECT_EQ(CL_DIR_RELEASE, out_.directive[CL_DOOR_LEFT]);

    ASSERT_EQ(CL_OK, safety_arbiter_init(&state_));
    cl_validated_t fault = AllValid();
    fault.value[CL_IN_SENSOR_FAULT] = 1;
    fault.value[CL_IN_CRASH] = CL_CRASH_CONFIRMED;
    ASSERT_EQ(CL_OK, Decide(fault));
    EXPECT_EQ(CL_HEALTH_FAULT, out_.health);
    EXPECT_EQ(CL_DIR_RELEASE, out_.directive[CL_DOOR_RIGHT]);
}

TEST_F(SafetyArbiterTest, ApproachRiskLocksOnlyThatDoor)
{
    cl_validated_t in = AllValid();
    in.value[CL_IN_APPROACH_LEFT] = 1;
    ASSERT_EQ(CL_OK, Decide(in));
    EXPECT_EQ(CL_DIR_LOCK, out_.directive[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_DIR_FREE, out_.directive[CL_DOOR_RIGHT]);
    EXPECT_EQ(CL_REASON_APPROACH_RISK_LOCK, out_.reason_mask);
}

TEST_F(SafetyArbiterTest, RightApproachRiskLocksRightDoor)
{
    cl_validated_t in = AllValid();
    in.value[CL_IN_APPROACH_RIGHT] = 1;
    ASSERT_EQ(CL_OK, Decide(in));
    EXPECT_EQ(CL_DIR_FREE, out_.directive[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_DIR_LOCK, out_.directive[CL_DOOR_RIGHT]);
}

TEST_F(SafetyArbiterTest, ApproachRiskLockAppliesWhileDegradedAndFault)
{
    cl_validated_t in = AllValid();
    in.value[CL_IN_APPROACH_LEFT] = 1;
    in.validity[CL_IN_IGNITION] = CL_INVALID;
    in.value[CL_IN_IGNITION] = 0;
    ASSERT_EQ(CL_OK, Decide(in));
    EXPECT_EQ(CL_HEALTH_DEGRADED, out_.health);
    EXPECT_EQ(CL_DIR_LOCK, out_.directive[CL_DOOR_LEFT]);

    cl_validated_t fault = AllValid();
    fault.value[CL_IN_APPROACH_LEFT] = 1;
    fault.value[CL_IN_SENSOR_FAULT] = 1;
    ASSERT_EQ(CL_OK, Decide(fault));
    EXPECT_EQ(CL_HEALTH_FAULT, out_.health);
    EXPECT_EQ(CL_DIR_LOCK, out_.directive[CL_DOOR_LEFT]);
}

TEST_F(SafetyArbiterTest, UnreliableApproachInputHoldsThatDoorWithReason)
{
    cl_validated_t in = AllValid();
    in.validity[CL_IN_APPROACH_RIGHT] = CL_STALE;
    in.value[CL_IN_APPROACH_RIGHT] = 0;
    ASSERT_EQ(CL_OK, Decide(in));
    EXPECT_EQ(CL_DIR_HOLD, out_.directive[CL_DOOR_RIGHT]);
    EXPECT_EQ(CL_DIR_HOLD, out_.directive[CL_DOOR_LEFT]);
    EXPECT_NE(0U, out_.reason_mask & CL_REASON_APPROACH_INPUT_UNRELIABLE);
    EXPECT_EQ(CL_HEALTH_DEGRADED, out_.health);
}

TEST_F(SafetyArbiterTest, InvalidApproachInputAlsoHoldsAndReports)
{
    cl_validated_t in = AllValid();
    in.validity[CL_IN_APPROACH_LEFT] = CL_INVALID;
    in.value[CL_IN_APPROACH_LEFT] = 0;
    ASSERT_EQ(CL_OK, Decide(in));
    EXPECT_EQ(CL_DIR_HOLD, out_.directive[CL_DOOR_LEFT]);
    EXPECT_NE(0U, out_.reason_mask & CL_REASON_APPROACH_INPUT_UNRELIABLE);
}

TEST_F(SafetyArbiterTest, SensorFaultGivesFaultHealthAndHoldsFreeDoors)
{
    cl_validated_t in = AllValid();
    in.value[CL_IN_SENSOR_FAULT] = 1;
    ASSERT_EQ(CL_OK, Decide(in));
    EXPECT_EQ(CL_HEALTH_FAULT, out_.health);
    EXPECT_EQ(CL_DIR_HOLD, out_.directive[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_DIR_HOLD, out_.directive[CL_DOOR_RIGHT]);
}

TEST_F(SafetyArbiterTest, FaultWinsOverDegraded)
{
    cl_validated_t in = AllValid();
    in.value[CL_IN_SENSOR_FAULT] = 1;
    in.validity[CL_IN_IGNITION] = CL_STALE;
    in.value[CL_IN_IGNITION] = 0;
    ASSERT_EQ(CL_OK, Decide(in));
    EXPECT_EQ(CL_HEALTH_FAULT, out_.health);
}

TEST_F(SafetyArbiterTest, InvalidSensorFaultInputIsDegradedNotFault)
{
    cl_validated_t in = AllValid();
    in.validity[CL_IN_SENSOR_FAULT] = CL_INVALID;
    in.value[CL_IN_SENSOR_FAULT] = 0;
    ASSERT_EQ(CL_OK, Decide(in));
    EXPECT_EQ(CL_HEALTH_DEGRADED, out_.health);
    EXPECT_EQ(CL_DIR_HOLD, out_.directive[CL_DOOR_LEFT]);
}

TEST_F(SafetyArbiterTest, EachStaleInputMakesHealthDegraded)
{
    for (int i = 0; i < CL_IN_COUNT; ++i)
    {
        ASSERT_EQ(CL_OK, safety_arbiter_init(&state_));
        cl_validated_t in = AllValid();
        in.validity[i] = CL_STALE;
        in.value[i] = 0;
        ASSERT_EQ(CL_OK, Decide(in));
        EXPECT_EQ(CL_HEALTH_DEGRADED, out_.health) << "input " << i;
    }
}

TEST_F(SafetyArbiterTest, DoorsAreDecidedIndependently)
{
    cl_validated_t base = AllValid();
    base.value[CL_IN_APPROACH_RIGHT] = 1;
    ASSERT_EQ(CL_OK, Decide(base));
    safety_decision_t left_false = out_;
    cl_validated_t other = base;
    other.value[CL_IN_APPROACH_LEFT] = 1;
    safety_arbiter_state_t fresh = {};
    safety_decision_t left_true = {};
    ASSERT_EQ(CL_OK, safety_arbiter_decide(&fresh, &other, &left_true));
    EXPECT_EQ(left_false.directive[CL_DOOR_RIGHT], left_true.directive[CL_DOOR_RIGHT]);
}

} // namespace
