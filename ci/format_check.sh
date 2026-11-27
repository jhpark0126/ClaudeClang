#!/usr/bin/env bash
# clang-format 검사: 저장소 루트의 .clang-format 과 다르면 실패한다. (수정은 하지 않는다)
# 고치려면: bash scripts/dev.sh clang-format -i <파일>
set -u
. "$(dirname "$0")/common.sh"

if ! has_sources "${ALL_SOURCE_DIRS[@]}"; then
    log "검사할 소스가 없습니다. (통과)"
    exit 0
fi
style="$PROJECT_DIR/.clang-format"
[ -f "$style" ] || style="$CI_DIR/../.clang-format"
files=$(find $(existing_dirs "${ALL_SOURCE_DIRS[@]}") -type f \( -name '*.c' -o -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) | sort)
# shellcheck disable=SC2086
if clang-format --style=file:"$style" --dry-run -Werror $files; then
    log "clang-format: 위반 0건"
else
    fail "clang-format 위반이 있습니다. 'clang-format -i' 로 정리하세요."
    exit 1
fi
