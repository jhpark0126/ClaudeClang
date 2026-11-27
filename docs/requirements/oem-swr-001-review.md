# OEM-SWR-001 검토 기록과 분석 가정

- 대상: `OEM/OEM-SWR-001_OEM SW 요구사항 사양서.docx` (Revision 1.0, BL-OEM-1.0)
- 요구사항 목록(기계 판독): `oem-requirements.csv`, 추적 매트릭스: `../traceability/trace.csv`
- 이 문서는 사양서의 모호점을 정리하고, **OEM 확정 전까지 개발이 따를 분석 가정**을 정한다.
  가정은 OEM이 답하면 바꾸고, 바꾼 이력은 아래 "변경 이력"에 남긴다.
- 교육용 가상 프로젝트다. 실제 OEM 승인, Automotive SPICE 능력수준, ISO 26262 준수, 인증, ASIL 달성, 양산 적합성을
  나타내지 않는다. (저작권 © 2026 Synetics, 교육 과정 안 열람·복제·실습 허용. 저장소 공개 배포는 사용자가 승인함.)

## 1. 확정된 개발 전제
| 항목 | 값 | 근거 |
|---|---|---|
| 제품 코드 언어 | **C (Clang 22, C17)** | 요구사항 변경으로 사용자가 확정. 사양서의 "Python 3.12" 실행환경 항목은 대체됨 |
| 시험 | GoogleTest/GMock (C++20)로 C 코드 시험 | CLAUDE.md 4장 |
| 범위 | PC/SIL 논리까지. HIL, 실차, ECU/HW, HARA, 인증은 범위 밖 | 사양서 1.3 |
| Web 시뮬레이터(사양서 3장) | 제품 범위에서 **분리**: 제품 코드는 상태 조회 계약(OEM-IF-006)을 C 구조체/함수로 제공. HTTP 표시는 시험 도구로 후순위 | 가정 A-09 |

## 2. 요구사항 간 우선순위 (가정 A-01)
위가 높다. 위 항목의 조건이 성립하면 아래 항목의 결과를 덮어쓴다. 좌/우 도어는 독립적으로 판단한다.

| 순위 | 조건(유효 입력) | 결과 | 요구사항 |
|---|---|---|---|
| 1 | crash_status = CONFIRMED | 양쪽 RELEASE, 이후 주기에도 유지 | OEM-SR-001 |
| 2 | fire_detected 또는 overtemperature_detected | 양쪽 RELEASE | OEM-FR-005 |
| 3 | 해당 도어 approach_risk = TRUE | 그 도어 LOCK, 해제 요청 억제 (FR-003 override 제외) | OEM-SR-002 |
| 4 | adult_present = TRUE | 양쪽 RELEASE | OEM-FR-005 |
| 5 | 해당 도어 isofix = TRUE | 그 도어 LOCK | OEM-FR-006 |
| 6 | ignition_on = FALSE | 양쪽 RELEASE, state OFF | OEM-FR-007 |
| 7 | 유효 차속 ≥ 3 km/h | 양쪽 LOCK | OEM-FR-002 |
| 8 | 운전자 명령 (side, action) | 선택한 도어에만 적용 | OEM-FR-001 |
| 9 | (해당 없음) | 직전 출력 유지 | - |

- 입력 유효성(OEM-SR-003)과 sensor_fault(OEM-SR-004)는 순위표보다 앞서 적용한다: 무효 입력은 그 입력을 쓰는 규칙에서 제외하고,
  sensor_fault = TRUE이면 어떤 새 명령도 적용하지 않고 직전 확정 출력을 유지한다. (단, 순위 1의 유효 crash CONFIRMED는
  안전 방향이므로 sensor_fault 중에도 적용한다 — 가정 A-02, OEM 확인 필요)

## 3. 사양서 이슈와 분석 가정
| ID | 이슈 | 가정(기본 처리) | OEM 질의 |
|---|---|---|---|
| A-01 | 요구 간 우선순위 미정의(2장 표) | 위 표 | Q1 |
| A-02 | sensor_fault 중 crash 처리 | 유효 crash CONFIRMED는 적용 | Q2 |
| A-03 | 평가 주기 값 없음 | 10 ms 고정 주기(설정 상수 `EVAL_PERIOD_MS`) | Q3 |
| A-04 | SR-003 "200 ms 초과 시 100 ms 이내 DEGRADED" 기준 시점 | 마지막 갱신 후 200 ms 초과를 감지한 평가주기부터 100 ms 이내 = 최악 300 ms 이내 | Q4 |
| A-05 | DEGRADED 상태의 출력 동작 없음 | 직전 확정 출력 유지 + 해당 입력 무효 처리, 순위 1~2는 유효 입력이면 적용 | Q4 |
| A-06 | 기동 직후 "직전 확정 출력" 없음 | 초기 출력 RELEASE (FR-007과 일관) | Q5 |
| A-07 | crash_status = PENDING 처리 없음(추적성에만 언급) | 출력 변경 없음, 이유코드 기록 | Q6 |
| A-08 | "일반 해제 허용 조건" 본문에 없음(추적성 SWR-004) | 차속 < 3 km/h 이고 순위 1~6 조건이 없을 때만 운전자 해제 허용 | Q7 |
| A-09 | Web 시뮬레이터는 C 제품 범위인가 | 분리(1장) | Q8 |
| A-10 | state/reason_code 열거형 없음 | 상세 설계에서 정의하고 OEM에 승인 요청 | Q9 |
| A-11 | NFR-001 "공개결과 해시" 대상·알고리즘 없음 | 공개 상태 구조체의 직렬화 바이트에 대한 CRC-32 | Q10 |
| A-12 | IF-004에 요청 시각 필드 없음 | VehicleSnapshot.timestamp_s 사용(사양서 명시), 단조 증가 가정 | - |
| A-13 | FR-003 override(QM)와 SR-002(ASIL B)의 간섭 | override는 순위 3 억제를 **명시 상태**로만 우회하며, 안전 로직과 분리된 모듈에서 판단. 간섭 방지 근거를 아키텍처 문서에 남김 | Q11 |
| A-14 | FR-001 음성/모바일 경로 | IF-004 enum 값으로만 처리(경로별 동작 차이 없음) | - |
| A-15 | 추적성의 SWR-001~021 내용 없음, SWR-019 미연결 | 이터레이션 1~4의 분석 단계에서 SWR을 작성하고 SWR-019 용도를 OEM에 질의 | Q12 |

## 4. OEM 질의 목록
Q1 우선순위 표 승인 / Q2 sensor_fault 중 crash / Q3 평가 주기 / Q4 DEGRADED 시간 기준과 출력 / Q5 초기 출력 /
Q6 crash PENDING / Q7 일반 해제 허용 조건 / Q8 Web 시뮬레이터 범위 / Q9 state·reason_code 열거형 /
Q10 결정론 해시 정의 / Q11 ASIL B-QM 간섭 방지 방식 / Q12 SWR-019

## 변경 이력
| 일자 | 내용 |
|---|---|
| 2026-11-27 | 초안 작성 (이터레이션 0). OEM 회신 전까지 위 가정으로 진행 |
