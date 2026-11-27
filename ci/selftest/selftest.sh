#!/usr/bin/env bash
# CI 게이트 자가 검증.
#  1) 샘플 프로젝트(fixture)가 모든 게이트를 통과하는지 확인한다.
#  2) 샘플 프로젝트를 일부러 망가뜨려, 각 게이트가 정말 실패하는지 확인한다.
# 게이트가 "항상 통과"하면 CI는 아무 의미가 없으므로, 이 검증이 필요하다.
#
# 사용법: bash scripts/dev.sh bash ci/selftest/selftest.sh
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
REPO="$(cd "$HERE/../.." && pwd)"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

pass=0; failcount=0
check() {  # check <설명> <기대: pass|fail> <명령...>
    local desc="$1" expect="$2"; shift 2
    local out rc
    out="$("$@" 2>&1)"; rc=$?
    if { [ "$expect" = pass ] && [ "$rc" -eq 0 ]; } || { [ "$expect" = fail ] && [ "$rc" -ne 0 ]; }; then
        printf '  [OK]   %s\n' "$desc"; pass=$((pass + 1))
    else
        printf '  [FAIL] %s (기대: %s, 종료 코드: %s)\n' "$desc" "$expect" "$rc"
        printf '%s\n' "$out" | tail -15 | sed 's/^/         /'
        failcount=$((failcount + 1))
    fi
}

fresh() {  # fresh <이름>: fixture 사본을 새로 만든다
    local d="$WORK/$1"
    mkdir -p "$d"
    cp -r "$HERE/fixture/." "$d/"
    cp "$REPO/CMakeLists.txt" "$REPO/Doxyfile" "$d/"
    printf '%s' "$d"
}

run_ci() { PROJECT_DIR="$1" bash "$REPO/ci/$2" "${@:3}"; }

echo "== 1. 정상 샘플: 모든 게이트 통과 =="
P="$(fresh good)"
check "빌드"                 pass run_ci "$P" build.sh
check "TDD 테스트"           pass run_ci "$P" run_tests.sh tdd
check "단위 테스트 (MC/DC)"  pass run_ci "$P" run_tests.sh unit
check "통합 테스트 (함수)"   pass run_ci "$P" run_tests.sh integration
check "Cppcheck"             pass run_ci "$P" static_analysis.sh
check "Lizard"               pass run_ci "$P" metrics.sh
check "CLOC 주석률"          pass run_ci "$P" comment_ratio.sh
check "사람 확인용 태그"     pass run_ci "$P" doc_tags.sh
check "Doxygen"              pass run_ci "$P" doxygen.sh
check "TDD/단위 결과 분리"   pass test -f "$P/build/reports/tdd/results.xml" -a -f "$P/build/reports/unit/results.xml"

echo "== 2. 망가뜨린 샘플: 각 게이트가 실패해야 한다 =="
# (a) MC/DC 미달: (T,F) 케이스를 제거 -> 조건 독립성 쌍이 깨진다
P="$(fresh mcdc)"
python3 - "$P/test/unit/limiter_unit_test.cpp" <<'PY'
import re, sys
p = sys.argv[1]
s = open(p, encoding="utf-8").read()
# AboveHighIsClampedToHigh 테스트 본문을 비운다
s = re.sub(r"(TEST\(LimiterUnit, AboveHighIsClampedToHigh\)\n\{)(.*?)(\n\})", r"\1\n    SUCCEED();\3", s, flags=re.S)
open(p, "w", encoding="utf-8").write(s)
PY
check "MC/DC 미달이면 단위 테스트 게이트 실패" fail run_ci "$P" run_tests.sh unit

# (b) 테스트 실패
P="$(fresh testfail)"
sed -i 's/EXPECT_EQ(50, out);/EXPECT_EQ(51, out);/' "$P/test/unit/limiter_unit_test.cpp"
check "테스트가 실패하면 단위 테스트 게이트 실패" fail run_ci "$P" run_tests.sh unit

# (c) 함수 커버리지 미달: 통합 테스트가 호출하지 않는 함수 추가
P="$(fresh funccov)"
cat >> "$P/src/limiter.c" <<'C'

/**
 * @brief 통합 테스트가 호출하지 않는 함수 (게이트 검증용)
 * @return STATUS_OK
 */
Status Limiter_Unused(void)
{
    Status status = STATUS_OK;
    return status;
}
C
check "호출되지 않은 함수가 있으면 통합 테스트 게이트 실패" fail run_ci "$P" run_tests.sh integration

# (d) 순환복잡도 초과 (11)
P="$(fresh ccn)"
{
    echo '#include "status.h"'
    echo '/** @brief 복잡한 함수 @param x 입력 @return STATUS_OK'
    echo ' * @generated_by x'; echo ' * @generated_at x'; echo ' * @requested_by x'
    echo ' * @human_review pending'; echo ' * @reviewed_by -'; echo ' * @reviewed_at -'
    echo ' */'
    echo 'Status Complex_Fn(int x) { Status status = STATUS_OK;'
    for i in 1 2 3 4 5 6 7 8 9 10; do echo "    if (x == $i) { status = STATUS_ERR_RANGE; }"; done
    echo '    return status; }'
} > "$P/src/complex.c"
check "순환복잡도 11이면 Lizard 게이트 실패" fail run_ci "$P" metrics.sh

# (e) 함수 길이 초과 (81줄 이상)
P="$(fresh len)"
{
    echo '#include "status.h"'
    echo 'Status Long_Fn(void) {'
    echo '    Status status = STATUS_OK;'
    for i in $(seq 1 85); do echo "    status = STATUS_OK;"; done
    echo '    return status; }'
} > "$P/src/long.c"
check "함수 길이 80라인 초과면 Lizard 게이트 실패" fail run_ci "$P" metrics.sh

# (f) Cppcheck 지적: 배열 범위 초과
P="$(fresh cppcheck)"
cat >> "$P/src/limiter.c" <<'C'

Status Limiter_Bug(void)
{
    int arr[2] = {0, 0};
    arr[5] = 1;
    return STATUS_OK;
}
C
check "Cppcheck 지적이 있으면 정적 분석 게이트 실패" fail run_ci "$P" static_analysis.sh

# (g) Cppcheck 억제 주석은 예외로 인정되지 않는다
P="$(fresh suppress)"
sed -i 's|^    Status status = STATUS_OK;$|    // cppcheck-suppress unreadVariable\n    Status status = STATUS_OK;|' "$P/src/limiter.c"
check "cppcheck-suppress 주석을 쓰면 정적 분석 게이트 실패" fail run_ci "$P" static_analysis.sh

# (h) 사람 확인용 태그 누락
P="$(fresh tags)"
printf '#include "status.h"\nStatus No_Tags(void);\n' > "$P/include/notags.h"
check "태그가 없으면 태그 게이트 실패" fail run_ci "$P" doc_tags.sh

# (i) 근거 없는 human_review done
P="$(fresh done)"
sed -i 's/@human_review   pending/@human_review   done/' "$P/include/status.h"
check "reviewed_by 없이 done이면 태그 게이트 실패" fail run_ci "$P" doc_tags.sh

# (j) 자리표시자가 남은 태그
P="$(fresh placeholder)"
sed -i 's/@generated_by   Claude Sonnet 5.5 (claude-sonnet-5-5)/@generated_by   <작성한 AI 모델>/' "$P/include/status.h"
check "자리표시자가 남으면 태그 게이트 실패" fail run_ci "$P" doc_tags.sh

# (k) Doxygen 경고: 문서화되지 않은 매개변수 (@brief 만 빠진 경우는 Doxygen이 경고하지 않는다)
P="$(fresh doxy)"
sed -i '/@param out /d' "$P/include/limiter.h"
check "Doxygen 경고가 있으면 Doxygen 게이트 실패" fail run_ci "$P" doxygen.sh

# (l) 컴파일 경고는 오류
P="$(fresh warn)"
printf '\nstatic int unused_helper(void) { return 0; }\n' >> "$P/src/limiter.c"
check "컴파일 경고가 있으면 빌드 실패" fail run_ci "$P" build.sh

echo
echo "== 결과: 성공 ${pass}, 실패 ${failcount} =="
[ "$failcount" -eq 0 ]
