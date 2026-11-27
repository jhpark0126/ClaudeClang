# 소프트웨어 요구사항 (SWE.1) — 이터레이션 1

- 입력: `OEM/OEM-SWR-001_OEM SW 요구사항 사양서.docx` (BL-OEM-1.0), 분석 가정 `oem-swr-001-review.md` (A-01~A-20)
- 범위: OEM-SR-001~004, OEM-FR-007 (이터레이션 1). 나머지는 이터레이션 2~4에서 추가한다.
- 분리 규칙(확정): ASIL B에서 파생한 **기능·제한 요구**는 `safety-requirements.md`(SSR)에만 둔다. 이 문서는 QM 기능 요구, 인터페이스·품질·제약 요구, 공통 정의를 둔다. 인터페이스·품질 요구가 ASIL B 요구에서 파생되면 "상속 ASIL"로 표시한다. 인터페이스 요구는 기능 요구의 하위 분류이다.
- 교육용 가상 프로젝트다. 실제 OEM 승인, A-SPICE 능력수준, ISO 26262 준수, 인증, ASIL 달성을 주장하지 않는다.
- 작성: Claude Sonnet 5.5 (claude-sonnet-5-5), 2026-11-27, 요청자 jhpark0126. 사용자 리뷰: pending. 상태: 초안(재리뷰 대기).

## 1. 용어와 공통 정의
| 용어 | 정의 |
|---|---|
| 평가주기 | `EVAL_PERIOD_MS`=10 ms 마다 입력 스냅샷 1개를 평가하는 단위 (가정 A-03) |
| 필수 입력 (이터 1) | `crash_status`, `rear_left_approach_risk`, `rear_right_approach_risk`, `ignition_on`, `sensor_fault`. 차속, 기어, 운전자 명령 등은 이터 2부터 필수 여부를 정한다 |
| 입력의 나이 | SW 시계 기준 현재 시각 − 그 입력이 마지막으로 유효하게 갱신된 시각 (가정 A-20) |
| 유효 입력 | 필수 입력 중 INVALID도 STALE도 아닌 것 |
| 직전 확정 출력 | 직전 평가주기에 확정한 `lock_left`, `lock_right` ∈ {LOCK, RELEASE} |
| SW 시계 | 단조 증가하는 ms 단위 시각 소스(시각이 역행하지 않는다고 가정). 입력 수신 시각과 평가 시각을 읽는 데 쓴다. 시험에서는 지정한 값을 쓴다(SWR-Q-002) |
| SW 초기화 | 제어 SW 프로세스(또는 시험 대상 객체)를 초기 상태로 되돌리는 것. 차량 점화 사이클과 구별한다 |
| 상태 | NORMAL, DEGRADED, FAULT, OFF (표시 우선순위는 SSR-027) |
| 이유코드 | `CRASH_PENDING`, `APPROACH_RISK_LOCK`, `APPROACH_RISK_SUPPRESS`, `APPROACH_INPUT_UNRELIABLE`, `IGNITION_OFF`, `SENSOR_FAULT`(경고코드). 열거형 확정은 상세 설계(가정 A-10) |

## 2. 규칙 적용 순서 (비규범 요약: 규범은 각 SSR/SWR이고 이 절은 이해를 돕는다. 가정 A-01, A-17 반영)
1. 유효 CONFIRMED → 양쪽 RELEASE, 이후 유지 (SSR-001~005)
2. 도어별 유효 접근위험 TRUE → 그 도어 LOCK, 해제 요청 억제 (SSR-009~017). DEGRADED·FAULT에서도 적용
3. `ignition_on`=FALSE → RELEASE, OFF (SWR-F-001~003). 접근위험 LOCK 도어는 제외. DEGRADED·FAULT에서는 적용하지 않음 (SWR-F-004)
4. `sensor_fault`=TRUE 또는 DEGRADED → 나머지 도어는 직전 확정 출력 유지 (SSR-023, 024)
5. 해당 없음 → 직전 확정 출력 유지

## 3. 기능 요구사항 (SWR-F, QM)
| ID | 설명 | 분류 | 출처 | 검증 기준 | 상위 추적 | 하위 추적 |
|---|---|---|---|---|---|---|
| SWR-F-001 | SW는 `ignition_on`이 유효하고 FALSE이며 DEGRADED·FAULT가 아니면, 접근위험 LOCK 대상이 아닌 도어의 출력을 RELEASE로 해야 한다. | 기능 (QM) | OEM-FR-007 | 점화 OFF 첫 평가주기에 좌/우 RELEASE. 좌 접근위험 TRUE이면 좌 LOCK, 우 RELEASE | OEM-FR-007 | 미정 |
| SWR-F-002 | SW는 `ignition_on`이 유효하고 FALSE이며 DEGRADED·FAULT가 아니면 상태를 OFF로 해야 한다. | 기능 (QM) | OEM-FR-007 | 점화 OFF 첫 평가주기 state=OFF | OEM-FR-007 | 미정 |
| SWR-F-003 | SW는 SWR-F-001의 조건이 성립하는 평가주기마다(출력이 이미 RELEASE이더라도) 이유코드 `IGNITION_OFF`를 기록해야 한다. | 기능 (QM) | OEM-FR-007 | 이유코드 존재 | OEM-FR-007 | 미정 |
| SWR-F-004 | SW는 DEGRADED 또는 FAULT 상태에서는 점화 OFF에 의한 해제를 적용하지 않아야 한다. | 기능 (QM) | OEM-FR-007, 가정 A-17 | DEGRADED, FAULT에서 점화 OFF 주입 시 출력 불변 | OEM-FR-007, SSR-023, SSR-024 | 미정 |
| SWR-F-005 | SW는 SW 초기화 직후 첫 평가주기의 출력을 좌/우 RELEASE로 해야 한다. 단, 그 평가주기에 유효한 접근위험 TRUE 입력이 있으면 해당 도어는 SSR-009, 010을 따른다. | 기능 (QM) | 가정 A-06 | 새 객체에 아무 입력 없이 한 번 평가하면 출력 RELEASE | OEM-FR-007 (일관성) | 미정 |

## 4. 인터페이스 요구사항 (SWR-I)
| ID | 분류 | 상속 ASIL | 설명 | 출처 | 검증 기준 | 하위 추적 |
|---|---|---|---|---|---|---|
| SWR-I-001 | 인터페이스(기능) | B | SW는 `crash_status`(NONE/PENDING/CONFIRMED)를 OEM-IF-002 형식으로 입력받아야 한다. | OEM-IF-002 | 열거값 외 값은 INVALID | 미정 |
| SWR-I-002 | 인터페이스(기능) | B | SW는 `rear_left_approach_risk`, `rear_right_approach_risk`(boolean)를 OEM-IF-003 형식으로 입력받아야 한다. | OEM-IF-003 | 누락, 형식 오류는 INVALID | 미정 |
| SWR-I-003 | 인터페이스(기능) | QM | SW는 `ignition_on`(boolean)을 OEM-IF-009 형식으로 입력받아야 한다. | OEM-IF-009 | 누락, 형식 오류는 INVALID | 미정 |
| SWR-I-008 | 인터페이스(기능) | B | SW는 `sensor_fault`(boolean)를 OEM-IF-009 형식으로 입력받아야 한다. | OEM-IF-009 | 누락, 형식 오류는 INVALID | 미정 |
| SWR-I-004 | 인터페이스(기능) | B | SW는 운전자 해제 요청(OEM-IF-004의 side, action=unlock)을 입력받아 접근위험 억제 판단(SSR-011)에 전달해야 한다. `side`=all인 해제 요청은 좌/우 각각 독립으로 판단해, 억제되는 도어는 LOCK을 유지하고 나머지 도어에만 요청을 전달한다. 그 밖의 운전자 명령 동작은 이터 2에서 정한다. | OEM-IF-004 | 미등록 enum은 거절, 해제 요청이 억제 판단에 반영 | 미정 |
| SWR-I-005 | 인터페이스(기능) | B | SW는 `lock_left`, `lock_right`(LOCK/RELEASE)를 OEM-IF-005 형식으로 출력해야 한다. | OEM-IF-005 | 매 평가주기 정의된 값 출력 | 미정 |
| SWR-I-006a | 인터페이스(기능) | B | SW는 state를 호출자가 읽을 수 있는 값으로 제공해야 한다. 조회 계약 전체(OEM-IF-006)는 이터 3에서 정한다. | OEM-IF-006(일부) | 평가 후 state 조회 가능 | 미정 |
| SWR-I-006b | 인터페이스(기능) | B | SW는 이유코드(경고코드 포함)를 호출자가 읽을 수 있는 값으로 제공해야 한다. | OEM-IF-006(일부) | 평가 후 이유코드 조회 가능 | 미정 |
| SWR-I-007 | 인터페이스(기능) | B | SW는 입력 스냅샷마다 SW 시계로 읽은 수신 시각을 기록해야 한다. 입력 나이 계산은 수신 시각만 쓰고, 스냅샷의 `timestamp_s`(요청·소스 시각, 가정 A-12)는 나이 계산에 쓰지 않는다. (가정 A-20) | OEM-SR-003, 가정 A-12 | 같은 `timestamp_s`라도 수신 시각이 200 ms 넘게 벌어지면 STALE, `timestamp_s`만 달라도 수신 시각이 같으면 나이는 같음 | 미정 |

## 5. 비기능 요구사항
### 5.1 제한 (SWR-L)
| ID | 설명 | 출처 | 검증 기준 | 하위 추적 |
|---|---|---|---|---|
| SWR-L-001 | SW는 `EVAL_PERIOD_MS`=10 ms 마다 평가 결과를 생성해야 한다. (가정 A-03) | 가정 A-03 | 1000 ms 동안 입력을 유지하면 평가 결과 100개 | 미정 |

SSR-008(300 ms), SSR-022(100 ms)은 ASIL B 제한이므로 SSR 문서에 있다.

### 5.2 품질 (SWR-Q)
| ID | ISO 25010 | 설명 | 출처 | 검증 기준 | 하위 추적 |
|---|---|---|---|---|---|
| SWR-Q-001 | 신뢰성(결함 허용) | SW는 필수 입력 5개가 각각 {유효 값 전부, INVALID, 누락} 중 어느 조합이어도 비정상 종료 없이 출력이 {LOCK, RELEASE}, 상태가 {NORMAL, DEGRADED, FAULT, OFF} 중 하나가 되게 해야 한다. 상속 ASIL: B(OEM-SR-003) | OEM-SR-003 파생 | 입력 클래스 조합을 전수 실행해(crash 3값+INVALID+누락, boolean 입력 각 2값+INVALID+누락) 모든 결과가 정의 집합 안이고 종료함 | 미정 |
| SWR-Q-002 | 유지보수성(시험성) | SW는 시험이 실제 경과 시간 없이 입력 수신 시각을 지정해 200 ms, 300 ms 경계를 재현할 수 있어야 한다. | OEM-SR-001, 003 검증성 | 시험이 시각을 지정해 경계값 4점(199/200/201/300 ms)을 재현 | 미정 |
| SWR-Q-003 | 기능 적합성(정확성) | SW는 같은 초기 상태와 같은 입력 열에 같은 출력 열을 생성해야 한다. (OEM-NFR-001의 선행 성질, 해시 기반 확인은 이터 4) | OEM-NFR-001 | 같은 입력 열 1,000회 재생 시 출력·상태 열이 모두 일치 (공개결과 해시 정의는 이터 4, 가정 A-11) | 미정 |

### 5.3 제약 (SWR-C)
| ID | 설명 | 출처 | 검증 기준 |
|---|---|---|---|
| SWR-C-001 | 제품 코드는 C17, Clang 18 이상으로만 작성해야 한다. 사양서 3장 "Python 3.12" 실행환경과의 편차는 사용자가 2026-11-27 요구사항 변경으로 확정했다. (가정 A-19) | 사용자 결정, CLAUDE.md 4장 | CMake가 `src/*.cpp`를 거부하고 C로 컴파일 성공 |
| SWR-C-002 | 개인 식별 정보, 영상, 음성은 입력·저장하지 않아야 한다. | 사양서 2장 프로젝트 제약 | 입력 구조와 로그 필드에 해당 항목이 없음(리뷰) |
| SWR-C-003 | 제품 코드는 Cppcheck 규칙 집합(`ci/static_analysis.sh`의 `CPPCHECK_ENABLE`, 현재 `warning,style,performance,portability`, 사용자 확정 전 임시값)을 예외 없이 만족해야 한다. | CLAUDE.md 2.2, 미확정 사항 표 | CI 정적 분석 게이트 통과 |
| SWR-C-004 | 제품 코드의 함수는 순환복잡도 10 이하, 길이 80라인 이하여야 한다. | CLAUDE.md 2.3 | Lizard 게이트 통과 |

## 6. 추적: OEM 사양서 9장 힌트 ID와의 대응
사양서 9장은 `SWR-001`~`SWR-021`을 "계획"으로 언급하지만 내용이 없다(가정 A-15). 이 프로젝트는 ID 체계를 SSR-xxx, SWR-F/I/L/Q/C-xxx로 정했고, OEM 힌트 번호와 직접 대응하지 않는다. 대응은 OEM 요구 단위로 둔다.

| OEM 요구 | OEM 9장 힌트 ID | 이 프로젝트 ID |
|---|---|---|
| OEM-SR-001 | SWR-007, 008 | SSR-001~008 |
| OEM-SR-002 | SWR-005, 006, 009 | SSR-009~017 (재입력 override 006은 이터 2) |
| OEM-SR-003 | SWR-013 | SSR-018~023, 027 |
| OEM-SR-004 | SWR-021 | SSR-024~027 |
| OEM-FR-007 | SWR-020 | SWR-F-001~005 |
| OEM-NFR-001 | SWR-016 | SWR-Q-003 (이터 4에서 완성) |

상위가 OEM 요구가 아닌 요구(`trace.csv` 대상 아님): SWR-L-001(가정 A-03), SWR-C-001~004(사용자 결정, 사양서 2장, CLAUDE.md). 이들은 출처 열에 근거를 둔다.
`trace.csv`의 하위 열(design, code, unit_test, integration_test, system_test)은 해당 단계 산출물이 생길 때 채운다. 적용 시점은 리뷰 체크리스트 REQ-11 해석과 함께 사용자 결정 대기(아래 결과 보고 참조).

## 7. 미해결·가정 의존과 영향
OEM 회신으로 아래 가정이 바뀌면 표의 요구를 다시 분석한다.

| 가정 | OEM 질의 | 영향받는 요구 |
|---|---|---|
| A-01 우선순위 | Q1 | SSR-003, 009, 010, 015, SWR-F-001, 004, 2장 전체 |
| A-02 FAULT 중 CONFIRMED | Q2 | SSR-005 |
| A-03 평가주기 | Q3 | SWR-L-001, SSR-008, 022 |
| A-04/05 DEGRADED | Q4 | SSR-020~023, 021a |
| A-06 초기 출력 | Q5 | SWR-F-005 |
| A-07 PENDING | Q6 | SSR-006, 007 |
| A-10 열거형 | Q9 | 1장 이유코드·상태 정의, SWR-I-006a/b |
| A-11 해시 | Q10 | SWR-Q-003 (이터 4) |
| A-12 timestamp_s 역할 | - | SWR-I-004, SWR-I-007 |
| A-13 ASIL B-QM 간섭 | Q11 | SSR 간섭 방지 절, SWR-F-001~004 |
| A-16 CONFIRMED 래치 | Q13 | SSR-002 |
| A-17 FAULT·DEGRADED 규칙과 상태 우선순위 | Q14 | SSR-004, 014, 021, 021a, 023, 024, 027, SWR-F-002, 004 |
| A-18 접근위험 입력 무효 처리 | Q15 | SSR-016a/b, 017 |
| A-19 언어 편차 | Q16 | SWR-C-001 |
| A-20 입력 나이 기준 | Q17 | SSR-020, SWR-I-007 |
| A-21 접근위험 LOCK 원인 기록 | Q18 | SSR-012a |

## 변경 이력
| 일자 | 내용 |
|---|---|
| 2026-11-27 | 초안. 리뷰(requirements-review-2026-11-27, 반려 27건) 반영: ASIL B 요구를 SSR로 이관, 요구 분할, 용어·규칙 순서·가정 A-16~A-20 추가, 인터페이스 ID 부여, 대응표와 영향 목록 추가 |
| 2026-11-27 | 재리뷰(r2) 반영: I-003/I-006 분할(I-008, I-006a/b), 분류·상속 ASIL 열, SW 시계 정의, F-001/003/005 조건 보강, Q-001/Q-003 기준 재작성, 가정 A-10~A-13, A-21 영향 목록 추가 |

### 초안 ID → 현재 ID 대응 (폐기·개명)
첫 초안(미베이스라인)의 ID는 아래로 대응한다: SWR-F-001→SSR-001, F-002→SSR-002, F-003→SSR-006·007, F-004/005→SSR-009/010, F-006→SSR-011·012, F-007→SSR-013, F-008→SSR-003, F-009→SSR-018·019, F-010→SSR-020, F-011→SSR-021·022, F-012→SSR-004·023, F-013→SSR-024, F-014→SSR-025·026, F-015→SSR-005, F-016→SWR-F-001~003, F-017→SSR-015, F-018→SWR-F-005, L-001→SSR-008, L-002→SSR-022, L-003→SWR-L-001, C-003→SWR-C-003·004.
