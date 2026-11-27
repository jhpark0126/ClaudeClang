#!/usr/bin/env bash
# 빌드 이미지(claudeclangbuild)를 만든다. 개발 환경과 CI가 같은 이미지를 쓴다.
# 사용법: bash scripts/build-image.sh [LLVM_VERSION]
set -eu
cd "$(dirname "$0")/.."
LLVM_VERSION="${1:-22}"
docker build -f docker/Dockerfile --build-arg LLVM_VERSION="$LLVM_VERSION" -t claudeclangbuild:latest -t "claudeclangbuild:llvm${LLVM_VERSION}" .
