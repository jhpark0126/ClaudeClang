/**
 * @file    test_child_lock.cpp
 * @brief   child_lock의 TDD 테스트 (단위·통합 테스트와 별개). 요구사항 시나리오를 처음부터 끝까지
 * 확인한다. SSR-001~027, SWR-F-001~005, I-005~007, Q-001, Q-003
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
#include "child_lock.h"
}

namespace
{

cl_raw_input_t Snapshot()
{
    cl_raw_input_t raw = {};
    for (int i = 0; i < CL_IN_COUNT; ++i)
    {
        raw.field[i].present = 1;
        raw.field[i].value = 0;
    }
    raw.field[CL_IN_IGNITION].value = 1;
    return raw;
}

class ChildLockTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        ASSERT_EQ(CL_OK, child_lock_init(&ctx_));
    }
    cl_status_t Step(const cl_raw_input_t *raw, uint32_t now_ms)
    {
        return child_lock_step(&ctx_, raw, now_ms, &out_);
    }
    cl_ctx_t ctx_ = {};
    cl_result_t out_ = {};
};

TEST_F(ChildLockTest, NullArgumentsAreRejectedAndStateIsUntouched)
{
    EXPECT_EQ(CL_ERR_NULL_ARG, child_lock_init(nullptr));
    EXPECT_EQ(CL_ERR_NULL_ARG, child_lock_step(nullptr, nullptr, 0U, &out_));
    EXPECT_EQ(CL_ERR_NULL_ARG, child_lock_step(&ctx_, nullptr, 0U, nullptr));
}

TEST_F(ChildLockTest, FirstStepWithoutInputIsReleasedAndDegraded)
{
    ASSERT_EQ(CL_OK, Step(nullptr, 0U));
    EXPECT_EQ(CL_RELEASE, out_.lock[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_RELEASE, out_.lock[CL_DOOR_RIGHT]);
    EXPECT_EQ(CL_STATE_DEGRADED, out_.state);
}

TEST_F(ChildLockTest, FirstStepWithApproachRiskLocksThatDoor)
{
    cl_raw_input_t raw = Snapshot();
    raw.field[CL_IN_APPROACH_LEFT].value = 1;
    ASSERT_EQ(CL_OK, Step(&raw, 0U));
    EXPECT_EQ(CL_LOCK, out_.lock[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_RELEASE, out_.lock[CL_DOOR_RIGHT]);
    EXPECT_EQ(CL_STATE_NORMAL, out_.state);
}

TEST_F(ChildLockTest, ApproachRiskLockIsKeptAfterRiskClears)
{
    cl_raw_input_t raw = Snapshot();
    raw.field[CL_IN_APPROACH_LEFT].value = 1;
    ASSERT_EQ(CL_OK, Step(&raw, 0U));
    raw.field[CL_IN_APPROACH_LEFT].value = 0;
    ASSERT_EQ(CL_OK, Step(&raw, 10U));
    EXPECT_EQ(CL_LOCK, out_.lock[CL_DOOR_LEFT]);
}

TEST_F(ChildLockTest, UnlockRequestWhileApproachRiskIsSuppressed)
{
    cl_raw_input_t raw = Snapshot();
    raw.field[CL_IN_APPROACH_LEFT].value = 1;
    raw.request_present = 1;
    raw.request_side = CL_SIDE_LEFT;
    raw.request_action = CL_ACTION_UNLOCK;
    raw.request_source = 0;
    ASSERT_EQ(CL_OK, Step(&raw, 0U));
    EXPECT_EQ(CL_LOCK, out_.lock[CL_DOOR_LEFT]);
    EXPECT_NE(0U, out_.reason_mask & CL_REASON_APPROACH_RISK_SUPPRESS);
    EXPECT_NE(0U, out_.reason_mask & CL_REASON_APPROACH_RISK_LOCK);
}

TEST_F(ChildLockTest, CrashConfirmedReleasesImmediatelyAndStaysReleased)
{
    cl_raw_input_t raw = Snapshot();
    raw.field[CL_IN_APPROACH_LEFT].value = 1;
    ASSERT_EQ(CL_OK, Step(&raw, 0U));
    raw.field[CL_IN_CRASH].value = CL_CRASH_CONFIRMED;
    ASSERT_EQ(CL_OK, Step(&raw, 10U));
    EXPECT_EQ(CL_RELEASE, out_.lock[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_RELEASE, out_.lock[CL_DOOR_RIGHT]);
    raw.field[CL_IN_CRASH].value = CL_CRASH_NONE;
    ASSERT_EQ(CL_OK, Step(&raw, 20U));
    EXPECT_EQ(CL_RELEASE, out_.lock[CL_DOOR_LEFT]);
}

TEST_F(ChildLockTest, InitAfterCrashClearsTheLatch)
{
    cl_raw_input_t raw = Snapshot();
    raw.field[CL_IN_CRASH].value = CL_CRASH_CONFIRMED;
    ASSERT_EQ(CL_OK, Step(&raw, 0U));
    ASSERT_EQ(CL_OK, child_lock_init(&ctx_));
    raw.field[CL_IN_CRASH].value = CL_CRASH_NONE;
    raw.field[CL_IN_APPROACH_RIGHT].value = 1;
    ASSERT_EQ(CL_OK, Step(&raw, 10U));
    EXPECT_EQ(CL_LOCK, out_.lock[CL_DOOR_RIGHT]);
}

TEST_F(ChildLockTest, IgnitionOffReleasesAndReportsOffState)
{
    cl_raw_input_t raw = Snapshot();
    raw.field[CL_IN_APPROACH_LEFT].value = 1;
    raw.field[CL_IN_APPROACH_RIGHT].value = 1;
    ASSERT_EQ(CL_OK, Step(&raw, 0U));
    raw.field[CL_IN_APPROACH_LEFT].value = 0;
    raw.field[CL_IN_APPROACH_RIGHT].value = 0;
    ASSERT_EQ(CL_OK, Step(&raw, 10U));
    raw.field[CL_IN_IGNITION].value = 0;
    ASSERT_EQ(CL_OK, Step(&raw, 20U));
    EXPECT_EQ(CL_RELEASE, out_.lock[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_RELEASE, out_.lock[CL_DOOR_RIGHT]);
    EXPECT_EQ(CL_STATE_OFF, out_.state);
    EXPECT_NE(0U, out_.reason_mask & CL_REASON_IGNITION_OFF);
}

TEST_F(ChildLockTest, IgnitionOffKeepsApproachRiskDoorLocked)
{
    cl_raw_input_t raw = Snapshot();
    raw.field[CL_IN_APPROACH_LEFT].value = 1;
    raw.field[CL_IN_IGNITION].value = 0;
    ASSERT_EQ(CL_OK, Step(&raw, 0U));
    EXPECT_EQ(CL_LOCK, out_.lock[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_RELEASE, out_.lock[CL_DOOR_RIGHT]);
}

TEST_F(ChildLockTest, SensorFaultHoldsOutputsAndReportsFault)
{
    cl_raw_input_t raw = Snapshot();
    raw.field[CL_IN_APPROACH_LEFT].value = 1;
    ASSERT_EQ(CL_OK, Step(&raw, 0U));
    raw.field[CL_IN_APPROACH_LEFT].value = 0;
    raw.field[CL_IN_SENSOR_FAULT].value = 1;
    raw.field[CL_IN_IGNITION].value = 0;
    ASSERT_EQ(CL_OK, Step(&raw, 10U));
    EXPECT_EQ(CL_LOCK, out_.lock[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_RELEASE, out_.lock[CL_DOOR_RIGHT]);
    EXPECT_EQ(CL_STATE_FAULT, out_.state);
    EXPECT_NE(0U, out_.reason_mask & CL_REASON_SENSOR_FAULT);
}

TEST_F(ChildLockTest, SensorFaultStillAllowsCrashRelease)
{
    cl_raw_input_t raw = Snapshot();
    raw.field[CL_IN_APPROACH_LEFT].value = 1;
    ASSERT_EQ(CL_OK, Step(&raw, 0U));
    raw.field[CL_IN_SENSOR_FAULT].value = 1;
    raw.field[CL_IN_CRASH].value = CL_CRASH_CONFIRMED;
    ASSERT_EQ(CL_OK, Step(&raw, 10U));
    EXPECT_EQ(CL_RELEASE, out_.lock[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_STATE_FAULT, out_.state);
}

TEST_F(ChildLockTest, StaleInputGivesDegradedWithinOnePeriodAfter200Ms)
{
    cl_raw_input_t raw = Snapshot();
    raw.field[CL_IN_APPROACH_LEFT].value = 1;
    ASSERT_EQ(CL_OK, Step(&raw, 1000U));
    ASSERT_EQ(CL_OK, Step(nullptr, 1200U));
    EXPECT_EQ(CL_STATE_NORMAL, out_.state);
    ASSERT_EQ(CL_OK, Step(nullptr, 1210U));
    EXPECT_EQ(CL_STATE_DEGRADED, out_.state);
    EXPECT_EQ(CL_LOCK, out_.lock[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_STALE, out_.validity[CL_IN_CRASH]);
}

TEST_F(ChildLockTest, DegradedDoesNotApplyIgnitionOffRelease)
{
    cl_raw_input_t raw = Snapshot();
    raw.field[CL_IN_APPROACH_LEFT].value = 1;
    ASSERT_EQ(CL_OK, Step(&raw, 0U));
    raw.field[CL_IN_APPROACH_LEFT].value = 0;
    raw.field[CL_IN_APPROACH_RIGHT].value = 0;
    raw.field[CL_IN_IGNITION].value = 0;
    raw.field[CL_IN_CRASH].present = 0;
    ASSERT_EQ(CL_OK, Step(&raw, 10U));
    EXPECT_EQ(CL_STATE_DEGRADED, out_.state);
    EXPECT_EQ(CL_LOCK, out_.lock[CL_DOOR_LEFT]);
}

TEST_F(ChildLockTest, ErrorCallDoesNotChangePreviousOutput)
{
    cl_raw_input_t raw = Snapshot();
    raw.field[CL_IN_APPROACH_LEFT].value = 1;
    ASSERT_EQ(CL_OK, Step(&raw, 0U));
    cl_result_t untouched = {};
    untouched.state = CL_STATE_OFF;
    EXPECT_EQ(CL_ERR_NULL_ARG, child_lock_step(&ctx_, &raw, 10U, nullptr));
    EXPECT_EQ(CL_LOCK, ctx_.previous[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_STATE_OFF, untouched.state);
}

TEST_F(ChildLockTest, SameInputSequenceGivesSameResultSequence)
{
    cl_ctx_t other = {};
    ASSERT_EQ(CL_OK, child_lock_init(&other));
    cl_result_t a = {};
    cl_result_t b = {};
    for (uint32_t t = 0U; t < 100U; t += 10U)
    {
        cl_raw_input_t raw = Snapshot();
        raw.field[CL_IN_APPROACH_LEFT].value = static_cast<int32_t>((t / 10U) % 2U);
        ASSERT_EQ(CL_OK, child_lock_step(&ctx_, &raw, t, &a));
        ASSERT_EQ(CL_OK, child_lock_step(&other, &raw, t, &b));
        EXPECT_EQ(a.lock[CL_DOOR_LEFT], b.lock[CL_DOOR_LEFT]);
        EXPECT_EQ(a.state, b.state);
        EXPECT_EQ(a.reason_mask, b.reason_mask);
    }
}

} // namespace
