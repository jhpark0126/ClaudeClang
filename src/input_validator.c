/**
 * @file    input_validator.c
 * @brief   입력 스냅샷을 입력별 VALID/INVALID/STALE로 분류한다. (MOD-001, ARCH-001)
 *
 * 설계: docs/design/detailed-design.md 4장. FUNC-001~006.
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
#include "input_validator.h"

/** 필수 입력별 허용 최댓값. 최솟값은 모두 0이다. (OEM-IF-002, 003, 009) */
static const int32_t k_max_value[CL_IN_COUNT] = {2, 1, 1, 1, 1};

/** 운전자 요청 side의 허용 최댓값. (OEM-IF-004) */
#define REQUEST_SIDE_MAX 2
/** 운전자 요청 action의 허용 최댓값. (OEM-IF-004) */
#define REQUEST_ACTION_MAX 1
/** 운전자 요청 source의 허용 최댓값(0~3: 버튼, AVN, 음성, 앱). (OEM-IF-004) */
#define REQUEST_SOURCE_MAX 3

/**
 * @brief   값이 0 이상 max_value 이하인지 1/0으로 돌려준다.
 * @param   value      검사할 값
 * @param   max_value  허용 최댓값
 * @return  범위 안이면 1, 아니면 0
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
static int32_t validator_in_range(int32_t value, int32_t max_value)
{
    int32_t in_range = 0;

    if ((value >= 0) && (value <= max_value))
    {
        in_range = 1;
    }

    return in_range;
}

/**
 * @brief   필드가 있고 허용 범위 안인지 1/0으로 돌려준다. (FUNC-003)
 * @param   field      원시 필드
 * @param   max_value  허용 최댓값
 * @return  유효하면 1, 아니면 0
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
static int32_t validator_field_is_valid(const cl_field_raw_t *field, int32_t max_value)
{
    int32_t is_valid = 0;

    if (field->present == 1)
    {
        is_valid = validator_in_range(field->value, max_value);
    }

    return is_valid;
}

/**
 * @brief   수신한 필드를 입력 상태에 기록한다. (FUNC-004)
 * @param   entry      입력 상태 (입출력)
 * @param   field      원시 필드
 * @param   max_value  허용 최댓값
 * @param   now_ms     수신 시각 [ms]
 * @return  CL_OK
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
static cl_status_t validator_record_field(input_entry_t *entry, const cl_field_raw_t *field,
                                          int32_t max_value, uint32_t now_ms)
{
    if (validator_field_is_valid(field, max_value) == 1)
    {
        entry->last_value = field->value;
        entry->last_valid_ms = now_ms;
        entry->has_valid = 1;
        entry->last_was_bad = 0;
    }
    else
    {
        entry->last_was_bad = 1;
    }

    return CL_OK;
}

/**
 * @brief   입력 상태의 나이를 계산한다. 시계가 거꾸로 가면 0이다. (DD-D3)
 * @param   entry   입력 상태
 * @param   now_ms  현재 시각 [ms]
 * @return  마지막 유효 수신 후 경과 시간 [ms]
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
static uint32_t validator_age_ms(const input_entry_t *entry, uint32_t now_ms)
{
    uint32_t age = 0U;

    if (now_ms >= entry->last_valid_ms)
    {
        age = now_ms - entry->last_valid_ms;
    }

    return age;
}

/**
 * @brief   입력 상태를 VALID/INVALID/STALE와 값으로 바꾼다. (FUNC-005)
 * @param   entry     입력 상태
 * @param   now_ms    현재 시각 [ms]
 * @param   validity  분류 결과 (출력)
 * @param   value     VALID이면 값, 아니면 0 (출력)
 * @return  CL_OK
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
static cl_status_t validator_classify_input(const input_entry_t *entry, uint32_t now_ms,
                                            cl_validity_t *validity, int32_t *value)
{
    *validity = CL_VALID;
    *value = entry->last_value;

    if ((entry->has_valid == 0) || (entry->last_was_bad == 1))
    {
        *validity = CL_INVALID;
    }
    else if (validator_age_ms(entry, now_ms) > CL_FRESHNESS_LIMIT_MS)
    {
        *validity = CL_STALE;
    }
    else
    {
        /* VALID: 위에서 정한 값을 그대로 쓴다. */
    }

    if (*validity != CL_VALID)
    {
        *value = 0;
    }

    return CL_OK;
}

/**
 * @brief   운전자 요청 세 필드가 모두 허용 범위인지 1/0으로 돌려준다.
 * @param   raw  스냅샷
 * @return  모두 범위 안이면 1, 아니면 0
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
static int32_t validator_request_in_range(const cl_raw_input_t *raw)
{
    int32_t side_ok = validator_in_range(raw->request_side, REQUEST_SIDE_MAX);
    int32_t action_ok = validator_in_range(raw->request_action, REQUEST_ACTION_MAX);
    int32_t source_ok = validator_in_range(raw->request_source, REQUEST_SOURCE_MAX);

    return (side_ok == 1) && (action_ok == 1) && (source_ok == 1);
}

/**
 * @brief   운전자 요청의 유효 여부와 side/action을 정한다. (FUNC-006)
 * @param   raw  스냅샷. 수신이 없으면 NULL
 * @param   out  검증 결과 (request_* 필드만 채운다)
 * @return  CL_OK
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
static cl_status_t validator_classify_request(const cl_raw_input_t *raw, cl_validated_t *out)
{
    out->request_valid = 0;
    out->request_side = CL_SIDE_LEFT;
    out->request_action = CL_ACTION_LOCK;

    if ((raw != NULL) && (raw->request_present == 1) && (validator_request_in_range(raw) == 1))
    {
        out->request_valid = 1;
        out->request_side = (cl_side_t)raw->request_side;
        out->request_action = (cl_action_t)raw->request_action;
    }

    return CL_OK;
}

cl_status_t input_validator_init(input_validator_state_t *state)
{
    cl_status_t status = CL_OK;

    if (state == NULL)
    {
        status = CL_ERR_NULL_ARG;
    }
    else
    {
        for (int32_t index = 0; index < (int32_t)CL_IN_COUNT; index++)
        {
            state->entry[index].last_value = 0;
            state->entry[index].last_valid_ms = 0U;
            state->entry[index].has_valid = 0;
            state->entry[index].last_was_bad = 0;
        }
    }

    return status;
}

cl_status_t input_validator_validate(input_validator_state_t *state, const cl_raw_input_t *raw,
                                     uint32_t now_ms, cl_validated_t *out)
{
    cl_status_t status = CL_OK;

    if ((state == NULL) || (out == NULL))
    {
        status = CL_ERR_NULL_ARG;
    }
    else
    {
        for (int32_t index = 0; index < (int32_t)CL_IN_COUNT; index++)
        {
            if (raw != NULL)
            {
                (void)validator_record_field(&state->entry[index], &raw->field[index],
                                             k_max_value[index], now_ms);
            }
            (void)validator_classify_input(&state->entry[index], now_ms, &out->validity[index],
                                           &out->value[index]);
        }
        (void)validator_classify_request(raw, out);
    }

    return status;
}
