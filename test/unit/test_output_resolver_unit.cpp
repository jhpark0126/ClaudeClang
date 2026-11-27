/**
 * @file    test_output_resolver_unit.cpp
 * @brief   output_resolver 단위 테스트 (SWE.4). 진리표, MC/DC 독립 쌍, 오류 추측으로 설계했다.
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
#include "output_resolver.h"
}

namespace
{

using unit::MakeValidated;

safety_decision_t MakeDecision(cl_directive_t left, cl_directive_t right, cl_health_t health,
                               uint32_t reason = 0U)
{
    safety_decision_t d = {};
    d.directive[CL_DOOR_LEFT] = left;
    d.directive[CL_DOOR_RIGHT] = right;
    d.health = health;
    d.reason_mask = reason;
    return d;
}

/** 요구사항(SSR-011, 012)에서 직접 계산한 억제 이유 기대값. 제품 코드 구조를 따르지 않는다. */
bool ExpectSuppress(int32_t valid, cl_action_t action, cl_side_t side, bool left_lock,
                    bool right_lock)
{
    const bool hits_left = left_lock && (side != CL_SIDE_RIGHT);
    const bool hits_right = right_lock && (side != CL_SIDE_LEFT);
    return (valid == 1) && (action == CL_ACTION_UNLOCK) && (hits_left || hits_right);
}

class OutputResolverUnit : public ::testing::Test
{
  protected:
    cl_status_t Resolve(const safety_decision_t &d)
    {
        return output_resolver_resolve(&d, &proposal_, &in_, previous_, &out_);
    }
    ignition_proposal_t proposal_ = {0};
    cl_validated_t in_ = MakeValidated();
    cl_lock_t previous_[CL_DOOR_COUNT] = {CL_RELEASE, CL_RELEASE};
    cl_result_t out_ = {};
};

/**
 * @test    UT-040: NULL 인자는 CL_ERR_NULL_ARG를 반환하고 출력을 바꾸지 않는다.
 * @trace   FUNC-030
 * @purpose 5개 포인터 || 결정의 MC/DC. 포인터 하나씩만 NULL인 5쌍과 모두 정상.
 * @input   decision, proposal, in, previous, out 중 하나만 NULL
 * @steps   1. 각 경우에 호출한다. 2. out의 센티널을 확인한다.
 * @expect  NULL이 있으면 CL_ERR_NULL_ARG이고 out 불변, 정상은 CL_OK
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(OutputResolverUnit, UT040_NullArguments)
{
    const safety_decision_t d = MakeDecision(CL_DIR_FREE, CL_DIR_FREE, CL_HEALTH_NORMAL);
    out_.reason_mask = 0x5A5AU;
    EXPECT_EQ(CL_ERR_NULL_ARG,
              output_resolver_resolve(nullptr, &proposal_, &in_, previous_, &out_));
    EXPECT_EQ(CL_ERR_NULL_ARG, output_resolver_resolve(&d, nullptr, &in_, previous_, &out_));
    EXPECT_EQ(CL_ERR_NULL_ARG, output_resolver_resolve(&d, &proposal_, nullptr, previous_, &out_));
    EXPECT_EQ(CL_ERR_NULL_ARG, output_resolver_resolve(&d, &proposal_, &in_, nullptr, &out_));
    EXPECT_EQ(CL_ERR_NULL_ARG, output_resolver_resolve(&d, &proposal_, &in_, previous_, nullptr));
    EXPECT_EQ(0x5A5AU, out_.reason_mask);
    EXPECT_EQ(CL_OK, Resolve(d));
}

/**
 * @test    UT-041: 도어 출력 진리표: 지시 4종 × 점화 해제 제안 × 직전 출력 전체 조합.
 * @trace   FUNC-031, SSR-001, SSR-009, SWR-F-001, SWR-F-004
 * @purpose 지시 RELEASE/LOCK/FREE+제안/그 외 결정의 MC/DC. (directive==FREE)&&(release_all==1)의
 * 독립 쌍 포함.
 * @input   directive {FREE,HOLD,LOCK,RELEASE} × release_all {0,1} × previous {RELEASE,LOCK} (건전성
 * NORMAL)
 * @steps   1. 16개 조합을 좌측 도어로 결정한다. 2. 요구사항 표의 기대값과 비교한다.
 * @expect  RELEASE→RELEASE, LOCK→LOCK, FREE는 제안이 있으면 RELEASE 아니면 직전, HOLD는 제안이
 * 있어도 직전
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(OutputResolverUnit, UT041_DoorOutputTruthTable)
{
    const cl_directive_t directives[] = {CL_DIR_FREE, CL_DIR_HOLD, CL_DIR_LOCK, CL_DIR_RELEASE};
    for (cl_directive_t directive : directives)
    {
        for (int32_t release_all = 0; release_all <= 1; ++release_all)
        {
            for (int prev = 0; prev <= 1; ++prev)
            {
                proposal_.release_all = release_all;
                previous_[CL_DOOR_LEFT] = static_cast<cl_lock_t>(prev);
                ASSERT_EQ(CL_OK, Resolve(MakeDecision(directive, CL_DIR_HOLD, CL_HEALTH_NORMAL)));
                cl_lock_t expected = static_cast<cl_lock_t>(prev);
                if (directive == CL_DIR_RELEASE)
                {
                    expected = CL_RELEASE;
                }
                else if (directive == CL_DIR_LOCK)
                {
                    expected = CL_LOCK;
                }
                else if ((directive == CL_DIR_FREE) && (release_all == 1))
                {
                    expected = CL_RELEASE;
                }
                EXPECT_EQ(expected, out_.lock[CL_DOOR_LEFT])
                    << directive << "," << release_all << "," << prev;
            }
        }
    }
}

/**
 * @test    UT-042: 좌/우 도어를 독립적으로 합성한다.
 * @trace   FUNC-030, SSR-013, SWR-I-005
 * @purpose 좌/우 지시가 다를 때 각 도어 출력이 자기 지시만 따르는지 확인한다.
 * @input   (좌 LOCK, 우 RELEASE), (좌 RELEASE, 우 LOCK), 직전 출력은 반대 값
 * @steps   1. 두 조합을 결정한다.
 * @expect  각 도어가 자기 지시대로 출력된다.
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(OutputResolverUnit, UT042_DoorsAreIndependent)
{
    previous_[CL_DOOR_LEFT] = CL_RELEASE;
    previous_[CL_DOOR_RIGHT] = CL_LOCK;
    ASSERT_EQ(CL_OK, Resolve(MakeDecision(CL_DIR_LOCK, CL_DIR_RELEASE, CL_HEALTH_NORMAL)));
    EXPECT_EQ(CL_LOCK, out_.lock[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_RELEASE, out_.lock[CL_DOOR_RIGHT]);
    previous_[CL_DOOR_LEFT] = CL_LOCK;
    previous_[CL_DOOR_RIGHT] = CL_RELEASE;
    ASSERT_EQ(CL_OK, Resolve(MakeDecision(CL_DIR_RELEASE, CL_DIR_LOCK, CL_HEALTH_NORMAL)));
    EXPECT_EQ(CL_RELEASE, out_.lock[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_LOCK, out_.lock[CL_DOOR_RIGHT]);
}

/**
 * @test    UT-043: 점화 해제는 제안이 있고 건전성이 NORMAL일 때만 적용된다. (상태, 이유코드 포함)
 * @trace   FUNC-032, FUNC-034, SWR-F-001~004, SSR-025, SSR-027
 * @purpose (release_all==1)&&(health==NORMAL)의 MC/DC와 상태 우선순위(FAULT>DEGRADED>OFF>NORMAL).
 * @input   (제안 0,NORMAL), (1,NORMAL), (1,DEGRADED), (1,FAULT), (0,DEGRADED). 지시는 모두 FREE
 * @steps   1. 조합별로 결정한다.
 * @expect  (1,NORMAL)만 RELEASE, 상태 OFF, 이유 IGNITION_OFF. 나머지는 직전(LOCK) 유지,
 *          상태 NORMAL/DEGRADED/FAULT/DEGRADED, IGNITION_OFF 이유 없음
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(OutputResolverUnit, UT043_IgnitionOffAppliesOnlyWhenNormal)
{
    struct Probe
    {
        int32_t proposal;
        cl_health_t health;
        cl_lock_t lock;
        cl_state_t state;
        bool reason;
    };
    const Probe probes[] = {
        {0, CL_HEALTH_NORMAL, CL_LOCK, CL_STATE_NORMAL, false},
        {1, CL_HEALTH_NORMAL, CL_RELEASE, CL_STATE_OFF, true},
        {1, CL_HEALTH_DEGRADED, CL_LOCK, CL_STATE_DEGRADED, false},
        {1, CL_HEALTH_FAULT, CL_LOCK, CL_STATE_FAULT, false},
        {0, CL_HEALTH_DEGRADED, CL_LOCK, CL_STATE_DEGRADED, false},
    };
    for (const Probe &p : probes)
    {
        proposal_.release_all = p.proposal;
        previous_[CL_DOOR_LEFT] = CL_LOCK;
        previous_[CL_DOOR_RIGHT] = CL_LOCK;
        ASSERT_EQ(CL_OK, Resolve(MakeDecision(CL_DIR_FREE, CL_DIR_FREE, p.health)));
        EXPECT_EQ(p.lock, out_.lock[CL_DOOR_LEFT]) << p.proposal << "," << p.health;
        EXPECT_EQ(p.lock, out_.lock[CL_DOOR_RIGHT]);
        EXPECT_EQ(p.state, out_.state);
        EXPECT_EQ(p.reason, (out_.reason_mask & CL_REASON_IGNITION_OFF) != 0U);
    }
}

/**
 * @test    UT044: 점화 OFF 이유코드는 출력이 이미 RELEASE여도 조건이 성립하는 주기마다 기록된다.
 * @trace   FUNC-030, SWR-F-003
 * @purpose 이유코드 기록 기준이 "출력 변화"가 아니라 "조건 성립"임을 확인한다.
 * @input   직전 출력 RELEASE/RELEASE, 제안 1, NORMAL, 지시 FREE
 * @steps   1. 연속 두 번 결정한다.
 * @expect  두 번 모두 IGNITION_OFF 이유 비트가 있다.
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(OutputResolverUnit, UT044_IgnitionReasonRecordedEveryCycle)
{
    proposal_.release_all = 1;
    for (int i = 0; i < 2; ++i)
    {
        ASSERT_EQ(CL_OK, Resolve(MakeDecision(CL_DIR_FREE, CL_DIR_FREE, CL_HEALTH_NORMAL)));
        EXPECT_NE(0U, out_.reason_mask & CL_REASON_IGNITION_OFF);
        EXPECT_EQ(CL_RELEASE, out_.lock[CL_DOOR_LEFT]);
    }
}

/**
 * @test    UT-045: 건전성이 FAULT일 때만 경고코드 SENSOR_FAULT를 낸다.
 * @trace   FUNC-030, SSR-026
 * @purpose health==FAULT 결정의 참/거짓.
 * @input   health NORMAL, DEGRADED, FAULT
 * @steps   1. 각각 결정하고 이유 비트를 본다.
 * @expect  FAULT에서만 SENSOR_FAULT 비트
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(OutputResolverUnit, UT045_SensorFaultWarning)
{
    const cl_health_t healths[] = {CL_HEALTH_NORMAL, CL_HEALTH_DEGRADED, CL_HEALTH_FAULT};
    for (cl_health_t health : healths)
    {
        ASSERT_EQ(CL_OK, Resolve(MakeDecision(CL_DIR_HOLD, CL_DIR_HOLD, health)));
        EXPECT_EQ(health == CL_HEALTH_FAULT, (out_.reason_mask & CL_REASON_SENSOR_FAULT) != 0U);
    }
}

/**
 * @test    UT-046: 해제 요청 억제 이유: 요청이 유효한 해제이고 대상 도어가 LOCK 지시일 때만
 * 기록된다.
 * @trace   FUNC-033, FUNC-035, SSR-011, SSR-012, SWR-I-004
 * @purpose (request_valid==1)&&(action==UNLOCK), (targets)&&(directive==LOCK),
 * (side==ALL)||(side==door)의 MC/DC.
 * @input   요청(무효/유효) × 동작(LOCK/UNLOCK) × side(LEFT/RIGHT/ALL) × LOCK 지시가 있는
 * 도어(없음/좌/우/양쪽)
 * @steps   1. 모든 조합을 결정한다. 2. 대상 도어 중 LOCK 지시가 있는지로 기대값을 계산한다.
 * @expect  유효한 UNLOCK이고 대상 도어 중 LOCK 지시가 있을 때만 APPROACH_RISK_SUPPRESS 비트
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(OutputResolverUnit, UT046_SuppressReasonMatrix)
{
    const cl_side_t sides[] = {CL_SIDE_LEFT, CL_SIDE_RIGHT, CL_SIDE_ALL};
    const cl_action_t actions[] = {CL_ACTION_LOCK, CL_ACTION_UNLOCK};
    for (int32_t valid = 0; valid <= 1; ++valid)
    {
        for (cl_action_t action : actions)
        {
            for (cl_side_t side : sides)
            {
                for (int mask = 0; mask < 4; ++mask)
                {
                    const bool left_lock = (mask & 1) != 0;
                    const bool right_lock = (mask & 2) != 0;
                    in_.request_valid = valid;
                    in_.request_side = side;
                    in_.request_action = action;
                    ASSERT_EQ(CL_OK, Resolve(MakeDecision(left_lock ? CL_DIR_LOCK : CL_DIR_FREE,
                                                          right_lock ? CL_DIR_LOCK : CL_DIR_FREE,
                                                          CL_HEALTH_NORMAL)));
                    const bool expected =
                        ExpectSuppress(valid, action, side, left_lock, right_lock);
                    EXPECT_EQ(expected, (out_.reason_mask & CL_REASON_APPROACH_RISK_SUPPRESS) != 0U)
                        << valid << "," << action << "," << side << "," << mask;
                }
            }
        }
    }
}

/**
 * @test    UT-047: 안전 판단 이유코드는 그대로 전달되고 억제·점화·경고 이유와 합쳐진다.
 * @trace   FUNC-036, SSR-012a, SSR-017, SWR-I-006b
 * @purpose 이유코드 합성(OR)이 기존 비트를 지우지 않는지 확인한다.
 * @input   decision 이유 = CRASH_PENDING|APPROACH_RISK_LOCK|APPROACH_INPUT_UNRELIABLE, 유효 UNLOCK
 * 요청과 LOCK 지시
 * @steps   1. 결정하고 이유 마스크를 읽는다.
 * @expect  입력 이유 3개와 SUPPRESS가 모두 포함, 그 밖의 비트(IGNITION_OFF, SENSOR_FAULT)는 없음
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(OutputResolverUnit, UT047_ReasonsAreMerged)
{
    const uint32_t incoming = CL_REASON_CRASH_PENDING | CL_REASON_APPROACH_RISK_LOCK |
                              CL_REASON_APPROACH_INPUT_UNRELIABLE;
    in_.request_valid = 1;
    in_.request_side = CL_SIDE_LEFT;
    in_.request_action = CL_ACTION_UNLOCK;
    ASSERT_EQ(CL_OK, Resolve(MakeDecision(CL_DIR_LOCK, CL_DIR_HOLD, CL_HEALTH_NORMAL, incoming)));
    EXPECT_EQ(incoming | CL_REASON_APPROACH_RISK_SUPPRESS, out_.reason_mask);
}

/**
 * @test    UT-048: 입력 유효성 5개가 결과에 그대로 복사된다.
 * @trace   FUNC-030, SWR-I-006a, SWR-I-006b
 * @purpose 조회용 유효성 복사(루프 전 구간).
 * @input   입력마다 VALID, INVALID, STALE이 섞인 검증 결과
 * @steps   1. 결정하고 out.validity를 비교한다.
 * @expect  out.validity[i] == in.validity[i] (5개 모두)
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(OutputResolverUnit, UT048_ValidityIsCopied)
{
    in_.validity[CL_IN_CRASH] = CL_STALE;
    in_.validity[CL_IN_APPROACH_LEFT] = CL_INVALID;
    in_.validity[CL_IN_APPROACH_RIGHT] = CL_VALID;
    in_.validity[CL_IN_IGNITION] = CL_STALE;
    in_.validity[CL_IN_SENSOR_FAULT] = CL_INVALID;
    ASSERT_EQ(CL_OK, Resolve(MakeDecision(CL_DIR_HOLD, CL_DIR_HOLD, CL_HEALTH_DEGRADED)));
    for (int i = 0; i < CL_IN_COUNT; ++i)
    {
        EXPECT_EQ(in_.validity[i], out_.validity[i]) << i;
    }
}

} // namespace
