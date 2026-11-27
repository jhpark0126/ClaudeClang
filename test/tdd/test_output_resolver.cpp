/**
 * @file    test_output_resolver.cpp
 * @brief   output_resolver의 TDD 테스트 (단위 테스트와 별개).
 *          SSR-008, 011, 012, 015, 025~027, SWR-F-001~004
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
#include "output_resolver.h"
}

namespace
{

safety_decision_t Decision(cl_directive_t left, cl_directive_t right, cl_health_t health,
                           uint32_t reason = 0U)
{
    safety_decision_t d = {};
    d.directive[CL_DOOR_LEFT] = left;
    d.directive[CL_DOOR_RIGHT] = right;
    d.health = health;
    d.reason_mask = reason;
    return d;
}

class OutputResolverTest : public ::testing::Test
{
  protected:
    cl_status_t Resolve(const safety_decision_t &d)
    {
        return output_resolver_resolve(&d, &proposal_, &in_, previous_, &out_);
    }
    ignition_proposal_t proposal_ = {0};
    cl_validated_t in_ = {};
    cl_lock_t previous_[CL_DOOR_COUNT] = {CL_LOCK, CL_LOCK};
    cl_result_t out_ = {};
};

TEST_F(OutputResolverTest, NullArgumentsAreRejected)
{
    safety_decision_t d = Decision(CL_DIR_FREE, CL_DIR_FREE, CL_HEALTH_NORMAL);
    EXPECT_EQ(CL_ERR_NULL_ARG,
              output_resolver_resolve(nullptr, &proposal_, &in_, previous_, &out_));
    EXPECT_EQ(CL_ERR_NULL_ARG, output_resolver_resolve(&d, nullptr, &in_, previous_, &out_));
    EXPECT_EQ(CL_ERR_NULL_ARG, output_resolver_resolve(&d, &proposal_, nullptr, previous_, &out_));
    EXPECT_EQ(CL_ERR_NULL_ARG, output_resolver_resolve(&d, &proposal_, &in_, nullptr, &out_));
    EXPECT_EQ(CL_ERR_NULL_ARG, output_resolver_resolve(&d, &proposal_, &in_, previous_, nullptr));
}

TEST_F(OutputResolverTest, FreeWithoutProposalKeepsPrevious)
{
    ASSERT_EQ(CL_OK, Resolve(Decision(CL_DIR_FREE, CL_DIR_FREE, CL_HEALTH_NORMAL)));
    EXPECT_EQ(CL_LOCK, out_.lock[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_LOCK, out_.lock[CL_DOOR_RIGHT]);
    EXPECT_EQ(CL_STATE_NORMAL, out_.state);
    EXPECT_EQ(0U, out_.reason_mask);
}

TEST_F(OutputResolverTest, ReleaseAndLockDirectivesOverridePrevious)
{
    previous_[CL_DOOR_LEFT] = CL_LOCK;
    previous_[CL_DOOR_RIGHT] = CL_RELEASE;
    ASSERT_EQ(CL_OK, Resolve(Decision(CL_DIR_RELEASE, CL_DIR_LOCK, CL_HEALTH_NORMAL)));
    EXPECT_EQ(CL_RELEASE, out_.lock[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_LOCK, out_.lock[CL_DOOR_RIGHT]);
}

TEST_F(OutputResolverTest, HoldKeepsPrevious)
{
    previous_[CL_DOOR_LEFT] = CL_RELEASE;
    ASSERT_EQ(CL_OK, Resolve(Decision(CL_DIR_HOLD, CL_DIR_HOLD, CL_HEALTH_NORMAL)));
    EXPECT_EQ(CL_RELEASE, out_.lock[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_LOCK, out_.lock[CL_DOOR_RIGHT]);
}

TEST_F(OutputResolverTest, IgnitionOffReleasesFreeDoorsAndSetsOffStateAndReason)
{
    proposal_.release_all = 1;
    ASSERT_EQ(CL_OK, Resolve(Decision(CL_DIR_FREE, CL_DIR_FREE, CL_HEALTH_NORMAL)));
    EXPECT_EQ(CL_RELEASE, out_.lock[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_RELEASE, out_.lock[CL_DOOR_RIGHT]);
    EXPECT_EQ(CL_STATE_OFF, out_.state);
    EXPECT_EQ(CL_REASON_IGNITION_OFF, out_.reason_mask);
}

TEST_F(OutputResolverTest, IgnitionOffReasonIsRecordedEvenWhenAlreadyReleased)
{
    proposal_.release_all = 1;
    previous_[CL_DOOR_LEFT] = CL_RELEASE;
    previous_[CL_DOOR_RIGHT] = CL_RELEASE;
    ASSERT_EQ(CL_OK, Resolve(Decision(CL_DIR_FREE, CL_DIR_FREE, CL_HEALTH_NORMAL)));
    EXPECT_EQ(CL_REASON_IGNITION_OFF, out_.reason_mask);
}

TEST_F(OutputResolverTest, ApproachLockBeatsIgnitionOff)
{
    proposal_.release_all = 1;
    previous_[CL_DOOR_LEFT] = CL_RELEASE;
    ASSERT_EQ(CL_OK, Resolve(Decision(CL_DIR_LOCK, CL_DIR_FREE, CL_HEALTH_NORMAL)));
    EXPECT_EQ(CL_LOCK, out_.lock[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_RELEASE, out_.lock[CL_DOOR_RIGHT]);
}

TEST_F(OutputResolverTest, IgnitionOffIsNotAppliedWhileDegradedOrFault)
{
    proposal_.release_all = 1;
    ASSERT_EQ(CL_OK, Resolve(Decision(CL_DIR_FREE, CL_DIR_FREE, CL_HEALTH_DEGRADED)));
    EXPECT_EQ(CL_LOCK, out_.lock[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_STATE_DEGRADED, out_.state);
    EXPECT_EQ(0U, out_.reason_mask & CL_REASON_IGNITION_OFF);
    ASSERT_EQ(CL_OK, Resolve(Decision(CL_DIR_FREE, CL_DIR_FREE, CL_HEALTH_FAULT)));
    EXPECT_EQ(CL_LOCK, out_.lock[CL_DOOR_RIGHT]);
    EXPECT_EQ(CL_STATE_FAULT, out_.state);
}

TEST_F(OutputResolverTest, FaultStateAddsSensorFaultReason)
{
    ASSERT_EQ(CL_OK, Resolve(Decision(CL_DIR_HOLD, CL_DIR_HOLD, CL_HEALTH_FAULT)));
    EXPECT_NE(0U, out_.reason_mask & CL_REASON_SENSOR_FAULT);
}

TEST_F(OutputResolverTest, DegradedHasNoSensorFaultReason)
{
    ASSERT_EQ(CL_OK, Resolve(Decision(CL_DIR_HOLD, CL_DIR_HOLD, CL_HEALTH_DEGRADED)));
    EXPECT_EQ(0U, out_.reason_mask & CL_REASON_SENSOR_FAULT);
}

TEST_F(OutputResolverTest, StatePriorityFaultOverDegradedOverOffOverNormal)
{
    proposal_.release_all = 1;
    ASSERT_EQ(CL_OK, Resolve(Decision(CL_DIR_FREE, CL_DIR_FREE, CL_HEALTH_NORMAL)));
    EXPECT_EQ(CL_STATE_OFF, out_.state);
    ASSERT_EQ(CL_OK, Resolve(Decision(CL_DIR_HOLD, CL_DIR_HOLD, CL_HEALTH_DEGRADED)));
    EXPECT_EQ(CL_STATE_DEGRADED, out_.state);
    ASSERT_EQ(CL_OK, Resolve(Decision(CL_DIR_HOLD, CL_DIR_HOLD, CL_HEALTH_FAULT)));
    EXPECT_EQ(CL_STATE_FAULT, out_.state);
}

TEST_F(OutputResolverTest, ArbiterReasonsArePassedThrough)
{
    ASSERT_EQ(CL_OK, Resolve(Decision(CL_DIR_LOCK, CL_DIR_FREE, CL_HEALTH_NORMAL,
                                      CL_REASON_APPROACH_RISK_LOCK)));
    EXPECT_NE(0U, out_.reason_mask & CL_REASON_APPROACH_RISK_LOCK);
}

TEST_F(OutputResolverTest, ValidityIsCopiedForQuery)
{
    in_.validity[CL_IN_CRASH] = CL_STALE;
    in_.validity[CL_IN_IGNITION] = CL_INVALID;
    ASSERT_EQ(CL_OK, Resolve(Decision(CL_DIR_HOLD, CL_DIR_HOLD, CL_HEALTH_DEGRADED)));
    EXPECT_EQ(CL_STALE, out_.validity[CL_IN_CRASH]);
    EXPECT_EQ(CL_INVALID, out_.validity[CL_IN_IGNITION]);
}

TEST_F(OutputResolverTest, UnlockRequestOnLockedDoorIsSuppressedWithReason)
{
    in_.request_valid = 1;
    in_.request_side = CL_SIDE_LEFT;
    in_.request_action = CL_ACTION_UNLOCK;
    ASSERT_EQ(CL_OK, Resolve(Decision(CL_DIR_LOCK, CL_DIR_FREE, CL_HEALTH_NORMAL)));
    EXPECT_EQ(CL_LOCK, out_.lock[CL_DOOR_LEFT]);
    EXPECT_NE(0U, out_.reason_mask & CL_REASON_APPROACH_RISK_SUPPRESS);
}

TEST_F(OutputResolverTest, UnlockRequestForOtherDoorIsNotSuppressed)
{
    in_.request_valid = 1;
    in_.request_side = CL_SIDE_RIGHT;
    in_.request_action = CL_ACTION_UNLOCK;
    ASSERT_EQ(CL_OK, Resolve(Decision(CL_DIR_LOCK, CL_DIR_FREE, CL_HEALTH_NORMAL)));
    EXPECT_EQ(0U, out_.reason_mask & CL_REASON_APPROACH_RISK_SUPPRESS);
}

TEST_F(OutputResolverTest, UnlockAllIsJudgedPerDoor)
{
    in_.request_valid = 1;
    in_.request_side = CL_SIDE_ALL;
    in_.request_action = CL_ACTION_UNLOCK;
    ASSERT_EQ(CL_OK, Resolve(Decision(CL_DIR_FREE, CL_DIR_LOCK, CL_HEALTH_NORMAL)));
    EXPECT_NE(0U, out_.reason_mask & CL_REASON_APPROACH_RISK_SUPPRESS);
    EXPECT_EQ(CL_LOCK, out_.lock[CL_DOOR_RIGHT]);
}

TEST_F(OutputResolverTest, LockRequestOrInvalidRequestIsNotSuppressed)
{
    in_.request_valid = 1;
    in_.request_side = CL_SIDE_ALL;
    in_.request_action = CL_ACTION_LOCK;
    ASSERT_EQ(CL_OK, Resolve(Decision(CL_DIR_LOCK, CL_DIR_LOCK, CL_HEALTH_NORMAL)));
    EXPECT_EQ(0U, out_.reason_mask & CL_REASON_APPROACH_RISK_SUPPRESS);
    in_.request_valid = 0;
    in_.request_action = CL_ACTION_UNLOCK;
    ASSERT_EQ(CL_OK, Resolve(Decision(CL_DIR_LOCK, CL_DIR_LOCK, CL_HEALTH_NORMAL)));
    EXPECT_EQ(0U, out_.reason_mask & CL_REASON_APPROACH_RISK_SUPPRESS);
}

TEST_F(OutputResolverTest, UnlockOnNonLockDirectiveIsNotSuppressed)
{
    in_.request_valid = 1;
    in_.request_side = CL_SIDE_ALL;
    in_.request_action = CL_ACTION_UNLOCK;
    ASSERT_EQ(CL_OK, Resolve(Decision(CL_DIR_FREE, CL_DIR_HOLD, CL_HEALTH_NORMAL)));
    EXPECT_EQ(0U, out_.reason_mask & CL_REASON_APPROACH_RISK_SUPPRESS);
}

} // namespace
