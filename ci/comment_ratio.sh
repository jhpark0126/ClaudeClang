#!/usr/bin/env bash
# CLOC: 주석률 = 주석 줄 / (주석 줄 + 코드 줄) * 100.
# 기준값(COMMENT_RATIO_MIN)은 아직 정해지지 않아 기본 0 = 측정·기록만 하고 판정하지 않는다.
# 기준이 정해지면 이 기본값을 바꾸면 CI 게이트가 된다.
set -u
. "$(dirname "$0")/common.sh"

COMMENT_RATIO_MIN="${COMMENT_RATIO_MIN:-0}"
status=0

if ! has_sources "${PRODUCT_DIRS[@]}"; then
    log "측정할 제품 소스가 없습니다. (통과)"
    exit 0
fi

mkdir -p "$REPORT_DIR"
cloc --json --quiet $(existing_dirs "${PRODUCT_DIRS[@]}") > "$REPORT_DIR/cloc.json" || status=1
cloc --quiet $(existing_dirs "${PRODUCT_DIRS[@]}") > "$REPORT_DIR/cloc.txt" || status=1

ratio=$(jq -r '.SUM as $s | if ($s.comment + $s.code) == 0 then 0 else (100 * $s.comment / ($s.comment + $s.code)) end' "$REPORT_DIR/cloc.json" 2>/dev/null || echo 0)
printf '%s\n' "$ratio" > "$REPORT_DIR/comment_ratio.txt"
log "주석률: ${ratio}% (기준 ${COMMENT_RATIO_MIN}%)"

if awk -v r="$ratio" -v m="$COMMENT_RATIO_MIN" 'BEGIN { exit !(r < m) }'; then
    fail "주석률 ${ratio}% 가 기준 ${COMMENT_RATIO_MIN}% 미만입니다."
    status=1
fi
exit "$status"
