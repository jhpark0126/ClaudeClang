#!/usr/bin/env bash
# Doxygen 경고 0건 검사. 제품 코드(include, src)의 주석 형식을 검사한다.
set -u
. "$(dirname "$0")/common.sh"

status=0
if ! has_sources "${PRODUCT_DIRS[@]}"; then
    log "문서화할 제품 소스가 없습니다. (통과)"
    exit 0
fi

mkdir -p "$REPORT_DIR"
(
    cd "$PROJECT_DIR" || exit 1
    # Doxyfile의 INPUT 이 존재하지 않는 폴더를 가리켜도 되도록 덮어쓴다.
    {
        cat Doxyfile
        echo "OUTPUT_DIRECTORY = $REPORT_DIR/doxygen"
        echo "INPUT = $(existing_dirs "${PRODUCT_DIRS[@]}" | tr '\n' ' ')"
    } | doxygen - 2> "$REPORT_DIR/doxygen-warnings.txt"
) || status=1

if [ -s "$REPORT_DIR/doxygen-warnings.txt" ]; then
    fail "Doxygen 경고가 있습니다:"
    head -20 "$REPORT_DIR/doxygen-warnings.txt" >&2
    status=1
else
    log "Doxygen: 경고 0건"
fi
exit "$status"
