/**
 * @file    test_ignition_policy_unit.cpp
 * @brief   ignition_policy 단위 테스트 (SWE.4). 오류 추측과 MC/DC 독립 쌍으로 설계했다.
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
#include "unit_helpers.hpp"

extern "C"
{
#include "ignition_policy.h"
}

namespace
{

using unit::MakeValidated;

/**
 * @test    UT-030: NULL 인자는 CL_ERR_NULL_ARG를 반환하고 출력을 바꾸지 않는다.
 * @trace   FUNC-020
 * @purpose (in==NULL)||(out==NULL)의 MC/DC. 각 포인터만 NULL인 쌍과 모두 정상.
 * @input   in만 NULL, out만 NULL, 모두 정상
 * @steps   1. 조합별로 호출한다. 2. out의 센티널을 확인한다.
 * @expect  NULL이 있으면 CL_ERR_NULL_ARG이고 out 불변, 정상은 CL_OK
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST(IgnitionPolicyUnit, UT030_NullArguments)
{
    cl_validated_t in = MakeValidated();
    ignition_proposal_t out = {42};
    EXPECT_EQ(CL_ERR_NULL_ARG, ignition_policy_propose(nullptr, &out));
    EXPECT_EQ(CL_ERR_NULL_ARG, ignition_policy_propose(&in, nullptr));
    EXPECT_EQ(42, out.release_all);
    EXPECT_EQ(CL_OK, ignition_policy_propose(&in, &out));
}

/**
 * @test    UT-031: 점화 입력이 유효하고 OFF(0)일 때만 전체 해제를 제안한다.
 * @trace   FUNC-020, SWR-F-001, SWR-F-002
 * @purpose (validity==VALID)&&(value==0)의 MC/DC와 이전 출력 값이 덮어써지는지 확인한다.
 * @input   (VALID,0), (VALID,1), (INVALID,0), (STALE,0). 출력은 매번 반대 값으로 미리 채운다.
 * @steps   1. 입력별로 제안을 요청한다.
 * @expect  release_all은 (VALID,0)에서만 1, 나머지는 0
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST(IgnitionPolicyUnit, UT031_ProposalDecision)
{
    struct Probe
    {
        cl_validity_t validity;
        int32_t value;
        int32_t expected;
    };
    const Probe probes[] = {
        {CL_VALID, 0, 1}, {CL_VALID, 1, 0}, {CL_INVALID, 0, 0}, {CL_STALE, 0, 0}};
    for (const Probe &p : probes)
    {
        cl_validated_t in = MakeValidated();
        in.validity[CL_IN_IGNITION] = p.validity;
        in.value[CL_IN_IGNITION] = p.value;
        ignition_proposal_t out = {p.expected == 1 ? 0 : 1};
        ASSERT_EQ(CL_OK, ignition_policy_propose(&in, &out));
        EXPECT_EQ(p.expected, out.release_all) << p.validity << "," << p.value;
    }
}

} // namespace
