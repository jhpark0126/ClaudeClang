# ClaudeClang

## 프로젝트 개요
- 저장소: https://github.com/jhpark0126/ClaudeClang
- 현재 상태: 초기 단계. 아직 소스 코드가 없음.

## 작업 규칙
- 사용자에게 보이는 답변은 모두 한국어로 작성한다. 코드, 명령어, 파일 경로는 원문 그대로 둔다.
- 사용자 프롬프트와 주요 답변은 `prompt.md`에 계속 기록한다.

## 개발 프로세스 (A-SPICE 기준)
단계별 스킬을 해당 작업 전에 사용하고, 모든 산출물은 `review` 스킬로 리뷰한다.

| 단계 | 스킬 | A-SPICE |
|---|---|---|
| 요구사항 분석 | `requirements-analysis` | SWE.1 |
| 아키텍처 설계 | `architecture-design` | SWE.2 |
| 상세 설계 | `detailed-design` | SWE.3 |
| 구현 | `implementation` | SWE.3 |
| 단위 테스트 | `unit-test` | SWE.4 |
| 통합 테스트 | `integration-test` | SWE.5 |
| 리뷰 | `review` | 전 단계 |

- 요구사항과 이후 산출물(아키텍처, 설계, 코드, 시험)은 양방향 추적성을 유지한다.
- 기능 안전 요구사항은 별도 문서에서 관리한다.
- 구현 언어는 C/C++로 가정한다.

## 폴더 구조
- `.claude/skills/` — 프로젝트 전용 스킬. 스킬마다 하위 폴더를 만들고 그 안에 `SKILL.md`를 둔다.
- `.claude/agents/` — 프로젝트 전용 서브에이전트. 에이전트마다 frontmatter가 있는 `.md` 파일 하나를 둔다.
