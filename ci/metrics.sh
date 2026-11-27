#!/usr/bin/env bash
# Lizard: 순환복잡도 10 이하, 함수 길이 80라인 이하. (CLAUDE.md 2.3)
# 기준값은 이 스크립트에서만 바꾼다. 구현 종료 Hook(scripts/hooks)도 이 값을 쓴다.
set -u
. "$(dirname "$0")/common.sh"

MAX_CCN="${MAX_CCN:-10}"
MAX_LENGTH="${MAX_LENGTH:-80}"
status=0

if ! has_sources "${ALL_SOURCE_DIRS[@]}"; then
    log "측정할 소스가 없습니다. (통과)"
    exit 0
fi

mkdir -p "$REPORT_DIR"
dirs=$(existing_dirs "${ALL_SOURCE_DIRS[@]}")

# 리포트(Jenkins에서 보는 용도): 위반 여부와 무관하게 항상 생성
lizard -C "$MAX_CCN" -L "$MAX_LENGTH" -H $dirs > "$REPORT_DIR/lizard.html" 2>/dev/null || true
lizard -C "$MAX_CCN" -L "$MAX_LENGTH" --xml $dirs > "$REPORT_DIR/lizard.xml" 2>/dev/null || true

# 판정: -w 는 기준 초과 함수만 출력하고, 하나라도 있으면 비정상 종료한다.
if lizard -C "$MAX_CCN" -L "$MAX_LENGTH" -w $dirs; then
    log "Lizard: 순환복잡도 <= $MAX_CCN, 함수 길이 <= $MAX_LENGTH 만족"
else
    fail "Lizard 기준 위반 (순환복잡도 > $MAX_CCN 또는 함수 길이 > $MAX_LENGTH)"
    status=1
fi
exit "$status"
