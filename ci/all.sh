#!/usr/bin/env bash
# CI의 모든 게이트를 순서대로 실행한다. 하나라도 실패하면 마지막에 비정상 종료한다.
# 개발 중에도 같은 명령을 쓴다: bash scripts/dev.sh bash ci/all.sh
# 사용법: bash ci/all.sh
set -u
. "$(dirname "$0")/common.sh"

stages=(
    "빌드|bash $CI_DIR/build.sh"
    "TDD 테스트|bash $CI_DIR/run_tests.sh tdd"
    "단위 테스트|bash $CI_DIR/run_tests.sh unit"
    "통합 테스트|bash $CI_DIR/run_tests.sh integration"
    "정적 분석(Cppcheck)|bash $CI_DIR/static_analysis.sh"
    "메트릭(Lizard)|bash $CI_DIR/metrics.sh"
    "요구사항 커버리지|bash $CI_DIR/requirement_coverage.sh"
    "코드 스타일(clang-format)|bash $CI_DIR/format_check.sh"
    "주석률(CLOC)|bash $CI_DIR/comment_ratio.sh"
    "사람 확인용 태그|bash $CI_DIR/doc_tags.sh"
    "Doxygen|bash $CI_DIR/doxygen.sh"
)

failed=()
for entry in "${stages[@]}"; do
    name="${entry%%|*}"
    cmd="${entry#*|}"
    printf '\n===== %s =====\n' "$name"
    if ! eval "$cmd"; then
        failed+=("$name")
    fi
done

printf '\n===== 요약 =====\n'
if [ "${#failed[@]}" -eq 0 ]; then
    log "모든 게이트 통과"
    exit 0
fi
fail "실패한 게이트: ${failed[*]}"
exit 1
