/**
 * @file    child_lock.h
 * @brief   평가주기 1회의 실행 순서를 조율하고 외부 제어 인터페이스를 제공한다.
 *          (MOD-005, ARCH-005, IF-005)
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
#ifndef CHILD_LOCK_H
#define CHILD_LOCK_H

#include "cl_types.h"
#include "input_validator.h"
#include "safety_arbiter.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /** 제어 컨텍스트. 호출자가 소유하고, 필드는 child_lock과 각 하위 모듈만 해석한다. */
    typedef struct
    {
        input_validator_state_t validator;
        safety_arbiter_state_t arbiter;
        cl_lock_t previous[CL_DOOR_COUNT];
    } cl_ctx_t;

    /**
     * @brief   컨텍스트를 초기화한다. 직전 출력은 좌/우 RELEASE다. (FUNC-040)
     * @param   ctx  초기화할 컨텍스트
     * @return  CL_OK, CL_ERR_NULL_ARG
     *
     * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
     * @generated_at   2026-11-27
     * @requested_by   jhpark0126
     * @human_review   pending
     * @reviewed_by    -
     * @reviewed_at    -
     */
    cl_status_t child_lock_init(cl_ctx_t *ctx);

    /**
     * @brief   평가주기 1회를 실행한다. 호출자가 CL_EVAL_PERIOD_MS마다 호출한다. (FUNC-041)
     * @param   ctx     제어 컨텍스트 (입출력)
     * @param   raw     이번 주기에 수신한 스냅샷. 수신이 없으면 NULL
     * @param   now_ms  단조 증가하는 SW 시계 [ms]
     * @param   out     평가 결과 (출력)
     * @return  CL_OK, CL_ERR_NULL_ARG (이때 ctx의 직전 출력과 out은 바뀌지 않는다)
     *
     * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
     * @generated_at   2026-11-27
     * @requested_by   jhpark0126
     * @human_review   pending
     * @reviewed_by    -
     * @reviewed_at    -
     */
    cl_status_t child_lock_step(cl_ctx_t *ctx, const cl_raw_input_t *raw, uint32_t now_ms,
                                cl_result_t *out);

#ifdef __cplusplus
}
#endif

#endif /* CHILD_LOCK_H */
