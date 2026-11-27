# 상세 설계 (SWE.3) — 이터레이션 1

- 입력: `docs/architecture/software-architecture.md`(ARCH, IF-001~005), SWR/SSR, 가정 `oem-swr-001-review.md`
- 작성: Claude Sonnet 5.5 (claude-sonnet-5-5), 2026-11-27, 요청자 jhpark0126. 사용자 리뷰: pending.
- 리뷰 상태: 사용자 지시로 이번 이터레이션의 `reviewer` 리뷰는 면제됨(`docs/reviews/iteration-1-review-waiver.md`).
- 교육용 가상 프로젝트다. ISO 26262 준수, ASIL 달성, 인증을 주장하지 않는다.

## 1. 공통 규칙
- 구현 규칙(`implementation` 스킬): `void` 금지, `return`은 함수 끝에 한 번, 결과는 출력 인자, 호출자는 반환 상태를 확인한다.
- 상태 코드 `cl_status_t`는 `implementation` 스킬의 `Status` 규칙을 이 프로젝트용으로 확장한 것이다: `CL_OK = 0`, `CL_ERR_NULL_ARG`(필수 포인터 인자가 `NULL`). 이 두 값만 쓴다.
- 공개 함수는 `NULL` 인자를 검사해 `CL_ERR_NULL_ARG`를 반환하고, 그때 출력 인자는 건드리지 않는다. `static` 내부 함수는 호출한 공개 함수가 `NULL`이 아님을 보장하므로 검사하지 않는다(중복 검사 금지).
- 내부 함수 중 오류가 없는 순수 계산은 계산 결과를 값으로 반환한다(예: 건전성 판정). 상태를 바꾸거나 출력 인자를 채우는 함수는 `cl_status_t`를 반환한다.
- 시계는 `uint32_t now_ms`로만 들어온다. 함수는 시계를 읽지 않고 전역·`static` 데이터를 쓰지 않는다.
- 파일 위치: 헤더 `include/<모듈>.h`, 구현 `src/<모듈>.c`.

## 2. 모듈 목록과 인터페이스 판단
| 모듈 | 파일 | 책임 | ARCH | 인터페이스 정의 판단 |
|---|---|---|---|---|
| MOD-000 | `include/cl_types.h` | 공유 상수·열거형·구조체를 정의한다. | 전체 | 함수가 없는 헤더라 인터페이스를 정의하지 않는다(판단 기록). |
| MOD-001 | `input_validator.h/.c` | 입력 스냅샷을 입력별 VALID/INVALID/STALE로 분류한다. | ARCH-001 | 정의: IF-001 제공, 상태 소유 |
| MOD-002 | `safety_arbiter.h/.c` | 안전 지시와 건전성을 정한다. | ARCH-002 | 정의: IF-002 제공, 상태(충돌 래치) 소유 |
| MOD-003 | `ignition_policy.h/.c` | 점화 OFF 해제 제안을 만든다. | ARCH-003 | 정의: IF-003 제공 |
| MOD-004 | `output_resolver.h/.c` | 지시와 제안을 합성해 결과를 정한다. | ARCH-004 | 정의: IF-004 제공 |
| MOD-005 | `child_lock.h/.c` | 평가주기 1회의 순서를 조율하고 외부에 제공한다. | ARCH-005 | 정의: IF-005 제공, 컨텍스트 소유 |

상태는 `static` 데이터가 아니라 호출자가 가진 구조체이며, 구조체 정의는 소유 모듈 헤더에 있고 소유 모듈만 필드를 읽고 쓴다. 다른 모듈은 `.c` 내부와 전역 데이터를 쓰지 않는다. 컴포넌트 간 호출은 MOD-005만 한다.

## 3. 공유 정의 (MOD-000 `cl_types.h`)
```c
#define CL_FRESHNESS_LIMIT_MS 200U   /* SSR-020 */
#define CL_EVAL_PERIOD_MS      10U   /* SWR-L-001, 가정 A-03 */

typedef enum { CL_OK = 0, CL_ERR_NULL_ARG } cl_status_t;
typedef enum { CL_VALID = 0, CL_INVALID, CL_STALE } cl_validity_t;
typedef enum { CL_RELEASE = 0, CL_LOCK = 1 } cl_lock_t;
typedef enum { CL_DOOR_LEFT = 0, CL_DOOR_RIGHT = 1, CL_DOOR_COUNT = 2 } cl_door_t;
typedef enum { CL_IN_CRASH = 0, CL_IN_APPROACH_LEFT, CL_IN_APPROACH_RIGHT,
               CL_IN_IGNITION, CL_IN_SENSOR_FAULT, CL_IN_COUNT } cl_input_id_t;
typedef enum { CL_CRASH_NONE = 0, CL_CRASH_PENDING = 1, CL_CRASH_CONFIRMED = 2 } cl_crash_t;
typedef enum { CL_SIDE_LEFT = 0, CL_SIDE_RIGHT = 1, CL_SIDE_ALL = 2 } cl_side_t;
typedef enum { CL_ACTION_LOCK = 0, CL_ACTION_UNLOCK = 1 } cl_action_t;
typedef enum { CL_DIR_FREE = 0, CL_DIR_HOLD, CL_DIR_LOCK, CL_DIR_RELEASE } cl_directive_t;
typedef enum { CL_HEALTH_NORMAL = 0, CL_HEALTH_DEGRADED, CL_HEALTH_FAULT } cl_health_t;
typedef enum { CL_STATE_NORMAL = 0, CL_STATE_DEGRADED, CL_STATE_FAULT, CL_STATE_OFF } cl_state_t;
/* 이유코드 비트마스크 (가정 A-10, A-21) */
#define CL_REASON_CRASH_PENDING             (1U << 0)
#define CL_REASON_APPROACH_RISK_LOCK        (1U << 1)
#define CL_REASON_APPROACH_RISK_SUPPRESS    (1U << 2)
#define CL_REASON_APPROACH_INPUT_UNRELIABLE (1U << 3)
#define CL_REASON_IGNITION_OFF              (1U << 4)
#define CL_REASON_SENSOR_FAULT              (1U << 5)
```
구조체:
| 타입 | 필드 |
|---|---|
| `cl_field_raw_t` | `int32_t present`(0/1), `int32_t value` |
| `cl_raw_input_t` | `cl_field_raw_t field[CL_IN_COUNT]`(필수 입력), `int32_t request_present`, `int32_t request_side`, `int32_t request_action`, `int32_t request_source` (운전자 요청, 선택 입력) |
| `cl_validated_t` | `cl_validity_t validity[CL_IN_COUNT]`, `int32_t value[CL_IN_COUNT]`(VALID일 때만 의미, 아니면 0), `int32_t request_valid`(0/1), `cl_side_t request_side`, `cl_action_t request_action` |
| `cl_result_t` | `cl_lock_t lock[CL_DOOR_COUNT]`, `cl_state_t state`, `uint32_t reason_mask`, `cl_validity_t validity[CL_IN_COUNT]` |

입력별 허용 범위 표(`input_validator.c` 내부 상수): `CL_IN_CRASH` 0~2, 나머지 필수 입력 0~1. 요청 `side` 0~2, `action` 0~1, `source` 0~3(`physical_button`, `avn`, `voice`, `mobile_app`; 값은 검증만 하고 동작 차이는 없음, 가정 A-14).

## 4. MOD-001 `input_validator`
상태: `input_validator_state_t` = 입력별 `{int32_t last_value; uint32_t last_valid_ms; int32_t has_valid; int32_t last_was_bad}` × `CL_IN_COUNT`. 소유: MOD-001.

| ID | 함수 | 가시성 | 책임 | 추적 |
|---|---|---|---|---|
| FUNC-001 | `cl_status_t input_validator_init(input_validator_state_t *state)` | 공개 | 모든 입력 상태를 "유효값 없음"으로 초기화한다. | ARCH-001, SSR-018, SWR-F-005 |
| FUNC-002 | `cl_status_t input_validator_validate(input_validator_state_t *state, const cl_raw_input_t *raw, uint32_t now_ms, cl_validated_t *out)` | 공개 | 한 주기의 입력을 분류한다. | ARCH-001, SSR-018~020, SWR-I-001~004, I-007, I-008 |
| FUNC-003 | `static int32_t validator_field_is_valid(const cl_field_raw_t *field, int32_t max_value)` | 내부 | 필드가 있고 0~`max_value` 범위인지 1/0으로 돌려준다. | SSR-018 |
| FUNC-004 | `static cl_status_t validator_record_field(input_state_t *entry, const cl_field_raw_t *field, int32_t max_value, uint32_t now_ms)` | 내부 | 수신한 필드를 상태에 기록한다. | SSR-018, SWR-I-007 |
| FUNC-005 | `static cl_status_t validator_classify_input(const input_state_t *entry, uint32_t now_ms, cl_validity_t *validity, int32_t *value)` | 내부 | 입력 상태를 VALID/INVALID/STALE와 값으로 바꾼다. | SSR-018~020, SWR-I-007 |
| FUNC-006 | `static cl_status_t validator_classify_request(const cl_raw_input_t *raw, cl_validated_t *out)` | 내부 | 운전자 요청의 유효 여부와 side/action을 정한다. | SWR-I-004 |

**FUNC-001**: `state`가 `NULL`이면 `CL_ERR_NULL_ARG`. 아니면 모든 항목을 0으로 채운다. 반환 `CL_OK`.

**FUNC-002** 동작:
```
status = CL_OK
if state==NULL or out==NULL: status = CL_ERR_NULL_ARG
else:
  for i in 0..CL_IN_COUNT-1:
     if raw != NULL: validator_record_field(&state[i], &raw->field[i], max[i], now_ms)
     validator_classify_input(&state[i], now_ms, &out->validity[i], &out->value[i])
  validator_classify_request(raw, out)      # raw==NULL이면 request_valid = 0
return status
```
오류 처리: `NULL` 인자 외의 실패 경로는 없다. 입력 불량은 상태(INVALID)로 흡수한다.

**FUNC-003**: `field->present == 1` 이고 `0 <= value <= max_value`이면 1, 아니면 0. `present`가 0/1이 아닌 값이면 불량(0)으로 본다(형식 오류).

**FUNC-004**: 유효하면 `last_value = value; last_valid_ms = now_ms; has_valid = 1; last_was_bad = 0`. 불량이면 `last_was_bad = 1`(마지막 유효 값과 시각은 보존). 반환 `CL_OK`.

**FUNC-005**:
```
if !has_valid or last_was_bad: validity = CL_INVALID, value = 0
else:
   age = (now_ms >= last_valid_ms) ? now_ms - last_valid_ms : 0     # 시각 역행 방어, 가정: 단조 증가
   if age > CL_FRESHNESS_LIMIT_MS: validity = CL_STALE, value = 0
   else: validity = CL_VALID, value = last_value
```
반환 `CL_OK`. `uint32_t` 오버플로: 뺄셈은 부호 없는 산술이라 49.7일 주기 래핑에서도 올바르다(래핑 후에도 `now_ms >= last`가 거짓이 되는 경우는 시각 역행으로 간주해 age 0, 문서화된 한계).

**FUNC-006**: `raw == NULL` 또는 `request_present != 1`이면 `request_valid = 0`. 아니면 `side`(0~2), `action`(0~1), `source`(0~3)가 모두 범위 안일 때 `request_valid = 1`과 side/action 설정, 아니면 0.

## 5. MOD-002 `safety_arbiter`
상태: `safety_arbiter_state_t` = `{int32_t crash_latched}`. 소유: MOD-002.
출력 `safety_decision_t` = `{cl_directive_t directive[CL_DOOR_COUNT]; cl_health_t health; uint32_t reason_mask}`.

| ID | 함수 | 가시성 | 책임 | 추적 |
|---|---|---|---|---|
| FUNC-010 | `cl_status_t safety_arbiter_init(safety_arbiter_state_t *state)` | 공개 | 충돌 래치를 해제 상태로 만든다. | SSR-002 |
| FUNC-011 | `cl_status_t safety_arbiter_decide(safety_arbiter_state_t *state, const cl_validated_t *in, safety_decision_t *out)` | 공개 | 건전성과 도어별 지시를 정한다. | ARCH-002, SSR-001~007, 009~017, 021, 021a, 023, 024 |
| FUNC-012 | `static uint32_t arbiter_update_crash_latch(safety_arbiter_state_t *state, const cl_validated_t *in)` | 내부 | 충돌 상태를 반영해 래치를 갱신하고 이유 비트를 돌려준다. | SSR-001, 002, 004~007 |
| FUNC-013 | `static cl_health_t arbiter_compute_health(const cl_validated_t *in)` | 내부 | 건전성(FAULT/DEGRADED/NORMAL)을 판정한다. | SSR-021, 021a, 024 |
| FUNC-014 | `static cl_directive_t arbiter_decide_door(int32_t crash_latched, const cl_validated_t *in, cl_input_id_t approach_input, cl_health_t health, uint32_t *reason_mask)` | 내부 | 한 도어의 지시를 우선순위로 정한다. | SSR-003, 009, 010, 011, 013~017, 023 |

**FUNC-011**: 널 검사 후 `reason = arbiter_update_crash_latch`, `health = arbiter_compute_health`, 도어 2개에 `arbiter_decide_door`(좌: `CL_IN_APPROACH_LEFT`, 우: `CL_IN_APPROACH_RIGHT`). 결과 `out`에 기록한다. 반환 `CL_OK`.

**FUNC-012**:
```
reason = 0
if validity[CL_IN_CRASH] == CL_VALID:
    if value == CL_CRASH_CONFIRMED: state->crash_latched = 1
    else if value == CL_CRASH_PENDING: reason |= CL_REASON_CRASH_PENDING
return reason
```
CONFIRMED는 DEGRADED·FAULT에도 적용된다(건전성과 무관, SSR-004, 005). 래치는 `safety_arbiter_init` 전까지 풀리지 않는다(SSR-002).

**FUNC-013**:
```
if validity[CL_IN_SENSOR_FAULT]==CL_VALID and value[CL_IN_SENSOR_FAULT]==1: FAULT
else if any validity[i] != CL_VALID (i in 0..CL_IN_COUNT-1): DEGRADED
else NORMAL
```
`sensor_fault` 입력 자체가 INVALID/STALE이면 FAULT가 아니라 DEGRADED가 된다(SSR-021, 021a).

**FUNC-014** (위에서 아래 순서, 먼저 맞는 것):
```
if crash_latched:                                  return CL_DIR_RELEASE        # SSR-001,002,003
if validity[approach_input]==CL_VALID and value==1: reason|=APPROACH_RISK_LOCK; return CL_DIR_LOCK   # SSR-009,010,014,015
if validity[approach_input] != CL_VALID:            reason|=APPROACH_INPUT_UNRELIABLE; return CL_DIR_HOLD  # SSR-016a,b,017
if health != CL_HEALTH_NORMAL:                      return CL_DIR_HOLD           # SSR-023,024
return CL_DIR_FREE
```
(구현은 단일 return으로 `if/else if` 사슬에 결과 변수를 쓴다.) 좌/우는 같은 함수를 다른 `approach_input`으로 호출하므로 서로의 데이터를 읽지 않는다(SSR-013). 접근위험 LOCK이 점화 OFF보다 우선하는 것(SSR-015)은 LOCK 지시가 FREE가 아니라 ARCH-004에서 QM 제안을 막기 때문에 성립한다.

## 6. MOD-003 `ignition_policy`
| ID | 함수 | 가시성 | 책임 | 추적 |
|---|---|---|---|---|
| FUNC-020 | `cl_status_t ignition_policy_propose(const cl_validated_t *in, ignition_proposal_t *out)` | 공개 | 유효한 점화 OFF에서 전체 해제를 제안한다. | ARCH-003, SWR-F-001, F-002 |

`ignition_proposal_t` = `{int32_t release_all}`. 동작: 널 검사 후 `release_all = (validity[CL_IN_IGNITION]==CL_VALID && value[CL_IN_IGNITION]==0) ? 1 : 0`. 반환 `CL_OK`. 상태가 없고 순수 함수다.

## 7. MOD-004 `output_resolver`
| ID | 함수 | 가시성 | 책임 | 추적 |
|---|---|---|---|---|
| FUNC-030 | `cl_status_t output_resolver_resolve(const safety_decision_t *decision, const ignition_proposal_t *proposal, const cl_validated_t *in, const cl_lock_t previous[CL_DOOR_COUNT], cl_result_t *out)` | 공개 | 최종 출력, 상태, 이유코드를 정한다. | ARCH-004, SSR-008, 012, 015, 022, 025~027, SWR-F-001~005, I-005, I-006a/b |
| FUNC-031 | `static cl_lock_t resolver_door_output(cl_directive_t directive, int32_t release_all, cl_lock_t previous)` | 내부 | 한 도어의 출력을 정한다. | SSR-001, 009, 011, SWR-F-001, 004 |
| FUNC-032 | `static int32_t resolver_ignition_applies(const safety_decision_t *decision, const ignition_proposal_t *proposal)` | 내부 | 점화 OFF 해제가 적용 가능한 주기인지(`release_all`이고 건전성 NORMAL) 돌려준다. | SWR-F-001~004 |
| FUNC-033 | `static uint32_t resolver_suppress_reason(const safety_decision_t *decision, const cl_validated_t *in)` | 내부 | 해제 요청이 LOCK 지시 도어를 건드리면 억제 이유 비트를 돌려준다. | SSR-011, 012, SWR-I-004 |
| FUNC-034 | `static cl_state_t resolver_select_state(cl_health_t health, int32_t ignition_applies)` | 내부 | 상태 우선순위(FAULT > DEGRADED > OFF > NORMAL)로 상태를 정한다. | SSR-025, 027, SWR-F-002 |

**FUNC-030**:
```
status = CL_OK
if any pointer is NULL: status = CL_ERR_NULL_ARG
else:
  applies = resolver_ignition_applies(decision, proposal)
  for door in {LEFT, RIGHT}:
     out->lock[door] = resolver_door_output(decision->directive[door], applies, previous[door])
  out->state = resolver_select_state(decision->health, applies)
  out->reason_mask = decision->reason_mask
        | resolver_suppress_reason(decision, in)
        | (applies ? CL_REASON_IGNITION_OFF : 0)
        | (decision->health == CL_HEALTH_FAULT ? CL_REASON_SENSOR_FAULT : 0)
  out->validity[] = in->validity[]
return status
```

**FUNC-031**:
```
RELEASE → CL_RELEASE ; LOCK → CL_LOCK ; HOLD → previous
FREE    → release_all ? CL_RELEASE : previous
```
DEGRADED·FAULT에서는 지시가 FREE로 나오지 않으므로(FUNC-014) 점화 OFF 해제가 적용되지 않는다(SWR-F-004). `release_all`은 FUNC-032가 건전성 NORMAL일 때만 1로 넘기므로 이중으로 막힌다.

**FUNC-032**: `proposal->release_all == 1 && decision->health == CL_HEALTH_NORMAL`이면 1.

**FUNC-033**:
```
reason = 0
if in->request_valid and in->request_action == CL_ACTION_UNLOCK:
   for door: if request targets door (side==door or side==ALL) and directive[door]==CL_DIR_LOCK: reason = SUPPRESS
return reason
```
(대상 판정은 `side == CL_SIDE_ALL || side == door`를 한 식으로 쓴다.)

**FUNC-034**: `health==FAULT`→FAULT; `health==DEGRADED`→DEGRADED; `ignition_applies`→OFF; else NORMAL.

## 8. MOD-005 `child_lock`
컨텍스트: `cl_ctx_t` = `{input_validator_state_t validator; safety_arbiter_state_t arbiter; cl_lock_t previous[CL_DOOR_COUNT];}`. 소유: MOD-005(하위 상태는 각 모듈이 해석).

| ID | 함수 | 가시성 | 책임 | 추적 |
|---|---|---|---|---|
| FUNC-040 | `cl_status_t child_lock_init(cl_ctx_t *ctx)` | 공개 | 컨텍스트를 초기화한다(직전 출력 RELEASE). | ARCH-005, SWR-F-005 |
| FUNC-041 | `cl_status_t child_lock_step(cl_ctx_t *ctx, const cl_raw_input_t *raw, uint32_t now_ms, cl_result_t *out)` | 공개 | 평가주기 1회를 실행한다. | ARCH-005, SWR-I-004~007, L-001, Q-001~003, SSR-008 |
| FUNC-042 | `static cl_status_t child_lock_commit(cl_ctx_t *ctx, const cl_result_t *result)` | 내부 | 결과를 직전 출력으로 저장한다. | SWR-F-005 |

**FUNC-040**: 널 검사 후 `input_validator_init`, `safety_arbiter_init`을 호출하고 둘 다 `CL_OK`일 때 `previous[]`를 `CL_RELEASE`로 설정.

**FUNC-041**:
```
status = CL_OK
if ctx==NULL or out==NULL: status = CL_ERR_NULL_ARG
else:
  status = input_validator_validate(&ctx->validator, raw, now_ms, &validated)
  if status == CL_OK: status = safety_arbiter_decide(&ctx->arbiter, &validated, &decision)
  if status == CL_OK: status = ignition_policy_propose(&validated, &proposal)
  if status == CL_OK: status = output_resolver_resolve(&decision, &proposal, &validated, ctx->previous, &result)
  if status == CL_OK: *out = result; status = child_lock_commit(ctx, &result)
return status
```
오류 시 `ctx`의 직전 출력은 바뀌지 않고(`commit` 전), `out`도 쓰지 않는다. (`validate`가 입력 상태를 이미 갱신했다면 그 갱신은 유지되며, 오류 원인이 `NULL` 인자뿐이라 실제로 발생하지 않는다.)

**FUNC-042**: 도어별로 `previous[door] = result->lock[door]`. 반환 `CL_OK`.

## 9. 시간 특성 (SSR-008, SSR-022, SWR-L-001)
`child_lock_step`은 호출 즉시 같은 호출에서 결과를 낸다. CONFIRMED는 수신한 주기에 RELEASE(SSR-008: 300 ms 이내, 같은 주기). 입력 나이가 200 ms를 초과한 첫 호출에서 STALE → DEGRADED(SSR-022: 호출 간격이 10 ms이면 초과 후 최대 10 ms). 호출 간격이 길어지면 지연도 길어지므로 주기 호출은 호출자 책임이다(SWR-L-001, 시험은 `now_ms`를 지정해 재현: SWR-Q-002).

## 10. 크기 점검 (CC ≤ 10, 길이 ≤ 80)
가장 복잡한 함수는 FUNC-014(분기 4)와 FUNC-030(분기 약 6)이며 모두 10 이하로 예상한다. `child_lock_step`은 중첩 `if` 5단으로 CC 약 7이다. 구현에서 넘으면 이 문서의 함수를 더 나눈다.

## 11. 정적 데이터와 상태
전역·`static` 변수 없음. 입력 허용 범위 표만 파일 범위 `static const` 상수로 `input_validator.c`에 둔다(변경 불가).

## 12. 설계 결정
| ID | 결정 | 근거 |
|---|---|---|
| DD-D1 | 지시 열거형 순서를 `FREE, HOLD, LOCK, RELEASE`로 한다 | 숫자 비교로 우선순위를 읽지 않고 분기로 다룬다(오독 방지). 이후 순위 확장은 분기 추가로 한다 |
| DD-D2 | `present`가 0/1이 아닌 값도 불량으로 취급 | 형식 오류 입력(OEM-IF 오류 처리)을 정수로 재현하기 위함 |
| DD-D3 | 시각 역행은 age 0으로 방어 | 단조 증가 가정(A-12) 위반 시에도 정의된 동작을 하고 STALE 오판을 막는다. 역행 자체를 감지·보고하지 않는 한계는 이터 4 잔여 문제에 올린다 |
| DD-D4 | 내부 `static` 함수는 `NULL` 검사를 하지 않는다 | 공개 함수가 한 번 검사하므로 중복 분기를 만들어 MC/DC 불가능 분기를 늘리지 않는다 |

## 13. 추적
모든 `FUNC`는 ARCH와 SSR/SWR에 연결했다. 아키텍처 9장의 할당 요구사항이 모두 하나 이상의 FUNC에 닿는다: SWR-I-001~003, I-008은 FUNC-002/004(입력 해석), SWR-L-001, Q-001~003, C-001~004는 MOD-005 및 전체 규칙(1장)과 CI 게이트. `trace.csv`의 `design` 열에 `MOD-xxx`를 덧붙여 갱신한다.
## 14. 구현 중 설계 보완 (이터레이션 1)
구현 단계에서 함수 크기와 Cppcheck(변수 범위)를 맞추려고 아래 내부 함수를 설계에 추가했다. 동작은 위 설계와 같고 책임만 나눈 것이다.

| ID | 함수 | 책임 | 호출하는 함수 |
|---|---|---|---|
| FUNC-007 | `static int32_t validator_in_range(int32_t value, int32_t max_value)` | 값이 0~`max_value`인지 1/0으로 돌려준다. | FUNC-003, FUNC-008 |
| FUNC-008 | `static int32_t validator_request_in_range(const cl_raw_input_t *raw)` | 운전자 요청 side/action/source가 모두 범위 안인지 돌려준다. | FUNC-006 |
| FUNC-009 | `static uint32_t validator_age_ms(const input_entry_t *entry, uint32_t now_ms)` | 입력 나이를 계산한다(시각 역행이면 0, DD-D3). | FUNC-005 |
| FUNC-015 | `static int32_t arbiter_any_input_unreliable(const cl_validated_t *in)` | 필수 입력 중 VALID가 아닌 것이 있는지 돌려준다. | FUNC-013 |
| FUNC-035 | `static int32_t resolver_request_targets(cl_side_t side, int32_t door)` | 운전자 요청이 그 도어를 대상으로 하는지 돌려준다. | FUNC-033 |
| FUNC-036 | `static uint32_t resolver_collect_reasons(const safety_decision_t *decision, const cl_validated_t *in, int32_t ignition_applies)` | 이유코드를 합친다. | FUNC-030 |
| FUNC-043 | `static cl_status_t child_lock_run_pipeline(cl_ctx_t *ctx, const cl_raw_input_t *raw, uint32_t now_ms, cl_result_t *result)` | 검증, 안전 판단, 점화 제안, 합성을 순서대로 실행하고 첫 실패에서 멈춘다. | FUNC-041 |

그 밖의 구현 규칙: 반복 변수는 `for` 문 안에서 선언한다(C17, Cppcheck `variableScope`·`unreadVariable` 대응). 헤더의 `extern "C"`는 시험(C++)이 C 헤더를 쓰기 위한 것이다.

## 변경 이력
| 일자 | 내용 |
|---|---|
| 2026-11-27 | 초안 (이터레이션 1) |
| 2026-11-27 | 구현 중 보조 함수 FUNC-007~009, 015, 035, 036, 043 추가(14장) |
