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
