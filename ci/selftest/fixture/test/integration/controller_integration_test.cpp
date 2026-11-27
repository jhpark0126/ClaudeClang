/**
 * @file controller_integration_test.cpp
 * @brief Controller와 Limiter의 연동 통합 테스트. 함수 커버리지 100%를 목표로 한다.
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

/**
 * @test    IT-001: Controller가 Limiter 인터페이스를 통해 입력을 제어 범위로 제한한다.
 * @trace   IF-001, ARCH-001
 * @purpose Controller→Limiter 데이터 흐름 확인 (경계값: 상한 초과)
 * @input   input=1000
 * @steps   1. 실제 Controller와 Limiter를 연결해 Controller_Step을 호출한다.
 * @expect  STATUS_OK, output == CONTROLLER_MAX
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST(ControllerIntegration, ClampsThroughLimiterInterface)
{
    int32_t output = -1;
    EXPECT_EQ(STATUS_OK, Controller_Step(1000, &output));
    EXPECT_EQ(CONTROLLER_MAX, output);
}
