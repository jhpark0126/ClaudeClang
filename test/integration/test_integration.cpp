/**
 * @file    test_integration.cpp
 * @brief   통합 테스트 (SWE.5). 실제 컴포넌트 구현을 인터페이스(IF-001~005)로 연결해
 *          데이터 흐름, 오류 반환, 호출 순서, 요구사항 시나리오를 확인한다.
 *          단위 테스트(test/unit), TDD 테스트(test/tdd)와 분리된 별도 실행 파일에서 측정한다.
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
#include <cstring>
#include <gtest/gtest.h>

extern "C"
{
#include "child_lock.h"
#include "ignition_policy.h"
#include "input_validator.h"
#include "output_resolver.h"
#include "safety_arbiter.h"
}

namespace
{

cl_raw_input_t MakeRaw()
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

/** IF-001~IF-004를 호출자(child_lock)와 같은 순서로 직접 연결한다. */
struct Chain
{
    input_validator_state_t validator = {};
    safety_arbiter_state_t arbiter = {};
    cl_lock_t previous[CL_DOOR_COUNT] = {CL_RELEASE, CL_RELEASE};
    cl_validated_t validated = {};
    safety_decision_t decision = {};
    ignition_proposal_t proposal = {};

    void Init()
    {
        ASSERT_EQ(CL_OK, input_validator_init(&validator));
        ASSERT_EQ(CL_OK, safety_arbiter_init(&arbiter));
    }

    cl_result_t Step(const cl_raw_input_t *raw, uint32_t now_ms)
    {
        cl_result_t result = {};
        EXPECT_EQ(CL_OK, input_validator_validate(&validator, raw, now_ms, &validated));
        EXPECT_EQ(CL_OK, safety_arbiter_decide(&arbiter, &validated, &decision));
        EXPECT_EQ(CL_OK, ignition_policy_propose(&validated, &proposal));
        EXPECT_EQ(CL_OK,
                  output_resolver_resolve(&decision, &proposal, &validated, previous, &result));
        previous[CL_DOOR_LEFT] = result.lock[CL_DOOR_LEFT];
        previous[CL_DOOR_RIGHT] = result.lock[CL_DOOR_RIGHT];
        return result;
    }
};

class IntegrationTest : public ::testing::Test
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

/**
 * @test    IT-001: 검증기(IF-001)가 분류한 유효성이 안전 판단기(IF-002)의 입력으로 올바르게
 * 전달된다.
 * @trace   IF-001, IF-002, ARCH-001, ARCH-002, SSR-004, SSR-005, SSR-018, SSR-019, SSR-021a
 * @purpose ARCH-001→ARCH-002 데이터 흐름: 범위 밖 충돌 값은 INVALID로 분류되어 래치를 만들지 못하고
 *          DEGRADED를 일으킨다. (동치 클래스: 불량 값)
 * @input   crash=3(범위 밖) 스냅샷, 이어서 crash=CONFIRMED(2) 유효 스냅샷
 * @steps   1. 두 컴포넌트를 실제 구현으로 연결한다. 2. 범위 밖 스냅샷을 검증기에 넣고 판단기에
 * 넘긴다.
 *          3. 유효한 CONFIRMED를 넣는다.
 * @expect  첫 결과는 health=DEGRADED, 지시 HOLD(래치 없음). 둘째 결과는 양쪽 RELEASE 지시.
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST(IntegrationFlow, IT001_ValidatorToArbiter)
{
    input_validator_state_t v = {};
    safety_arbiter_state_t a = {};
    cl_validated_t validated = {};
    safety_decision_t decision = {};
    ASSERT_EQ(CL_OK, input_validator_init(&v));
    ASSERT_EQ(CL_OK, safety_arbiter_init(&a));
    cl_raw_input_t raw = MakeRaw();
    raw.field[CL_IN_CRASH].value = 3;
    ASSERT_EQ(CL_OK, input_validator_validate(&v, &raw, 0U, &validated));
    ASSERT_EQ(CL_OK, safety_arbiter_decide(&a, &validated, &decision));
    EXPECT_EQ(CL_INVALID, validated.validity[CL_IN_CRASH]);
    EXPECT_EQ(CL_HEALTH_DEGRADED, decision.health);
    EXPECT_EQ(CL_DIR_HOLD, decision.directive[CL_DOOR_LEFT]);
    raw.field[CL_IN_CRASH].value = CL_CRASH_CONFIRMED;
    ASSERT_EQ(CL_OK, input_validator_validate(&v, &raw, 10U, &validated));
    ASSERT_EQ(CL_OK, safety_arbiter_decide(&a, &validated, &decision));
    EXPECT_EQ(CL_DIR_RELEASE, decision.directive[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_DIR_RELEASE, decision.directive[CL_DOOR_RIGHT]);
}

/**
 * @test    IT-002: 검증기(IF-001)의 점화 유효성이 점화 정책(IF-003)의 제안을 결정한다.
 * @trace   IF-001, IF-003, ARCH-001, ARCH-003, SWR-F-001, SWR-I-003
 * @purpose ARCH-001→ARCH-003 데이터 흐름. 점화 OFF가 불량이면 제안하지 않는다. (동치 클래스: OFF,
 * ON, 불량)
 * @input   ignition=0(OFF), 1(ON), 7(범위 밖)
 * @steps   1. 검증기에 스냅샷을 넣는다. 2. 결과를 점화 정책에 전달한다.
 * @expect  release_all은 OFF에서만 1, ON과 범위 밖에서는 0
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST(IntegrationFlow, IT002_ValidatorToIgnitionPolicy)
{
    const int32_t values[] = {0, 1, 7};
    const int32_t expect[] = {1, 0, 0};
    for (int i = 0; i < 3; ++i)
    {
        input_validator_state_t v = {};
        cl_validated_t validated = {};
        ignition_proposal_t proposal = {5};
        ASSERT_EQ(CL_OK, input_validator_init(&v));
        cl_raw_input_t raw = MakeRaw();
        raw.field[CL_IN_IGNITION].value = values[i];
        ASSERT_EQ(CL_OK, input_validator_validate(&v, &raw, 0U, &validated));
        ASSERT_EQ(CL_OK, ignition_policy_propose(&validated, &proposal));
        EXPECT_EQ(expect[i], proposal.release_all) << values[i];
    }
}

/**
 * @test    IT-003: 안전 판단(IF-002)과 점화 제안(IF-003)이 합성기(IF-004)에서 안전 우선으로
 * 합쳐진다.
 * @trace   IF-002, IF-003, IF-004, ARCH-002, ARCH-003, ARCH-004, SSR-015, SWR-F-001, SWR-F-004
 * @purpose ASIL B 지시와 QM 제안의 간섭 방지: 접근위험 LOCK과 DEGRADED는 점화 OFF 해제를 막는다.
 * @input   점화 OFF + 좌 접근위험 TRUE(정상), 양쪽 LOCK 뒤 점화 OFF + 충돌 입력 누락(DEGRADED),
 * 점화 OFF만(정상)
 * @steps   1. 세 시나리오의 스냅샷을 연결된 컴포넌트에 넣는다.
 * @expect  (1) 좌 LOCK, 우 RELEASE (2) 양쪽 LOCK 유지, 상태 DEGRADED, 점화 OFF 이유 없음 (3) 양쪽
 * RELEASE, 상태 OFF
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST(IntegrationFlow, IT003_SafetyBeatsQmProposal)
{
    Chain chain;
    chain.Init();
    cl_raw_input_t raw = MakeRaw();
    raw.field[CL_IN_IGNITION].value = 0;
    raw.field[CL_IN_APPROACH_LEFT].value = 1;
    cl_result_t r = chain.Step(&raw, 0U);
    EXPECT_EQ(CL_LOCK, r.lock[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_RELEASE, r.lock[CL_DOOR_RIGHT]);

    Chain degraded;
    degraded.Init();
    cl_raw_input_t both_risk = MakeRaw();
    both_risk.field[CL_IN_APPROACH_LEFT].value = 1;
    both_risk.field[CL_IN_APPROACH_RIGHT].value = 1;
    degraded.Step(&both_risk, 0U);
    cl_raw_input_t off = MakeRaw();
    off.field[CL_IN_IGNITION].value = 0;
    cl_raw_input_t off_missing_crash = off;
    off_missing_crash.field[CL_IN_CRASH].present = 0;
    r = degraded.Step(&off_missing_crash, 10U);
    EXPECT_EQ(CL_STATE_DEGRADED, r.state);
    EXPECT_EQ(CL_LOCK, r.lock[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_LOCK, r.lock[CL_DOOR_RIGHT]);
    EXPECT_EQ(0U, r.reason_mask & CL_REASON_IGNITION_OFF);

    Chain plain;
    plain.Init();
    r = plain.Step(&off, 0U);
    EXPECT_EQ(CL_RELEASE, r.lock[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_RELEASE, r.lock[CL_DOOR_RIGHT]);
    EXPECT_EQ(CL_STATE_OFF, r.state);
}

/**
 * @test    IT-004: 운전자 해제 요청이 검증기(IF-001)→합성기(IF-004)로 전달되어 접근위험 도어에서
 * 억제된다.
 * @trace   IF-001, IF-002, IF-004, ARCH-001, ARCH-004, SWR-I-004, SSR-011, SSR-012
 * @purpose 요청 필드의 디코딩 결과(side, action)가 합성기의 억제 판단에 쓰이는지 확인한다.
 *          (동치 클래스: 좌 해제, 전체 해제, 잠금 요청, 미등록 side)
 * @input   좌 접근위험 TRUE에서 해제(좌), 해제(전체), 잠금(전체), 미등록 side(9)
 * @steps   1. 각 요청을 실은 스냅샷을 연결된 컴포넌트에 넣는다. 2. 이유코드를 확인한다.
 * @expect  앞의 둘만 APPROACH_RISK_SUPPRESS 이유가 있고 좌 도어는 항상 LOCK
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST(IntegrationFlow, IT004_RequestSuppressionAcrossComponents)
{
    struct Probe
    {
        int32_t side, action;
        bool suppressed;
    };
    const Probe probes[] = {{CL_SIDE_LEFT, CL_ACTION_UNLOCK, true},
                            {CL_SIDE_ALL, CL_ACTION_UNLOCK, true},
                            {CL_SIDE_ALL, CL_ACTION_LOCK, false},
                            {9, CL_ACTION_UNLOCK, false}};
    for (const Probe &p : probes)
    {
        Chain chain;
        chain.Init();
        cl_raw_input_t raw = MakeRaw();
        raw.field[CL_IN_APPROACH_LEFT].value = 1;
        raw.request_present = 1;
        raw.request_side = p.side;
        raw.request_action = p.action;
        raw.request_source = 2;
        cl_result_t r = chain.Step(&raw, 0U);
        EXPECT_EQ(CL_LOCK, r.lock[CL_DOOR_LEFT]);
        EXPECT_EQ(p.suppressed, (r.reason_mask & CL_REASON_APPROACH_RISK_SUPPRESS) != 0U)
            << p.side << "," << p.action;
    }
}

/**
 * @test    IT-005: 모든 인터페이스 함수가 NULL 인자에서 CL_ERR_NULL_ARG를 반환한다.
 * @trace   IF-001, IF-002, IF-003, IF-004, IF-005
 * @purpose 인터페이스에 정의된 유일한 오류 반환(CL_ERR_NULL_ARG)이 모든 IF에서 발생·전파되는지
 * 확인한다.
 * @input   각 IF 함수의 필수 포인터 인자 하나를 NULL로 한 호출
 * @steps   1. 정상 컨텍스트를 만든다. 2. IF-001~005의 함수마다 NULL을 넣어 호출한다.
 * @expect  모든 호출이 CL_ERR_NULL_ARG를 반환한다. 정상 호출은 CL_OK.
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST(IntegrationInterfaces, IT005_NullArgumentErrorOnEveryInterface)
{
    input_validator_state_t v = {};
    safety_arbiter_state_t a = {};
    cl_ctx_t ctx = {};
    cl_validated_t validated = {};
    safety_decision_t decision = {};
    ignition_proposal_t proposal = {};
    cl_result_t result = {};
    cl_lock_t previous[CL_DOOR_COUNT] = {CL_RELEASE, CL_RELEASE};
    ASSERT_EQ(CL_OK, input_validator_init(&v));
    ASSERT_EQ(CL_OK, safety_arbiter_init(&a));
    ASSERT_EQ(CL_OK, child_lock_init(&ctx));
    EXPECT_EQ(CL_ERR_NULL_ARG, input_validator_init(nullptr));                      /* IF-001 */
    EXPECT_EQ(CL_ERR_NULL_ARG, input_validator_validate(&v, nullptr, 0U, nullptr)); /* IF-001 */
    EXPECT_EQ(CL_ERR_NULL_ARG, safety_arbiter_init(nullptr));                       /* IF-002 */
    EXPECT_EQ(CL_ERR_NULL_ARG, safety_arbiter_decide(&a, nullptr, &decision));      /* IF-002 */
    EXPECT_EQ(CL_ERR_NULL_ARG, ignition_policy_propose(nullptr, &proposal));        /* IF-003 */
    EXPECT_EQ(CL_ERR_NULL_ARG, output_resolver_resolve(nullptr, &proposal, &validated, previous,
                                                       &result));            /* IF-004 */
    EXPECT_EQ(CL_ERR_NULL_ARG, child_lock_init(nullptr));                    /* IF-005 */
    EXPECT_EQ(CL_ERR_NULL_ARG, child_lock_step(&ctx, nullptr, 0U, nullptr)); /* IF-005 */
    EXPECT_EQ(CL_OK, child_lock_step(&ctx, nullptr, 0U, &result));
}

/**
 * @test    IT-006: child_lock_step(IF-005)의 결과가 컴포넌트를 직접 연결한 결과와 같다. (PICT 2-way
 * 20건)
 * @trace   IF-005, IF-001~IF-004, ARCH-005, SWR-I-005, SWR-I-006a, SWR-I-006b, SWR-Q-003
 * @purpose child_lock이 호출 순서와 직전 출력 저장을 올바르게 조율하는지 확인한다. 같은 입력 열에서
 *          외부 인터페이스 결과가 내부 인터페이스를 직접 연결한 결과와 일치해야 한다. 요구사항
 * 불변식도 검사한다.
 * @input   test/model/pipeline.pict(충돌×접근위험×점화×센서고장×요청) 20건, 각 건은 첫 주기 스냅샷
 * + 둘째 주기 수신 없음
 * @steps   1. child_lock 컨텍스트와 직접 연결 체인을 같은 입력으로 두 주기 실행한다.
 *          2. 매 주기 결과를 바이트 단위로 비교한다. 3. 불변식을 확인한다.
 * @expect  두 결과가 같다. 유효한 CONFIRMED이면 양쪽 RELEASE. 충돌이 아니면서 좌 접근위험이 유효
 * TRUE이면 좌 LOCK.
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST(IntegrationScenarios, IT006_PictPipelineMatchesDirectChain)
{
    struct PipelineCase
    {
        int32_t crash, app_left, app_right, ignition, fault, request;
    };
    const PipelineCase cases[] = {
#include "pipeline_pict_cases.inc"
    };
    int index = 0;
    for (const PipelineCase &c : cases)
    {
        cl_ctx_t ctx = {};
        ASSERT_EQ(CL_OK, child_lock_init(&ctx));
        Chain chain;
        chain.Init();
        cl_raw_input_t raw = MakeRaw();
        raw.field[CL_IN_CRASH].value = c.crash;
        raw.field[CL_IN_APPROACH_LEFT].value = c.app_left;
        raw.field[CL_IN_APPROACH_RIGHT].value = c.app_right;
        raw.field[CL_IN_IGNITION].value = c.ignition;
        raw.field[CL_IN_SENSOR_FAULT].value = c.fault;
        raw.request_present = (c.request == 0) ? 0 : 1;
        raw.request_side = (c.request == 1) ? CL_SIDE_LEFT : (c.request == 4 ? 9 : CL_SIDE_ALL);
        raw.request_action = (c.request == 3) ? CL_ACTION_LOCK : CL_ACTION_UNLOCK;
        raw.request_source = 1;
        cl_result_t actual = {};
        ASSERT_EQ(CL_OK, child_lock_step(&ctx, &raw, 0U, &actual));
        const cl_result_t expected = chain.Step(&raw, 0U);
        EXPECT_EQ(0, std::memcmp(&expected, &actual, sizeof(actual))) << "case " << index;
        if (c.crash == CL_CRASH_CONFIRMED)
        {
            EXPECT_EQ(CL_RELEASE, actual.lock[CL_DOOR_LEFT]) << "case " << index;
            EXPECT_EQ(CL_RELEASE, actual.lock[CL_DOOR_RIGHT]) << "case " << index;
        }
        else if (c.app_left == 1)
        {
            EXPECT_EQ(CL_LOCK, actual.lock[CL_DOOR_LEFT]) << "case " << index;
        }
        ASSERT_EQ(CL_OK, child_lock_step(&ctx, nullptr, 10U, &actual));
        const cl_result_t expected2 = chain.Step(nullptr, 10U);
        EXPECT_EQ(0, std::memcmp(&expected2, &actual, sizeof(actual))) << "case " << index;
        ++index;
    }
}

/**
 * @test    IT-007: OEM-SR-001 충돌 시나리오: CONFIRMED 수신 주기에 해제되고, 이후 모든 주기에서
 * 유지된다.
 * @trace   IF-005, ARCH-002, ARCH-004, ARCH-005, SSR-001, SSR-002, SSR-003, SSR-008, SWR-L-001
 * @purpose 300 ms 이내 해제와 해제 유지를 컴포넌트 연결 상태에서 확인한다.
 * @input   t=0..90(10 ms) 좌/우 접근위험 TRUE로 LOCK 상태, t=100에 CONFIRMED, 이후 NONE과 점화
 * ON/접근위험 유지
 * @steps   1. 10 ms 주기로 평가한다. 2. t=100 주기의 출력과 이후 30주기(300 ms)의 출력을 본다.
 * @expect  t=100 주기에 양쪽 RELEASE(300 ms 이내), t=110~400에도 RELEASE 유지
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(IntegrationTest, IT007_CrashScenario)
{
    cl_raw_input_t raw = MakeRaw();
    raw.field[CL_IN_APPROACH_LEFT].value = 1;
    raw.field[CL_IN_APPROACH_RIGHT].value = 1;
    for (uint32_t t = 0U; t < 100U; t += CL_EVAL_PERIOD_MS)
    {
        ASSERT_EQ(CL_OK, Step(&raw, t));
        ASSERT_EQ(CL_LOCK, out_.lock[CL_DOOR_LEFT]);
        ASSERT_EQ(CL_LOCK, out_.lock[CL_DOOR_RIGHT]);
    }
    raw.field[CL_IN_CRASH].value = CL_CRASH_CONFIRMED;
    ASSERT_EQ(CL_OK, Step(&raw, 100U));
    EXPECT_EQ(CL_RELEASE, out_.lock[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_RELEASE, out_.lock[CL_DOOR_RIGHT]);
    raw.field[CL_IN_CRASH].value = CL_CRASH_NONE;
    for (uint32_t t = 110U; t <= 400U; t += CL_EVAL_PERIOD_MS)
    {
        ASSERT_EQ(CL_OK, Step(&raw, t));
        ASSERT_EQ(CL_RELEASE, out_.lock[CL_DOOR_LEFT]) << t;
        ASSERT_EQ(CL_RELEASE, out_.lock[CL_DOOR_RIGHT]) << t;
    }
}

/**
 * @test    IT-008: OEM-SR-002/FR-007 시나리오: 접근위험 LOCK은 점화 OFF에도 유지되고, 위험이
 * 사라져도 해제 요청 없이 유지된다.
 * @trace   IF-005, ARCH-002, ARCH-003, ARCH-004, SSR-009, SSR-011, SSR-015, SWR-F-001~003
 * @purpose 접근위험과 점화 OFF가 번갈아 일어나는 연속 주기에서 도어별 출력과 상태를 확인한다.
 * @input   t=0 좌 위험 TRUE, t=10 위험 FALSE, t=20 점화 OFF(위험 FALSE), t=30 좌 위험 TRUE + 점화
 * OFF
 * @steps   1. 네 주기를 순서대로 평가한다.
 * @expect  t=0 좌 LOCK/우 RELEASE, t=10 좌 LOCK 유지, t=20 양쪽 RELEASE+OFF+IGNITION_OFF 이유,
 *          t=30 좌 LOCK/우 RELEASE
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(IntegrationTest, IT008_ApproachRiskAndIgnitionOff)
{
    cl_raw_input_t raw = MakeRaw();
    raw.field[CL_IN_APPROACH_LEFT].value = 1;
    ASSERT_EQ(CL_OK, Step(&raw, 0U));
    EXPECT_EQ(CL_LOCK, out_.lock[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_RELEASE, out_.lock[CL_DOOR_RIGHT]);
    raw.field[CL_IN_APPROACH_LEFT].value = 0;
    ASSERT_EQ(CL_OK, Step(&raw, 10U));
    EXPECT_EQ(CL_LOCK, out_.lock[CL_DOOR_LEFT]);
    raw.field[CL_IN_IGNITION].value = 0;
    ASSERT_EQ(CL_OK, Step(&raw, 20U));
    EXPECT_EQ(CL_RELEASE, out_.lock[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_RELEASE, out_.lock[CL_DOOR_RIGHT]);
    EXPECT_EQ(CL_STATE_OFF, out_.state);
    EXPECT_NE(0U, out_.reason_mask & CL_REASON_IGNITION_OFF);
    raw.field[CL_IN_APPROACH_LEFT].value = 1;
    ASSERT_EQ(CL_OK, Step(&raw, 30U));
    EXPECT_EQ(CL_LOCK, out_.lock[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_RELEASE, out_.lock[CL_DOOR_RIGHT]);
}

/**
 * @test    IT-009: OEM-SR-003 시나리오: 입력이 끊기면 200 ms 초과 첫 주기에 DEGRADED가 되고 출력은
 * 유지된다. 수신이 재개되면 NORMAL로 복귀한다.
 * @trace   IF-005, ARCH-001, ARCH-002, ARCH-004, SSR-020, SSR-021, SSR-022, SSR-023, SWR-Q-002
 * @purpose 시간 경과(주입 시계)에 따른 상태 전이와 복구를 컴포넌트 연결 상태에서 확인한다.
 * @input   t=0 수신(좌 위험 TRUE), 이후 10 ms 주기로 수신 없음(t=10..220), t=230 수신 재개
 * @steps   1. 수신 후 수신 없이 평가한다. 2. 200 ms 직전(t=200)과 직후(t=210)를 비교한다. 3. 수신
 * 재개.
 * @expect  t=200 NORMAL, t=210 DEGRADED(좌 LOCK 유지, 모든 입력 STALE), t=230 NORMAL
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(IntegrationTest, IT009_StaleScenarioAndRecovery)
{
    cl_raw_input_t raw = MakeRaw();
    raw.field[CL_IN_APPROACH_LEFT].value = 1;
    ASSERT_EQ(CL_OK, Step(&raw, 0U));
    for (uint32_t t = 10U; t <= 200U; t += CL_EVAL_PERIOD_MS)
    {
        ASSERT_EQ(CL_OK, Step(nullptr, t));
        ASSERT_EQ(CL_STATE_NORMAL, out_.state) << t;
    }
    ASSERT_EQ(CL_OK, Step(nullptr, 210U));
    EXPECT_EQ(CL_STATE_DEGRADED, out_.state);
    EXPECT_EQ(CL_LOCK, out_.lock[CL_DOOR_LEFT]);
    for (int i = 0; i < CL_IN_COUNT; ++i)
    {
        EXPECT_EQ(CL_STALE, out_.validity[i]) << i;
    }
    ASSERT_EQ(CL_OK, Step(nullptr, 220U));
    EXPECT_EQ(CL_STATE_DEGRADED, out_.state);
    ASSERT_EQ(CL_OK, Step(&raw, 230U));
    EXPECT_EQ(CL_STATE_NORMAL, out_.state);
}

/**
 * @test    IT-010: OEM-SR-004 시나리오: sensor_fault는 새 명령(점화 OFF 해제)을 막고 FAULT와
 * 경고코드를 낸다. 충돌 CONFIRMED는 FAULT 중에도 해제한다.
 * @trace   IF-005, ARCH-002, ARCH-004, SSR-005, SSR-024, SSR-025, SSR-026, SSR-027, SWR-F-004
 * @purpose FAULT 상태에서 ASIL B 해제(충돌)는 통과하고 QM 해제(점화)는 차단되는 간섭 방지를 연결
 * 상태에서 확인한다.
 * @input   좌 위험 TRUE로 LOCK 후 sensor_fault=TRUE + 점화 OFF, 이어서 CONFIRMED 추가
 * @steps   1. LOCK 상태를 만든다. 2. 고장+점화 OFF 주기를 평가한다. 3. 충돌을 추가한다.
 * @expect  고장 주기: 좌 LOCK 유지, 상태 FAULT, SENSOR_FAULT 이유, 점화 OFF 이유 없음.
 *          충돌 주기: 양쪽 RELEASE, 상태 FAULT
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(IntegrationTest, IT010_FaultScenario)
{
    cl_raw_input_t raw = MakeRaw();
    raw.field[CL_IN_APPROACH_LEFT].value = 1;
    ASSERT_EQ(CL_OK, Step(&raw, 0U));
    raw.field[CL_IN_APPROACH_LEFT].value = 0;
    raw.field[CL_IN_SENSOR_FAULT].value = 1;
    raw.field[CL_IN_IGNITION].value = 0;
    ASSERT_EQ(CL_OK, Step(&raw, 10U));
    EXPECT_EQ(CL_LOCK, out_.lock[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_STATE_FAULT, out_.state);
    EXPECT_NE(0U, out_.reason_mask & CL_REASON_SENSOR_FAULT);
    EXPECT_EQ(0U, out_.reason_mask & CL_REASON_IGNITION_OFF);
    raw.field[CL_IN_CRASH].value = CL_CRASH_CONFIRMED;
    ASSERT_EQ(CL_OK, Step(&raw, 20U));
    EXPECT_EQ(CL_RELEASE, out_.lock[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_RELEASE, out_.lock[CL_DOOR_RIGHT]);
    EXPECT_EQ(CL_STATE_FAULT, out_.state);
}

/**
 * @test    IT-011: 초기화 순서: child_lock_init은 모든 하위 컴포넌트 상태를 초기화하고, 첫 주기는
 * RELEASE+DEGRADED이다.
 * @trace   IF-005, IF-001, IF-002, ARCH-005, SWR-F-005, SSR-002, SSR-021a
 * @purpose 사용 중이던 컨텍스트를 init으로 되돌렸을 때 래치, 입력 상태, 직전 출력이 모두
 * 초기화되는지 확인한다.
 * @input   충돌 래치와 LOCK 출력을 만든 뒤 init, 이어서 입력 없는 주기와 접근위험 입력 주기
 * @steps   1. 충돌+접근위험을 처리한다. 2. init한다. 3. 입력 없이 평가한다. 4. 우 접근위험으로
 * 평가한다.
 * @expect  입력 없는 주기: 양쪽 RELEASE, DEGRADED. 이후: 래치가 없어 우 LOCK
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(IntegrationTest, IT011_InitResetsAllComponents)
{
    cl_raw_input_t raw = MakeRaw();
    raw.field[CL_IN_CRASH].value = CL_CRASH_CONFIRMED;
    raw.field[CL_IN_APPROACH_LEFT].value = 1;
    ASSERT_EQ(CL_OK, Step(&raw, 0U));
    ASSERT_EQ(CL_OK, child_lock_init(&ctx_));
    ASSERT_EQ(CL_OK, Step(nullptr, 10U));
    EXPECT_EQ(CL_RELEASE, out_.lock[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_RELEASE, out_.lock[CL_DOOR_RIGHT]);
    EXPECT_EQ(CL_STATE_DEGRADED, out_.state);
    raw.field[CL_IN_CRASH].value = CL_CRASH_NONE;
    raw.field[CL_IN_APPROACH_LEFT].value = 0;
    raw.field[CL_IN_APPROACH_RIGHT].value = 1;
    ASSERT_EQ(CL_OK, Step(&raw, 20U));
    EXPECT_EQ(CL_LOCK, out_.lock[CL_DOOR_RIGHT]);
    EXPECT_EQ(CL_RELEASE, out_.lock[CL_DOOR_LEFT]);
}

} // namespace
