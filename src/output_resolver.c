/**
 * @file    output_resolver.c
 * @brief   안전 지시와 QM 제안을 합성해 최종 출력, 상태, 이유코드를 정한다.
 *          (MOD-004, ARCH-004)
 *
 * 설계: docs/design/detailed-design.md 7장. FUNC-030~034. 상태가 없는 순수 함수다.
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
#include "output_resolver.h"

/**
 * @brief   한 도어의 출력을 정한다. (FUNC-031)
 * @param   directive    안전 지시
 * @param   release_all  점화 OFF 해제가 적용되는지(1/0)
 * @param   previous     직전 확정 출력
 * @return  도어 출력
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
static cl_lock_t resolver_door_output(cl_directive_t directive, int32_t release_all,
                                      cl_lock_t previous)
{
    cl_lock_t lock = previous;

    if (directive == CL_DIR_RELEASE)
    {
        lock = CL_RELEASE;
    }
    else if (directive == CL_DIR_LOCK)
    {
        lock = CL_LOCK;
    }
    else if ((directive == CL_DIR_FREE) && (release_all == 1))
    {
        lock = CL_RELEASE;
    }
    else
    {
        /* HOLD, 또는 제안이 없는 FREE: 직전 출력을 유지한다. */
    }

    return lock;
}

/**
 * @brief   점화 OFF 해제가 적용 가능한 주기인지 돌려준다. (FUNC-032)
 * @param   decision  안전 판단 결과
 * @param   proposal  QM 해제 제안
 * @return  제안이 있고 건전성이 NORMAL이면 1, 아니면 0
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
static int32_t resolver_ignition_applies(const safety_decision_t *decision,
                                         const ignition_proposal_t *proposal)
{
    int32_t applies = 0;

    if ((proposal->release_all == 1) && (decision->health == CL_HEALTH_NORMAL))
    {
        applies = 1;
    }

    return applies;
}

/**
 * @brief   운전자 요청이 그 도어를 대상으로 하는지 돌려준다.
 * @param   side  요청 대상
 * @param   door  도어 인덱스
 * @return  대상이면 1, 아니면 0
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
static int32_t resolver_request_targets(cl_side_t side, int32_t door)
{
    int32_t targets = 0;

    if ((side == CL_SIDE_ALL) || ((int32_t)side == door))
    {
        targets = 1;
    }

    return targets;
}

/**
 * @brief   해제 요청이 LOCK 지시 도어를 건드리면 억제 이유 비트를 돌려준다. (FUNC-033)
 * @param   decision  안전 판단 결과
 * @param   in        검증된 입력
 * @return  APPROACH_RISK_SUPPRESS 비트 또는 0
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
static uint32_t resolver_suppress_reason(const safety_decision_t *decision,
                                         const cl_validated_t *in)
{
    uint32_t reason = 0U;

    if ((in->request_valid == 1) && (in->request_action == CL_ACTION_UNLOCK))
    {
        for (int32_t door = 0; door < (int32_t)CL_DOOR_COUNT; door++)
        {
            if ((resolver_request_targets(in->request_side, door) == 1) &&
                (decision->directive[door] == CL_DIR_LOCK))
            {
                reason = CL_REASON_APPROACH_RISK_SUPPRESS;
            }
        }
    }

    return reason;
}

/**
 * @brief   상태 우선순위(FAULT > DEGRADED > OFF > NORMAL)로 상태를 정한다. (FUNC-034)
 * @param   health            시스템 건전성
 * @param   ignition_applies  점화 OFF 해제 적용 여부(1/0)
 * @return  공개 상태
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
static cl_state_t resolver_select_state(cl_health_t health, int32_t ignition_applies)
{
    cl_state_t state = CL_STATE_NORMAL;

    if (health == CL_HEALTH_FAULT)
    {
        state = CL_STATE_FAULT;
    }
    else if (health == CL_HEALTH_DEGRADED)
    {
        state = CL_STATE_DEGRADED;
    }
    else if (ignition_applies == 1)
    {
        state = CL_STATE_OFF;
    }
    else
    {
        /* NORMAL */
    }

    return state;
}

/**
 * @brief   안전 판단, 점화 적용, 건전성에서 이유코드를 합친다.
 * @param   decision          안전 판단 결과
 * @param   in                검증된 입력
 * @param   ignition_applies  점화 OFF 해제 적용 여부(1/0)
 * @return  이유 비트마스크
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
static uint32_t resolver_collect_reasons(const safety_decision_t *decision,
                                         const cl_validated_t *in, int32_t ignition_applies)
{
    uint32_t reason = decision->reason_mask | resolver_suppress_reason(decision, in);

    if (ignition_applies == 1)
    {
        reason |= CL_REASON_IGNITION_OFF;
    }
    if (decision->health == CL_HEALTH_FAULT)
    {
        reason |= CL_REASON_SENSOR_FAULT;
    }

    return reason;
}

cl_status_t output_resolver_resolve(const safety_decision_t *decision,
                                    const ignition_proposal_t *proposal, const cl_validated_t *in,
                                    const cl_lock_t previous[CL_DOOR_COUNT], cl_result_t *out)
{
    cl_status_t status = CL_OK;

    if ((decision == NULL) || (proposal == NULL) || (in == NULL) || (previous == NULL) ||
        (out == NULL))
    {
        status = CL_ERR_NULL_ARG;
    }
    else
    {
        int32_t applies = resolver_ignition_applies(decision, proposal);

        for (int32_t index = 0; index < (int32_t)CL_DOOR_COUNT; index++)
        {
            out->lock[index] =
                resolver_door_output(decision->directive[index], applies, previous[index]);
        }
        out->state = resolver_select_state(decision->health, applies);
        out->reason_mask = resolver_collect_reasons(decision, in, applies);
        for (int32_t index = 0; index < (int32_t)CL_IN_COUNT; index++)
        {
            out->validity[index] = in->validity[index];
        }
    }

    return status;
}
