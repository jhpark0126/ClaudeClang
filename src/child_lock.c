/**
 * @file    child_lock.c
 * @brief   평가주기 1회의 실행 순서를 조율하고 외부 제어 인터페이스를 제공한다.
 *          (MOD-005, ARCH-005)
 *
 * 설계: docs/design/detailed-design.md 8장. FUNC-040~042.
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
#include "child_lock.h"

#include "ignition_policy.h"
#include "input_validator.h"
#include "output_resolver.h"
#include "safety_arbiter.h"

/**
 * @brief   결과를 직전 확정 출력으로 저장한다. (FUNC-042)
 * @param   ctx     제어 컨텍스트 (입출력)
 * @param   result  이번 주기 결과
 * @return  CL_OK
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
static cl_status_t child_lock_commit(cl_ctx_t *ctx, const cl_result_t *result)
{
    for (int32_t door = 0; door < (int32_t)CL_DOOR_COUNT; door++)
    {
        ctx->previous[door] = result->lock[door];
    }

    return CL_OK;
}

/**
 * @brief   검증, 안전 판단, 점화 제안, 합성을 순서대로 실행한다. 한 단계가 실패하면 멈춘다.
 * @param   ctx     제어 컨텍스트 (입출력)
 * @param   raw     이번 주기 스냅샷. 수신이 없으면 NULL
 * @param   now_ms  SW 시계 [ms]
 * @param   result  이번 주기 결과 (출력)
 * @return  CL_OK 또는 첫 번째로 실패한 단계의 상태 코드
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
static cl_status_t child_lock_run_pipeline(cl_ctx_t *ctx, const cl_raw_input_t *raw,
                                           uint32_t now_ms, cl_result_t *result)
{
    cl_validated_t validated;
    safety_decision_t decision;
    ignition_proposal_t proposal;
    cl_status_t status = input_validator_validate(&ctx->validator, raw, now_ms, &validated);

    if (status == CL_OK)
    {
        status = safety_arbiter_decide(&ctx->arbiter, &validated, &decision);
    }
    if (status == CL_OK)
    {
        status = ignition_policy_propose(&validated, &proposal);
    }
    if (status == CL_OK)
    {
        status = output_resolver_resolve(&decision, &proposal, &validated, ctx->previous, result);
    }

    return status;
}

cl_status_t child_lock_init(cl_ctx_t *ctx)
{
    cl_status_t status = CL_OK;

    if (ctx == NULL)
    {
        status = CL_ERR_NULL_ARG;
    }
    else
    {
        status = input_validator_init(&ctx->validator);
        if (status == CL_OK)
        {
            status = safety_arbiter_init(&ctx->arbiter);
        }
        if (status == CL_OK)
        {
            for (int32_t door = 0; door < (int32_t)CL_DOOR_COUNT; door++)
            {
                ctx->previous[door] = CL_RELEASE;
            }
        }
    }

    return status;
}

cl_status_t child_lock_step(cl_ctx_t *ctx, const cl_raw_input_t *raw, uint32_t now_ms,
                            cl_result_t *out)
{
    cl_status_t status = CL_OK;
    cl_result_t result;

    if ((ctx == NULL) || (out == NULL))
    {
        status = CL_ERR_NULL_ARG;
    }
    else
    {
        status = child_lock_run_pipeline(ctx, raw, now_ms, &result);
        if (status == CL_OK)
        {
            *out = result;
            status = child_lock_commit(ctx, &result);
        }
    }

    return status;
}
