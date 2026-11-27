/**
 * @file    cl_types.h
 * @brief   차일드락 제어 SW 전체가 공유하는 상수, 열거형, 구조체를 정의한다. (MOD-000)
 *
 * 함수가 없는 헤더이며 동작을 갖지 않는다. 설계: docs/design/detailed-design.md 3장.
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
#ifndef CL_TYPES_H
#define CL_TYPES_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/** 입력 신선도 한도 [ms]. 나이가 이 값을 넘으면 STALE (SSR-020). */
#define CL_FRESHNESS_LIMIT_MS 200U
/** 평가주기 [ms] (SWR-L-001, 가정 A-03). */
#define CL_EVAL_PERIOD_MS 10U

/** 이유코드 비트 (가정 A-10, A-21). */
#define CL_REASON_CRASH_PENDING (1U << 0)
#define CL_REASON_APPROACH_RISK_LOCK (1U << 1)
#define CL_REASON_APPROACH_RISK_SUPPRESS (1U << 2)
#define CL_REASON_APPROACH_INPUT_UNRELIABLE (1U << 3)
#define CL_REASON_IGNITION_OFF (1U << 4)
#define CL_REASON_SENSOR_FAULT (1U << 5)

    /** 함수 상태 코드. */
    typedef enum
    {
        CL_OK = 0,
        CL_ERR_NULL_ARG
    } cl_status_t;

    /** 입력 유효성 분류. */
    typedef enum
    {
        CL_VALID = 0,
        CL_INVALID,
        CL_STALE
    } cl_validity_t;

    /** 도어 출력. */
    typedef enum
    {
        CL_RELEASE = 0,
        CL_LOCK = 1
    } cl_lock_t;

    /** 도어 인덱스. */
    typedef enum
    {
        CL_DOOR_LEFT = 0,
        CL_DOOR_RIGHT = 1,
        CL_DOOR_COUNT = 2
    } cl_door_t;

    /** 필수 입력 인덱스. */
    typedef enum
    {
        CL_IN_CRASH = 0,
        CL_IN_APPROACH_LEFT,
        CL_IN_APPROACH_RIGHT,
        CL_IN_IGNITION,
        CL_IN_SENSOR_FAULT,
        CL_IN_COUNT
    } cl_input_id_t;

    /** 충돌 상태 값 (OEM-IF-002). */
    typedef enum
    {
        CL_CRASH_NONE = 0,
        CL_CRASH_PENDING = 1,
        CL_CRASH_CONFIRMED = 2
    } cl_crash_t;

    /** 운전자 요청 대상 (OEM-IF-004). */
    typedef enum
    {
        CL_SIDE_LEFT = 0,
        CL_SIDE_RIGHT = 1,
        CL_SIDE_ALL = 2
    } cl_side_t;

    /** 운전자 요청 동작 (OEM-IF-004). */
    typedef enum
    {
        CL_ACTION_LOCK = 0,
        CL_ACTION_UNLOCK = 1
    } cl_action_t;

    /** 도어별 안전 지시. */
    typedef enum
    {
        CL_DIR_FREE = 0,
        CL_DIR_HOLD,
        CL_DIR_LOCK,
        CL_DIR_RELEASE
    } cl_directive_t;

    /** 시스템 건전성. */
    typedef enum
    {
        CL_HEALTH_NORMAL = 0,
        CL_HEALTH_DEGRADED,
        CL_HEALTH_FAULT
    } cl_health_t;

    /** 공개 상태. */
    typedef enum
    {
        CL_STATE_NORMAL = 0,
        CL_STATE_DEGRADED,
        CL_STATE_FAULT,
        CL_STATE_OFF
    } cl_state_t;

    /** 원시 입력 필드 하나. present가 1이고 value가 허용 범위일 때만 유효하다. */
    typedef struct
    {
        int32_t present;
        int32_t value;
    } cl_field_raw_t;

    /** 한 주기에 수신한 원시 입력 스냅샷. */
    typedef struct
    {
        cl_field_raw_t field[CL_IN_COUNT];
        int32_t request_present;
        int32_t request_side;
        int32_t request_action;
        int32_t request_source;
    } cl_raw_input_t;

    /** 검증을 마친 입력. value는 VALID일 때만 의미가 있고 아니면 0이다. */
    typedef struct
    {
        cl_validity_t validity[CL_IN_COUNT];
        int32_t value[CL_IN_COUNT];
        int32_t request_valid;
        cl_side_t request_side;
        cl_action_t request_action;
    } cl_validated_t;

    /** 한 평가주기의 결과. */
    typedef struct
    {
        cl_lock_t lock[CL_DOOR_COUNT];
        cl_state_t state;
        uint32_t reason_mask;
        cl_validity_t validity[CL_IN_COUNT];
    } cl_result_t;

#ifdef __cplusplus
}
#endif

#endif /* CL_TYPES_H */
