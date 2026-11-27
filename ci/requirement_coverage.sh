#!/usr/bin/env bash
# 시스템 테스트 요구사항 커버리지 게이트. (CLAUDE.md 2.1: 요구사항 커버리지 100%)
#
# 입력
#   docs/requirements/oem-requirements.csv : id,class,asil,iteration,title
#   docs/traceability/trace.csv            : oem_id,swr_id,design,code,unit_test,integration_test,system_test
#   docs/traceability/iteration.txt        : completed_iteration=N (이터레이션을 마칠 때 올린다)
#
# 판정
#   1) iteration <= N 인 요구사항은 모두 system_test 가 연결되어야 한다 (커버리지 100%).
#   2) trace.csv 의 oem_id 는 요구사항 목록에 있어야 한다 (고아 추적 금지).
#   3) 완료 이터레이션의 요구사항은 swr_id, design, code, unit_test 도 비어 있으면 안 된다 (양방향 추적).
# 한계: system_test 값이 실제 시험 문서에 존재하는지는 검사하지 않는다 (리뷰어가 확인).
set -u
. "$(dirname "$0")/common.sh"

REQ="$PROJECT_DIR/docs/requirements/oem-requirements.csv"
TRACE="$PROJECT_DIR/docs/traceability/trace.csv"
ITER="$PROJECT_DIR/docs/traceability/iteration.txt"

if [ ! -f "$REQ" ]; then
    log "요구사항 목록이 없습니다. (통과)"
    exit 0
fi
[ -f "$TRACE" ] || { fail "추적 매트릭스(docs/traceability/trace.csv)가 없습니다."; exit 1; }
done_iter=$(sed -n 's/^completed_iteration=\([0-9]*\).*/\1/p' "$ITER" 2>/dev/null | head -1)
done_iter="${done_iter:-0}"

mkdir -p "$REPORT_DIR"
awk -F, -v done="$done_iter" -v req="$REQ" -v out="$REPORT_DIR/requirement_coverage.json" '
    function trim(s) { gsub(/\r|^ +| +$/, "", s); return s }
    BEGIN {
        while ((getline line < req) > 0) {
            if (++n == 1) continue
            split(line, f, ","); id = trim(f[1]); it[id] = trim(f[4]); known[id] = 1
            if (it[id] + 0 <= done + 0) { target[id] = 1; total++ }
        }
    }
    FNR == 1 { next }
    {
        id = trim($1)
        if (!(id in known)) { printf "[고아 추적] %s 는 요구사항 목록에 없습니다.\n", id; bad++; next }
        if (trim($7) != "") sys[id] = 1
        if (trim($2) == "" || trim($3) == "" || trim($4) == "" || trim($5) == "") incomplete[id] = 1
    }
    END {
        for (id in target) {
            if (!(id in sys)) { printf "[미커버] %s: system_test 가 연결되지 않았습니다.\n", id; bad++ }
            else { covered++; if (id in incomplete) { printf "[추적 누락] %s: swr/design/code/unit_test 중 빈 칸이 있습니다.\n", id; bad++ } }
        }
        pct = (total > 0) ? 100 * covered / total : 100
        printf "요구사항 커버리지: %d/%d (%.1f%%), 완료 이터레이션 %d\n", covered, total, pct, done
        printf "{\"completed_iteration\":%d,\"total\":%d,\"covered\":%d,\"percent\":%.1f}\n", done, total, covered, pct > out
        exit (bad > 0)
    }
' "$TRACE"
