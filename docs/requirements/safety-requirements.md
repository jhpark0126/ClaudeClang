# 소프트웨어 안전 요구사항 (SSR) — 이터레이션 1

- ASIL B 분류는 OEM이 입력으로 제공한 값이다. 이 프로젝트는 HARA·ASIL 도출을 하지 않으며 ISO 26262 준수나 ASIL 달성을 주장하지 않는다.
- 분리 규칙(CLAUDE.md 1.2): ASIL B로 할당된 요구(OEM-SR-001~004)에서 파생한 요구는 **모두 이 문서**에 둔다. `software-requirements.md`(SWR)에는 QM 요구, 인터페이스, 비기능 요구만 둔다.
- 작성: Claude Sonnet 5.5 (claude-sonnet-5-5), 2026-11-27, 요청자 jhpark0126. 사용자 리뷰: pending. 상태: 초안(재리뷰 대기).
- 용어·시간 기준·규칙 적용 순서는 `software-requirements.md` 1~2장을 따른다.

각 요구는 한 사항만 말한다. "ASIL" 열은 상위 OEM 요구의 분류를 상속한다. 하위 추적(ARCH/UT/IT)은 이후 단계에서 채운다.

## 1. 충돌 CONFIRMED (OEM-SR-001)
| ID | ASIL | 설명 | 상위 | 검증 기준 |
|---|---|---|---|---|
| SSR-001 | B | SW는 유효한 `crash_status`=CONFIRMED를 수신한 평가주기에 좌/우 출력을 RELEASE로 정해야 한다. | OEM-SR-001 | CONFIRMED 주입 후 그 평가주기 출력이 좌/우 모두 RELEASE |
| SSR-002 | B | SW는 `crash_status`=CONFIRMED를 수신한 뒤, SW 초기화 전까지 모든 평가주기에서 좌/우 출력을 RELEASE로 유지해야 한다. (가정 A-16) | OEM-SR-001 | CONFIRMED 이후 `crash_status`=NONE, 접근위험 TRUE, 점화 ON을 주입해도 출력 RELEASE |
| SSR-003 | B | SW는 `crash_status`=CONFIRMED가 성립하면 접근위험 입력의 LOCK 결과보다 RELEASE를 우선해야 한다. (가정 A-01) | OEM-SR-001, OEM-SR-002 | CONFIRMED + 좌/우 접근위험 TRUE에서 출력 RELEASE |
| SSR-004 | B | SW는 DEGRADED 상태에서도 유효한 `crash_status`=CONFIRMED를 적용해야 한다. (가정 A-17) | OEM-SR-001, OEM-SR-003 | DEGRADED에서 CONFIRMED 주입 시 출력 RELEASE |
| SSR-005 | B | SW는 FAULT 상태에서도 유효한 `crash_status`=CONFIRMED를 적용해야 한다. (가정 A-02) | OEM-SR-001, OEM-SR-004 | `sensor_fault`=TRUE에서 CONFIRMED 주입 시 출력 RELEASE |
| SSR-006 | B | SW는 `crash_status`=PENDING이면 좌/우 출력을 바꾸지 않아야 한다. (가정 A-07) | OEM-SR-001 | PENDING 주입 시 출력 불변 |
| SSR-007 | B | SW는 `crash_status`=PENDING을 처리하면 이유코드 `CRASH_PENDING`을 기록해야 한다. (가정 A-07) | OEM-SR-001 | PENDING 주입 시 이유코드 기록 |
| SSR-008 | B | SW는 CONFIRMED 스냅샷 수신 시각(SW 시계 기준)으로부터 300 ms 이내에 좌/우 출력을 RELEASE로 만들어야 한다. | OEM-SR-001 | 주입 시각 T에 수신한 스냅샷이면 T+300 ms 이전 출력 RELEASE (평가주기 10 ms에서 같은 주기) |

## 2. 접근위험 (OEM-SR-002)
| ID | ASIL | 설명 | 상위 | 검증 기준 |
|---|---|---|---|---|
| SSR-009 | B | SW는 `rear_left_approach_risk`가 유효하고 TRUE이면 좌측 출력을 LOCK으로 정해야 한다. (CONFIRMED가 아닐 때) | OEM-SR-002 | 좌 위험 TRUE에서 좌 출력 LOCK |
| SSR-010 | B | SW는 `rear_right_approach_risk`가 유효하고 TRUE이면 우측 출력을 LOCK으로 정해야 한다. (CONFIRMED가 아닐 때) | OEM-SR-002 | 우 위험 TRUE에서 우 출력 LOCK |
| SSR-011 | B | SW는 접근위험이 TRUE인 도어에 대한 해제 요청을 적용하지 않아야 한다. | OEM-SR-002 | 좌 위험 TRUE에서 좌 해제 요청 시 좌 LOCK 유지 |
| SSR-012 | B | SW는 해제 요청을 억제하면 이유코드 `APPROACH_RISK_SUPPRESS`를 기록해야 한다. | OEM-SR-002 | 억제 시 이유코드 존재 |
| SSR-013 | B | SW는 좌/우 접근위험 판단을 서로 독립적으로 수행해야 한다. | OEM-SR-002 | 좌 위험만 TRUE일 때 우 출력이 좌 입력과 무관하게 결정됨 |
| SSR-014 | B | SW는 DEGRADED 또는 FAULT 상태에서도 유효한 접근위험 TRUE 도어를 LOCK해야 한다. (가정 A-17) | OEM-SR-002, OEM-SR-003, OEM-SR-004 | DEGRADED, FAULT 각각에서 유효 위험 TRUE 시 해당 도어 LOCK |
| SSR-015 | B | SW는 접근위험 TRUE 도어의 LOCK을 점화 OFF 해제보다 우선해야 한다. (가정 A-01) | OEM-SR-002 | 점화 OFF + 좌 위험 TRUE에서 좌 LOCK, 우 RELEASE |
| SSR-016 | B | SW는 한 도어의 접근위험 입력이 INVALID 또는 STALE이면 그 도어 출력을 직전 확정 출력으로 유지하고 해제 요청을 적용하지 않아야 한다. (가정 A-18) | OEM-SR-002, OEM-SR-003 | 무효·stale 위험 입력에서 그 도어 출력 불변, 해제 요청 미적용 |
| SSR-017 | B | SW는 SSR-016에 해당하면 이유코드 `APPROACH_INPUT_UNRELIABLE`을 기록해야 한다. (가정 A-18) | OEM-SR-002, OEM-SR-003 | 이유코드 존재 |

## 3. 입력 유효성과 freshness (OEM-SR-003)
| ID | ASIL | 설명 | 상위 | 검증 기준 |
|---|---|---|---|---|
| SSR-018 | B | SW는 필수 입력이 누락, 형식 오류, 범위 오류, 미등록 열거값 중 하나이면 그 입력을 INVALID로 표시해야 한다. | OEM-SR-003 | 입력별 오류 벡터마다 INVALID 표시 |
| SSR-019 | B | SW는 INVALID 또는 STALE인 입력을 어떤 판단 규칙에도 사용하지 않아야 한다. | OEM-SR-003 | INVALID 값이 TRUE여도 출력 결정에 영향 없음 |
| SSR-020 | B | SW는 한 필수 입력의 나이(마지막 유효 갱신 후 경과 시간)가 200 ms를 초과하면 그 입력을 STALE로 표시해야 한다. (가정 A-20) | OEM-SR-003 | 나이 200 ms 유효, 나이 210 ms STALE |
| SSR-021 | B | SW는 필수 입력 중 하나라도 STALE이면 그 평가주기에 상태를 DEGRADED로 해야 한다. (가정 A-17) | OEM-SR-003 | STALE 판정 주기에 state=DEGRADED |
| SSR-022 | B | SW는 입력 나이가 200 ms를 초과한 시점으로부터 100 ms 이내에 DEGRADED를 표시해야 한다. | OEM-SR-003 | 나이 200 ms 초과 후 100 ms 이내 DEGRADED (평가주기 10 ms에서 최대 1주기) |
| SSR-023 | B | SW는 DEGRADED 상태에서 접근위험 LOCK(SSR-014)과 CONFIRMED(SSR-004)에 해당하지 않는 도어 출력을 직전 확정 출력으로 유지해야 한다. (가정 A-17) | OEM-SR-003 | DEGRADED에서 점화 OFF 주입 시 출력 불변 |

## 4. sensor_fault (OEM-SR-004)
| ID | ASIL | 설명 | 상위 | 검증 기준 |
|---|---|---|---|---|
| SSR-024 | B | SW는 `sensor_fault`=TRUE이면 새 명령(운전자 명령, 점화 OFF 해제 포함)을 적용하지 않고 좌/우 출력을 직전 확정 출력으로 유지해야 한다. 단, SSR-005와 SSR-014는 적용한다. (가정 A-17) | OEM-SR-004 | `sensor_fault`=TRUE 첫 평가주기에 출력 불변 |
| SSR-025 | B | SW는 `sensor_fault`=TRUE인 첫 평가주기에 상태를 FAULT로 해야 한다. | OEM-SR-004 | 첫 평가주기 state=FAULT |
| SSR-026 | B | SW는 `sensor_fault`=TRUE인 동안 경고코드 `SENSOR_FAULT`를 제공해야 한다. | OEM-SR-004 | 경고코드 존재 |
| SSR-027 | B | SW는 상태가 둘 이상 성립하면 FAULT, DEGRADED, OFF, NORMAL 순으로 우선하여 하나를 표시해야 한다. (가정 A-17) | OEM-SR-003, OEM-SR-004 | `sensor_fault`와 STALE 동시 → FAULT, STALE과 점화 OFF 동시 → DEGRADED |

## 간섭 방지 (QM과의 분리)
QM 요구 FR-007(점화 OFF 해제)은 SSR-001~005, SSR-009~015의 결과를 약화하지 못한다. 순위는 CONFIRMED > 접근위험 LOCK > 점화 OFF 해제이고, DEGRADED·FAULT에서는 점화 OFF 해제가 적용되지 않는다(SSR-023, SSR-024). 이터 2의 FR-003 override 간섭 방지는 이터 2 아키텍처에서 다룬다.

## 안전 영향 분석 메모
- 기동 직후 기본 출력 RELEASE(SWR-F-005, 가정 A-06)는 입력이 모두 비어 있는 동안 RELEASE를 유지한다. 필수 입력이 없으면 STALE이므로 DEGRADED이고, 접근위험이 유효해지면 SSR-014에 따라 LOCK한다. OEM이 다른 기본값을 정하면 Q5로 바꾼다.

## 변경 이력
| 일자 | 내용 |
|---|---|
| 2026-11-27 | 초안. 리뷰(requirements-review-2026-11-27, 반려 27건)를 반영해 ASIL B 파생 요구를 이 문서로 이관하고 요구마다 한 사항으로 분할 |
