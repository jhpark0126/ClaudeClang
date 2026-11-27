/**
 * @file status.h
 * @brief 함수 실행 결과를 나타내는 상태 코드. (CI 자가 검증용 샘플)
 *
 * @generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)
 * @generated_at   2026-11-27
 * @requested_by   jhpark0126
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
#ifndef STATUS_H
#define STATUS_H

/** @brief 함수 실행 결과 상태 코드 */
typedef enum
{
    STATUS_OK = 0,       /**< 정상 */
    STATUS_ERR_NULL_PTR, /**< NULL 포인터 인자 */
    STATUS_ERR_RANGE     /**< 범위 오류 */
} Status;

#endif /* STATUS_H */
