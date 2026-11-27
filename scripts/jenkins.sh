#!/usr/bin/env bash
# Jenkins 컨테이너 관리.
# 사용법: bash scripts/jenkins.sh <up|down|logs|status>
set -eu
cd "$(dirname "$0")/../docker"

ENV_FILE=".env"
ensure_env() {
    if [ ! -f "$ENV_FILE" ]; then
        # 관리자 비밀번호를 한 번 무작위로 만든다. 이 파일은 git에 올라가지 않는다.
        pw="$(head -c 32 /dev/urandom | base64 | tr -dc 'A-Za-z0-9' | head -c 24)"
        printf 'JENKINS_ADMIN_PASSWORD=%s\n' "$pw" > "$ENV_FILE"
        echo "docker/.env 를 만들었습니다 (관리자 ID: admin)."
    fi
}

case "${1:-}" in
    up)
        ensure_env
        docker compose up -d --build
        echo "Jenkins: http://localhost:8080  (ID: admin, 비밀번호: docker/.env)"
        ;;
    down)   docker compose down ;;
    logs)   docker compose logs -f --tail=100 jenkins ;;
    status) docker compose ps ;;
    *) echo "사용법: bash scripts/jenkins.sh <up|down|logs|status>" >&2; exit 2 ;;
esac
