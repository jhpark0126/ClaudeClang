#!/usr/bin/env bash
# Claude Code Stop 훅: 응답(구현)이 끝날 때 Lizard 기준을 검사한다. (CLAUDE.md 2.3)
#   순환복잡도 10 이하, 함수 길이 80라인 이하. 기준값은 ci/metrics.sh 에서만 관리한다.
#
# 종료 코드
#   0 : 통과, 또는 검사할 소스가 없음, 또는 검사 수단이 없음(경고만 출력)
#   2 : 기준 위반. Claude Code가 응답 종료를 막고 stderr 내용을 Claude에게 돌려준다.
#
# 훅이 막은 뒤 다시 종료하려는 시점(stop_hook_active=true)에는 무한 반복을 막기 위해
# 위반을 출력만 하고 통과시킨다. 그 경우에도 CI(Jenkins)가 최종 게이트다.
set -u
cd "$(dirname "$0")/../.." || exit 0

input="$(cat 2>/dev/null || true)"
active=0
printf '%s' "$input" | grep -qE '"stop_hook_active"[[:space:]]*:[[:space:]]*true' && active=1

# 검사할 C/C++ 소스가 없으면 통과
if ! find src include test -type f \( -name '*.c' -o -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) 2>/dev/null | grep -q .; then
    exit 0
fi

# Lizard 실행: 로컬에 있으면 로컬, 없으면 빌드 이미지(claudeclangbuild) 사용
if command -v lizard >/dev/null 2>&1; then
    out="$(bash ci/metrics.sh 2>&1)"; rc=$?
elif command -v docker >/dev/null 2>&1 && docker image inspect claudeclangbuild:latest >/dev/null 2>&1; then
    out="$(bash scripts/dev.sh bash ci/metrics.sh 2>&1)"; rc=$?
else
    echo "[경고] Lizard 검사를 실행할 수 없습니다 (lizard 미설치, claudeclangbuild 이미지 없음). 'bash scripts/build-image.sh' 로 이미지를 만드세요." >&2
    exit 0
fi

if [ "$rc" -ne 0 ]; then
    {
        echo "[Lizard 기준 위반] 순환복잡도 10 이하, 함수 길이 80라인 이하를 만족하도록 함수를 분리하세요."
        echo "$out" | tail -30
    } >&2
    if [ "$active" -eq 1 ]; then
        exit 0
    fi
    exit 2
fi
exit 0
