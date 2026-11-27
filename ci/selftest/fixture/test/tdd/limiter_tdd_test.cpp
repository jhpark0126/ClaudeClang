/**
 * @file limiter_tdd_test.cpp
 * @brief 구현 중 TDD로 만든 테스트. 단위 테스트와 분리하여 별도 결과·커버리지로 관리한다.
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
#include <gtest/gtest.h>

#include "limiter.h"

/**
 * @test    TDD-001: 하한과 같은 값은 그대로 통과한다.
 * @trace   FUNC-001
 * @purpose TDD 사이클에서 먼저 작성한 경계 테스트
 * @input   value=0, low=0, high=100
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
TEST(LimiterTdd, ValueEqualToLowPasses)
{
    int32_t out = -1;
    EXPECT_EQ(STATUS_OK, Limiter_Clamp(0, 0, 100, &out));
    EXPECT_EQ(0, out);
}
