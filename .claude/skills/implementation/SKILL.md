---
name: implementation
description: Use when writing or modifying C/C++ source code for this project, including function return values, single return statement, replacing void functions with status codes, or unit construction under A-SPICE SWE.3. 구현, 코딩, 리턴값 규칙.
---

# 구현 (A-SPICE SWE.3 Unit Construction)

상세 설계(`detailed-design`)를 입력으로 코드를 작성한다. 설계에 없는 동작을 코드에 추가하지 않는다. 필요하면 설계를 먼저 고친다.

## 함수 반환 규칙 (필수)

1. **모든 함수는 값을 반환한다.** `void` 함수를 만들지 않는다.
2. **`return`은 함수 맨 끝에 한 번만 쓴다.** 중간 `return`, 조기 종료 `return`을 쓰지 않는다.
3. 의미 있는 반환값이 없으면 **정상 실행 여부를 나타내는 값**을 반환한다 (상태 코드).
4. 반환값이 있는 함수에서 오류를 알려야 할 때는 상태 코드를 반환값으로 쓰고, 결과는 출력 매개변수(포인터)로 돌려준다.

### 상태 코드

```c
typedef enum {
    STATUS_OK = 0,
    STATUS_ERR_NULL_PTR,
    STATUS_ERR_RANGE,
    STATUS_ERR_STATE
} Status;
```

모듈 고유 오류가 필요하면 이 열거형을 확장하고 설계 문서에 기록한다.

### 단일 return 패턴

중간 `return` 대신 **결과 변수 + 중첩 조건 + 마지막 return** 으로 쓴다.

```c
Status Sensor_Read(const Sensor *sensor, uint16_t *out_value)
{
    Status status = STATUS_OK;

    if ((sensor == NULL) || (out_value == NULL)) {
        status = STATUS_ERR_NULL_PTR;
    } else if (!Sensor_IsReady(sensor)) {
        status = STATUS_ERR_STATE;
    } else {
        *out_value = sensor->raw;
    }

    return status;
}
```

반복문 안에서 조기 종료가 필요하면 `break`/루프 조건에 상태를 넣어 빠져나오고, `return`은 여전히 끝에서 한 번만 쓴다.

### 호출자 규칙
- 반환된 상태 코드를 **반드시 확인**한다. 의도적으로 무시할 때만 `(void)`로 캐스팅하고 이유를 주석으로 남긴다.
- 출력 매개변수는 실패 시 값이 정의되도록(예: 초기값) 문서화한다.

## 그 밖의 규칙
- 모듈 간 접근은 헤더에 공개된 인터페이스만 쓴다 (`detailed-design` 스킬).
- 내부 함수와 데이터는 `static`으로 둔다.
- 함수는 작게, 한 가지 일만 하게 쓴다. 이름은 의도를 드러내게 쓴다.
- 공개 함수에는 Doxygen 주석(`@brief`, `@param`, `@return`)을 쓴다. `@return`에는 반환 가능한 모든 상태 코드를 적는다.
- 구현한 함수와 파일 머리에 설계 ID(`FUNC-xxx`, `MOD-xxx`)를 적어 추적성을 유지한다.

## 자체 점검 (커밋 전)
- [ ] `void`로 선언된 함수가 없다. (`grep -nE '^\s*(static\s+)?void\s+\w+\s*\(' src`)
- [ ] 각 함수에 `return`이 정확히 하나이고 마지막에 있다.
- [ ] 모든 반환값을 호출자가 확인한다.
- [ ] 설계 문서와 동작이 일치한다.
- [ ] `review` 스킬로 코드 리뷰를 마쳤다.
