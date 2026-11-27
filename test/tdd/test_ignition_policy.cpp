/**
 * @file    test_ignition_policy.cpp
 * @brief   ignition_policy의 TDD 테스트 (단위 테스트와 별개). SWR-F-001, F-002
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
#include "ignition_policy.h"
}

namespace
{

cl_validated_t IgnitionInput(cl_validity_t validity, int32_t value)
{
    cl_validated_t in = {};
    in.validity[CL_IN_IGNITION] = validity;
    in.value[CL_IN_IGNITION] = value;
    return in;
}

TEST(IgnitionPolicyTest, NullArgumentsAreRejected)
{
    cl_validated_t in = IgnitionInput(CL_VALID, 0);
    ignition_proposal_t out = {};
    EXPECT_EQ(CL_ERR_NULL_ARG, ignition_policy_propose(nullptr, &out));
    EXPECT_EQ(CL_ERR_NULL_ARG, ignition_policy_propose(&in, nullptr));
}

TEST(IgnitionPolicyTest, ValidIgnitionOffProposesReleaseAll)
{
    cl_validated_t in = IgnitionInput(CL_VALID, 0);
    ignition_proposal_t out = {};
    ASSERT_EQ(CL_OK, ignition_policy_propose(&in, &out));
    EXPECT_EQ(1, out.release_all);
}

TEST(IgnitionPolicyTest, IgnitionOnProposesNothing)
{
    cl_validated_t in = IgnitionInput(CL_VALID, 1);
    ignition_proposal_t out = {1};
    ASSERT_EQ(CL_OK, ignition_policy_propose(&in, &out));
    EXPECT_EQ(0, out.release_all);
}

TEST(IgnitionPolicyTest, InvalidOrStaleIgnitionProposesNothing)
{
    ignition_proposal_t out = {1};
    cl_validated_t invalid = IgnitionInput(CL_INVALID, 0);
    ASSERT_EQ(CL_OK, ignition_policy_propose(&invalid, &out));
    EXPECT_EQ(0, out.release_all);
    out.release_all = 1;
    cl_validated_t stale = IgnitionInput(CL_STALE, 0);
    ASSERT_EQ(CL_OK, ignition_policy_propose(&stale, &out));
    EXPECT_EQ(0, out.release_all);
}

} // namespace
