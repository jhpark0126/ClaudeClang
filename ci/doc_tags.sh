#!/usr/bin/env bash
# 사람 확인용 Doxygen 태그 검사. (CLAUDE.md 2.4)
#  1) 모든 소스·헤더 파일에 6개 태그가 있어야 한다.
#  2) 값이 채워지지 않은 자리표시자('<...>')가 남아 있으면 안 된다.
#  3) @human_review 가 done 이면 @reviewed_by, @reviewed_at 이 '-' 가 아니어야 한다.
# 함수 단위가 아니라 파일 단위로 검사한다. (함수별 태그는 리뷰어가 코드 리뷰에서 확인)
set -u
. "$(dirname "$0")/common.sh"

TAGS=(generated_by generated_at requested_by human_review reviewed_by reviewed_at)
status=0

if ! has_sources "${ALL_SOURCE_DIRS[@]}"; then
    log "검사할 소스가 없습니다. (통과)"
    exit 0
fi

files=$(find $(existing_dirs "${ALL_SOURCE_DIRS[@]}") -type f \( -name '*.c' -o -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) | sort)
checked=0
for f in $files; do
    checked=$((checked + 1))
    for t in "${TAGS[@]}"; do
        if ! grep -qE "@${t}[[:space:]]" "$f"; then
            fail "$f: @${t} 태그가 없습니다."
            status=1
        fi
    done
    if grep -nE '@(generated_by|generated_at|requested_by)[[:space:]]+<' "$f" >/dev/null; then
        fail "$f: 태그 값이 자리표시자(<...>)로 남아 있습니다."
        status=1
    fi
    # done 인데 리뷰 정보가 비어 있는 경우 (파일 안의 각 태그 줄을 순서대로 비교)
    awk -v file="$f" '
        /@human_review[[:space:]]+done/ { done = 1 }
        /@reviewed_by[[:space:]]+-[[:space:]]*$/ { by_empty = 1 }
        /@reviewed_at[[:space:]]+-[[:space:]]*$/ { at_empty = 1 }
        END {
            if (done && (by_empty || at_empty)) {
                printf "[ci][실패] %s: @human_review done 인데 @reviewed_by/@reviewed_at 이 비어 있습니다.\n", file > "/dev/stderr"
                exit 1
            }
        }' "$f" || status=1
done

if [ "$status" -eq 0 ]; then
    log "사람 확인용 태그: ${checked}개 파일 모두 충족"
fi
exit "$status"
