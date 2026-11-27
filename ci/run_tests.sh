#!/usr/bin/env bash
# 시험 묶음 하나를 빌드·실행하고 결과와 커버리지를 따로 낸다.
#
# 사용법: bash ci/run_tests.sh <tdd|unit|integration>
#
# 묶음마다 별도 실행 파일·별도 결과·별도 커버리지다. (CLAUDE.md 1.4)
#   tdd         : 판정 없음 (결과와 커버리지만 기록)
#   unit        : 테스트 성공률 100% + MC/DC 커버리지 100%
#   integration : 테스트 성공률 100% + 함수 커버리지 100%
# 결과는 $REPORT_DIR/<묶음>/ 에 남는다 (results.xml, cobertura.xml, html/, summary.json).
set -u
. "$(dirname "$0")/common.sh"

kind="${1:-}"
case "$kind" in
    tdd|unit|integration) ;;
    *) fail "사용법: bash ci/run_tests.sh <tdd|unit|integration>"; exit 2 ;;
esac

target="${kind}_tests"
out="$REPORT_DIR/$kind"
cov_build="$BUILD_DIR/cov"
status=0

# 이 묶음의 시험 소스가 있는지
suite_has_tests() {
    [ -d "$PROJECT_DIR/test/$kind" ] && \
        [ "$(find "$PROJECT_DIR/test/$kind" -type f \( -name '*.c' -o -name '*.cpp' \) | wc -l)" -gt 0 ]
}

if ! has_sources src; then
    log "[$kind] 제품 소스(src/)가 없어 시험할 대상이 없습니다. (통과)"
    exit 0
fi

if ! suite_has_tests; then
    if [ "$kind" = "tdd" ]; then
        log "[tdd] test/tdd 에 시험이 없습니다. (판정 대상 아님, 통과)"
        exit 0
    fi
    fail "[$kind] 제품 소스는 있는데 test/$kind 에 시험이 없습니다. 커버리지 기준을 충족할 수 없습니다."
    exit 1
fi

rm -rf "$out"
mkdir -p "$out"

# --- 빌드 (커버리지 계측) ---
cmake -S "$PROJECT_DIR" -B "$cov_build" -G Ninja -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON >"$out/configure.log" 2>&1 \
    || { fail "[$kind] CMake 구성 실패 (상세: $out/configure.log)"; tail -20 "$out/configure.log" >&2; exit 1; }
cmake --build "$cov_build" --target "$target" >"$out/build.log" 2>&1 \
    || { fail "[$kind] 빌드 실패 (상세: $out/build.log)"; tail -30 "$out/build.log" >&2; exit 1; }

# --- 실행 ---
export LLVM_PROFILE_FILE="$out/$kind-%p.profraw"
"$cov_build/$target" --gtest_output="xml:$out/results.xml" >"$out/run.log" 2>&1
run_rc=$?
tail -5 "$out/run.log"

# 테스트 성공률 (gtest XML 루트 속성)
tests=$(grep -m1 -oE '<testsuites [^>]*' "$out/results.xml" | grep -oE ' tests="[0-9]+"' | grep -oE '[0-9]+' || echo 0)
failures=$(grep -m1 -oE '<testsuites [^>]*' "$out/results.xml" | grep -oE ' failures="[0-9]+"' | grep -oE '[0-9]+' || echo 0)
errors=$(grep -m1 -oE '<testsuites [^>]*' "$out/results.xml" | grep -oE ' errors="[0-9]+"' | grep -oE '[0-9]+' || echo 0)
disabled=$(grep -m1 -oE '<testsuites [^>]*' "$out/results.xml" | grep -oE ' disabled="[0-9]+"' | grep -oE '[0-9]+' || echo 0)
passed=$((tests - failures - errors - disabled))
if [ "$tests" -gt 0 ]; then
    rate=$(awk -v p="$passed" -v t="$tests" 'BEGIN { printf "%.2f", 100 * p / t }')
else
    rate="0.00"
fi
log "[$kind] 테스트 ${tests}개 중 성공 ${passed}개 (성공률 ${rate}%, 실패 ${failures}, 오류 ${errors}, 비활성 ${disabled})"

# --- 커버리지 리포트 (제품 소스 src/ 만 대상) ---
ls "$out"/*.profraw >/dev/null 2>&1 || { fail "[$kind] 커버리지 데이터(.profraw)가 생성되지 않았습니다."; exit 1; }
llvm-profdata merge -sparse "$out"/*.profraw -o "$out/coverage.profdata" || { fail "[$kind] llvm-profdata 병합 실패"; exit 1; }
exe="$cov_build/$target"
src_filter="$PROJECT_DIR/src"

llvm-cov report "$exe" -instr-profile="$out/coverage.profdata" -show-mcdc-summary "$src_filter" > "$out/coverage.txt" 2>&1
llvm-cov export "$exe" -instr-profile="$out/coverage.profdata" -summary-only "$src_filter" > "$out/summary.json" 2>/dev/null
# 검증 결과 대시보드(VERReport)용: 함수 단위(라인·분기·MC/DC) 상세가 들어 있는 전체 export
llvm-cov export "$exe" -instr-profile="$out/coverage.profdata" "$src_filter" > "$out/coverage.json" 2>/dev/null
llvm-cov export "$exe" -instr-profile="$out/coverage.profdata" -format=lcov "$src_filter" > "$out/coverage.lcov" 2>/dev/null
llvm-cov show "$exe" -instr-profile="$out/coverage.profdata" -format=html -output-dir="$out/html" \
    -show-mcdc -show-branches=count "$src_filter" >/dev/null 2>&1
# Jenkins Coverage 플러그인이 읽는 Cobertura 형식 (MC/DC는 표현할 수 없어 HTML 리포트로 확인)
lcov_cobertura "$out/coverage.lcov" --base-dir "$PROJECT_DIR" --output "$out/cobertura.xml" >/dev/null 2>&1 || true

metric() { jq -r ".data[0].totals.$1.$2" "$out/summary.json"; }
f_cnt=$(metric functions count); f_cov=$(metric functions covered)
m_cnt=$(metric mcdc count);      m_cov=$(metric mcdc covered)
l_cnt=$(metric lines count);     l_cov=$(metric lines covered)
b_cnt=$(metric branches count);  b_cov=$(metric branches covered)
log "[$kind] 함수 ${f_cov}/${f_cnt}, 라인 ${l_cov}/${l_cnt}, 분기 ${b_cov}/${b_cnt}, MC/DC 조건 ${m_cov}/${m_cnt}"

# --- 판정 ---
if [ "$run_rc" -ne 0 ] || [ "$tests" -eq 0 ] || [ "$passed" -ne "$tests" ]; then
    fail "[$kind] 테스트 성공률이 100%가 아닙니다. (종료 코드 $run_rc)"
    status=1
fi

case "$kind" in
    unit)
        # 판단 대상 조건이 하나도 없으면(count=0) MC/DC는 자명하게 충족이다.
        if [ "$m_cov" -ne "$m_cnt" ]; then
            fail "[unit] MC/DC 커버리지 100% 미달: ${m_cov}/${m_cnt}. 상세: $out/coverage.txt, $out/html/index.html"
            status=1
        fi
        ;;
    integration)
        if [ "$f_cov" -ne "$f_cnt" ]; then
            fail "[integration] 함수 커버리지 100% 미달: ${f_cov}/${f_cnt}. 상세: $out/coverage.txt"
            status=1
        fi
        ;;
    tdd)
        log "[tdd] 커버리지는 기록만 하고 판정하지 않습니다."
        ;;
esac

[ "$status" -eq 0 ] && log "[$kind] 통과"
exit "$status"
