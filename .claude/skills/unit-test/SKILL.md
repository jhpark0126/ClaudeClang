---
name: unit-test
description: Use when writing or extending unit tests under A-SPICE (SWE.4), including coverage goals, 100% coverage loops, stubs and mocks, Doxygen test comments, boundary value analysis, equivalence classes, PICT combinations, or MC/DC with LLVM. 단위 테스트, 커버리지, MC/DC.
---

# 단위 테스트 (A-SPICE SWE.4)

입력: 상세 설계(`FUNC`/`MOD`)와 구현 코드. 단위 시험은 `UT-xxx` ID를 갖고 `FUNC`/`SWR`에 추적된다.

## 커버리지 루프 (목표 100%)

정해진 커버리지 목표(기본: 구문·분기 100%)를 달성할 때까지 반복한다.

1. 설계와 요구사항에서 시험 케이스를 도출한다 (아래 기법).
2. 시험을 실행하고 커버리지를 측정한다.
3. 미충족 라인/분기를 보고, **그 코드가 실행되어야 하는 의미 있는 시나리오**를 찾아 케이스를 추가한다.
4. 목표를 달성할 때까지 2~3을 반복한다.
5. 도달할 수 없는 코드가 나오면 시험을 억지로 만들지 말고, 죽은 코드이면 구현을 고치고, 방어 코드이면 근거와 함께 예외로 기록해 리뷰에서 승인받는다.

**커버리지만을 위한 의미 없는 시험을 쓰지 않는다.** 단언(assert)이 없거나, 반환값·상태를 검증하지 않거나, 요구사항·설계와 연결되지 않는 케이스는 추가하지 않는다. 모든 케이스는 "무엇을 확인하는가"에 답할 수 있어야 한다.

## 시험 케이스 도출 기법 (적극 사용)

| 기법 | 적용 |
|---|---|
| 동치 분할 | 입력 도메인을 유효/무효 클래스로 나누고 클래스당 대표값을 시험한다. |
| 경계값 분석 | 각 경계의 직전·경계·직후 값을 시험한다 (min-1, min, min+1, max-1, max, max+1). |
| PICT 조합 | 입력이 여러 개일 때 pairwise 조합으로 케이스 수를 줄인다. `references/pict.md` 참조. |
| 오류 추측 | NULL 포인터, 빈 입력, 상태 위반 등 설계의 모든 오류 반환 경로를 시험한다. |

## Stub / Mock
- 시험 대상 모듈이 의존하는 **인터페이스 너머**(하드웨어, 다른 모듈, 시간)에만 사용한다.
- 실제 구현으로 충분히 시험할 수 있으면 실제 구현을 쓴다. 대상 모듈 내부 호출을 Mock으로 바꾸지 않는다.
- Mock 호출 횟수·순서 검증에 시험이 치우치지 않게 하고, 결과(반환값, 출력, 상태)를 우선 검증한다.

## 시험 함수 Doxygen 주석 (필수)

모든 시험 함수에는 아래 항목을 Doxygen 주석으로 쓴다.

```c
/**
 * @test    UT-012: Sensor_Read가 준비되지 않은 센서에서 STATUS_ERR_STATE를 반환한다.
 * @trace   FUNC-007, SWR-F-003
 * @purpose 센서 미준비 상태의 오류 경로를 확인한다. (동치 클래스: 무효 상태)
 * @input   sensor->ready = false, out_value = 유효한 포인터
 * @steps   1. 미준비 상태의 센서를 만든다.
 *          2. Sensor_Read(&sensor, &value)를 호출한다.
 * @expect  반환값 == STATUS_ERR_STATE, value는 변경되지 않는다.
 */
static void test_sensor_read_not_ready(void) { ... }
```

시험 프레임워크가 `void` 시험 함수를 요구하면 프레임워크 규약을 따른다. 구현 스킬의 반환값 규칙은 제품 코드에 적용한다.

## MC/DC 커버리지

MC/DC 목표가 정해진 경우에만 적용한다. **LLVM 도구는 이 스킬의 스크립트로 실행한다.**

```bash
bash .claude/skills/unit-test/scripts/mcdc_coverage.sh <소스.c> <시험.c> [출력디렉터리]
```

- 요구 사항: clang 18 이상(`-fcoverage-mcdc`), `llvm-profdata`, `llvm-cov`. 스크립트가 없으면 환경변수 `CLANG`, `LLVM_PROFDATA`, `LLVM_COV`로 경로를 지정한다.
- 결과: 조건별 독립성 쌍이 충족되지 않은 결정(decision)을 보여 준다.

MC/DC 케이스 추가 절차:
1. 스크립트를 실행해 미충족 조건을 확인한다.
2. 보고서의 `Condition C?` 가 "not covered"인 결정에 대해, 해당 조건 하나만 바꿔 결과가 달라지는 입력 쌍을 만든다.
3. 케이스를 추가하고 스크립트를 다시 실행한다. 모든 결정이 충족될 때까지 반복한다.
4. 조건이 6개를 넘는 결정은 LLVM이 추적하지 못하므로, 결정을 분리하는 설계 변경을 먼저 검토한다.

## 완료 조건
- [ ] 목표 커버리지 100% 달성 (예외는 근거와 함께 기록·승인)
- [ ] 모든 시험 함수에 Doxygen 주석(목적, 입력값, 절차, 예상 출력값, 추적)이 있다
- [ ] 모든 `FUNC`와 `SWR`에 시험이 연결된 추적 매트릭스가 갱신되었다
- [ ] 의미 없는 시험이 없다
- [ ] `review` 스킬로 시험 명세와 결과를 리뷰했다
