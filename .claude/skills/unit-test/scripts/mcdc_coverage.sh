#!/usr/bin/env bash
# MC/DC 커버리지 측정 스크립트 (LLVM)
#
# 사용법: bash mcdc_coverage.sh <소스.c> <시험.c> [출력디렉터리]
#   소스.c : 측정 대상 소스 (시험 코드에서 #include 하거나 함께 링크)
#   시험.c : main()을 가진 시험 드라이버
#
# 환경변수: CLANG, LLVM_PROFDATA, LLVM_COV 로 도구 경로를 바꿀 수 있다.
#           EXTRA_FLAGS 로 추가 컴파일/링크 옵션을 넘긴다 (예: Google Test).
#           예) CLANG=clang++ EXTRA_FLAGS="-Iinclude -lgtest -lgtest_main -pthread" bash mcdc_coverage.sh ...
# 요구 사항: clang 18 이상 (-fcoverage-mcdc)

set -u

SRC="${1:-}"
TEST="${2:-}"
OUT="${3:-build/mcdc}"
CLANG="${CLANG:-clang}"
LLVM_PROFDATA="${LLVM_PROFDATA:-llvm-profdata}"
LLVM_COV="${LLVM_COV:-llvm-cov}"
EXTRA_FLAGS="${EXTRA_FLAGS:-}"

status=0

if [ -z "$SRC" ] || [ -z "$TEST" ]; then
    echo "사용법: bash mcdc_coverage.sh <소스.c> <시험.c> [출력디렉터리]" >&2
    status=2
fi

if [ "$status" -eq 0 ]; then
    for tool in "$CLANG" "$LLVM_PROFDATA" "$LLVM_COV"; do
        if ! command -v "$tool" >/dev/null 2>&1; then
            echo "오류: '$tool' 을(를) 찾을 수 없습니다. LLVM 18 이상을 설치하거나 환경변수로 경로를 지정하세요." >&2
            status=3
        fi
    done
fi

if [ "$status" -eq 0 ]; then
    mkdir -p "$OUT"
    EXE="$OUT/test_runner"
    case "$(uname -s 2>/dev/null)" in
        MINGW*|MSYS*|CYGWIN*) EXE="$EXE.exe" ;;
    esac

    # 계측 빌드: 라인/분기 커버리지 + MC/DC
    # EXTRA_FLAGS는 의도적으로 따옴표 없이 전개한다 (여러 옵션으로 분리되어야 함).
    # shellcheck disable=SC2086
    "$CLANG" -O0 -g -fprofile-instr-generate -fcoverage-mapping -fcoverage-mcdc \
        "$SRC" "$TEST" $EXTRA_FLAGS -o "$EXE" || status=4
fi

if [ "$status" -eq 0 ]; then
    LLVM_PROFILE_FILE="$OUT/default.profraw" "$EXE" || echo "경고: 시험 실행이 실패 상태로 끝났습니다 (종료 코드 $?)." >&2
    "$LLVM_PROFDATA" merge -sparse "$OUT/default.profraw" -o "$OUT/default.profdata" || status=5
fi

if [ "$status" -eq 0 ]; then
    echo "=== 커버리지 요약 (라인/분기/MC-DC) ==="
    "$LLVM_COV" report "$EXE" -instr-profile="$OUT/default.profdata" -show-mcdc-summary || status=6
    echo
    echo "=== 소스별 MC/DC 상세 (미충족 조건 확인용) ==="
    "$LLVM_COV" show "$EXE" -instr-profile="$OUT/default.profdata" \
        -show-mcdc -show-branches=count "$SRC" || status=7
    "$LLVM_COV" show "$EXE" -instr-profile="$OUT/default.profdata" \
        -format=html -output-dir="$OUT/html" -show-mcdc "$SRC" >/dev/null 2>&1 \
        && echo "HTML 보고서: $OUT/html/index.html"
fi

exit "$status"
