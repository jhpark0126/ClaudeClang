# 체크리스트: 소스 코드 (SWE.3 Unit Construction)
기준: A-SPICE 프로세스 SWE.3 (사용 중인 A-SPICE 버전의 문서와 대조해 갱신할 것), 스킬 `implementation`

| ID | 항목 | 근거 |
|---|---|---|
| COD-01 | 코드가 상세 설계와 일치하고 설계에 없는 동작이 없다. | SWE.3 BP: 유닛 구현, 일관성 |
| COD-02 | `void` 함수가 없다. 반환값이 없을 때 상태 코드를 반환한다. | 스킬 `implementation` |
| COD-03 | 모든 함수에 `return`이 정확히 하나이고 함수 마지막에 있다. | 스킬 `implementation` |
| COD-04 | 호출자가 반환된 상태 코드를 확인한다 (무시는 `(void)` 캐스팅과 사유 주석). | 스킬 `implementation` |
| COD-05 | 다른 모듈은 공개된 인터페이스(헤더)로만 접근한다. | 프로젝트 규칙 |
| COD-06 | 내부 함수와 데이터가 `static`이다. | 캡슐화 |
| COD-07 | 모든 입력 포인터·범위를 검증하고 실패 경로가 처리된다. | 견고성 |
| COD-08 | 이름이 명확하고 매직 넘버가 없으며 함수가 작다. | 클린코드 |
| COD-09 | 공개 함수에 Doxygen(`@brief`, `@param`, `@return` 전체 상태 코드)이 있다. | 프로젝트 규칙 |
| COD-09a | 모든 소스 파일과 공개 함수에 사람 확인용 태그(`@generated_by`, `@generated_at`, `@requested_by`, `@human_review`, `@reviewed_by`, `@reviewed_at`)가 있고 값이 실제 값이다. | CLAUDE.md 2.4 |
| COD-09b | `@human_review: done`인 곳은 사용자가 리뷰 완료를 알린 기록(`@reviewed_by`, `@reviewed_at`)이 있다. 근거 없이 `done`인 곳이 없다. | CLAUDE.md 2.4 |
| COD-10 | 파일/함수에 설계 ID(`MOD`, `FUNC`)가 적혀 있다. | 추적성 |
| COD-11 | 컴파일 경고가 없다. | SWE.3 BP: 검증 |
| COD-11a | `cppcheck`를 실행해 지적 사항이 0건이다. 억제 주석(`cppcheck-suppress`)이나 룰 비활성화가 없다 (예외 없음). | CLAUDE.md 2.2 |
| COD-11b | `lizard -C 10 -L 80`을 실행해 순환복잡도 10 초과, 80라인 초과 함수가 없다. | CLAUDE.md 2.3 |
| COD-12 | 주석이 코드의 내용이 아니라 이유를 설명한다. | 클린코드 |
| COD-13 | TDD 테스트가 `test/tdd/`에 있고 단위 테스트(`test/unit/`)와 섞이지 않았다. | CLAUDE.md 1.4 |
