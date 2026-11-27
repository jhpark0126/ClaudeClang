# 개발 이터레이션 계획

각 이터레이션 = 분석(SWE.1) → 아키텍처(SWE.2) → 상세 설계(SWE.3) → 구현(TDD) → 단위(SWE.4) → 통합(SWE.5) → 시스템 시험(SWE.6).
단계마다 `reviewer` 승인, 이터레이션 끝에 사용자 확인, 확인 뒤 `develop`으로 PR. 끝나면 `docs/traceability/iteration.txt`의
`completed_iteration`을 올려 요구사항 커버리지 게이트를 키운다. 가정(`oem-swr-001-review.md`)이 바뀌면 영향 단계부터 다시 한다.

| 이터 | 범위 | 완료 기준(요약) | 게이트 증거 |
|---|---|---|---|
| 0 | 환경 정합(C 확정), 요구사항 CSV, 추적 매트릭스, 요구사항 커버리지 게이트, 분석 가정 | 자가 검증 통과, Jenkins 통과 | - |
| 1 | OEM-SR-001~004, OEM-FR-007: 입력 검증·freshness, sensor_fault, 우선순위 중재 핵심, 충돌 해제, 접근위험 잠금, 시동 OFF | 해당 요구사항 시스템 시험 100%, 단위 MC/DC 100%, 통합 함수 100% | G1, G2 일부, G3 일부 |
| 2 | OEM-FR-001, 002, 005, 006, 003: 운전자 명령, 자동 잠금, 강제 해제/잠금, override | 위와 동일 + FR-003 간섭 방지 근거 리뷰 | G3 |
| 3 | OEM-FR-004, NFR-002: 상태 조회 계약(IF-006), 결정 이력 100건 링버퍼(개인정보 금지) | 위와 동일 | G3 |
| 4 | OEM-NFR-001 결정론 재생(1,000회 해시), 시스템 시험 전체 통합 확인, 잔여문제·미검증 범위 | 요구사항 커버리지 13/13(100%) | G4 |

## 초기 아키텍처 구상(확정은 이터 1 아키텍처 단계)
순수 함수 중심의 C 모듈. 시계·입출력은 주입(결정론, MC/DC 시험 용이).
`input_validator` → `command_handler` → `arbiter`(우선순위) → `state_machine` → `status_publisher`, `event_log`(링버퍼)
안전 로직(SR)과 QM 기능(FR-003 override)은 모듈로 분리해 간섭을 방지한다.

## 산출물 위치
요구사항 `docs/requirements/`, 설계 `docs/design/`, 리뷰 기록 `docs/reviews/`, 추적 `docs/traceability/`,
시스템 시험(엑셀) `docs/test/system/`. SBOM(G3)은 이터 1 구현 시점에 생성 방법을 정한다(권고: CycloneDX, 외부 의존성은 GoogleTest뿐).
