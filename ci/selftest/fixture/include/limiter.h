/**
 * @file limiter.h
 * @brief 값을 [low, high] 범위로 제한하는 모듈. (CI 자가 검증용 샘플)
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
#ifndef LIMITER_H
#define LIMITER_H

#include <stdint.h>

#include "status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 값을 [low, high] 범위로 제한한다.
 * @param value 입력 값
 * @param low   하한 (high 이하여야 한다)
 * @param high  상한
 * @param out   제한된 값을 돌려받을 포인터
 * @return STATUS_OK, STATUS_ERR_NULL_PTR(out이 NULL), STATUS_ERR_RANGE(low > high)
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
Status Limiter_Clamp(int32_t value, int32_t low, int32_t high, int32_t *out);

#ifdef __cplusplus
}
#endif

#endif /* LIMITER_H */
