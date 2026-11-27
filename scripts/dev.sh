#!/usr/bin/env bash
# 개발용 빌드 컨테이너에서 명령을 실행한다. CI와 같은 이미지(claudeclangbuild)를 쓴다.
#
# 사용법:
#   bash scripts/dev.sh                      # 컨테이너 셸
#   bash scripts/dev.sh bash ci/all.sh       # 모든 CI 게이트를 로컬에서 실행
#   bash scripts/dev.sh cmake --version
set -eu
cd "$(dirname "$0")/.."

IMAGE="${IMAGE:-claudeclangbuild:latest}"
if ! docker image inspect "$IMAGE" >/dev/null 2>&1; then
    echo "이미지 $IMAGE 가 없습니다. 먼저 실행하세요: bash scripts/build-image.sh" >&2
    exit 1
fi

# Git Bash(Windows)의 경로 변환을 끄고, 현재 폴더를 /work 로 마운트한다.
if command -v pwd >/dev/null && pwd -W >/dev/null 2>&1; then
    HOST_DIR="$(pwd -W)"
else
    HOST_DIR="$(pwd)"
fi

if [ "$#" -eq 0 ]; then set -- bash; fi
tty_flag=""
[ -t 0 ] && tty_flag="-it"

MSYS_NO_PATHCONV=1 docker run --rm $tty_flag -v "$HOST_DIR:/work" -w /work "$IMAGE" "$@"
