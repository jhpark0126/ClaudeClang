/**
 * @file controller.h
 * @brief Limiter를 사용하는 제어 모듈. (CI 자가 검증용 샘플)
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdint.h>

#include "status.h"

#ifdef __cplusplus
extern "C"
{
#endif

/** @brief 제어 출력의 하한 */
#define CONTROLLER_MIN (0)
/** @brief 제어 출력의 상한 */
#define CONTROLLER_MAX (100)

    /**
     * @brief 입력을 제어 범위로 제한해 출력한다.
     * @param input  입력 값
     * @param output 출력 값을 돌려받을 포인터
     * @return STATUS_OK, STATUS_ERR_NULL_PTR(output이 NULL)
     *
     * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
     * @generated_at   2026-11-27
     * @requested_by   jhpark0126
     * @human_review   pending
     * @reviewed_by    -
     * @reviewed_at    -
     */
    Status Controller_Step(int32_t input, int32_t *output);

#ifdef __cplusplus
}
#endif

#endif /* CONTROLLER_H */
