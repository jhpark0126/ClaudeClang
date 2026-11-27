/**
 * @file    safety_arbiter.h
 * @brief   도어별 안전 지시와 시스템 건전성을 정한다. (MOD-002, ARCH-002, IF-002)
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
#ifndef SAFETY_ARBITER_H
#define SAFETY_ARBITER_H

#include "cl_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /** 안전 판단기 상태. 필드는 safety_arbiter만 읽고 쓴다. */
    typedef struct
    {
        int32_t crash_latched;
    } safety_arbiter_state_t;

    /** 안전 판단 결과. */
    typedef struct
    {
        cl_directive_t directive[CL_DOOR_COUNT];
        cl_health_t health;
        uint32_t reason_mask;
    } safety_decision_t;

    /**
     * @brief   충돌 래치를 해제 상태로 만든다. (FUNC-010)
     * @param   state  초기화할 상태
     * @return  CL_OK, CL_ERR_NULL_ARG
     *
     * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
     * @generated_at   2026-11-27
     * @requested_by   jhpark0126
     * @human_review   pending
     * @reviewed_by    -
     * @reviewed_at    -
     */
    cl_status_t safety_arbiter_init(safety_arbiter_state_t *state);

    /**
     * @brief   건전성과 도어별 지시를 정한다. (FUNC-011)
     * @param   state  안전 판단기 상태 (입출력)
     * @param   in     검증된 입력
     * @param   out    판단 결과 (출력)
     * @return  CL_OK, CL_ERR_NULL_ARG (이때 out은 바뀌지 않는다)
     *
     * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
     * @generated_at   2026-11-27
     * @requested_by   jhpark0126
     * @human_review   pending
     * @reviewed_by    -
     * @reviewed_at    -
     */
    cl_status_t safety_arbiter_decide(safety_arbiter_state_t *state, const cl_validated_t *in,
                                      safety_decision_t *out);

#ifdef __cplusplus
}
#endif

#endif /* SAFETY_ARBITER_H */
