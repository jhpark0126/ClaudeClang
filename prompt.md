# 프롬프트 기록

## 1. 저장소 클론
**프롬프트:** 다음 저장소 클론해줘 https://github.com/jhpark0126/ClaudeClang.git

**답변 요약:**
- 비어 있던 `D:\Repo\ClaudeClang`에 저장소를 클론함.
- 커밋은 `0b88e1f Initial commit` 하나, 파일은 `README.md`(13바이트) 하나뿐인 사실상 빈 저장소.

## 2. 답변 언어 지정
**프롬프트:** 답은 한글로만 해줘

**답변 요약:**
- 앞으로 한국어로만 답하기로 함.
- 다음 대화에서도 유지되도록 메모리에 저장함.

## 3. Claude 프로젝트 구성
**프롬프트:** 이 프로젝트를 Claude 프로젝트로 만들어줘. .claude 폴더도 미리 만들어줘. 스킬과 서브에이전트 폴더까지. 지금까지 내가 작성한 프롬프트와 네가 답한 주요 내용은 prompt.md에 남겨줘

**답변 요약:**
- `CLAUDE.md` 생성: 프로젝트 개요, 작업 규칙(한국어 답변, `prompt.md` 기록), 폴더 구조 설명.
- `.claude/skills/`, `.claude/agents/` 폴더 생성. 빈 폴더는 git이 추적하지 않으므로 각각 `.gitkeep`을 넣음.
- 이 `prompt.md` 파일 생성.

## 4. 커밋 및 push
**프롬프트:** 커밋하고 push해줘

**답변 요약:**
- PC에 git 작성자 정보가 없어서 사용자에게 확인한 뒤 이 저장소에만 `jhpark0126` / `sdrevn@naver.com`으로 설정함.
- 사용자 선택에 따라 `main` 브랜치에 바로 커밋하고 `origin/main`에 push함.

## 5. A-SPICE 기준 스킬 추가
**프롬프트:** 다음 스킬을 추가해줘. 스킬은 A-SPICE를 기준으로 하고 있어 — 요구사항 분석(기능/비기능 구분, 기능 안전은 별도 문서, 품질은 ISO 25010, 양방향 추적성), 아키텍처 설계(컴포넌트·인터페이스 정의, 인터페이스를 통한 통신, 높은 응집도·낮은 결합도, 모듈화·캡슐화·SOLID), 상세 설계(모듈 정의, 일정 수준 이상 모듈은 인터페이스 정의, 클린코드, 단일 책임), 구현(함수는 반드시 리턴값, 리턴은 마지막에 한 번, void 대신 상태값), 단위 테스트(커버리지 100% 반복, Stub/Mock 최소화, 무의미한 케이스 금지, Doxygen 주석, 경계값·동치 클래스·PICT, MC/DC는 LLVM 스크립트), 통합 테스트(단위 테스트와 동일 원칙 + 인터페이스 중심), 리뷰(모든 산출물 리뷰, A-SPICE 요건에 따른 산출물별 체크리스트)

**답변 요약:**
- `.claude/skills/` 아래에 스킬 7개 생성: `requirements-analysis`, `architecture-design`, `detailed-design`, `implementation`, `unit-test`, `integration-test`, `review`.
- `unit-test`에 LLVM MC/DC 측정 스크립트(`scripts/mcdc_coverage.sh`)와 PICT 사용 안내(`references/pict.md`)를 포함함.
- `review`에 산출물별 체크리스트 6개(`checklists/`)를 포함함.
- `CLAUDE.md`에 단계별 스킬 표를 추가함.
- 스킬 작성 스킬이 권하는 하위 에이전트 사전 검증(압박 시나리오)은 하지 않음. 커밋은 요청이 없어 하지 않음.

## 6. 서브에이전트 생성 (리뷰)
**프롬프트:** 다음의 서브에이전트를 생성해. 리뷰 서브에이전트 — review 스킬을 적용한다. 분석, 설계, 구현, 테스트 등 각 단계가 완료되면 이 리뷰 서브에이전트가 산출물의 품질을 리뷰하고, 리뷰 부적합 사항은 모두 조치 완료해야 다음 단계로 넘어갈 수 있다. 너는 무자비하다. 봐주는거 없다. 빡세게 본다.

**답변 요약:**
- `.claude/agents/reviewer.md` 생성: `review` 스킬을 프리로드하고, 도구는 Read/Grep/Glob/Bash/Write로 제한함.
- 모든 항목이 `OK`(또는 사유 있는 `N/A`)일 때만 승인, `NG`가 하나라도 있으면 반려. 조건부 승인 없음. 근거를 확인하지 못한 항목도 `NG`.
- 조치 후에는 체크리스트 전체를 처음부터 재리뷰함. 승인 전에는 다음 단계로 넘어갈 수 없음.
- 리뷰어는 산출물을 직접 고치지 않고 `docs/reviews/`에 리뷰 기록만 씀.
- `CLAUDE.md`에 단계 게이트 규칙과 에이전트 목록을 추가함. 커밋은 하지 않음.

## 7. 리뷰어 모델 변경
**프롬프트:** 리뷰어 서브에이전트를 haiku 모델을 쓰도록 변경해줘

**답변 요약:**
- `.claude/agents/reviewer.md` frontmatter에 `model: haiku`를 추가함. 커밋은 하지 않음.

## 8. CLAUDE.md 개발 절차·품질 기준·환경 반영
**프롬프트:** 다음의 내용을 반영해서 CLAUDE.md 수정해줘. (1. 개발 절차: A-SPICE, 애자일 이터레이션, 단계별 Reviewer 리뷰와 부적합 조치, 이터레이션 완료 시 사용자 확인 후 PR, 메인 에이전트 개발, TDD 분리 / 2. 품질 기준: 단위 MC/DC 100%, 통합 함수 100%, 시스템 요구사항 100%, Cppcheck 100%, Lizard(CC 10, 80라인, Hook), Doxygen 주석과 HITL 항목 / 3. Gitflow, CI 통과 후 PR / 4. 개발 환경: Clang 18+, 빌드 도구 추천, Cppcheck, GoogleTest/GMock, LLVM, Lizard, CLOC, Docker 이미지 ClaudeCLangBuild / 5. Jenkins LTS, Docker, 포트 8080, 플러그인)

**답변 요약:**
- `CLAUDE.md`를 1~5장 구조(개발 절차, 품질 기준, Git/PR, 개발 환경, CI 환경)로 개편함.
- 빌드 도구는 CMake + Ninja를 추천함. 추가 추천: clang-format, Doxygen, PICT.
- 시스템 테스트 단계에 `sw-system-test-design` 스킬, 구현 단계에 `test-driven-development` 스킬을 연결함.
- Docker 이미지 이름은 소문자만 허용되어 `claudeclangbuild`로 정함.
- Dockerfile, Jenkins 컨테이너와 플러그인, Lizard Hook, `.gitattributes`는 이번 요청이 CLAUDE.md 수정이라 만들지 않음. 커밋은 하지 않음.

## 9. 스킬 정합
**프롬프트:** 스킬 정합부터 하자

**답변 요약:**
- 새 `CLAUDE.md` 기준에 맞춰 스킬과 체크리스트를 수정함.
  - `unit-test`: MC/DC 100%·성공률 100% 필수, `test/unit/`과 `test/tdd/` 분리, Google Test 사용, 시험 주석에 사람 확인용 태그 추가, 예외는 사용자 승인 필요.
  - `integration-test`: 함수 커버리지 100%·성공률 100%로 변경, 인터페이스·오류 반환·요구사항 커버리지는 보조 기준으로 격하.
  - `implementation`: TDD 분리, Lizard(CC ≤ 10, ≤ 80라인), Cppcheck 예외 없음, Doxygen 사람 확인용 태그 추가.
  - `detailed-design`: 함수 크기 기준(CC, 라인 수) 추가.
  - `review`: 시스템 테스트 체크리스트(`checklists/system-test.md`, SWE.6) 신설, 단위·통합·코드·설계 체크리스트 항목 갱신.
  - `mcdc_coverage.sh`: Google Test용 `EXTRA_FLAGS` 지원 추가.
- 커밋은 하지 않음.

## 10. Gitflow 적용 및 개발·CI 환경 전체 구성
**프롬프트:** gitflow 따라가고 이후 작업 네가 알아서 진행해 문제가 발생하면 원인 파악해서 해결하고 마지막에 나에게 보고해 환경 모두 구성 진행해 사전에 내가 정해줘야 하는 것 알려줘

**답변 요약:**
- Gitflow: `develop` 브랜치를 만들어 push하고, `feature/dev-environment`에서 작업함. 커밋 메시지가 잘못 들어가(`@ ...`) push 전에 되돌려 다시 커밋함. `.gitattributes`로 LF 고정.
- 사용자가 추가해 둔 스킬(`sw-system-test-design`, `test-driven-development`)을 별도 커밋으로 반영함.
- Docker 이미지 `claudeclangbuild`(Clang 22.1.8, CMake/Ninja, Cppcheck, GoogleTest/GMock, Lizard, CLOC, Doxygen, PICT) 빌드 및 도구 확인.
- `CMakeLists.txt`와 `ci/*.sh` 게이트 작성. 자가 검증(`ci/selftest`) 22개 항목 통과: 정상 샘플 통과 + 망가뜨린 샘플에서 각 게이트가 실패함을 확인.
- Jenkins LTS(2.580.3)를 Docker로 8080에 구성. 플러그인 96개, JCasC로 관리자·멀티브랜치 잡 생성. 임시 검증 브랜치로 JUnit, Cppcheck, Coverage, HTML 리포트 게시와 위반 시 `FAILURE`를 확인한 뒤 브랜치를 삭제함.
- 구현 종료 시 Lizard를 검사하는 Stop 훅(`.claude/settings.json`) 추가 및 동작 확인.
- 문제와 해결: (1) Doxygen 자가 검증 변형이 경고를 유발하지 못함 → 변형 수정, (2) Jenkins가 브랜치를 발견하지 못함 → 브랜치 탐색 trait 추가, (3) 재시작 시 빌드가 중복 예약되어 #1 로그가 깨짐 → 재시작 직후 경쟁 상태로 판단, #2부터 정상.
- `gh` CLI가 없어 PR은 만들지 못함. 미확정 사항을 `CLAUDE.md`에 기록함.
