#!/usr/bin/env bash
# Cppcheck: 예외 없이 지적 사항 0건이어야 통과한다. (CLAUDE.md 2.2)
#
# 룰 범위(--enable): 기본은 warning,style,performance,portability.
#   information/missingInclude는 "코드 결함"이 아니라 환경 잡음이라 범위에서 제외했다.
#   범위는 사용자가 확정하는 대로 CPPCHECK_ENABLE 한 곳에서 바꾼다.
#
# "예외 없음"을 지키기 위해 다음을 모두 막는다.
#   - 소스 안의 억제 주석(cppcheck-suppress)은 --inline-suppr 를 주지 않아 무시되고,
#     추가로 이 스크립트가 발견 즉시 실패시킨다.
#   - 억제 파일(--suppressions-list)은 사용하지 않는다.
set -u
. "$(dirname "$0")/common.sh"

CPPCHECK_ENABLE="${CPPCHECK_ENABLE:-warning,style,performance,portability}"
status=0

if ! has_sources "${ALL_SOURCE_DIRS[@]}"; then
    log "분석할 소스가 없습니다. (통과)"
    exit 0
fi

mkdir -p "$REPORT_DIR"

# 억제 주석 금지
if grep -rnE 'cppcheck-suppress' $(existing_dirs "${ALL_SOURCE_DIRS[@]}") 2>/dev/null; then
    fail "cppcheck-suppress 사용 금지: 지적 사항은 억제하지 말고 코드를 고치세요."
    status=1
fi

inc=()
[ -d "$PROJECT_DIR/include" ] && inc+=("-I$PROJECT_DIR/include")
[ -d "$PROJECT_DIR/src" ]     && inc+=("-I$PROJECT_DIR/src")

cppcheck --enable="$CPPCHECK_ENABLE" --error-exitcode=1 --library=googletest \
    --std=c17 --std=c++20 "${inc[@]}" \
    --xml --xml-version=2 \
    $(existing_dirs "${ALL_SOURCE_DIRS[@]}") 2> "$REPORT_DIR/cppcheck.xml"
rc=$?
if [ "$rc" -ne 0 ]; then
    fail "Cppcheck 지적 사항이 있습니다 (종료 코드 $rc). 상세: $REPORT_DIR/cppcheck.xml"
    grep -E '<error ' "$REPORT_DIR/cppcheck.xml" | sed -E 's/^ +//' | head -20 >&2
    status=1
else
    log "Cppcheck: 지적 사항 0건"
fi
exit "$status"
