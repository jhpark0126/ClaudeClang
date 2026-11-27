/**
 * @file    test_child_lock_unit.cpp
 * @brief   child_lock 단위 테스트 (SWE.4). 하위 모듈을 실제 구현으로 두고 조율 동작만 확인한다.
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
#include <cstring>

#include "unit_helpers.hpp"

extern "C"
{
#include "child_lock.h"
}

namespace
{

using unit::MakeRaw;

class ChildLockUnit : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        ASSERT_EQ(CL_OK, child_lock_init(&ctx_));
    }
    cl_ctx_t ctx_ = {};
    cl_result_t out_ = {};
};

/**
 * @test    UT-050: NULL 인자는 CL_ERR_NULL_ARG를 반환하고 직전 출력과 out을 바꾸지 않는다.
 * @trace   FUNC-040, FUNC-041, SWR-I-005
 * @purpose (ctx!=NULL)&&(out!=NULL)의 MC/DC. 각 포인터만 NULL인 쌍과 모두 정상. init(NULL).
 * @input   init(NULL), step(ctx만 NULL), step(out만 NULL), 정상 호출
 * @steps   1. 먼저 좌측 LOCK 출력을 만든다. 2. NULL 조합으로 호출한다. 3. 직전 출력을 확인한다.
 * @expect  NULL이 있으면 CL_ERR_NULL_ARG, ctx->previous와 out 불변. 정상은 CL_OK
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(ChildLockUnit, UT050_NullArguments)
{
    cl_raw_input_t raw = MakeRaw();
    raw.field[CL_IN_APPROACH_LEFT].value = 1;
    ASSERT_EQ(CL_OK, child_lock_step(&ctx_, &raw, 0U, &out_));
    ASSERT_EQ(CL_LOCK, ctx_.previous[CL_DOOR_LEFT]);
    cl_result_t sentinel = {};
    sentinel.reason_mask = 0x1234U;
    out_ = sentinel;
    EXPECT_EQ(CL_ERR_NULL_ARG, child_lock_init(nullptr));
    EXPECT_EQ(CL_ERR_NULL_ARG, child_lock_step(nullptr, &raw, 10U, &out_));
    EXPECT_EQ(CL_ERR_NULL_ARG, child_lock_step(&ctx_, &raw, 10U, nullptr));
    EXPECT_EQ(0x1234U, out_.reason_mask);
    EXPECT_EQ(CL_LOCK, ctx_.previous[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_OK, child_lock_step(&ctx_, &raw, 10U, &out_));
}

/**
 * @test    UT-051: init은 사용 중인 컨텍스트도 초기 상태(양쪽 RELEASE, 래치 없음, 유효값 없음)로
 * 되돌린다.
 * @trace   FUNC-040, SWR-F-005, SSR-002
 * @purpose 초기화 경로와 초기 출력. 래치, 직전 출력, 입력 상태가 모두 지워지는지 본다.
 * @input   CONFIRMED와 접근위험을 처리한 컨텍스트에 init
 * @steps   1. 처리 후 init한다. 2. 입력 없이 한 번 평가한다.
 * @expect  양쪽 RELEASE, 상태 DEGRADED, 모든 입력 INVALID
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(ChildLockUnit, UT051_InitResetsEverything)
{
    cl_raw_input_t raw = MakeRaw();
    raw.field[CL_IN_CRASH].value = CL_CRASH_CONFIRMED;
    raw.field[CL_IN_APPROACH_LEFT].value = 1;
    ASSERT_EQ(CL_OK, child_lock_step(&ctx_, &raw, 0U, &out_));
    ASSERT_EQ(CL_OK, child_lock_init(&ctx_));
    ASSERT_EQ(CL_OK, child_lock_step(&ctx_, nullptr, 10U, &out_));
    EXPECT_EQ(CL_RELEASE, out_.lock[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_RELEASE, out_.lock[CL_DOOR_RIGHT]);
    EXPECT_EQ(CL_STATE_DEGRADED, out_.state);
    for (int i = 0; i < CL_IN_COUNT; ++i)
    {
        EXPECT_EQ(CL_INVALID, out_.validity[i]);
    }
}

/**
 * @test    UT-052: 한 주기 동안 검증, 안전 판단, 점화 제안, 합성이 순서대로 연결된다.
 * @trace   FUNC-041, FUNC-043, SWR-I-005, SWR-I-006a, SWR-I-006b, SSR-009, SSR-011, SSR-012
 * @purpose 컴포넌트 연결(검증 결과 → 판단 → 합성)과 직전 출력 저장 확인.
 * @input   좌 접근위험 TRUE + 좌 해제 요청, 이어서 접근위험 FALSE와 같은 요청
 * @steps   1. 첫 주기를 실행한다. 2. 두 번째 주기를 실행한다.
 * @expect  첫 주기 좌 LOCK+억제 이유+LOCK 이유, 두 번째 주기 좌 LOCK, 우 RELEASE 유지(도어별 직전
 * 출력 저장)
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(ChildLockUnit, UT052_PipelineAndCommit)
{
    cl_raw_input_t raw = MakeRaw();
    raw.field[CL_IN_APPROACH_LEFT].value = 1;
    raw.request_present = 1;
    raw.request_side = CL_SIDE_LEFT;
    raw.request_action = CL_ACTION_UNLOCK;
    raw.request_source = 0;
    ASSERT_EQ(CL_OK, child_lock_step(&ctx_, &raw, 0U, &out_));
    EXPECT_EQ(CL_LOCK, out_.lock[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_RELEASE, out_.lock[CL_DOOR_RIGHT]);
    EXPECT_NE(0U, out_.reason_mask & CL_REASON_APPROACH_RISK_SUPPRESS);
    EXPECT_NE(0U, out_.reason_mask & CL_REASON_APPROACH_RISK_LOCK);
    EXPECT_EQ(CL_STATE_NORMAL, out_.state);
    raw.field[CL_IN_APPROACH_LEFT].value = 0;
    ASSERT_EQ(CL_OK, child_lock_step(&ctx_, &raw, 10U, &out_));
    EXPECT_EQ(CL_LOCK, out_.lock[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_RELEASE, out_.lock[CL_DOOR_RIGHT]);
}

/**
 * @test    UT-053: 입력 끊김 시나리오: 마지막 수신 후 200 ms를 넘긴 첫 평가주기에 DEGRADED가 된다.
 * @trace   FUNC-041, SSR-020, SSR-021, SSR-022, SWR-Q-002
 * @purpose now_ms 주입으로 200/210 ms 경계를 재현한다. (평가주기 10 ms)
 * @input   t=1000 수신(좌 접근위험 TRUE) 후 수신 없이 t=1200, 1210
 * @steps   1. 수신한다. 2. t=1200과 t=1210에 NULL 스냅샷으로 평가한다.
 * @expect  t=1200은 NORMAL, t=1210은 DEGRADED이고 좌 LOCK 유지, crash 입력 STALE
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(ChildLockUnit, UT053_StaleAfterTwoHundredMilliseconds)
{
    cl_raw_input_t raw = MakeRaw();
    raw.field[CL_IN_APPROACH_LEFT].value = 1;
    ASSERT_EQ(CL_OK, child_lock_step(&ctx_, &raw, 1000U, &out_));
    ASSERT_EQ(CL_OK, child_lock_step(&ctx_, nullptr, 1200U, &out_));
    EXPECT_EQ(CL_STATE_NORMAL, out_.state);
    ASSERT_EQ(CL_OK, child_lock_step(&ctx_, nullptr, 1210U, &out_));
    EXPECT_EQ(CL_STATE_DEGRADED, out_.state);
    EXPECT_EQ(CL_LOCK, out_.lock[CL_DOOR_LEFT]);
    EXPECT_EQ(CL_STALE, out_.validity[CL_IN_CRASH]);
}

/**
 * @test    UT-054: 같은 입력 열은 같은 결과 열을 만든다. (결정론)
 * @trace   FUNC-041, SWR-Q-003
 * @purpose 두 컨텍스트에 같은 입력 열을 주었을 때 결과가 모두 같은지 확인한다.
 * @input   10 ms 간격 20주기, 접근위험·점화·센서 고장·충돌 값이 번갈아 바뀜
 * @steps   1. 두 컨텍스트를 같은 순서로 평가한다. 2. 주기마다 결과를 비교한다.
 * @expect  출력, 상태, 이유 비트, 유효성이 매 주기 같다.
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST_F(ChildLockUnit, UT054_Determinism)
{
    cl_ctx_t other = {};
    ASSERT_EQ(CL_OK, child_lock_init(&other));
    for (uint32_t step = 0U; step < 20U; ++step)
    {
        cl_raw_input_t raw = MakeRaw();
        raw.field[CL_IN_APPROACH_LEFT].value = static_cast<int32_t>(step % 2U);
        raw.field[CL_IN_IGNITION].value = static_cast<int32_t>((step / 3U) % 2U);
        raw.field[CL_IN_SENSOR_FAULT].value = static_cast<int32_t>((step / 5U) % 2U);
        raw.field[CL_IN_CRASH].value = (step == 15U) ? CL_CRASH_CONFIRMED : CL_CRASH_NONE;
        cl_result_t a = {};
        cl_result_t b = {};
        ASSERT_EQ(CL_OK, child_lock_step(&ctx_, &raw, step * 10U, &a));
        ASSERT_EQ(CL_OK, child_lock_step(&other, &raw, step * 10U, &b));
        EXPECT_EQ(0, std::memcmp(&a, &b, sizeof(a))) << step;
    }
}

} // namespace
