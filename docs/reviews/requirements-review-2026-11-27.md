# 리뷰 기록: 소프트웨어 요구사항 (SWE.1, 이터레이션 1)
- 대상 버전/커밋: 작업 트리 기준 (브랜치 feature/iteration-1, 커밋 93bd353 이후 미커밋 변경 포함)
  - `docs/requirements/software-requirements.md`, `docs/requirements/safety-requirements.md`
  - `docs/traceability/trace.csv` (이터레이션 1 행), `docs/requirements/oem-swr-001-review.md` (Q13, 변경 이력)
  - 원본 입력: `OEM/OEM-SWR-001_OEM SW 요구사항 사양서.docx` (BL-OEM-1.0), 섹션 5/6/9
- 체크리스트: `.claude/skills/review/checklists/requirements.md` (REQ-01~REQ-13). 기준 A-SPICE 버전이 체크리스트에 명시되어 있지 않고, 대조할 A-SPICE 문서가 저장소에 없음 (미확인)
- 리뷰어: reviewer 서브에이전트 (Claude Haiku 5.5, claude-haiku-5-5) / 작성자: Claude Sonnet 5.5 (요청자 jhpark0126) / 일자: 2026-11-27

## 결과 요약
**반려** (NG 12건 / 체크리스트 13항목 중 OK 1건, 결함 27건)

승인 조건(모든 항목 OK 또는 사유 있는 N/A)을 충족하지 못했다. 조건부 승인은 없다.

## 항목별 판정
| 항목 ID | 판정 | 비고 (근거 / 결함 ID) |
|---|---|---|
| REQ-01 고유 ID | NG | SWR-F/L/Q/C, SSR ID는 중복 없음을 확인. 단 3장 인터페이스 요구사항에 ID가 없음 (D-20) |
| REQ-02 분류 | NG | 1~4장 구획으로 기능/제한/품질/제약 구분은 있음. 그러나 ASIL B 유래 SWR에 안전 등급 귀속이 없고 SWR-L-003의 분류가 설계 제약에 해당 (D-19) |
| REQ-03 SSR 분리 | NG | 헤더(L5)는 ASIL B 요구를 SSR 문서에서만 관리한다고 하나, 본문 2장 SWR-F-001~015, F-017 및 4.1장 SWR-L-001/002가 ASIL B 요구를 본문에 기술 (D-01) |
| REQ-04 품질 특성 | OK | 4.2장 SWR-Q-001 신뢰성(결함 허용), Q-002 유지보수성(시험성), Q-003 기능 적합성(정확성)으로 지정됨 (software-requirements.md L55~57) |
| REQ-05 원자성 | NG | 복수 사항을 한 요구에 기술 (D-17) |
| REQ-06 모호어 | NG | "필수 입력", "재기동", "갱신 시각", "시간 한계" 등 정의되지 않은 용어 (D-10, D-11, D-13, D-26의 SSR-003) |
| REQ-07 검증 기준 | NG | 측정 불가 또는 이터 1 범위 밖 입력에 의존하는 검증 기준 다수 (D-09, D-12, D-14, D-15, D-18, D-26) |
| REQ-08 구현 방법 배제 | NG | SWR-Q-002 "주입받아", SWR-L-003 "설정 상수로 정의해야" (D-18) |
| REQ-09 일관성·모순 | NG | 본문 내 직접 모순 3건 이상, 상위 산출물과 불일치 (D-02~D-07, D-14, D-16, D-27) |
| REQ-10 상향 추적 | NG | 4.2장 Q 표와 4.3장 C 표에 출처 열 없음, trace.csv에 상향 행 없음 (D-21, D-22) |
| REQ-11 하향 추적 | NG | trace.csv의 design/code/unit_test/integration_test/system_test 열이 모든 행에서 비어 있음 (D-23) |
| REQ-12 추적 매트릭스 실측 | NG | 열어서 확인: SWR 7개 누락, SSR 미포함 (D-24, D-25) |
| REQ-13 영향 분석·변경 이력 | NG | SWR/SSR 문서에 변경 이력 없음, 영향 분석 목록 불완전 (D-26) |

## 결함 목록
| 결함 ID | 위치 | 내용 | 조치 | 상태 |
|---|---|---|---|---|
| D-01 | software-requirements.md L5, 2장, 4.1장 | 헤더는 ASIL B 요구를 safety-requirements.md에서만 관리한다고 쓰지만, ASIL B 유래 SWR-F-001~015, F-017, L-001, L-002가 본문에 있음 (REQ-03) | 본문의 ASIL B 요구를 SSR 문서로 이관하고 SWR에는 참조만 남기거나, 분리 규칙(CLAUDE.md 1.2)의 해석을 사용자와 확정해 문서에 기록 | 미조치 |
| D-02 | software-requirements.md L36(F-016)와 L37(F-017), L73 | F-016은 ignition_on FALSE이면 좌/우 RELEASE로 "정해야 한다"고 무조건 기술. F-017은 같은 조건에서 접근위험 TRUE 도어 LOCK 유지. 직접 모순 (REQ-09) | F-016에 "접근위험 TRUE 도어는 제외(F-017 적용)" 조건 추가, SSR-002에 반영 | 미조치 |
| D-03 | software-requirements.md L24~27, L28(F-008), safety-requirements.md L9 | F-004/F-005/F-006과 SSR-002는 접근위험 TRUE에 무조건 LOCK/억제. F-008과 SSR-001은 CONFIRMED이면 접근위험보다 RELEASE 우선. 예외가 F-004~006과 SSR-002에 없음 (REQ-09) | F-004~006과 SSR-002에 F-008 예외를 명시하고 A-01 Q1을 참조 | 미조치 |
| D-04 | software-requirements.md L25, L33 / oem-swr-001-review.md L33~35 / software-requirements.md L69~70 | sensor_fault TRUE(F-013: 출력 불변)와 접근위험 TRUE(F-004/005: LOCK)가 동시에 성립하면 모순. oem-swr-001-review.md 2장은 sensor_fault를 순위표보다 앞서 적용(홀드)한다고 하고, software-requirements.md 5장은 sensor_fault를 순위 4(접근위험 3 이후)로 둠. 상위 산출물 내부 불일치. OEM-SR-002와 OEM-SR-004 원문 자체도 충돌하며 OEM 질의 목록(Q1~Q13)에 이 충돌이 없음 | (a) 두 문서의 적용 순서를 하나로 맞춤, (b) 충돌 해소 규칙을 SWR/SSR에 명시, (c) OEM 질의(Q14)로 등록 | 미조치 |
| D-05 | software-requirements.md L32(F-012), L24~25 / oem-swr-001-review.md L44 (A-05) | DEGRADED에서 직전 출력 유지(F-012, A-05)이므로 DEGRADED 중 유효한 접근위험 TRUE가 LOCK을 만들지 못함. A-05는 DEGRADED에서 순위 1~2만 적용. 접근위험 LOCK(SR-002)과 충돌하며 안전 공백 (REQ-09) | DEGRADED에서 접근위험 유효 입력의 처리를 명시 결정하고 F-012, A-05, SSR-003에 반영, OEM 질의(Q4) 범위 확장 | 미조치 |
| D-06 | software-requirements.md L34(F-014 FAULT), L36(F-016 OFF), L31(F-011 DEGRADED) | sensor_fault와 ignition FALSE가 동시에 성립하면 state가 FAULT인지 OFF인지 정의되지 않음. DEGRADED와 FAULT의 우선순위도 없음. 상태 집합(정상 상태 포함)도 정의되지 않음 (REQ-09, REQ-06) | 상태 우선순위표와 상태 집합 정의를 추가 (A-10/Q9 연계) | 미조치 |
| D-07 | safety-requirements.md L10(SSR-003), L11(SSR-004) / software-requirements.md L32(F-012), L35(F-015) | SSR-003/004 본문은 "직전 출력 유지"만 쓰고 CONFIRMED 예외가 없음. SWR-F-012/F-015는 예외를 추가함. 상위(SSR)와 하위(SWR) 불일치 (REQ-09) | SSR-003/004에 예외를 반영하거나 SWR을 SSR에 맞춤 | 미조치 |
| D-08 | software-requirements.md L22(F-002) / oem-swr-001-review.md L60, 변경 이력, 가정 표 | F-002의 "재기동 전까지" 래치는 원문 OEM-SR-001 수용기준 "이후 입력 주기에도 유지"보다 강한 해석이다. 가정 표(A-01~A-15)에 항목이 없고, F-002의 출처 열에도 가정 표시가 없음 (과잉 의심, 추적성) | 가정을 A-16으로 등록하고 F-002 출처 열에 "가정 A-16 / Q13"을 표시. Q13 회신 전까지 가정으로 명시 | 미조치 |
| D-09 | software-requirements.md L22(F-002 검증 기준), L41, L72 | F-002 검증 기준이 "LOCK 명령"과 "차속 3 km/h 이상"을 주입하라고 함. 3장과 5장은 이터 1에서 차속은 형식 검증만, 운전자 명령은 억제 경로만 쓴다고 함. 이터 1에서 이 시험은 성립하지 않음 (REQ-07) | 이터 1에서 관측 가능한 입력으로 검증 기준을 다시 쓰거나 이터 2로 이관 | 미조치 |
| D-10 | software-requirements.md L22, L78 / oem-requirements 원문 OEM-NFR-002 "프로세스 재기동" | "재기동"이 F-002와 L78에서는 차량 재기동(점화 사이클) 의미로 쓰이고, OEM-NFR-002에서는 프로세스 재기동 의미임. 두 의미가 섞여 있음 (REQ-06) | "재기동"을 용어 표에 정의하고 F-002에서 정의된 용어만 사용 | 미조치 |
| D-11 | software-requirements.md L29(F-009), L31(F-011), L47~48(L-001/L-002) | "필수 입력"이 1장 용어 표에 정의되어 있지 않고, 입력별 필수/선택 구분도 없음 (REQ-06) | 1장에 "필수 입력"을 정의하고 입력별 필수 여부 표 추가 (IF-001~IF-009 기준) | 미조치 |
| D-12 | software-requirements.md L47(L-001 검증 기준 "입력 시각 기준") | 입력 시각의 정의가 없음. OEM-IF-002(crash_status), IF-003, IF-007~009에는 시각 필드가 없고 IF-001과 IF-004만 시각을 가짐 (REQ-07) | 시각 기준(수신 시각 또는 스냅샷 시각)과 측정 방법을 정의 | 미조치 |
| D-13 | software-requirements.md L30(F-010), L31(F-011) | "한 입력의 마지막 갱신"을 판정할 시각 정보가 OEM 인터페이스 계약에 없음. 입력별 freshness 판정 기준이 미정 (REQ-06, REQ-07, 원문 누락) | 입력별 갱신 시각 판정 기준을 정의하고 필요 시 OEM 질의 등록 | 미조치 |
| D-14 | software-requirements.md L48(L-002 검증 기준 "최악 300 ms"), oem-swr-001-review.md L43 (A-04) | 10 ms 평가주기에서 STALE 감지는 마지막 갱신 후 최대 210 ms. 그 뒤 100 ms를 더하면 최대 310 ms로 "최악 300 ms"와 맞지 않음. 또한 L-002 기준점("감지한 평가주기로부터")과 검증 기준의 기준점("최악")이 다르고, F-011은 같은 평가주기 전환을 요구해 L-002와 중복/불일치 (REQ-07, REQ-09) | 기준점을 하나로 정하고 산식을 수정 (210+100 또는 F-011 기준) | 미조치 |
| D-15 | software-requirements.md L38(F-018 검증 기준 "첫 평가 전 출력 RELEASE") | 첫 평가 전의 출력은 외부에서 관측되는 인터페이스가 없어 측정 불가. 기동 직후 기본값 RELEASE가 접근위험 미확인 상태에서 SR-002를 약화하는지도 분석되지 않음 (REQ-07, 누락) | 관측 가능한 검증 기준으로 재작성하고 기동 직후 기본값의 안전 영향을 분석해 SSR 또는 A-06에 기록 | 미조치 |
| D-16 | software-requirements.md L29(F-009), oem-swr-001-review.md L44 (A-05) | 접근위험 입력이 무효 또는 STALE이면 "평가에서 제외"하고 A-05에 따라 직전 출력을 유지함. 직전 출력이 RELEASE이면 접근위험 TRUE가 있어도 RELEASE가 유지될 수 있음. OEM-SR-003이 요구하는 "정의된 방식"(원문 섹션 5)이 접근위험 입력에 대해 정의되지 않음 (누락, REQ-09) | 접근위험 입력의 무효/STALE 시 안전측 반응을 정의하고 SSR-002/003에 반영, OEM 질의 등록 | 미조치 |
| D-17 | software-requirements.md L23(F-003), L26(F-006), L29(F-009), L30(F-010), L32(F-012), L34(F-014), L36(F-016), L22(F-002), safety-requirements.md L8(SSR-001) | 한 요구에 복수 사항이 있음 (예: F-006 억제+이유코드 기록, F-014 상태+경고코드, F-016 출력+상태+이유코드, F-009 표시+제외). REQ-05 위반 | 각 사항을 분할하고 ID를 부여 | 미조치 |
| D-18 | software-requirements.md L55(SWR-Q-002), L49(SWR-L-003) | Q-002는 "시계와 입출력을 주입받아"라고 설계 수단을 지정함 (REQ-08). L-003은 "설정 상수로 정의해야 한다"는 설계 선택이며, 검증 기준 "시험이 따라 변함"은 합격/불합격 기준이 아님 (REQ-07, REQ-08). L-003은 제한으로 분류되어 있으나 성격은 설계 제약 (REQ-02) | Q-002를 "시간·입력을 시험에서 제어할 수 있다" 같은 성질로 재작성. L-003은 주기 값 자체를 요구로 쓰고 검증 기준을 합격/불합격으로 다시 씀 | 미조치 |
| D-19 | software-requirements.md 2장 표 전체, safety-requirements.md 헤더 | ASIL B 유래 SWR(F-001~015, F-017, L-001/002)에 ASIL 등급 또는 QM 구분 열이 없음. 안전 등급 귀속이 누락됨 (REQ-02, 확인 포인트 "분류 누락") | 기능/제한 표에 ASIL 또는 QM 열을 추가하고 OEM 분류(OEM-SR은 ASIL B, OEM-FR은 QM)를 상속 | 미조치 |
| D-20 | software-requirements.md 3장 L41 | 인터페이스 요구사항 절이 입력 소비 목록을 산문으로만 쓰고 ID가 없음. 출력 IF-005(LOCK/RELEASE 열거)와 IF-006 일부(상태·이유코드 제공)도 요구 ID로 존재하지 않음 (REQ-01, REQ-10, 누락) | 인터페이스 요구에 ID(예: SWR-I-xxx)를 부여하고 trace.csv에 연결 | 미조치 |
| D-21 | software-requirements.md 4.2장 L51~57, 4.3장 L59~63, trace.csv | Q-001~003과 C-001~003에 상위(출처) 열이 없거나 출처가 기재되지 않음. Q-001과 Q-003은 trace.csv에 행이 없음. Q-003의 상위(OEM-NFR-001)는 이터 4 요구이며 이터 1 SWR에서 이를 참조하는 근거와 이터 구분이 없음 (REQ-10) | Q/C 표에 출처 열을 추가하고 trace.csv에 행을 추가. Q-003의 상위 이터 4 요구 참조 처리를 명시 | 미조치 |
| D-22 | oem 원문 섹션 9 (SWR-001~021 참조) / software-requirements.md 전체 / oem-swr-001-review.md A-15 | 원문 9장은 OEM-SR-001의 하위로 SWR-007/008, OEM-SR-003의 하위로 SWR-013 등을 기대함. 본 문서는 SWR-F/L/Q/C 체계를 쓰며 원문 ID와의 대응표가 없음. 같은 "SWR-" 접두어가 다른 의미로 충돌함 (REQ-10, REQ-12) | OEM 9장 ID와 본 ID의 대응표를 작성하고 A-15에 기록, 필요 시 OEM에 ID 체계 확인을 요청 | 미조치 |
| D-23 | trace.csv 전체 행 (design, code, unit_test, integration_test, system_test 열) | 모든 행에서 하위 추적 열이 비어 있음. 이 게이트에서는 아키텍처·시험 산출물이 아직 없으므로 충족 불가. 체크리스트는 이 항목의 적용 시점을 정하지 않음 (REQ-11). "다음 단계에서 채운다"는 통과 사유가 아님 | 하위 추적 대상 ID를 예약하거나, 체크리스트 REQ-11의 적용 시점을 프로세스 소유자가 개정해 명시. 리뷰어가 N/A로 면제하지 않음 | 미조치 |
| D-24 | trace.csv 전체 / software-requirements.md | trace.csv의 swr_id 열에 없는 이터 1 SWR: SWR-L-003, SWR-Q-001, SWR-Q-002, SWR-Q-003, SWR-C-001, SWR-C-002, SWR-C-003 (7건). SSR-001~004도 행이 없음 (REQ-12) | 누락 항목을 trace.csv에 추가하고 SSR 열 또는 별도 매트릭스를 추가 | 미조치 |
| D-25 | trace.csv 행 OEM-SR-001 / safety-requirements.md L8, L9 연결 열 | SSR-001은 SWR-F-012, F-015를 연결하지만 trace.csv OEM-SR-001 행에는 없음. F-017은 OEM-FR-007(A-01 순위 6)에서도 파생되었으나 FR-007 행에 없음. F-008은 SR-001과 SR-002 양쪽에서 파생되었으나 한쪽만 기록 (REQ-12) | 다중 상위를 trace.csv에 반영 | 미조치 |
| D-26 | software-requirements.md 전체, oem-swr-001-review.md 6장과 변경 이력 | software-requirements.md와 safety-requirements.md에 변경 이력이 없음 (REQ-13). 6장의 영향 목록(F-003, 009~015, 018)은 Q1(순위표)이 영향을 주는 F-004~008, F-016, F-017을 포함하지 않음. Q13(F-002, SSR-001)의 영향은 기록되지 않음. A 표에는 Q13에 해당하는 항목이 없음 (REQ-13) | 두 문서에 변경 이력 절을 추가하고, 가정별 영향 목록을 모든 SWR/SSR ID로 완성. Q13을 A-16으로 등록 | 미조치 |
| D-27 | software-requirements.md L61(SWR-C-001) / 원문 1.3 실행 환경 "Python 3.12 PC/SIL" / oem-swr-001-review.md 1장 | 제품 언어를 C로 확정한 것이 원문 OEM 입력(Python 3.12)과 다름. 근거는 사용자 결정뿐이며 OEM 질의 목록에 편차 항목이 없음 (REQ-09, 상위 입력과의 불일치) | 편차를 OEM 질의 항목(Q14)으로 등록하거나, 사용자 승인 편차로 근거와 일자를 명시 | 미조치 |

## 확인하지 못한 항목과 이유
- A-SPICE 기준 문서(3.1 또는 4.0)가 저장소에 없어, 체크리스트 항목과 SWE.1 기본 실무를 원문 대조로 확인하지 못했다. 체크리스트 자체에 기준 버전이 적혀 있지 않다. 판정은 체크리스트 문장 그대로 적용했다.
- 원본 docx는 `word/document.xml`의 문단 텍스트만 추출했다(python -I). 표 구조와 병합 셀의 시각적 배치는 확인하지 않았다.
- Cppcheck, Lizard, 요구사항 커버리지 게이트(`ci/requirement_coverage.sh`)는 이 단계 산출물 대상이 아니어서 실행하지 않았다.
- trace.csv의 하위 추적 열(설계, 코드, 시험)은 상위 단계 산출물이 없어 대조할 대상이 없다 (D-23).

## 재리뷰 조건
다음 단계(아키텍처 설계)는 이 기록의 모든 결함이 조치되고 체크리스트 13항목 전체가 재판정되어 `승인`이 나온 뒤에만 시작한다.
