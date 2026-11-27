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

/** 이유코드 비트: 충돌 PENDING을 받았다. (가정 A-07, A-10) */
#define CL_REASON_CRASH_PENDING (1U << 0)
/** 이유코드 비트: 접근위험으로 도어를 LOCK했다. (가정 A-21) */
#define CL_REASON_APPROACH_RISK_LOCK (1U << 1)
/** 이유코드 비트: 접근위험 도어에 대한 해제 요청을 억제했다. */
#define CL_REASON_APPROACH_RISK_SUPPRESS (1U << 2)
/** 이유코드 비트: 접근위험 입력이 INVALID 또는 STALE이다. (가정 A-18) */
#define CL_REASON_APPROACH_INPUT_UNRELIABLE (1U << 3)
/** 이유코드 비트: 점화 OFF로 해제했다. */
#define CL_REASON_IGNITION_OFF (1U << 4)
/** 경고코드 비트: sensor_fault가 TRUE이다. */
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
        int32_t present; /**< 1이면 수신됨, 그 밖의 값은 누락 또는 형식 오류 */
        int32_t value;   /**< 수신한 원시 값 */
    } cl_field_raw_t;

    /** 한 주기에 수신한 원시 입력 스냅샷. */
    typedef struct
    {
        cl_field_raw_t field[CL_IN_COUNT]; /**< 필수 입력(cl_input_id_t 인덱스) */
        int32_t request_present;           /**< 1이면 운전자 요청이 있음 */
        int32_t request_side;              /**< 요청 대상(cl_side_t 범위 0~2) */
        int32_t request_action;            /**< 요청 동작(cl_action_t 범위 0~1) */
        int32_t request_source;            /**< 요청 경로(0~3: 버튼, AVN, 음성, 앱) */
    } cl_raw_input_t;

    /** 검증을 마친 입력. value는 VALID일 때만 의미가 있고 아니면 0이다. */
    typedef struct
    {
        cl_validity_t validity[CL_IN_COUNT]; /**< 입력별 유효성 */
        int32_t value[CL_IN_COUNT];          /**< 입력별 값(VALID일 때만 의미, 아니면 0) */
        int32_t request_valid;               /**< 1이면 운전자 요청이 유효함 */
        cl_side_t request_side;              /**< 유효한 요청의 대상 */
        cl_action_t request_action;          /**< 유효한 요청의 동작 */
    } cl_validated_t;

    /** 한 평가주기의 결과. */
    typedef struct
    {
        cl_lock_t lock[CL_DOOR_COUNT];       /**< 도어별 최종 출력 */
        cl_state_t state;                    /**< 공개 상태 */
        uint32_t reason_mask;                /**< 이유·경고코드 비트마스크 */
        cl_validity_t validity[CL_IN_COUNT]; /**< 입력별 유효성(조회용) */
    } cl_result_t;

#ifdef __cplusplus
}
#endif

#endif /* CL_TYPES_H */
