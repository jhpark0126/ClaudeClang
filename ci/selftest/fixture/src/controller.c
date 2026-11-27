/**
 * @file controller.c
 * @brief Controller 모듈 구현. (CI 자가 검증용 샘플)
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
#include "controller.h"

#include <stddef.h>

#include "limiter.h"

Status Controller_Step(int32_t input, int32_t *output)
{
    Status status = STATUS_OK;

    if (output == NULL)
    {
        status = STATUS_ERR_NULL_PTR;
    }
    else
    {
        status = Limiter_Clamp(input, CONTROLLER_MIN, CONTROLLER_MAX, output);
    }

    return status;
}
