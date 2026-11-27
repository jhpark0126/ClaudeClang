/**
 * @file limiter.c
 * @brief Limiter 모듈 구현. (CI 자가 검증용 샘플)
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
#include "limiter.h"

#include <stddef.h>

Status Limiter_Clamp(int32_t value, int32_t low, int32_t high, int32_t *out)
{
    Status status = STATUS_OK;

    if (out == NULL) {
        status = STATUS_ERR_NULL_PTR;
    } else if (low > high) {
        status = STATUS_ERR_RANGE;
    } else if ((value >= low) && (value <= high)) {
        *out = value;
    } else if (value < low) {
        *out = low;
    } else {
        *out = high;
    }

    return status;
}
