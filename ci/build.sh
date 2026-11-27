#!/usr/bin/env bash
# 일반 빌드(커버리지 계측 없음). 컴파일 경고는 오류로 취급한다.
set -u
. "$(dirname "$0")/common.sh"

status=0
if ! has_sources src; then
    log "제품 소스(src/)가 없어 빌드할 대상이 없습니다. (통과)"
else
    cmake -S "$PROJECT_DIR" -B "$BUILD_DIR/release" -G Ninja -DCMAKE_BUILD_TYPE=Release -DENABLE_COVERAGE=OFF || status=1
    if [ "$status" -eq 0 ]; then
        cmake --build "$BUILD_DIR/release" || status=1
    fi
fi
exit "$status"
