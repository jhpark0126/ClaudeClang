---
name: unit-test
description: Use when writing or extending unit tests under A-SPICE (SWE.4), including coverage goals, 100% coverage loops, stubs and mocks, Doxygen test comments, boundary value analysis, equivalence classes, PICT combinations, or MC/DC with LLVM. 단위 테스트, 커버리지, MC/DC.
---

# 단위 테스트 (A-SPICE SWE.4)

입력: 상세 설계(`FUNC`/`MOD`)와 구현 코드. 단위 시험은 `UT-xxx` ID를 갖고 `FUNC`/`SWR`에 추적된다.

## 품질 기준 (`CLAUDE.md` 2.1)
- **MC/DC 커버리지 100%**, **테스트 성공률 100%**.
- 단위 테스트는 `test/unit/`에 둔다. 구현 중 TDD로 만든 `test/tdd/`의 테스트와 **섞지 않는다.** 결과와 커버리지는 단위 테스트만으로 따로 측정한다. TDD 테스트가 통과했다는 사실은 이 기준을 충족한 근거가 아니다.
- 프레임워크는 Google Test / Google Mock이다.

## 커버리지 루프 (목표: MC/DC 100%)

MC/DC 100%와 성공률 100%를 달성할 때까지 반복한다.

1. 설계와 요구사항에서 시험 케이스를 도출한다 (아래 기법).
2. 시험을 실행하고 커버리지를 측정한다.
3. 미충족 라인/분기를 보고, **그 코드가 실행되어야 하는 의미 있는 시나리오**를 찾아 케이스를 추가한다.
4. 목표를 달성할 때까지 2~3을 반복한다.
5. 도달할 수 없는 코드가 나오면 시험을 억지로 만들지 말고 구현을 고친다 (죽은 코드는 제거, 방어 코드는 도달 가능하게 설계를 바꾸거나 제거). 100% 미달을 받아들여야 하면 **사용자 승인**을 받아 그 근거를 기록한다. 에이전트가 단독으로 예외를 만들지 않는다.

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

모든 시험 함수에는 아래 항목을 Doxygen 주석으로 쓴다. 시험 코드도 소스코드이므로 `CLAUDE.md` 2.4의 **사람 확인용 태그**(`@generated_by` 등)를 함께 쓴다.

```cpp
/**
 * @test    UT-012: Sensor_Read가 준비되지 않은 센서에서 STATUS_ERR_STATE를 반환한다.
 * @trace   FUNC-007, SWR-F-003
 * @purpose 센서 미준비 상태의 오류 경로를 확인한다. (동치 클래스: 무효 상태)
 * @input   sensor->ready = false, out_value = 유효한 포인터
 * @steps   1. 미준비 상태의 센서를 만든다.
 *          2. Sensor_Read(&sensor, &value)를 호출한다.
 * @expect  반환값 == STATUS_ERR_STATE, value는 변경되지 않는다.
 *
 * @generated_by   <작성한 AI 모델 이름과 ID>
 * @generated_at   <작성 날짜>
 * @requested_by   <요청한 사용자>
 * @human_review   pending
 * @reviewed_by    -
 * @reviewed_at    -
 */
TEST(SensorRead, NotReady) { ... }
```

`TEST()` 같은 프레임워크 매크로는 `void`로 전개된다. 그 경우 프레임워크 규약을 따른다. 구현 스킬의 반환값 규칙은 제품 코드에 적용한다. 태그 값은 작성 시점의 실제 값으로 쓰고, `@human_review`를 `done`으로 바꾸는 것은 사용자가 리뷰를 마쳤다고 알려 준 뒤에만 한다.

## MC/DC 커버리지

단위 테스트의 MC/DC 100%는 필수 기준이다. **LLVM 도구는 이 스킬의 스크립트로 실행한다.** 단위 테스트만 빌드·실행해 측정한다 (TDD 테스트는 넣지 않는다).

```bash
bash .claude/skills/unit-test/scripts/mcdc_coverage.sh <소스.c> <시험.c> [출력디렉터리]
```

- 요구 사항: clang 18 이상(`-fcoverage-mcdc`), `llvm-profdata`, `llvm-cov`. 스크립트가 없으면 환경변수 `CLANG`, `LLVM_PROFDATA`, `LLVM_COV`로 경로를 지정한다.
- Google Test(C++)를 쓰면 `CLANG=clang++`로 지정하고, 헤더 경로와 링크 옵션은 `EXTRA_FLAGS` 환경변수로 넘긴다 (예: `EXTRA_FLAGS="-Iinclude -lgtest -lgtest_main -pthread"`).
- 결과: 조건별 독립성 쌍이 충족되지 않은 결정(decision)을 보여 준다.

MC/DC 케이스 추가 절차:
1. 스크립트를 실행해 미충족 조건을 확인한다.
2. 보고서의 `Condition C?` 가 "not covered"인 결정에 대해, 해당 조건 하나만 바꿔 결과가 달라지는 입력 쌍을 만든다.
3. 케이스를 추가하고 스크립트를 다시 실행한다. 모든 결정이 충족될 때까지 반복한다.
4. 조건이 6개를 넘는 결정은 LLVM이 추적하지 못하므로, 결정을 분리하는 설계 변경을 먼저 검토한다.

## 완료 조건
- [ ] MC/DC 커버리지 100%, 테스트 성공률 100% 달성 (예외는 사용자 승인과 근거 기록)
- [ ] 단위 테스트가 `test/unit/`에 있고 TDD 테스트와 분리되어 측정되었다
- [ ] 모든 시험 함수에 Doxygen 주석(목적, 입력값, 절차, 예상 출력값, 추적)과 사람 확인용 태그가 있다
- [ ] 모든 `FUNC`와 `SWR`에 시험이 연결된 추적 매트릭스가 갱신되었다
- [ ] 의미 없는 시험이 없다
- [ ] `review` 스킬로 시험 명세와 결과를 리뷰했다
