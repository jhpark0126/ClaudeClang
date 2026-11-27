# 표준 요건과 이 스킬의 대응

표 번호와 문구는 판(edition)마다 다를 수 있다. 심사 대응 문서에 인용할 때는 프로젝트가 적용하는 판을 확인한다.

## Automotive SPICE — SWE.6

- v3.1: **SWE.6 Software Qualification Test**
- v4.0: **SWE.6 Software Verification**

| 요건 (BP 취지) | 이 스킬에서의 대응 |
|---|---|
| 테스트(검증) 명세 작성: 테스트 케이스, 기법, 합격/불합격 기준, 사전조건, 환경 | 케이스별 사전조건, 입력, 절차, 예상 출력값(허용 오차 포함), 사용 기법 열. 환경은 `environment` 필드 또는 요약 |
| 테스트 케이스 선택 (회귀 포함) | 각 케이스가 고유 파티션/경계/조합/엣지를 검증하도록 설계. 중복 의심 경고 |
| 양방향 추적성 (요구사항 ↔ 테스트 케이스) | `추적성 매트릭스` 시트. 스크립트가 순방향(모든 베이시스·조건 커버)과 역방향(모든 케이스가 유효한 베이시스 참조)을 검사 |
| 일관성 보장 | 존재하지 않는 ID 참조, 중복 ID를 오류로 처리 |
| 결과 요약·전달 | 최종 보고와 `커버리지 요약` 시트 |

## ISO 26262-6:2018 — Clause 11 Testing of the embedded software

테스트 방법 (requirements-based test, fault injection test):
- 요구사항 기반 테스트가 기본. 안전 메커니즘(고장 검출, 안전 상태 전환)이 있는 요구사항에는 결함 주입 테스트를 고려한다 (기법 표기: `결함 주입`).

테스트 케이스 도출 방법:
- 요구사항 분석 (analysis of requirements)
- 동치 클래스 생성 및 분석 (generation and analysis of equivalence classes)
- 경계값 분석 (analysis of boundary values)
- 지식/경험 기반 오류 추정 (error guessing) — WoW 검증이 여기에 해당한다
- 기능 의존성 분석 (analysis of functional dependencies) — 페어와이즈 조합, 요구사항 간 상호작용
- 운용 사용 사례 분석 (analysis of operational use cases)

권장 강도(+, ++)는 ASIL에 따라 다르다. ASIL이 주어지면 해당 판의 표에서 ++ 로 권장된 방법이 적용되었는지 확인하고, 적용하지 않은 방법이 있으면 이유를 보고한다.

테스트 환경: HIL, ECU 네트워크 환경, 차량 등. 케이스가 특정 환경에서만 수행 가능하면 `environment` 에 적는다.

## 커버리지의 의미

이 스킬의 "커버리지 100%"는 **테스트 베이시스 커버리지**다: 모든 요구사항과 그 하위 테스트 조건이 하나 이상의 케이스로 검증된다. 코드 구조 커버리지(문장/분기/MC/DC)는 단위·통합 테스트 단계의 지표이며 이 스킬의 범위가 아니다.
