# 통합 테스트 명세와 결과 (SWE.5) — 이터레이션 1

- 작성: Claude Sonnet 5.5 (claude-sonnet-5-5), 2026-11-27, 요청자 jhpark0126. 사용자 리뷰: pending.
- 리뷰 상태: 사용자 지시로 이번 이터레이션의 `reviewer` 리뷰는 면제됨(`docs/reviews/iteration-1-review-waiver.md`).
- 위치: `test/integration/` (실행 파일 `integration_tests`, 단위·TDD와 별도 결과·커버리지). 명세는 각 시험 함수의 Doxygen 주석에 있다.
- 실행: `bash scripts/dev.sh bash ci/run_tests.sh integration` (결과 `build/reports/integration/`)
- 컴포넌트는 모두 실제 구현을 연결했다. Stub/Mock은 쓰지 않았다(시스템 경계 밖 의존이 없다: 시계는 인자로 주입).

## 결과
| 항목 | 기준 | 결과 |
|---|---|---|
| 테스트 성공률 | 100% | 11/11 (100%) |
| 함수 커버리지 | 100% | 27/27 (100%) |
| 라인 / 분기 / MC/DC | 참고 | 383/383, 131/144, 28/40 (통합 기준 아님) |

## 설계 수준 보조 기준
- 인터페이스 커버리지: IF-001~005의 모든 공개 함수(`*_init`, `*_validate`, `*_decide`, `*_propose`, `*_resolve`, `child_lock_init/step`)를 호출했다.
- 오류 반환 커버리지: 인터페이스에 정의된 상태 코드는 `CL_OK`, `CL_ERR_NULL_ARG` 두 개이며, 모든 IF에서 `CL_ERR_NULL_ARG`를 발생시켰다(IT-005).
- 요구사항 커버리지: 아래 추적표에서 ARCH에 할당된 SWR/SSR이 모두 IT에 연결되었다.

## 시험 목록
| IT | 대상 인터페이스 | 확인하는 것 |
|---|---|---|
| IT-001 | IF-001→IF-002 | 불량 충돌 입력이 INVALID로 전달되어 래치를 만들지 못하고 DEGRADED를 일으킴 |
| IT-002 | IF-001→IF-003 | 점화 유효성이 QM 제안을 결정함 |
| IT-003 | IF-002, IF-003→IF-004 | 접근위험 LOCK과 DEGRADED가 점화 OFF 해제를 막음(간섭 방지) |
| IT-004 | IF-001→IF-004 | 운전자 해제 요청이 접근위험 도어에서 억제되고 이유가 기록됨 |
| IT-005 | IF-001~005 | 모든 IF의 `CL_ERR_NULL_ARG` 반환 |
| IT-006 | IF-005 ↔ IF-001~004 | PICT 2-way 20건: `child_lock_step` 결과가 컴포넌트 직접 연결 결과와 같음 + 요구사항 불변식 |
| IT-007 | IF-005 | OEM-SR-001 충돌 시나리오(해제와 유지, 300 ms 이내) |
| IT-008 | IF-005 | 접근위험 LOCK과 점화 OFF 연속 시나리오 |
| IT-009 | IF-005 | 입력 끊김, 200 ms 경계에서 DEGRADED, 복구 |
| IT-010 | IF-005 | sensor_fault가 QM 해제를 막고 충돌 해제는 통과 |
| IT-011 | IF-005 | 초기화 순서와 모든 하위 상태 초기화 |

PICT 모델: `test/model/pipeline.pict` → `test/integration/pipeline_pict_cases.inc`. 기대 결과는 PICT가 만들지 않는다. IT-006은 "외부 인터페이스 결과 = 내부 인터페이스 직접 연결 결과"와 요구사항 불변식으로 판정한다.

## 시험 중 발견
IT-003 최초 작성본이 실패했다. 원인은 제품이 아니라 시험 시나리오 오류였다(점화 OFF 스냅샷이 모든 입력이 유효해, 접근위험이 해제된 정상 상태에서 점화 OFF 해제가 올바르게 적용됨). 제품 동작은 SSR-015, SWR-F-001과 일치한다. 의도(DEGRADED가 점화 OFF 해제를 막음)를 정확히 검증하도록 시나리오를 고쳤다.

## SWR/SSR → ARCH → IT 추적
| ARCH | 할당 요구사항(요약) | IT |
|---|---|---|
| ARCH-001 | SSR-018~020, SWR-I-001~004, I-007, I-008 | IT-001, 002, 004, 009 |
| ARCH-002 | SSR-001~007, 009~017, 021, 021a, 023, 024 | IT-001, 003, 006, 007, 008, 010 |
| ARCH-003 | SWR-F-001, F-002 | IT-002, 003, 008 |
| ARCH-004 | SSR-008, 012, 015, 022, 025~027, SWR-F-001~005, I-005, I-006a/b | IT-003, 004, 006, 008, 010 |
| ARCH-005 | SWR-I-004~006, L-001, Q-001~003, SSR-008, SWR-F-005 | IT-005, 006, 007, 009, 011 |
