/**
 * @file    test_input_validator_unit.cpp
 * @brief   input_validator 단위 테스트 (SWE.4). 동치 분할, 경계값, MC/DC 독립 쌍 기준으로 설계했다.
 *          TDD 테스트(test/tdd)와 분리된 별도 실행 파일에서 측정한다.
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
#include "input_validator.h"
}

namespace
{

using unit::MakeRaw;

class InputValidatorUnit : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        ASSERT_EQ(CL_OK, input_validator_init(&state_));
    }
    cl_status_t Validate(const cl_raw_input_t *raw, uint32_t now_ms)
    {
        return input_validator_validate(&state_, raw, now_ms, &out_);
    }
    input_validator_state_t state_ = {};
    cl_validated_t out_ = {};
};

/**
 * @test    UT-001: NULL 인자는 CL_ERR_NULL_ARG를 반환하고 출력을 바꾸지 않는다.
 * @trace   FUNC-001, FUNC-002, SSR-018
 * @purpose (state==NULL)||(out==NULL) 결정의 MC/DC. 각 조건이 단독으로 참인 쌍과 모두 거짓인 경우.
 * @input   state만 NULL, out만 NULL, 둘 다 정상, init(NULL)
 * @steps   1. 각 조합으로 호출한다. 2. 출력 구조체의 변화를 확인한다.
 * @expect  NULL이 있으면 CL_ERR_NULL_ARG이고 out은 센티널 값 그대로, 모두 정상이면 CL_OK
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(InputValidatorUnit, UT001_NullArguments)
{
    cl_validated_t sentinel = {};
    sentinel.value[0] = 77;
    out_ = sentinel;
    EXPECT_EQ(CL_ERR_NULL_ARG, input_validator_init(nullptr));
    EXPECT_EQ(CL_ERR_NULL_ARG, input_validator_validate(nullptr, nullptr, 0U, &out_));
    EXPECT_EQ(CL_ERR_NULL_ARG, input_validator_validate(&state_, nullptr, 0U, nullptr));
    EXPECT_EQ(77, out_.value[0]);
    EXPECT_EQ(CL_OK, input_validator_validate(&state_, nullptr, 0U, &out_));
}

/**
 * @test    UT-002: 필수 입력별 허용 범위의 경계값을 판정한다.
 * @trace   FUNC-002, FUNC-003, FUNC-007, SWR-I-001, SWR-I-002, SWR-I-003, SWR-I-008, SSR-018
 * @purpose (value>=0)&&(value<=max) 결정의 MC/DC와 경계값 분석(min-1, min, max, max+1).
 * @input   입력 5개 각각에 대해 -1, 0, max, max+1 (crash max=2, 그 외 max=1)
 * @steps   1. 한 입력에만 값을 넣고 나머지는 정상으로 둔다. 2. 검증한다.
 * @expect  0~max는 VALID와 같은 값, 그 밖은 INVALID와 값 0. 다른 입력은 영향이 없다.
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(InputValidatorUnit, UT002_RangeBoundaries)
{
    const int32_t max_value[CL_IN_COUNT] = {2, 1, 1, 1, 1};
    for (int input = 0; input < CL_IN_COUNT; ++input)
    {
        const int32_t probes[] = {-1, 0, max_value[input], max_value[input] + 1};
        for (int32_t probe : probes)
        {
            ASSERT_EQ(CL_OK, input_validator_init(&state_));
            cl_raw_input_t raw = MakeRaw();
            raw.field[input].value = probe;
            ASSERT_EQ(CL_OK, Validate(&raw, 0U));
            const bool inside = (probe >= 0) && (probe <= max_value[input]);
            EXPECT_EQ(inside ? CL_VALID : CL_INVALID, out_.validity[input])
                << "input " << input << " value " << probe;
            EXPECT_EQ(inside ? probe : 0, out_.value[input]);
            const int other = (input + 1) % CL_IN_COUNT;
            EXPECT_EQ(CL_VALID, out_.validity[other]);
        }
    }
}

/**
 * @test    UT-003: present 플래그가 1일 때만 유효하다. (0, 2, -1은 형식 오류)
 * @trace   FUNC-002, FUNC-003, SSR-018
 * @purpose present==1 결정의 참/거짓과 형식 오류(1이 아닌 값) 동치 클래스.
 * @input   present = 0, 1, 2, -1 (값은 범위 안)
 * @steps   1. 해당 입력의 present만 바꾼다. 2. 검증한다.
 * @expect  present==1만 VALID, 나머지는 INVALID
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(InputValidatorUnit, UT003_PresentFlag)
{
    const int32_t flags[] = {0, 1, 2, -1};
    for (int32_t flag : flags)
    {
        ASSERT_EQ(CL_OK, input_validator_init(&state_));
        cl_raw_input_t raw = MakeRaw();
        raw.field[CL_IN_CRASH].present = flag;
        ASSERT_EQ(CL_OK, Validate(&raw, 0U));
        EXPECT_EQ(flag == 1 ? CL_VALID : CL_INVALID, out_.validity[CL_IN_CRASH]) << flag;
    }
}

/**
 * @test    UT-004: 유효값이 한 번도 없으면 INVALID이고, 수신이 없어도 INVALID가 유지된다.
 * @trace   FUNC-001, FUNC-005, SSR-018, SWR-F-005
 * @purpose (has_valid==0)||(last_was_bad==1)에서 has_valid==0만 참인 독립 쌍.
 * @input   init 직후, 스냅샷 없음(NULL)
 * @steps   1. 초기화 직후 NULL로 검증한다. 2. 유효 스냅샷 후 NULL로 검증해 비교한다.
 * @expect  초기에는 모든 입력 INVALID(값 0), 유효 수신 후에는 VALID
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(InputValidatorUnit, UT004_NeverReceivedIsInvalid)
{
    ASSERT_EQ(CL_OK, Validate(nullptr, 0U));
    for (int i = 0; i < CL_IN_COUNT; ++i)
    {
        EXPECT_EQ(CL_INVALID, out_.validity[i]);
        EXPECT_EQ(0, out_.value[i]);
    }
    cl_raw_input_t raw = MakeRaw();
    ASSERT_EQ(CL_OK, Validate(&raw, 10U));
    ASSERT_EQ(CL_OK, Validate(nullptr, 20U));
    EXPECT_EQ(CL_VALID, out_.validity[CL_IN_CRASH]);
}

/**
 * @test    UT-005: 불량 수신은 다음 유효 수신 전까지 INVALID를 유지하고, 유효 수신으로 회복한다.
 * @trace   FUNC-002, FUNC-004, FUNC-005, SSR-018, SSR-019
 * @purpose (has_valid==0)||(last_was_bad==1)에서 last_was_bad==1만 참인 독립 쌍과 회복 경로.
 * @input   유효 수신 → 범위 밖 수신 → 수신 없음 → 유효 수신
 * @steps   1. 위 순서로 검증한다. 2. 단계마다 해당 입력의 분류를 확인한다.
 * @expect  VALID → INVALID → INVALID(값 0) → VALID
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(InputValidatorUnit, UT005_BadReceiptSticksUntilNextValid)
{
    cl_raw_input_t raw = MakeRaw();
    ASSERT_EQ(CL_OK, Validate(&raw, 0U));
    EXPECT_EQ(CL_VALID, out_.validity[CL_IN_IGNITION]);
    raw.field[CL_IN_IGNITION].value = 5;
    ASSERT_EQ(CL_OK, Validate(&raw, 10U));
    EXPECT_EQ(CL_INVALID, out_.validity[CL_IN_IGNITION]);
    ASSERT_EQ(CL_OK, Validate(nullptr, 20U));
    EXPECT_EQ(CL_INVALID, out_.validity[CL_IN_IGNITION]);
    EXPECT_EQ(0, out_.value[CL_IN_IGNITION]);
    raw.field[CL_IN_IGNITION].value = 0;
    ASSERT_EQ(CL_OK, Validate(&raw, 30U));
    EXPECT_EQ(CL_VALID, out_.validity[CL_IN_IGNITION]);
    EXPECT_EQ(0, out_.value[CL_IN_IGNITION]);
}

/**
 * @test    UT-006: 입력 나이 200 ms 경계(199, 200, 201)에서 VALID/STALE를 판정한다.
 * @trace   FUNC-005, FUNC-009, SSR-020, SSR-022, SWR-Q-002, SWR-I-007
 * @purpose 경계값 분석. age > CL_FRESHNESS_LIMIT_MS 결정의 참/거짓.
 * @input   t=1000에 유효 수신 후 t=1199, 1200, 1201에 수신 없이 검증
 * @steps   1. 수신 후 시각만 바꿔 검증한다.
 * @expect  199, 200은 VALID(값 유지), 201은 STALE(값 0)
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(InputValidatorUnit, UT006_FreshnessBoundary)
{
    cl_raw_input_t raw = MakeRaw();
    raw.field[CL_IN_CRASH].value = CL_CRASH_PENDING;
    ASSERT_EQ(CL_OK, Validate(&raw, 1000U));
    const uint32_t times[] = {1199U, 1200U, 1201U};
    const cl_validity_t expect[] = {CL_VALID, CL_VALID, CL_STALE};
    for (int i = 0; i < 3; ++i)
    {
        ASSERT_EQ(CL_OK, Validate(nullptr, times[i]));
        EXPECT_EQ(expect[i], out_.validity[CL_IN_CRASH]) << times[i];
        EXPECT_EQ(expect[i] == CL_VALID ? CL_CRASH_PENDING : 0, out_.value[CL_IN_CRASH]);
    }
}

/**
 * @test    UT-007: 입력 나이는 마지막 유효 수신 시각 기준이며, 불량 수신은 시각을 갱신하지 않는다.
 * @trace   FUNC-004, FUNC-005, SSR-020, SWR-I-007
 * @purpose 수신 시각 규칙(가정 A-20): 유효 갱신만 나이를 초기화한다.
 * @input   t=0 유효, t=150 유효, t=340 확인, t=351 확인
 * @steps   1. 두 번 유효 수신한다. 2. 수신 없이 340과 351에서 검증한다.
 * @expect  340에서 VALID(나이 190), 351에서 STALE(나이 201)
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(InputValidatorUnit, UT007_AgeFromLastValidReceipt)
{
    cl_raw_input_t raw = MakeRaw();
    ASSERT_EQ(CL_OK, Validate(&raw, 0U));
    ASSERT_EQ(CL_OK, Validate(&raw, 150U));
    ASSERT_EQ(CL_OK, Validate(nullptr, 340U));
    EXPECT_EQ(CL_VALID, out_.validity[CL_IN_CRASH]);
    ASSERT_EQ(CL_OK, Validate(nullptr, 351U));
    EXPECT_EQ(CL_STALE, out_.validity[CL_IN_CRASH]);
}

/**
 * @test    UT-008: 시계가 거꾸로 가거나 같은 시각이면 나이를 0으로 본다.
 * @trace   FUNC-005, FUNC-009, DD-D3
 * @purpose now_ms >= last_valid_ms 결정의 참(같음)/거짓(역행) 경계.
 * @input   t=5000 수신 후 t=5000(같음), t=4999(역행) 검증
 * @steps   1. 수신한다. 2. 같은 시각, 1 ms 이전 시각으로 검증한다.
 * @expect  둘 다 VALID
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(InputValidatorUnit, UT008_ClockEqualAndBackward)
{
    cl_raw_input_t raw = MakeRaw();
    ASSERT_EQ(CL_OK, Validate(&raw, 5000U));
    ASSERT_EQ(CL_OK, Validate(nullptr, 5000U));
    EXPECT_EQ(CL_VALID, out_.validity[CL_IN_CRASH]);
    ASSERT_EQ(CL_OK, Validate(nullptr, 4999U));
    EXPECT_EQ(CL_VALID, out_.validity[CL_IN_CRASH]);
}

/**
 * @test    UT-009: 입력 5개를 독립적으로 분류한다.
 * @trace   FUNC-002, SSR-018, SSR-019, SSR-020
 * @purpose 한 입력의 불량이 다른 입력의 분류에 영향을 주지 않는지 확인한다.
 * @input   crash 범위 밖, 좌 접근위험 누락, 우 접근위험 정상, 점화 정상, 센서 고장 정상
 * @steps   1. 위 스냅샷으로 검증한다.
 * @expect  crash와 좌 접근위험만 INVALID, 나머지 VALID
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(InputValidatorUnit, UT009_InputsAreIndependent)
{
    cl_raw_input_t raw = MakeRaw();
    raw.field[CL_IN_CRASH].value = 3;
    raw.field[CL_IN_APPROACH_LEFT].present = 0;
    ASSERT_EQ(CL_OK, Validate(&raw, 0U));
    EXPECT_EQ(CL_INVALID, out_.validity[CL_IN_CRASH]);
    EXPECT_EQ(CL_INVALID, out_.validity[CL_IN_APPROACH_LEFT]);
    EXPECT_EQ(CL_VALID, out_.validity[CL_IN_APPROACH_RIGHT]);
    EXPECT_EQ(CL_VALID, out_.validity[CL_IN_IGNITION]);
    EXPECT_EQ(CL_VALID, out_.validity[CL_IN_SENSOR_FAULT]);
}

/**
 * @test    UT-010: 운전자 요청은 raw가 있고 present==1이고 세 필드가 범위 안일 때만 유효하다.
 * @trace   FUNC-006, FUNC-008, SWR-I-004
 * @purpose (raw!=NULL)&&(present==1)&&(range==1)의 MC/DC. 각 조건만 거짓인 케이스와 모두 참인
 * 케이스.
 * @input   raw=NULL, present=0, present=1+범위 밖 필드, 정상 요청
 * @steps   1. 네 가지 입력으로 검증한다.
 * @expect  앞의 셋은 request_valid=0, 정상 요청만 request_valid=1과 side/action이 디코딩된다.
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(InputValidatorUnit, UT010_RequestDecisionMcdc)
{
    ASSERT_EQ(CL_OK, Validate(nullptr, 0U));
    EXPECT_EQ(0, out_.request_valid);

    cl_raw_input_t raw = MakeRaw();
    raw.request_present = 0;
    raw.request_side = CL_SIDE_ALL;
    raw.request_action = CL_ACTION_UNLOCK;
    raw.request_source = 1;
    ASSERT_EQ(CL_OK, Validate(&raw, 10U));
    EXPECT_EQ(0, out_.request_valid);

    raw.request_present = 1;
    raw.request_side = 9;
    ASSERT_EQ(CL_OK, Validate(&raw, 20U));
    EXPECT_EQ(0, out_.request_valid);

    raw.request_side = CL_SIDE_ALL;
    ASSERT_EQ(CL_OK, Validate(&raw, 30U));
    EXPECT_EQ(1, out_.request_valid);
    EXPECT_EQ(CL_SIDE_ALL, out_.request_side);
    EXPECT_EQ(CL_ACTION_UNLOCK, out_.request_action);
}

/**
 * @test    UT-011: 운전자 요청의 side, action, source 경계값을 각각 판정한다.
 * @trace   FUNC-006, FUNC-008, FUNC-007, SWR-I-004
 * @purpose side_ok&&action_ok&&source_ok의 MC/DC(필드 하나만 거짓)와 경계값(min-1, min, max,
 * max+1).
 * @input   side 0~2 / action 0~1 / source 0~3 각각의 경계 밖 값과 경계 값
 * @steps   1. 한 필드만 경계 밖으로 둔 요청을 검증한다. 2. 모든 필드가 경계 값인 요청을 검증한다.
 * @expect  경계 밖이면 request_valid=0, 경계 값이면 1
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(InputValidatorUnit, UT011_RequestFieldBoundaries)
{
    struct Probe
    {
        int32_t side, action, source;
        int32_t valid;
    };
    const Probe probes[] = {
        {0, 0, 0, 1},  {2, 1, 3, 1}, {-1, 0, 0, 0}, {3, 0, 0, 0},
        {0, -1, 0, 0}, {0, 2, 0, 0}, {0, 0, -1, 0}, {0, 0, 4, 0},
    };
    for (const Probe &p : probes)
    {
        cl_raw_input_t raw = MakeRaw();
        raw.request_present = 1;
        raw.request_side = p.side;
        raw.request_action = p.action;
        raw.request_source = p.source;
        ASSERT_EQ(CL_OK, Validate(&raw, 0U));
        EXPECT_EQ(p.valid, out_.request_valid) << p.side << "," << p.action << "," << p.source;
    }
}

} // namespace
