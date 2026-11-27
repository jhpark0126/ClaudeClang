/**
 * @file    input_validator.h
 * @brief   입력 스냅샷을 입력별 VALID/INVALID/STALE로 분류한다. (MOD-001, ARCH-001, IF-001)
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
#ifndef INPUT_VALIDATOR_H
#define INPUT_VALIDATOR_H

#include "cl_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /** 입력 하나의 검증 상태. 필드는 input_validator만 읽고 쓴다. */
    typedef struct
    {
        int32_t last_value;
        uint32_t last_valid_ms;
        int32_t has_valid;
        int32_t last_was_bad;
    } input_entry_t;

    /** 검증기 상태. 필드는 input_validator만 읽고 쓴다. */
    typedef struct
    {
        input_entry_t entry[CL_IN_COUNT];
    } input_validator_state_t;

    /**
     * @brief   검증기 상태를 "유효값 없음"으로 초기화한다. (FUNC-001)
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
    cl_status_t input_validator_init(input_validator_state_t *state);

    /**
     * @brief   한 주기의 입력을 분류한다. (FUNC-002)
     * @param   state   검증기 상태 (입출력)
     * @param   raw     이번 주기에 수신한 스냅샷. 수신이 없으면 NULL
     * @param   now_ms  단조 증가하는 SW 시계 [ms]
     * @param   out     검증 결과 (출력)
     * @return  CL_OK, CL_ERR_NULL_ARG (state 또는 out이 NULL, 이때 out은 바뀌지 않는다)
     *
     * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
     * @generated_at   2026-11-27
     * @requested_by   jhpark0126
     * @human_review   pending
     * @reviewed_by    -
     * @reviewed_at    -
     */
    cl_status_t input_validator_validate(input_validator_state_t *state, const cl_raw_input_t *raw,
                                         uint32_t now_ms, cl_validated_t *out);

#ifdef __cplusplus
}
#endif

#endif /* INPUT_VALIDATOR_H */
