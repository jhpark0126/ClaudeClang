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
