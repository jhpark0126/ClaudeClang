#!/usr/bin/env bash
# CLOC: 주석률 = 주석 줄 / (주석 줄 + 코드 줄) * 100.
# 기준값(COMMENT_RATIO_MIN) 기본 20(%). 사용자 확정 전의 권고값이며 이 한 곳에서만 바꾼다.
set -u
. "$(dirname "$0")/common.sh"

COMMENT_RATIO_MIN="${COMMENT_RATIO_MIN:-20}"
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
