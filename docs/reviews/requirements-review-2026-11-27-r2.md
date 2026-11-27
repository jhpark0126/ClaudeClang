# 리뷰 기록: 소프트웨어 요구사항 (SWE.1, 이터레이션 1) 재리뷰 2차
- 대상 버전/커밋: 작업 트리 clean 확인, HEAD `bdf0f3b` (1차 리뷰 대상 `1c81c6d`의 이후 커밋 포함)
  - `docs/requirements/software-requirements.md`, `docs/requirements/safety-requirements.md`
  - `docs/requirements/oem-swr-001-review.md` (A-01~A-20, Q1~Q17, 변경 이력)
  - `docs/traceability/trace.csv`
  - 원본: `OEM/OEM-SWR-001_OEM SW 요구사항 사양서.docx` (BL-OEM-1.0). 문단 텍스트를 python -I로 추출해 대조 (표 구조는 미확인, 아래 참조)
- 1차 기록: `docs/reviews/requirements-review-2026-11-27.md` (반려, D-01~D-27)
- 체크리스트: `.claude/skills/review/checklists/requirements.md` (REQ-01~REQ-13, 전체 재판정). 기준 A-SPICE 버전이 체크리스트에 없고 대조할 A-SPICE 문서가 저장소에 없음 (미확인)
- 리뷰어: reviewer 서브에이전트 (Claude Haiku 5.5, claude-haiku-5-5) / 작성자: Claude Sonnet 5.5 (요청자 jhpark0126) / 일자: 2026-11-27

## 결과 요약
**반려** (체크리스트 13항목 중 NG 11건, OK 2건 (REQ-04, REQ-08). 신규/잔여 결함 24건(R2-01~R2-24, 그중 R2-23은 사용자 결정 필요), D-01~D-27 조치 판정은 "1차 결함 조치 확인" 참조)

REQ-11은 사용자 결정 대기 항목으로 별도 표기했다 (아래 "사용자 결정 필요"). 판정은 REQ-11을 어떻게 분류하든 동일하다. REQ-11을 N/A로 바꿔도 REQ-01 외 10건의 NG가 남기 때문이다.

## 항목별 판정 (REQ-01 ~ REQ-13 전체 재판정)
| 항목 ID | 판정 | 비고 (근거 / 결함 ID) |
|---|---|---|
| REQ-01 고유 ID | NG | SSR-001~027, SWR-F-001~005, SWR-I-001~007, SWR-L-001, SWR-Q-001~003, SWR-C-001~004는 중복 없음 (확인). 그러나 software-requirements.md 2장 "규칙 적용 순서" 1~5항은 규범 문장(우선순위 규칙)인데 ID가 없음 (R2-01) |
| REQ-02 분류 | NG | SWR-F는 "기능 (QM)", SWR-Q는 ISO 25010, SWR-L/C는 분류 확인. SWR-I-001~007(software-requirements.md 4장)은 분류 열이 없고 "인터페이스"는 체크리스트/스킬의 4분류(기능/품질/제한/제약) 밖 (R2-02). SSR 표(safety-requirements.md)에는 분류 열이 없음. ASIL 상속도 SWR-I, SWR-Q-001/002에 없음 (R2-03, D-19 잔여) |
| REQ-03 SSR 분리 | NG | 헤더 L5 "ASIL B 파생 요구는 모두 SSR"인데 SWR-Q-001 출처 "OEM-SR-003 파생", SWR-Q-002 출처 "OEM-SR-001, 003 검증성"은 ASIL B 파생이 본문에 있음. SWR-I-001~007은 trace상 ASIL B 상위(OEM-SR-001~004)를 갖지만 헤더는 인터페이스를 SWR에 두라고 해 규칙이 모순됨 (R2-04, D-01 잔여) |
| REQ-04 품질 특성 | OK | SWR-Q-001 신뢰성(결함 허용), SWR-Q-002 유지보수성(시험성), SWR-Q-003 기능 적합성(정확성) 모두 ISO 25010 특성 지정 (software-requirements.md 5.2장, L59~61). 3건 모두 확인 |
| REQ-05 원자성 | NG | SSR-016(safety-requirements.md L32): "직전 확정 출력 유지"와 "해제 요청 미적용" 2개 동작 (R2-05). SWR-I-006(L45): "state 제공"과 "이유코드(경고코드 포함) 제공" 2개 (R2-06, D-17 잔여) |
| REQ-06 모호어 | NG | "SW 시계" 용어 미정의와 수신 시각 측정 방법 없음 (R2-09, D-12 잔여). "정의된 출력과 상태"(SWR-Q-001) 정의 위치 없음 (R2-07). Cppcheck "규칙"의 집합 미정 (R2-08). "side=all" 처리 미정 (R2-17). "바꾼 평가주기"의 기록 조건 불명 (R2-18). "호출자가 읽을 수 있는 값"(SWR-I-006)은 조회 계약 이터 3 위임으로 허용 |
| REQ-07 검증 기준 | NG | SWR-Q-001 검증 기준 "정의된 값"은 순환이고 품질 요구의 수치·측정 조건 없음(스킬 requirements-analysis 2장 규칙) (R2-07). SSR-013 검증 기준 "무관하게 결정됨"은 합격/불합격 판정이 아님 (R2-19). SWR-Q-003 검증 "2회 일치"는 OEM-NFR-001 기준 "1,000회 해시 일치"와 다름 (R2-20). 필수 입력 INVALID 시 상태 규칙 없음으로 SSR-018/021 검증이 완결되지 않음 (R2-12). OEM-SR-002 원인 기록 기준 미커버 (R2-16) |
| REQ-08 구현 방법 배제 | OK | 1차 D-18의 "주입받아"(SWR-Q-002), "설정 상수로 정의해야"(SWR-L-003)는 제거·재작성됨. 현재 SWR-Q-002는 "시험이 시각을 지정해 재현할 수 있어야 한다"(시험성 성질), SWR-L-001은 주기 값 제한, SWR-I-007은 시각 부여 요구로 설계 수단을 지정하지 않음. 전 항목 확인 |
| REQ-09 일관성·모순 | NG | SWR-F-001(L31)은 점화 OFF 시 무조건 RELEASE, SWR-F-004(L34)는 DEGRADED·FAULT에서 점화 OFF 해제 금지. 문자 그대로 모순 (R2-11). SWR-F-005(L35)는 첫 평가주기 무조건 RELEASE, SSR-009/010은 유효 접근위험 TRUE에서 LOCK. 모순 (R2-10). 안전 영향 메모(safety-requirements.md L57)는 "입력 없음=STALE=DEGRADED"라 하나 SSR-018은 누락을 INVALID, SSR-021은 STALE만 DEGRADED로 함 (R2-12). A-04(oem-swr-001-review.md L43) "최악 300 ms"는 210+100 산술과 불일치 (R2-13). I-007 시각 기준이 OEM-IF-004 비고(요청 시각=VehicleSnapshot.timestamp_s)와 충돌 (R2-14). SSR-016 직전 유지가 RELEASE 고정을 만들 수 있으나 안전측 반응 근거 없음 (R2-15). SWR-Q-003 범위 불일치 (R2-20) |
| REQ-10 상향 추적 | NG | 전 항목에 출처/상위 열 또는 trace 상위 행이 있음 (확인). 그러나 SWR-I-007의 출처 "가정 A-12, A-20"은 A-12(oem-swr-001-review.md L51)가 말하지 않는 수신 시각을 근거로 댐. 상향 근거가 틀림 (R2-14) |
| REQ-11 하향 추적 | NG (사용자 결정 필요) | trace.csv의 design, code, unit_test, integration_test, system_test 열이 전 행 빈칸 (확인). 하위 산출물 부재. 아래 "사용자 결정 필요" 참조 (R2-23) |
| REQ-12 추적 매트릭스 실측 | NG | trace.csv를 열어 grep 대조. 문서 전체 ID 중 SWR-L-001, SWR-C-001, SWR-C-002, SWR-C-003, SWR-C-004 5건이 trace.csv에 없음 (R2-21). 반대 방향 고아 항목은 없음 (확인). SSR-001~027 전부 상위 행에 있음 (확인) |
| REQ-13 영향 분석·변경 이력 | NG | 세 문서 모두 변경 이력 절 있음 (확인). 그러나 software-requirements.md 7장 영향 목록에 A-10(Q9), A-11(Q10), A-12, A-13(Q11)이 없음 (R2-22). 1차 리뷰 이후 폐기·개명된 ID(예: 초안 SWR-F-006~018, SWR-L-002/003)의 대응이 변경 이력에 없음 (R2-24) |

## 결함 목록
| 결함 ID | 위치 | 내용 | 조치 요구 | 상태 |
|---|---|---|---|---|
| R2-01 | software-requirements.md 2장 (L22~26) | 규칙 적용 순서 1~5는 규범 문장(우선순위)인데 ID가 없다. SSR-003, SSR-015, SSR-024 등이 이 순서에 의존한다 (REQ-01, 규범 요구의 추적 불가) | 각 규칙에 ID를 부여해 SWR 또는 SSR로 옮기거나, 비규범 요약임을 명시하고 규범은 SSR/SWR 본문에만 둔다 | NG |
| R2-02 | software-requirements.md 4장 SWR-I-001~007 표 (L38~46) | 분류 열이 없고, 인터페이스가 기능/품질/제한/제약 4분류 밖이다. ASIL/QM 상속 열도 없다 (REQ-02, 1차 D-19 잔여) | 분류(기능 하위의 인터페이스 등)를 명시하고 ASIL/QM 상속 열을 추가. 분류 기준을 체크리스트와 맞춘다 | NG |
| R2-03 | safety-requirements.md 1~4장 표 전체 | SSR 표에 기능/제한 분류 열이 없다. SSR-008(300 ms), SSR-020(200 ms), SSR-022(100 ms)는 제한이다 (REQ-02) | 분류 열 추가 | NG |
| R2-04 | software-requirements.md 5.2장 SWR-Q-001(L59), SWR-Q-002(L60); 헤더 L5; 4장 SWR-I-001~007 | Q-001은 "OEM-SR-003 파생", Q-002는 "OEM-SR-001, 003 검증성"으로 ASIL B 상위를 가진다. 헤더는 "ASIL B 파생 요구는 모두 SSR"이라 하면서 "인터페이스·품질은 SWR"이라고도 해 규칙이 모순된다. SWR-I-001~007은 trace상 ASIL B 상위(OEM-SR-001~004)를 가진다. 참고로 SWR-F-004(L34)도 DEGRADED·FAULT의 안전 상태 행위를 기술해 SSR 귀속 여부 판단이 필요하다 (REQ-03, 1차 D-01 잔여) | Q-001, Q-002를 SSR로 이관하거나, 헤더의 분리 규칙을 사용자와 확정해 문서에 명시. I-series의 ASIL B 상위 처리 규칙을 명시. F-004의 귀속을 판단·기록 | NG |
| R2-05 | safety-requirements.md SSR-016 (L32) | "직전 확정 출력 유지"와 "해제 요청 미적용" 두 동작이 한 요구에 있다 (REQ-05, 1차 D-17 잔여) | SSR-016a/b로 분할 | NG |
| R2-06 | software-requirements.md SWR-I-006 (L45) | "state 제공"과 "이유코드(경고코드 포함) 제공"이 한 요구에 있다. "호출자가 읽을 수 있는 값"의 의미도 불명 (REQ-05, REQ-06) | state, 이유코드, (경고코드는 SSR-026과 연계)로 분할하고 제공 방식은 위임 대상임을 명시 | NG |
| R2-07 | software-requirements.md SWR-Q-001 (L59) | "정의된 출력과 상태"의 정의 위치가 없다. 검증 기준 "정의된 값"은 순환이다. 품질 요구에 수치·측정 조건이 없다 (REQ-06, REQ-07, 스킬 requirements-analysis 2장) | 허용 출력·상태 집합을 참조하고, 입력 조합 집합 크기와 합격 판정식을 기재 | NG |
| R2-08 | software-requirements.md SWR-C-003 (L68) | "Cppcheck 규칙을 예외 없이"의 규칙 집합이 정해지지 않았다. CLAUDE.md 미확정 항목(`warning,style,performance,portability`는 임시 값) (REQ-06) | 요구 본문에 규칙 집합의 출처(`ci/static_analysis.sh`의 `CPPCHECK_ENABLE`)를 명시하고, 임시값이면 그 사실을 기록 | NG |
| R2-09 | software-requirements.md 1장 용어표, SWR-I-007(L46), SSR-008(safety L20), 입력의 나이 정의 (L14) | "SW 시계"가 정의되지 않았다. 수신 시각의 측정 방법도 없다 (REQ-06, 1차 D-12 잔여) | "SW 시계"를 용어로 정의(단조 증가, 시험 주입 가능, 해상도)하고 수신 시각 측정 방법을 기재 | NG |
| R2-10 | software-requirements.md SWR-F-005 (L35); safety-requirements.md SSR-009/010 | F-005는 SW 초기화 직후 첫 평가주기 출력을 무조건 RELEASE로 한다. 같은 첫 평가주기에 유효 접근위험 TRUE가 있으면 SSR-009/010은 LOCK을 요구한다. 직접 모순 (REQ-09) | F-005에 "접근위험 LOCK 대상이 아닌 도어" 조건을 추가하고 검증 기준에 접근위험 TRUE 케이스를 추가 | NG |
| R2-11 | software-requirements.md SWR-F-001 (L31) / SWR-F-004 (L34) | F-001은 점화 OFF이면 무조건 RELEASE, F-004는 DEGRADED·FAULT에서 점화 OFF 해제를 금지한다. DEGRADED에서 문자 그대로 모순. 해결 규칙은 2장 요약에만 있다 (REQ-09, 1차 D-02 잔여) | F-001에 "DEGRADED·FAULT가 아닐 때"를 조건으로 추가하고 F-004와 상호 참조 | NG |
| R2-12 | safety-requirements.md 안전 영향 분석 메모 (L57), SSR-018, SSR-021; 필수 입력 INVALID 시 상태 | 메모는 "필수 입력 누락 = STALE = DEGRADED"라 하나 SSR-018은 누락을 INVALID로, SSR-021은 STALE만 DEGRADED로 한다. 또 필수 입력이 INVALID인 경우(누락 포함)의 상태 규칙이 어디에도 없다. 따라서 "입력 없음" 상태(SWR-F-005 검증의 전제)가 정의되지 않았다 (REQ-07, REQ-09, 1차 D-15 잔여, D-06 잔여) | INVALID 필수 입력이 있을 때의 상태 규칙을 SSR로 추가하고(예: 상태 결정 표), 메모를 SSR-018/021과 일치하도록 수정 | NG |
| R2-13 | oem-swr-001-review.md A-04 (L43) | "감지한 평가주기부터 100 ms 이내 = 최악 300 ms"라 하나, 감지는 최대 210 ms이고 SSR-021이 같은 주기에 DEGRADED를 요구하므로 A-04의 산식이 틀리고 SSR-020~022와 불일치 (REQ-09, 1차 D-14 잔여) | A-04를 "STALE 감지 최대 210 ms, 같은 평가주기 DEGRADED(SSR-021)"로 수정 | NG |
| R2-14 | software-requirements.md SWR-I-007 (L46); oem-swr-001-review.md A-12 (L51), A-20 (L59); OEM-IF-004 비고 | I-007은 SW 시계 수신 시각을 부여하고 출처로 "OEM-IF-004 비고, 가정 A-12"를 든다. OEM-IF-004 비고는 요청 시각을 VehicleSnapshot.timestamp_s로 정했고, A-12도 timestamp_s를 쓰라고 한다. 수신 시각을 근거로 대는 것은 출처 오인이다. 요청 시각(timestamp_s)과 입력 나이 기준(수신 시각)의 역할 구분이 명시되지 않았다 (REQ-09, REQ-10) | 두 시각의 역할을 분리 정의하고 I-007 출처와 A-12 문구를 바로잡는다 | NG |
| R2-15 | safety-requirements.md SSR-016 (L32); oem-swr-001-review.md A-18 (L57) | 접근위험 입력이 INVALID·STALE일 때 직전 확정 출력을 유지한다. 직전 출력이 RELEASE였다면 접근위험이 TRUE였을 가능성이 있어도 RELEASE가 고정된다. OEM-SR-003이 요구한 "정의된 방식"의 안전측 근거가 없다. 1차 D-16의 "안전측 반응 정의" 요구를 충족하지 않는다 (REQ-09, 안전 영향) | 안전측 반응(예: LOCK 유지)을 정의하거나, 직전 유지 선택의 안전 영향 분석을 SSR 메모에 기록하고 Q15에 안전측 선택을 질의 | NG |
| R2-16 | OEM-SR-002 수용기준 (원문 L151) / safety-requirements.md SSR-012 (L28) | 수용기준 "접근위험 TRUE가 되면 해당 출력이 LOCK이고 … 원인이 기록된다"는 접근위험 LOCK 자체의 원인 기록을 요구한다. SSR-012는 억제 시에만 기록하고, 이유코드 열거에 접근위험 LOCK 원인이 없다 (REQ-07, REQ-10 커버리지) | 접근위험 LOCK 원인 기록 요구와 이유코드를 추가하거나, 수용기준 해석을 가정으로 명시하고 Q로 등록 | NG |
| R2-17 | software-requirements.md SWR-I-004 (L43) | 해제 요청의 side 값에 "all"이 있는데, 접근위험 TRUE 도어가 포함된 all 해제 요청의 처리가 정의되지 않았다. SSR-011 "그 도어에 대한 해제 요청"으로 all을 해석할 수 없다 (REQ-06) | side=all의 처리 규칙을 명시 | NG |
| R2-18 | software-requirements.md SWR-F-003 (L33) | "점화 OFF로 출력을 RELEASE로 바꾼 평가주기"는 출력이 이미 RELEASE인 경우 기록 여부가 불명이다 (REQ-06) | 기록 조건을 "점화 OFF 조건으로 RELEASE가 판정된 평가주기"처럼 판정 기준으로 다시 쓴다 | NG |
| R2-19 | safety-requirements.md SSR-013 (L29) | 검증 기준 "좌 위험만 TRUE일 때 우 출력이 좌 입력과 무관하게 결정됨"은 합격/불합격을 판정하지 않는다 (REQ-07) | "좌 입력만 변경할 때 우 출력 불변" 같은 판정식으로 재작성 | NG |
| R2-20 | software-requirements.md SWR-Q-003 (L61); 범위 선언 (L4); OEM-NFR-001 (원문 L200~201) | 상위 OEM-NFR-001 기준은 1,000회 재생 해시 일치인데, 검증 기준은 "2회 일치"다. 또 NFR-001은 이터레이션 4 요구인데 이터 1 SWR에 포함하면서 범위 선언(이터 1은 SR-001~004, FR-007)과 어긋난다 (REQ-07, 범위 일관성) | 이터 1 범위에서 제외하거나, 이터 1 검증 기준과 이터 4 기준의 관계를 명시하고 범위 선언을 갱신 | NG |
| R2-21 | docs/traceability/trace.csv 전체 행 / software-requirements.md 6장 (L83) | SWR-L-001, SWR-C-001, SWR-C-002, SWR-C-003, SWR-C-004가 trace.csv에 없다. 6장은 "trace.csv 대상 아님"이라 쓰지만, 1차 D-24는 누락 항목을 trace.csv에 추가하라고 요구했다. 문서가 자체 기준으로 요구를 면제한 것이다 (REQ-12, 1차 D-24 잔여) | 5건을 trace.csv에 추가하고 상위(가정 A-03, 사양서 2장, CLAUDE.md, 사용자 결정)를 기재 | NG |
| R2-22 | software-requirements.md 7장 영향 목록 (L89~101) | 가정 A-10(Q9: 상태·이유코드 열거 → I-006, SSR-007/012/017), A-11(Q10: 해시 → SWR-Q-003), A-12(→ SWR-I-007), A-13(Q11: 간섭 방지 → SSR-011, SSR-015)이 영향 목록에 없다 (REQ-13, 1차 D-26 잔여) | 영향 목록을 모든 가정(A-01~A-20)에 대해 완성하고 해당 없는 가정은 사유를 기재 | NG |
| R2-23 | docs/traceability/trace.csv 하위 열 (design, code, unit_test, integration_test, system_test) | 전 행 빈칸 (REQ-11, 1차 D-23 잔여). 사용자 결정 필요. 아래 참조 | 사용자 결정 후 조치 (아래) | NG (사용자 결정 필요) |
| R2-24 | software-requirements.md 변경 이력 (L103~106), safety-requirements.md 변경 이력 (L59~62) | 1차 리뷰 반영 이후 폐기·개명된 ID (초안 SWR-F-006~018, SWR-L-002/003, SWR-I 이전 체계 등)의 대응표가 없다. 한 줄 요약만 있어 변경 추적이 불가하다 (REQ-13) | 변경 이력에 폐기·대체 ID 대응표와 영향 받은 ID를 기재 | NG |

## 1차 결함(D-01~D-27) 조치 확인
| 결함 ID | 판정 | 근거 / 잔여 |
|---|---|---|
| D-01 | 부분 | ASIL B 본문 요구 F-001~015, F-017, L-001/002는 제거되고 SSR-001~027에 이관됨. 잔여: Q-001/Q-002, SWR-I 처리 규칙 (R2-04) |
| D-02 | 부분 | F-016/F-017 충돌은 F-001의 접근위험 제외 조건으로 해소. 잔여: F-001과 F-004의 DEGRADED 모순 (R2-11) |
| D-03 | 해소 | F-004~006/F-008 구조 폐기. SSR-009/010의 "CONFIRMED가 아닐 때", SSR-003, SSR-015로 우선순위 명시 |
| D-04 | 해소 | SSR-024 예외, 2장 규칙 순서, A-17/Q14. 두 문서 적용 순서는 DEGRADED·FAULT에서 동치 |
| D-05 | 해소 | SSR-014, SSR-023으로 DEGRADED에서 유효 접근위험 LOCK 적용과 명시 |
| D-06 | 부분 | SSR-027 우선순위와 상태 집합 정의됨. 잔여: INVALID 필수 입력의 상태 규칙 (R2-12) |
| D-07 | 해소 | SSR-023이 CONFIRMED·LOCK 예외를 가짐 |
| D-08 | 해소 | A-16, Q13 등록, SSR-002에 "(가정 A-16)" 표기, F-002 제거 |
| D-09 | 해소 | F-002 검증 기준(차속, 운전자 명령) 제거. 신규 SSR 검증은 이터 1 관측 입력만 사용 |
| D-10 | 해소 | 문서 내 "재기동" 용어 없음 (grep 확인). "SW 초기화"를 1장에서 정의 |
| D-11 | 해소 | 1장에 "필수 입력 (이터 1)" 정의와 이터 2 위임 명시 |
| D-12 | 부분 | A-20으로 기준 시각 지정. 잔여: "SW 시계"와 측정 방법 (R2-09) |
| D-13 | 해소 | 입력 나이 정의 (마지막 유효 갱신 시각), A-20/Q17, SSR-020 (잔여 해석 충돌은 R2-14) |
| D-14 | 부분 | L-002, F-011 중복 제거, SSR-020~022로 재작성. 잔여: A-04 산식 (R2-13) |
| D-15 | 부분 | F-005 검증 기준은 관측 가능. 안전 영향 메모는 있으나 내용이 SSR-018/021과 모순 (R2-10, R2-12) |
| D-16 | 부분 | A-18, Q15, SSR-016/017 등록. 잔여: 안전측 반응 미정의 (R2-15) |
| D-17 | 부분 | 대부분 분할됨. 잔여: SSR-016, SWR-I-006 (R2-05, R2-06) |
| D-18 | 해소 | Q-002 재작성, L-003 제거, L-001로 주기 값 제한. 잔여 Cppcheck 규칙 집합은 R2-08 |
| D-19 | 부분 | SSR 표에 ASIL 열 추가, SWR-F는 "기능 (QM)". 잔여: SWR-I, SWR-Q의 ASIL/QM 상속 (R2-02, R2-04) |
| D-20 | 해소 | SWR-I-001~007 ID 부여, trace 연결 확인. 분류는 R2-02 |
| D-21 | 부분 | Q/C 표에 출처 열 추가, Q-001/Q-003 trace 행 있음. 잔여: C-001~004 trace 누락 (R2-21), Q-003 범위 (R2-20) |
| D-22 | 해소 | 6장 대응표 작성 (OEM 9장 ID와 본 ID) |
| D-23 | 미조치 | 하위 열 빈칸. 사용자 결정 필요 (R2-23) |
| D-24 | 부분 | Q-001~003, SWR-I, SWR-F, SSR-001~027 행 추가 확인. 잔여: SWR-L-001, SWR-C-001~004 (R2-21) |
| D-25 | 해소 | SSR-001의 SWR-F-012/015 연결 제거. 다중 상위(SSR-003/005/014/016/017/027)가 trace 양쪽 행에 반영됨 (확인) |
| D-26 | 부분 | 세 문서 변경 이력과 7장 영향 목록 추가, A-16 등록. 잔여: A-10~A-13 누락, 폐기 ID 대응 (R2-22, R2-24) |
| D-27 | 해소 | A-19/Q16 등록, C-001 출처에 사용자 결정 일자 2026-11-27 명시 |

## 사용자 결정 필요 (REQ-11, D-23)
- 분류: 사용자 결정 필요. 리뷰어가 이 항목을 N/A로 면제하지 않았다.
- 사유: 체크리스트 REQ-11 "모든 요구사항이 아키텍처 요소와 시험에 추적된다"는 이 단계에서 충족할 수 없다. 대상 산출물(SWE.2 아키텍처, SWE.4 단위 시험, SWE.6 시스템 시험)이 아직 없다. 체크리스트는 적용 시점을 정하지 않았다. 리뷰어가 시점을 임의로 정해 N/A로 바꾸면 1차 D-23의 조치 요구를 리뷰어가 스스로 무효화하게 된다.
- 판정에 대한 영향: REQ-11의 분류와 무관하게 본 재리뷰는 반려다. R2-01~R2-22, R2-24의 NG가 남아 있다.
- 사용자가 할 수 있는 결정 (둘 중 하나):
  1. 체크리스트 REQ-11의 적용 단계를 개정한다 (예: 아키텍처 게이트 이후 적용). 개정 후 이 단계의 REQ-11은 "이 단계에 해당하지 않음, 사유: 하위 산출물 생성 전"으로 N/A 처리할 수 있다. 개정은 프로세스 소유자 결정이며 체크리스트 파일을 바꾸는 행위다. 리뷰어가 직접 바꾸지 않는다.
  2. 하위 추적 대상 ID(아키텍처 요소, 시험 ID)를 예약해 trace.csv에 채운다. 이 경우 예약 ID가 실제 산출물에서 채워질 때까지 REQ-11은 미충족 상태로 남는다.

## 확인하지 못한 항목과 이유
- A-SPICE 기준 문서(3.1 또는 4.0)가 저장소에 없다. SWE.1 기본 실무와 체크리스트 대조는 체크리스트 문장 그대로 적용했다.
- 원본 docx는 `word/document.xml`의 문단 텍스트만 추출했다 (python -I, 스크래치패드). 표 구조와 병합 셀은 확인하지 않았다. OEM 요구 원문 인용은 문단 텍스트 기준이다.
- `ci/requirement_coverage.sh`는 실행하지 않았다. 완료 이터레이션을 올리는 단계의 게이트이며 REPORT_DIR에 산출물을 쓴다. 리뷰 기록 외 쓰기를 피했다. 이 단계의 커버리지 판정은 trace.csv 직접 대조로 갈음했다.
- Cppcheck, Lizard, Doxygen은 요구사항 산출물 대상이 아니어서 실행하지 않았다.
- 라인 번호는 `grep -n`과 Read 결과로 확인한 값이며, 일부 표 행은 ID로만 지정했다.

## 재리뷰 조건
R2-01~R2-22, R2-24의 조치 후 체크리스트 13항목 전체를 처음부터 다시 판정한다. R2-23(REQ-11)은 사용자 결정 후 처리한다. 다음 단계(아키텍처 설계)는 이 기록의 NG가 모두 해소되고 `승인`이 나온 뒤에만 시작한다.
