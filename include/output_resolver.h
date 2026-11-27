/**
 * @file    output_resolver.h
 * @brief   안전 지시와 QM 제안을 합성해 최종 출력, 상태, 이유코드를 정한다.
 *          (MOD-004, ARCH-004, IF-004)
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
#ifndef OUTPUT_RESOLVER_H
#define OUTPUT_RESOLVER_H

#include "cl_types.h"
#include "ignition_policy.h"
#include "safety_arbiter.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief   최종 출력, 상태, 이유코드를 정한다. (FUNC-030)
     * @param   decision  안전 판단 결과
     * @param   proposal  QM 해제 제안
     * @param   in        검증된 입력 (운전자 요청, 입력 유효성)
     * @param   previous  직전 확정 출력 (도어 2개)
     * @param   out       최종 결과 (출력)
     * @return  CL_OK, CL_ERR_NULL_ARG (이때 out은 바뀌지 않는다)
     *
     * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
     * @generated_at   2026-11-27
     * @requested_by   jhpark0126
     * @human_review   pending
     * @reviewed_by    -
     * @reviewed_at    -
     */
    cl_status_t output_resolver_resolve(const safety_decision_t *decision,
                                        const ignition_proposal_t *proposal,
                                        const cl_validated_t *in,
                                        const cl_lock_t previous[CL_DOOR_COUNT], cl_result_t *out);

#ifdef __cplusplus
}
#endif

#endif /* OUTPUT_RESOLVER_H */
