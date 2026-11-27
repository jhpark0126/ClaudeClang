/**
 * @file limiter_unit_test.cpp
 * @brief Limiter_Clamp 단위 테스트. MC/DC 100%를 목표로 한다.
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
#include <gtest/gtest.h>

#include "controller.h"
#include "limiter.h"

/**
 * @test    UT-001: 범위 안의 값은 그대로 반환된다.
 * @trace   FUNC-001
 * @purpose 정상 경로 확인 (동치 클래스: 범위 안)
 * @input   value=50, low=0, high=100
 * @steps   1. Limiter_Clamp를 호출한다.
 * @expect  STATUS_OK, out == 50
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST(LimiterUnit, InRangeReturnsValue)
{
    int32_t out = -1;
    EXPECT_EQ(STATUS_OK, Limiter_Clamp(50, 0, 100, &out));
    EXPECT_EQ(50, out);
}

/**
 * @test    UT-002: 하한 미만이면 하한으로 제한된다.
 * @trace   FUNC-001
 * @purpose 경계값 확인 (low-1)
 * @input   value=-1, low=0, high=100
 * @steps   1. Limiter_Clamp를 호출한다.
 * @expect  STATUS_OK, out == 0
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST(LimiterUnit, BelowLowIsClampedToLow)
{
    int32_t out = -1;
    EXPECT_EQ(STATUS_OK, Limiter_Clamp(-1, 0, 100, &out));
    EXPECT_EQ(0, out);
}

/**
 * @test    UT-003: 상한 초과이면 상한으로 제한된다.
 * @trace   FUNC-001
 * @purpose 경계값 확인 (high+1)
 * @input   value=101, low=0, high=100
 * @steps   1. Limiter_Clamp를 호출한다.
 * @expect  STATUS_OK, out == 100
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST(LimiterUnit, AboveHighIsClampedToHigh)
{
    int32_t out = -1;
    EXPECT_EQ(STATUS_OK, Limiter_Clamp(101, 0, 100, &out));
    EXPECT_EQ(100, out);
}

/**
 * @test    UT-004: out이 NULL이면 STATUS_ERR_NULL_PTR를 반환한다.
 * @trace   FUNC-001
 * @purpose 오류 경로 확인 (동치 클래스: 무효 포인터)
 * @input   out=NULL
 * @steps   1. Limiter_Clamp를 호출한다.
 * @expect  STATUS_ERR_NULL_PTR
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST(LimiterUnit, NullOutIsRejected)
{
    EXPECT_EQ(STATUS_ERR_NULL_PTR, Limiter_Clamp(50, 0, 100, nullptr));
}

/**
 * @test    UT-005: low가 high보다 크면 STATUS_ERR_RANGE를 반환하고 out은 바뀌지 않는다.
 * @trace   FUNC-001
 * @purpose 오류 경로 확인 (동치 클래스: 무효 범위)
 * @input   low=10, high=0
 * @steps   1. Limiter_Clamp를 호출한다.
 * @expect  STATUS_ERR_RANGE, out은 초기값 유지
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST(LimiterUnit, InvertedRangeIsRejected)
{
    int32_t out = 7;
    EXPECT_EQ(STATUS_ERR_RANGE, Limiter_Clamp(5, 10, 0, &out));
    EXPECT_EQ(7, out);
}

/**
 * @test    UT-006: Controller_Step이 NULL 출력과 정상 출력을 구분한다.
 * @trace   FUNC-002
 * @purpose 두 분기(NULL / 정상) 확인
 * @input   output=NULL 과 유효한 포인터
 * @steps   1. 두 경우로 Controller_Step을 호출한다.
 * @expect  STATUS_ERR_NULL_PTR 와 STATUS_OK
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST(ControllerUnit, NullAndValidOutput)
{
    int32_t out = -1;
    EXPECT_EQ(STATUS_ERR_NULL_PTR, Controller_Step(10, nullptr));
    EXPECT_EQ(STATUS_OK, Controller_Step(10, &out));
    EXPECT_EQ(10, out);
}
