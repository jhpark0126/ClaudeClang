/**
 * @file    safety_arbiter.c
 * @brief   도어별 안전 지시와 시스템 건전성을 정한다. (MOD-002, ARCH-002)
 *
 * 설계: docs/design/detailed-design.md 5장. FUNC-010~014.
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
#include "safety_arbiter.h"

/**
 * @brief   충돌 상태를 반영해 래치를 갱신하고 이유 비트를 돌려준다. (FUNC-012)
 * @param   state  안전 판단기 상태 (입출력)
 * @param   in     검증된 입력
 * @return  이유 비트마스크 (CRASH_PENDING 또는 0)
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
static uint32_t arbiter_update_crash_latch(safety_arbiter_state_t *state, const cl_validated_t *in)
{
    uint32_t reason = 0U;

    if (in->validity[CL_IN_CRASH] == CL_VALID)
    {
        if (in->value[CL_IN_CRASH] == (int32_t)CL_CRASH_CONFIRMED)
        {
            state->crash_latched = 1;
        }
        else if (in->value[CL_IN_CRASH] == (int32_t)CL_CRASH_PENDING)
        {
            reason = CL_REASON_CRASH_PENDING;
        }
        else
        {
            /* NONE: 래치와 이유를 바꾸지 않는다. */
        }
    }

    return reason;
}

/**
 * @brief   필수 입력 중 VALID가 아닌 것이 있는지 1/0으로 돌려준다.
 * @param   in  검증된 입력
 * @return  INVALID 또는 STALE이 하나라도 있으면 1, 아니면 0
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
static int32_t arbiter_any_input_unreliable(const cl_validated_t *in)
{
    int32_t unreliable = 0;
    int32_t index = 0;

    for (index = 0; index < (int32_t)CL_IN_COUNT; index++)
    {
        if (in->validity[index] != CL_VALID)
        {
            unreliable = 1;
        }
    }

    return unreliable;
}

/**
 * @brief   건전성(FAULT/DEGRADED/NORMAL)을 판정한다. (FUNC-013)
 * @param   in  검증된 입력
 * @return  CL_HEALTH_FAULT, CL_HEALTH_DEGRADED, CL_HEALTH_NORMAL
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
static cl_health_t arbiter_compute_health(const cl_validated_t *in)
{
    cl_health_t health = CL_HEALTH_NORMAL;

    if ((in->validity[CL_IN_SENSOR_FAULT] == CL_VALID) && (in->value[CL_IN_SENSOR_FAULT] == 1))
    {
        health = CL_HEALTH_FAULT;
    }
    else if (arbiter_any_input_unreliable(in) == 1)
    {
        health = CL_HEALTH_DEGRADED;
    }
    else
    {
        /* NORMAL */
    }

    return health;
}

/**
 * @brief   한 도어의 지시를 우선순위로 정한다. (FUNC-014)
 * @param   crash_latched   충돌 래치(1이면 세워짐)
 * @param   in              검증된 입력
 * @param   approach_input  그 도어의 접근위험 입력 인덱스
 * @param   health          시스템 건전성
 * @param   reason_mask     이유 비트를 더할 마스크 (입출력)
 * @return  도어 지시
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
static cl_directive_t arbiter_decide_door(int32_t crash_latched, const cl_validated_t *in,
                                          cl_input_id_t approach_input, cl_health_t health,
                                          uint32_t *reason_mask)
{
    cl_directive_t directive = CL_DIR_FREE;

    if (crash_latched == 1)
    {
        directive = CL_DIR_RELEASE;
    }
    else if ((in->validity[approach_input] == CL_VALID) && (in->value[approach_input] == 1))
    {
        directive = CL_DIR_LOCK;
        *reason_mask |= CL_REASON_APPROACH_RISK_LOCK;
    }
    else if (in->validity[approach_input] != CL_VALID)
    {
        directive = CL_DIR_HOLD;
        *reason_mask |= CL_REASON_APPROACH_INPUT_UNRELIABLE;
    }
    else if (health != CL_HEALTH_NORMAL)
    {
        directive = CL_DIR_HOLD;
    }
    else
    {
        /* FREE */
    }

    return directive;
}

cl_status_t safety_arbiter_init(safety_arbiter_state_t *state)
{
    cl_status_t status = CL_OK;

    if (state == NULL)
    {
        status = CL_ERR_NULL_ARG;
    }
    else
    {
        state->crash_latched = 0;
    }

    return status;
}

cl_status_t safety_arbiter_decide(safety_arbiter_state_t *state, const cl_validated_t *in,
                                  safety_decision_t *out)
{
    cl_status_t status = CL_OK;

    if ((state == NULL) || (in == NULL) || (out == NULL))
    {
        status = CL_ERR_NULL_ARG;
    }
    else
    {
        uint32_t reason = arbiter_update_crash_latch(state, in);
        cl_health_t health = arbiter_compute_health(in);

        out->directive[CL_DOOR_LEFT] =
            arbiter_decide_door(state->crash_latched, in, CL_IN_APPROACH_LEFT, health, &reason);
        out->directive[CL_DOOR_RIGHT] =
            arbiter_decide_door(state->crash_latched, in, CL_IN_APPROACH_RIGHT, health, &reason);
        out->health = health;
        out->reason_mask = reason;
    }

    return status;
}
