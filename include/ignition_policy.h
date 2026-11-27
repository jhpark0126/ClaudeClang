/**
 * @file    ignition_policy.h
 * @brief   점화 OFF에 따른 QM 해제 제안을 만든다. (MOD-003, ARCH-003, IF-003)
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
#ifndef IGNITION_POLICY_H
#define IGNITION_POLICY_H

#include "cl_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /** QM 해제 제안. */
    typedef struct
    {
        int32_t release_all;
    } ignition_proposal_t;

    /**
     * @brief   유효한 점화 OFF에서 전체 해제를 제안한다. (FUNC-020)
     * @param   in   검증된 입력
     * @param   out  제안 (출력)
     * @return  CL_OK, CL_ERR_NULL_ARG (이때 out은 바뀌지 않는다)
     *
     * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
     * @generated_at   2026-11-27
     * @requested_by   jhpark0126
     * @human_review   pending
     * @reviewed_by    -
     * @reviewed_at    -
     */
    cl_status_t ignition_policy_propose(const cl_validated_t *in, ignition_proposal_t *out);

#ifdef __cplusplus
}
#endif

#endif /* IGNITION_POLICY_H */
