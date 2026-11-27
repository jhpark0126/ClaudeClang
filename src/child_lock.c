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

/*
 * 상태 코드는 CL_OK(0)와 CL_ERR_NULL_ARG(1)뿐이라 비트 OR로 합쳐도 의미가 유지된다.
 * 하위 모듈은 NULL 인자에서만 실패하고, 호출 전에 NULL을 걸러내므로 단계별 실패 분기는
 * 도달할 수 없다. 그래서 상태를 분기 없이 합산한다. (DD-D5)
 */
_Static_assert((CL_OK == 0) && (CL_ERR_NULL_ARG == 1), "status codes must stay 0/1");

/**
 * @brief   검증, 안전 판단, 점화 제안, 합성을 순서대로 실행한다.
 * @param   ctx     제어 컨텍스트 (입출력)
 * @param   raw     이번 주기 스냅샷. 수신이 없으면 NULL
 * @param   now_ms  SW 시계 [ms]
 * @param   result  이번 주기 결과 (출력)
 * @return  CL_OK 또는 한 단계라도 실패했으면 CL_ERR_NULL_ARG
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
    cl_validated_t validated = {0};
    safety_decision_t decision = {0};
    ignition_proposal_t proposal = {0};
    uint32_t failures = 0U;

    failures |= (uint32_t)input_validator_validate(&ctx->validator, raw, now_ms, &validated);
    failures |= (uint32_t)safety_arbiter_decide(&ctx->arbiter, &validated, &decision);
    failures |= (uint32_t)ignition_policy_propose(&validated, &proposal);
    failures |=
        (uint32_t)output_resolver_resolve(&decision, &proposal, &validated, ctx->previous, result);

    return (cl_status_t)failures;
}

cl_status_t child_lock_init(cl_ctx_t *ctx)
{
    cl_status_t status = CL_ERR_NULL_ARG;

    if (ctx != NULL)
    {
        uint32_t failures = 0U;

        failures |= (uint32_t)input_validator_init(&ctx->validator);
        failures |= (uint32_t)safety_arbiter_init(&ctx->arbiter);
        for (int32_t door = 0; door < (int32_t)CL_DOOR_COUNT; door++)
        {
            ctx->previous[door] = CL_RELEASE;
        }
        status = (cl_status_t)failures;
    }

    return status;
}

cl_status_t child_lock_step(cl_ctx_t *ctx, const cl_raw_input_t *raw, uint32_t now_ms,
                            cl_result_t *out)
{
    cl_status_t status = CL_ERR_NULL_ARG;

    if ((ctx != NULL) && (out != NULL))
    {
        cl_result_t result = {{CL_RELEASE, CL_RELEASE}, CL_STATE_NORMAL, 0U, {CL_VALID}};

        status = child_lock_run_pipeline(ctx, raw, now_ms, &result);
        *out = result;
        (void)child_lock_commit(ctx, &result); /* commit은 항상 CL_OK를 돌려준다. */
    }

    return status;
}
