# 소프트웨어 아키텍처 (SWE.2) — 이터레이션 1

- 입력: `docs/requirements/software-requirements.md`(SWR), `safety-requirements.md`(SSR), 가정 `oem-swr-001-review.md`
- 범위: 이터레이션 1(OEM-SR-001~004, FR-007). 이터 2~4에서 컴포넌트를 추가·확장한다(개방-폐쇄).
- 작성: Claude Sonnet 5.5 (claude-sonnet-5-5), 2026-11-27, 요청자 jhpark0126. 사용자 리뷰: pending.
- 리뷰 상태: 사용자 지시로 이번 이터레이션의 `reviewer` 리뷰는 면제됨(`docs/reviews/iteration-1-review-waiver.md`).
- 교육용 가상 프로젝트다. ISO 26262 준수, ASIL 달성, 인증을 주장하지 않는다.
- 인터페이스 ID `IF-xxx`는 내부 컴포넌트 인터페이스이며, OEM 외부 인터페이스(`OEM-IF-xxx`)와 구별한다.

## 1. 설계 방침
- 언어는 C17. 컴포넌트마다 헤더(`include/`)와 구현(`src/`) 한 쌍이고, 헤더에 있는 함수만 인터페이스다.
- 전역 변수 없음. 상태는 호출자가 가진 컨텍스트 구조체에 두고 인터페이스 함수가 인자로 받는다(결정론, 시험성: SWR-Q-002, Q-003).
- 시계는 호출자가 `now_ms`로 넘긴다(단조 증가 ms). 컴포넌트가 시계를 직접 읽지 않는다.
- 동적 메모리 할당 없음. 모든 함수는 반환값으로 `cl_status_t`를 돌려주고 결과는 출력 인자로 낸다(`implementation` 스킬 규칙).
- ASIL B 로직(ARCH-002)과 QM 로직(ARCH-003)은 별도 컴포넌트로 분리하고, 합성은 ARCH-004가 한 곳에서만 한다.

## 2. 컴포넌트 목록
| ID | 이름 | 책임 (한 문장) | 안전 분류 |
|---|---|---|---|
| ARCH-001 | `input_validator` | 원시 입력 스냅샷의 각 필수 입력을 VALID/INVALID/STALE로 분류한다. | ASIL B |
| ARCH-002 | `safety_arbiter` | 검증된 입력으로 도어별 안전 지시(RELEASE/LOCK/HOLD/FREE)와 시스템 건전성(NORMAL/DEGRADED/FAULT)을 정한다. | ASIL B |
| ARCH-003 | `ignition_policy` | 점화 OFF에 따른 QM 해제 제안을 만든다. | QM |
| ARCH-004 | `output_resolver` | 안전 지시와 QM 제안을 정해진 순서로 합성해 최종 출력, 상태, 이유코드를 정한다. | ASIL B |
| ARCH-005 | `child_lock` | 평가주기 1회의 실행 순서를 조율하고 외부에 초기화·평가·결과 조회 인터페이스를 제공한다. | ASIL B |
| (헤더) | `cl_types` | 모든 컴포넌트가 공유하는 열거형·구조체 정의를 제공한다. 동작이 없는 헤더이며 컴포넌트가 아니다. | - |

ARCH-004는 ASIL B 지시를 그대로 적용하므로 ASIL B로 개발한다. ARCH-003의 출력은 ARCH-004에서 안전 지시가 `FREE`인 도어에만 반영된다(4장).

## 3. 공유 타입 (`cl_types.h`)
| 타입 | 값/필드 |
|---|---|
| `cl_status_t` | `CL_OK`, `CL_ERR_NULL_ARG` |
| `cl_validity_t` | `CL_VALID`, `CL_INVALID`, `CL_STALE` |
| `cl_lock_t` | `CL_RELEASE`, `CL_LOCK` |
| `cl_door_t` | `CL_DOOR_LEFT`, `CL_DOOR_RIGHT` (배열 인덱스로도 사용) |
| `cl_directive_t` | `CL_DIR_RELEASE`(강제 해제), `CL_DIR_LOCK`(강제 잠금), `CL_DIR_HOLD`(직전 출력 유지), `CL_DIR_FREE`(안전 제약 없음) |
| `cl_health_t` | `CL_HEALTH_NORMAL`, `CL_HEALTH_DEGRADED`, `CL_HEALTH_FAULT` |
| `cl_state_t` | `CL_STATE_NORMAL`, `CL_STATE_DEGRADED`, `CL_STATE_FAULT`, `CL_STATE_OFF` |
| `cl_reason_t` (비트마스크) | `CRASH_PENDING`, `APPROACH_RISK_LOCK`, `APPROACH_RISK_SUPPRESS`, `APPROACH_INPUT_UNRELIABLE`, `IGNITION_OFF`, `SENSOR_FAULT` |
| `cl_raw_input_t` | 필수 입력마다 `present`(0/1)와 원시 정수값. 형식·범위·열거 오류를 표현하려고 정수로 받는다 |
| `cl_validated_t` | 입력마다 `cl_validity_t`와 해석된 값. 운전자 요청은 유효 여부, 도어 선택(좌/우/전체), 동작(lock/unlock) |
| `cl_result_t` | 도어별 출력(`cl_lock_t[2]`), `cl_state_t`, `reason_mask`, 입력별 `cl_validity_t`(조회용) |

열거형 값 확정은 상세 설계(가정 A-10)에서 한다.

## 4. 인터페이스 정의
### IF-001 입력 검증
| 필드 | 내용 |
|---|---|
| 제공 / 사용 | ARCH-001 / ARCH-005 |
| 시그니처 | `cl_status_t iv_validate(iv_state_t *st, const cl_raw_input_t *raw, uint32_t now_ms, cl_validated_t *out)` |
| 매개변수 | `st` in/out 검증기 상태, `raw` in 이번 주기에 수신한 스냅샷(수신 없음이면 `NULL` 허용), `now_ms` in 단조 증가 ms, `out` out 검증 결과 |
| 반환값 | `CL_OK`, `CL_ERR_NULL_ARG`(`st` 또는 `out`이 `NULL`) |
| 사전/사후 | 사전: `st`가 `iv_init`으로 초기화됨. 사후: `out`의 모든 필수 입력에 `cl_validity_t`가 채워짐. INVALID·STALE 입력의 값은 사용하면 안 된다(값은 0으로 채움) |
| 호출 규칙 | 평가주기마다 1회, `child_lock`만 호출. 재진입 불가(상태 갱신). 인터럽트 안전 아님 |
| 오류 처리 | `CL_ERR_NULL_ARG`이면 호출자는 결과를 쓰지 않고 오류를 그대로 상위에 반환 |
| 부가 인터페이스 | `cl_status_t iv_init(iv_state_t *st)` |
| 규칙 | 입력별 상태(마지막 값, 마지막 유효 수신 시각, 유효값 보유 여부, 마지막 수신이 불량인지)를 갖는다. `raw`가 있고 필드가 누락·형식·범위·열거 오류면 INVALID. 마지막 수신이 불량이면 다음 유효 수신 전까지 INVALID. 유효값이 한 번도 없으면 INVALID. 유효값이 있고 나이(`now_ms` − 마지막 유효 수신 시각)가 200 ms 초과이면 STALE, 이하이면 VALID |

### IF-002 안전 판단
| 필드 | 내용 |
|---|---|
| 제공 / 사용 | ARCH-002 / ARCH-005 |
| 시그니처 | `cl_status_t sa_decide(sa_state_t *st, const cl_validated_t *in, sa_decision_t *out)` |
| 매개변수 | `st` in/out 안전 상태(충돌 래치), `in` in 검증 결과, `out` out 도어별 지시 `cl_directive_t[2]`, 건전성 `cl_health_t`, 안전 이유 비트마스크 |
| 반환값 | `CL_OK`, `CL_ERR_NULL_ARG` |
| 사전/사후 | 사전: `sa_init` 완료. 사후: 도어마다 지시가 정해짐. 래치가 한 번 세워지면 `sa_init` 전까지 유지 |
| 호출 규칙 | 평가주기마다 1회, `iv_validate` 직후. 재진입 불가 |
| 오류 처리 | `CL_ERR_NULL_ARG`이면 호출자는 `out`을 쓰지 않는다 |
| 부가 인터페이스 | `cl_status_t sa_init(sa_state_t *st)` |
| 판단 규칙 (우선순위 순) | ① 래치 또는 유효 `crash_status`=CONFIRMED → 양쪽 `CL_DIR_RELEASE`(래치 설정). ② 도어별: 유효 접근위험 TRUE → `CL_DIR_LOCK`, 이유 `APPROACH_RISK_LOCK`. ③ 도어별: 접근위험 입력이 INVALID/STALE → `CL_DIR_HOLD`, 이유 `APPROACH_INPUT_UNRELIABLE`. ④ 건전성이 FAULT/DEGRADED이면 나머지 도어 `CL_DIR_HOLD`. ⑤ 그 외 `CL_DIR_FREE`. PENDING은 지시를 만들지 않고 이유 `CRASH_PENDING`만 낸다. 건전성: 유효 `sensor_fault`=TRUE → FAULT, 아니면 필수 입력 중 INVALID/STALE 하나라도 있으면 DEGRADED, 그 외 NORMAL |

### IF-003 점화 정책 (QM)
| 필드 | 내용 |
|---|---|
| 제공 / 사용 | ARCH-003 / ARCH-005 |
| 시그니처 | `cl_status_t ip_propose(const cl_validated_t *in, ip_proposal_t *out)` |
| 매개변수 | `in` in 검증 결과, `out` out `release_all`(0/1) |
| 반환값 | `CL_OK`, `CL_ERR_NULL_ARG` |
| 사전/사후 | 사전: 없음(상태 없음). 사후: 유효 `ignition_on`=FALSE일 때만 `release_all`=1 |
| 호출 규칙 | 평가주기마다 1회. 순수 함수이므로 재진입 가능 |
| 오류 처리 | `CL_ERR_NULL_ARG`이면 호출자는 제안을 쓰지 않는다 |

### IF-004 출력 합성
| 필드 | 내용 |
|---|---|
| 제공 / 사용 | ARCH-004 / ARCH-005 |
| 시그니처 | `cl_status_t or_resolve(const sa_decision_t *dec, const ip_proposal_t *prop, const cl_validated_t *in, const cl_lock_t prev[2], cl_result_t *out)` |
| 매개변수 | `dec` in 안전 지시, `prop` in QM 제안, `in` in 검증 결과(운전자 요청, 입력 유효성), `prev` in 직전 확정 출력, `out` out 최종 결과 |
| 반환값 | `CL_OK`, `CL_ERR_NULL_ARG` |
| 사전/사후 | 사후: 출력은 항상 {LOCK, RELEASE}, 상태는 항상 정의된 값 |
| 합성 규칙 | 도어별: 지시 RELEASE → RELEASE, LOCK → LOCK, HOLD → `prev`. FREE일 때만 `prop.release_all`이면 RELEASE, 아니면 `prev`. 건전성이 DEGRADED/FAULT이면 지시가 FREE가 될 수 없으므로 QM 제안은 자동으로 반영되지 않는다(SWR-F-004). 해제 요청이 LOCK 지시 도어에 대한 것이면 이유 `APPROACH_RISK_SUPPRESS` 추가(`side`=all은 도어별 판단). 이터 1에서 해제 요청의 출력 효과는 억제 판단뿐이다(적용은 FR-001, 이터 2). 상태: FAULT > DEGRADED > OFF(제안 반영) > NORMAL. 이유코드는 ARCH-002 이유와 `IGNITION_OFF`(제안이 반영된 주기마다), `SENSOR_FAULT`를 합친다 |
| 호출 규칙 | 평가주기마다 1회, 재진입 가능(상태 없음) |
| 오류 처리 | `CL_ERR_NULL_ARG`이면 호출자는 `out`을 쓰지 않는다 |

### IF-005 외부 제어 인터페이스
| 필드 | 내용 |
|---|---|
| 제공 / 사용 | ARCH-005 / 호출자(시험, 추후 상태 조회·시뮬레이터) |
| 시그니처 | `cl_status_t cl_init(cl_ctx_t *ctx)`, `cl_status_t cl_step(cl_ctx_t *ctx, const cl_raw_input_t *raw, uint32_t now_ms, cl_result_t *out)` |
| 매개변수 | `ctx` in/out 컨텍스트(검증기·안전 상태·직전 출력 포함, 불투명), `raw` in 이번 주기 스냅샷(`NULL`이면 수신 없음), `now_ms` in 단조 증가 ms, `out` out 결과 |
| 반환값 | `CL_OK`, `CL_ERR_NULL_ARG`(`ctx` 또는 `out`이 `NULL`) |
| 사전/사후 | 사전: `cl_init` 완료. 사후: `out`과 `ctx`의 직전 출력이 같은 값. `cl_init` 직후 직전 출력은 양쪽 RELEASE |
| 호출 규칙 | 호출자가 `EVAL_PERIOD_MS`(10 ms)마다 1회 호출한다. 단일 스레드, 재진입 불가 |
| 오류 처리 | 오류이면 `ctx`를 바꾸지 않는다. 호출자는 직전 `out`을 신뢰하지 않고 오류를 보고한다 |

`cl_step` 내부 호출 순서는 `iv_validate` → `sa_decide` → `ip_propose` → `or_resolve` → 직전 출력 갱신이다. 한 컴포넌트가 오류를 반환하면 이후 단계를 건너뛰고 그 오류를 반환한다.

## 5. 의존 관계 (인터페이스 단위)
```
호출자 --IF-005--> ARCH-005 child_lock
                     |--IF-001--> ARCH-001 input_validator
                     |--IF-002--> ARCH-002 safety_arbiter
                     |--IF-003--> ARCH-003 ignition_policy   (QM)
                     |--IF-004--> ARCH-004 output_resolver
모든 컴포넌트 --> cl_types.h (타입만, 동작 없음)
```
- 컴포넌트 사이에 직접 호출이 없다. ARCH-001~004는 서로 호출하지 않고 ARCH-005가 값을 전달한다(결합도 최소, 순환 의존 없음).
- ARCH-003은 ARCH-002의 상태나 지시에 접근하지 못하고, ARCH-002는 ARCH-003의 제안을 보지 못한다. 서로 영향을 줄 수 있는 유일한 지점은 ARCH-004의 합성 규칙이다.

## 6. 동적 동작
1. 초기화: `cl_init` → `iv_init`(모든 입력 INVALID 상태), `sa_init`(래치 없음), 직전 출력 RELEASE.
2. 주기 실행(10 ms): 위 4장 호출 순서. 입력이 없는 주기(`raw`=`NULL`)에도 호출해 나이를 갱신한다.
3. 오류 처리: `NULL` 인자는 `CL_ERR_NULL_ARG`로 즉시 반환. 입력 오류는 오류코드가 아니라 입력 INVALID/STALE로 흡수되어 정상 `CL_OK` 경로를 탄다(SWR-Q-001).
4. 첫 주기: 유효 입력이 없으면 필수 입력 전부 INVALID → 건전성 DEGRADED, 출력 RELEASE 유지(SSR-021a, SWR-F-005).

## 7. 설계 결정 기록
| ID | 결정 | 근거 | 대안과 기각 이유 |
|---|---|---|---|
| DD-01 | 전역 상태 없이 컨텍스트 구조체를 인자로 전달 | 결정론과 시험성(SWR-Q-002/003), 캡슐화 | 모듈 내부 static: 시험 간 상태가 새고 재생 시험이 어려워 기각 |
| DD-02 | 시계를 `now_ms`로 주입 | 200/300 ms 경계를 시험에서 재현 | 직접 시계 읽기: 시험이 실시간에 의존해 기각 |
| DD-03 | 안전(ARCH-002)과 QM(ARCH-003) 분리, 합성은 ARCH-004 한 곳 | QM이 안전 결과를 약화하지 못하게 하는 간섭 방지(A-13, SSR 간섭 방지 절) | 한 모듈에 통합: 변경이 안전 로직에 번져 기각 |
| DD-04 | 도어별 지시(RELEASE/LOCK/HOLD/FREE)로 합성 | 우선순위를 데이터로 표현해 MC/DC 시험 조합을 줄이고 좌우 독립(SSR-013)을 구조로 보장 | 규칙마다 직접 출력 덮어쓰기: 순서 의존 버그 위험으로 기각 |
| DD-05 | 원시 입력을 정수+`present`로 받아 검증기가 해석 | 형식·범위·열거 오류를 입력에서 재현해야 INVALID 규칙을 시험할 수 있음 | 이미 열거형으로 받기: 불량 값을 표현할 수 없어 기각 |
| DD-06 | `raw`=`NULL`을 "수신 없음"으로 허용 | STALE 규칙은 입력이 끊긴 주기에서만 발생 | 항상 스냅샷 요구: 끊김을 시험할 수 없어 기각 |
| DD-07 | 이터 1에서 해제 요청은 억제 판단에만 사용 | FR-001(이터 2) 범위를 넘지 않으면서 SSR-011/012 시험 가능 | 요청 적용까지 구현: 범위 초과, 일반 해제 조건(Q7) 미확정으로 기각 |
| DD-08 | 상태 우선순위는 ARCH-004의 한 함수에 집중 | SSR-027을 한 곳에서 시험·검증 | 컴포넌트별 상태 보고: 충돌 가능성으로 기각 |

## 8. 설계 품질 점검
- 응집도: 5개 컴포넌트의 책임을 각각 한 문장으로 썼고 "및" 분리가 필요한 것이 없다. ARCH-005는 조율만 하고 규칙을 갖지 않는다.
- 결합도: 컴포넌트 간 호출 없음, 전달은 값 구조체만. 순환 의존 없음.
- 모듈화: 각 컴포넌트가 입력 구조체만으로 단독 시험된다. ARCH-003, 004는 상태 없는 순수 함수.
- 캡슐화: `iv_state_t`, `sa_state_t`, `cl_ctx_t` 필드는 소유 컴포넌트만 해석한다(헤더에 구조체를 두되 외부는 필드를 쓰지 않는 규칙, 코드 리뷰 항목).
- SOLID: 단일 책임(위 표), 개방-폐쇄(이터 2에서 `command_handler`를 ARCH-004 앞에 추가하고 ARCH-004 합성 규칙만 확장), 의존 역전(상위 `child_lock`이 인터페이스 함수 시그니처에만 의존, 단위 시험에서 스텁 대체 가능). 인터페이스 분리(각 IF는 한 컴포넌트 기능만 노출).
- 알려진 한계: C의 헤더 구조체는 완전한 은닉을 주지 않는다. 불투명 타입으로 바꾸면 정적 할당이 어려워져 이터 1에서는 규칙으로 관리한다.

## 9. 요구사항 할당 (SWR/SSR → ARCH)
| ARCH | 할당된 요구사항 |
|---|---|
| ARCH-001 | SSR-018, 019, 020, SWR-I-001, I-002, I-003, I-008, I-004(검증), I-007 |
| ARCH-002 | SSR-001~007, 009, 010, 011, 012a, 013, 014, 015, 016a, 016b, 017, 021, 021a, 023, 024, SSR-004(적용), 005(적용) |
| ARCH-003 | SWR-F-001(제안), F-002(제안) |
| ARCH-004 | SSR-008, 012, 015, 021a(상태), 022, 025, 026, 027, SWR-F-001~005, SWR-I-005, I-006a, I-006b |
| ARCH-005 | SWR-I-004(전달), I-005, I-006a/b(제공), SWR-L-001, SWR-Q-001, Q-002, Q-003, SSR-008(순서), SWR-F-005(초기화) |
| 전체 | SWR-C-001~004 |

모든 SSR(001~027, 012a, 016a/b, 021a)과 SWR-F/I/L/Q/C가 하나 이상의 ARCH에 할당되었다. 할당표는 `docs/traceability/trace.csv`의 `design` 열에 반영한다.

## 변경 이력
| 일자 | 내용 |
|---|---|
| 2026-11-27 | 초안 (이터레이션 1) |
