#!/usr/bin/env bash
# CI 스크립트 공통 설정. 직접 실행하지 않고 source 한다.
#
# PROJECT_DIR : 검사할 프로젝트 루트 (기본: 저장소 루트). selftest가 샘플 프로젝트로 바꿔 쓴다.
# BUILD_DIR   : 빌드 산출물 위치
# REPORT_DIR  : 결과 리포트 위치 (Jenkins 플러그인이 여기서 읽는다)

CI_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="${PROJECT_DIR:-$(cd "$CI_DIR/.." && pwd)}"
BUILD_DIR="${BUILD_DIR:-$PROJECT_DIR/build}"
REPORT_DIR="${REPORT_DIR:-$BUILD_DIR/reports}"

# 제품/시험 소스 위치
PRODUCT_DIRS=(include src)
ALL_SOURCE_DIRS=(include src test)

log()  { printf '[ci] %s\n' "$*"; }
fail() { printf '[ci][실패] %s\n' "$*" >&2; }

# 지정한 디렉터리 안에 C/C++ 소스·헤더가 있는지 확인한다.
has_sources() {
    local d count=0
    for d in "$@"; do
        [ -d "$PROJECT_DIR/$d" ] || continue
        count=$((count + $(find "$PROJECT_DIR/$d" -type f \( -name '*.c' -o -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) | wc -l)))
    done
    [ "$count" -gt 0 ]
}

# 존재하는 디렉터리만 골라 stdout에 한 줄씩 낸다.
existing_dirs() {
    local d
    for d in "$@"; do
        [ -d "$PROJECT_DIR/$d" ] && printf '%s\n' "$PROJECT_DIR/$d"
    done
}
