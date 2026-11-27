---
name: review
description: Use when reviewing any work product (requirements, architecture, detailed design, code, unit tests, integration tests, system tests) under A-SPICE, or when creating or updating a review checklist for a work product. 리뷰, 체크리스트, 산출물 검토.
---

# 리뷰 (A-SPICE 검증 활동)

## 핵심 원칙
**모든 산출물은 이 스킬로 리뷰한다.** 리뷰는 산출물별 체크리스트를 근거로 하고, 체크리스트 없이 인상으로 판단하지 않는다.

## 절차
1. 산출물 종류를 확인하고 `checklists/` 에서 해당 체크리스트를 고른다.
2. **체크리스트가 없거나 오래되었으면 먼저 개발한다** (아래 "체크리스트 개발").
3. 항목을 하나씩 확인하고 판정한다: `OK` / `NG`(결함) / `N/A`(사유 필수).
4. `NG`마다 결함 ID, 위치, 내용, 요구되는 조치를 기록한다.
5. 리뷰 기록을 `docs/reviews/<산출물>-review-<날짜>.md` 에 남긴다 (아래 형식).
6. 결함을 수정한 뒤 **`NG` 항목을 재확인**한다. 모든 `NG`가 해소되어야 승인한다.
7. 승인되지 않은 산출물은 다음 단계의 입력으로 쓰지 않는다.

## 체크리스트 위치
| 산출물 | 체크리스트 | 대응 A-SPICE |
|---|---|---|
| 소프트웨어 요구사항 | `checklists/requirements.md` | SWE.1 |
| 아키텍처 설계 | `checklists/architecture.md` | SWE.2 |
| 상세 설계 | `checklists/detailed-design.md` | SWE.3 |
| 소스 코드 | `checklists/code.md` | SWE.3 |
| 단위 시험 | `checklists/unit-test.md` | SWE.4 |
| 통합 시험 | `checklists/integration-test.md` | SWE.5 |
| 시스템 시험 | `checklists/system-test.md` | SWE.6 |

체크리스트는 `.claude/skills/review/checklists/` 에 있다.

## 체크리스트 개발 (A-SPICE 요건 기반)
A-SPICE는 산출물이 갖춰야 할 **기준(검증 기준)과 일관성·추적성**을 요구한다. 체크리스트를 새로 만들 때는 다음에서 항목을 도출한다.
1. 해당 프로세스의 **기본 실무(Base Practice)**: 산출물이 충족해야 할 활동 결과.
2. 해당 프로세스의 **출력 정보 항목(Output Information Item)** 이 요구하는 내용.
3. **양방향 추적성**과 **일관성**(상·하위 산출물 사이의 모순 없음).
4. 이 프로젝트의 스킬 규칙(요구사항 분류, 인터페이스 정의, 반환값 규칙, 시험 주석 등).

각 항목은 **예/아니오로 판정 가능한 문장**으로 쓰고, 항목마다 근거(A-SPICE 항목 또는 프로젝트 규칙)를 단다.
A-SPICE 버전(3.1/4.0)에 따라 프로세스·정보 항목 이름이 다르므로, 체크리스트 머리에 기준 버전을 적고 실제 적용 문서와 대조해 갱신한다.

## 리뷰 기록 형식

```
# 리뷰 기록: <산출물 이름>
- 대상 버전/커밋: 
- 체크리스트: <파일>, 기준 버전
- 리뷰어 / 작성자 / 일자:

## 결과 요약
승인 / 조건부 승인 / 반려 (NG n건)

## 항목별 판정
| 항목 ID | 판정 | 비고 |

## 결함 목록
| 결함 ID | 위치 | 내용 | 조치 | 상태 |
```

## 주의
- 작성자와 리뷰어의 관점을 분리한다. 방금 쓴 산출물을 기억에 의존해 통과시키지 말고 체크리스트를 처음부터 다시 읽으며 검토한다.
- `N/A` 판정은 사유 없이 허용하지 않는다.
- 추적성 항목은 추적 매트릭스를 실제로 열어 확인한다.
