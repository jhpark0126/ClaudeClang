/**
 * @file    ignition_policy.c
 * @brief   점화 OFF에 따른 QM 해제 제안을 만든다. (MOD-003, ARCH-003)
 *
 * 설계: docs/design/detailed-design.md 6장. FUNC-020. 상태가 없는 순수 함수다.
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
#include "ignition_policy.h"

cl_status_t ignition_policy_propose(const cl_validated_t *in, ignition_proposal_t *out)
{
    cl_status_t status = CL_OK;

    if ((in == NULL) || (out == NULL))
    {
        status = CL_ERR_NULL_ARG;
    }
    else
    {
        out->release_all = 0;
        if ((in->validity[CL_IN_IGNITION] == CL_VALID) && (in->value[CL_IN_IGNITION] == 0))
        {
            out->release_all = 1;
        }
    }

    return status;
}
