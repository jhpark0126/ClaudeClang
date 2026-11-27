---
name: integration-test
description: Use when writing or extending integration tests between software components under A-SPICE (SWE.5), including interface-centered tests, coverage goals, stubs and mocks, Doxygen test comments, boundary value analysis, equivalence classes, or PICT combinations. 통합 테스트, 인터페이스 테스트, 컴포넌트 연동.
---

# 통합 테스트 (A-SPICE SWE.5)

입력: 아키텍처(`ARCH`, `IF`)와 요구사항(`SWR`). 통합 시험은 `IT-xxx` ID를 갖고 `IF`/`ARCH`/`SWR`에 추적된다.

## 핵심 원칙
**인터페이스를 중심으로, 컴포넌트가 함께 동작하는지 확인한다.** 컴포넌트 내부 로직 검증은 `unit-test`의 몫이다.

## 시험 대상
- 각 인터페이스(`IF`)의 정상 호출, 모든 오류 반환, 사전/사후 조건 위반.
- 호출 순서, 주기, 초기화 순서, 상태 전이처럼 컴포넌트 사이에서만 드러나는 동작.
- 데이터 흐름: 한 컴포넌트의 출력이 다음 컴포넌트의 입력으로 올바르게 전달되는가 (단위, 범위, 타입 변환 포함).
- 요구사항 중 여러 컴포넌트가 협력해야 만족되는 것.

## 커버리지 루프 (목표 100%)
정해진 커버리지 목표를 달성할 때까지 반복한다. 통합 시험의 커버리지 단위는 다음과 같다.
- **인터페이스 커버리지:** 모든 `IF`의 모든 함수가 호출되었는가.
- **오류 반환 커버리지:** 인터페이스에 정의된 모든 상태 코드가 한 번 이상 발생·처리되었는가.
- **요구사항 커버리지:** 아키텍처에 할당된 모든 `SWR`이 시험에 연결되었는가.
- 필요하면 코드 커버리지(구문·분기)도 측정해 `unit-test` 스킬과 같은 방식으로 미충족 항목을 분석한다.

미충족 항목마다 그것을 실행하는 **의미 있는 시나리오**를 찾아 케이스를 추가하고, 목표 달성까지 반복한다.
커버리지만을 위한 의미 없는 시험(단언 없음, 요구사항·인터페이스와 무관)은 추가하지 않는다.

## 케이스 도출 기법
`unit-test` 스킬의 기법을 인터페이스 매개변수에 적용한다: **동치 분할, 경계값 분석, PICT 조합** (`.claude/skills/unit-test/references/pict.md`). 여러 컴포넌트·매개변수가 얽힌 시나리오는 PICT로 조합을 줄인다.

## Stub / Mock
- 통합 대상이 아닌 **시스템 경계 밖**(하드웨어, OS, 외부 시스템, 시간)에만 사용한다.
- 통합 대상 컴포넌트끼리는 **실제 구현을 연결**한다. 통합 시험에서 컴포넌트를 Mock으로 바꾸면 인터페이스를 검증하지 못한다.
- Mock 호출 검증보다 컴포넌트 연결 결과(출력, 상태)를 우선 검증한다.

## 시험 함수 Doxygen 주석 (필수)

```c
/**
 * @test    IT-005: Controller가 Sensor 오류를 받으면 안전 상태로 전환한다.
 * @trace   IF-003, ARCH-002, SWR-F-010
 * @purpose Sensor→Controller 인터페이스의 오류 전파를 확인한다. (동치 클래스: 센서 오류)
 * @input   Sensor 스텁이 STATUS_ERR_STATE를 반환
 * @steps   1. Controller와 Sensor 실제 구현을 연결한다.
 *          2. 센서를 미준비 상태로 둔다.
 *          3. Controller_Step()을 호출한다.
 * @expect  Controller_Step()이 STATUS_OK를 반환하고 상태가 SAFE로 바뀐다.
 */
```

목적, 입력값, 절차, 예상 출력값, 추적 항목을 모두 쓴다.

## 완료 조건
- [ ] 모든 `IF`와 정의된 모든 오류 반환이 시험되었다
- [ ] 목표 커버리지 100% 달성 (예외는 근거와 함께 기록·승인)
- [ ] 모든 시험 함수에 Doxygen 주석이 있다
- [ ] `SWR`↔`ARCH`/`IF`↔`IT` 추적 매트릭스가 갱신되었다
- [ ] 의미 없는 시험이 없다
- [ ] `review` 스킬로 시험 명세와 결과를 리뷰했다
